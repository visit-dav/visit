// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCGNSFileReader.C                            //
// ************************************************************************* //

#include <avtCGNSFileReader.h>
#include <cgnslib.h>

#include <algorithm>
#include <string>

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <Expression.h>
#include <DebugStream.h>

#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#define INVALID_FILE_HANDLE -1

// Include more source code.
#include <CGNSHelpers.C>
#include <CGNSUnitsStack.C>

// ****************************************************************************
// Method: MakeSafeVariableName
//
// Purpose:
//   Replace characters that might confuse VisIt's expression parser.
//
// Arguments:
//   var : The variable name to fix.
//
// Returns:    A safe variable name.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:16:39 PDT 2008
//
// Modifications:
//
// ****************************************************************************

std::string
MakeSafeVariableName(const std::string &var)
{
    char *tmp = new char[var.size() + 1];
    strcpy(tmp, var.c_str());

    // Replace characters that could confuse VisIt.
    char invalids[][2] = {
        {' ', '_'},
        {'{', '_'},
        {'}', '_'},
        {'[', '_'},
        {']', '_'},

        {'!', '_'},
        {'@', '_'},
        {'#', '_'},
        {'$', '_'},
        {'%', '_'},
        {'^', '_'},
        {'&', '_'},
        {'*', '_'},
        {'(', '_'},
        {')', '_'},

        {'+', '_'},
        {'-', '_'},
        {'/', '_'},

        {';', '_'},
        {':', '_'},
        {',', '_'},
        {'.', '_'},
        {'<', '_'},
        {'>', '_'},
    };
    int nInvalids = sizeof(invalids) / (sizeof(char)*2);
    for(int i = 0; i < nInvalids; ++i)
    {
        for(size_t ci = 0; ci < var.size(); ++ci)
        {
            if(tmp[ci] == invalids[i][0])
                tmp[ci] = invalids[i][1];
        }
    }

    std::string retval(tmp);
    delete [] tmp;
    return retval;
}

// ****************************************************************************
//  Method: avtCGNSFileReader constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Brad Whitlock, Tue Apr 15 16:00:52 PDT 2008
//    Added cgnsFileName, BaseNameToIndices, VisItNameToCGNSName.
//
//    Eric Brugger, Thu Jul  2 10:56:36 PDT 2020
//    Corrected a bug that caused a crash when doing a Subset plot of "zones"
//    when reading data decomposed across multiple CGNS files.
//
// ****************************************************************************

avtCGNSFileReader::avtCGNSFileReader(const char *filename, bool isMTMD)
{
    cgnsFileName = new char[strlen(filename) + 1];
    strcpy(cgnsFileName, filename);
    cgnsIsMTMD = isMTMD;

    debug1 << "avtCGNSFileReader::avtCGNSFileReader: filename=" << cgnsFileName << endl;
    fn = INVALID_FILE_HANDLE;
    timesRead = false;
    cgnsCyclesAccurate = false;
    cgnsTimesAccurate = false;
    initializedMaps = false;
}

// ****************************************************************************
// Method: avtCGNSFileReader::~avtCGNSFileReader
//
// Purpose:
//   Destructor for the avtCGNSFileReader class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 30 16:12:20 PST 2005
//
// Modifications:
//
// ****************************************************************************

avtCGNSFileReader::~avtCGNSFileReader()
{
    delete [] cgnsFileName;
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
// ****************************************************************************

int
avtCGNSFileReader::GetNTimesteps(void)
{
    ReadTimes();

    return (int)times.size();
}

// ****************************************************************************
//  Method: avtCGNSFileReader::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
// ****************************************************************************

void
avtCGNSFileReader::FreeUpResources(void)
{
    if(fn != INVALID_FILE_HANDLE)
    {
        cg_close(fn);
        fn = INVALID_FILE_HANDLE;
    }
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetFileHandle
//
// Purpose:
//   This method opens the CGNS file and returns the file handle.
//
// Returns:    The file handle.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:35:12 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 16 10:15:07 PDT 2008
//   Made it use cgnsFileName.
//
// ****************************************************************************

int
avtCGNSFileReader::GetFileHandle()
{
    if(fn == INVALID_FILE_HANDLE)
    {
#ifdef CG_MODE_READ
        if(cg_open(cgnsFileName, CG_MODE_READ, &fn) != CG_OK)
#else
        // Still support pre 2.5 CGNS
        if(cg_open(cgnsFileName, MODE_READ, &fn) != CG_OK)
#endif
        {
            debug4 << cg_get_error() << endl;
            EXCEPTION1(InvalidFilesException, cgnsFileName);
        }
    }

    return fn;
}

// ****************************************************************************
// Method: avtCGNSFileReader::ReadTimes
//
// Purpose:
//   This method reads the times from the file and stores them in the
//   local times vector.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:34:43 PDT 2005
//
// Modifications:
//   Maxim Loginov, Tue Mar  4 12:28:12 NOVT 2008
//   Bugfixes and improve reading times from BaseIterativeData_t node
//
//   Brad Whitlock, Wed Apr 16 10:15:21 PDT 2008
//   Made it use cgnsFileName.
//
//   Kathleen Biagas, Tue Apr 24 12:23:03 PDT 2012
//   Added call to FreeUpResources to prevent crash when opening many files
//   in a virtual database.
//
// ****************************************************************************

void
avtCGNSFileReader::ReadTimes()
{
    const char *mName = "avtCGNSFileReader::ReadTimes: ";

    if(!timesRead)
    {
        debug4 << mName << "Start" << endl;

        // Read the number of bases and use that for the time step.
        int nbases = 0;
        if(cg_nbases(GetFileHandle(), &nbases) != CG_OK)
        {
            debug4 << cg_get_error() << endl;
            EXCEPTION1(InvalidFilesException, cgnsFileName);
        }

        bool createTimeStates = true;
        bool createCycleStates = true;

        // TODO only one (first) base is considered currently
        int base = 1;
        int nstates = 1;
        char namenode[33];
        if(cg_biter_read(GetFileHandle(), base, namenode, &nstates) == CG_OK)
        {
            debug4 << mName << "nstates determined to be: " << nstates << endl;
            debug4 << mName << "node name: " << namenode << endl;
            if(cg_goto(GetFileHandle(), base, "BaseIterativeData_t", 1, "end") == CG_OK)
            {
                // check all arrays under the BaseIterativeData_t node
                int narrays = 0;
                if(cg_narrays(&narrays) == CG_OK)
                {
                    debug4 << mName << narrays
                           << " array(s) under BaseIterative" << endl;
                    for(int i = 0; i < narrays; ++i)
                    {
                        int ndims = 1;
                        cgsize_t dims[10];
                        DataType_t dt;
                        if(cg_array_info(i+1, namenode, &dt, &ndims, dims) == CG_OK)
                        {
                            debug4 << mName << "array name: " << namenode << endl;
                            if(strcmp(namenode,"TimeValues") == 0 ||
                               strcmp(namenode,"TimeIterValues") == 0 ||
                               strcmp(namenode,"Times") == 0
                               )
                            {
                                double *dvals = new double[nstates+1];
                                if(cg_array_read_as(i+1, RealDouble, dvals) == CG_OK)
                                {
                                    createTimeStates = false;
                                    times.clear();
                                    debug4 << mName << "Times = {";
                                    for(int j = 0; j < nstates; ++j)
                                    {
                                        debug4 << dvals[j] << ", ";
                                        times.push_back(dvals[j]);
                                    }
                                    debug4 << "}" << endl;
                                }
                                else
                                {
                                    debug4 << mName << "Could not read the array: "
                                           << cg_get_error() << endl;
                                }
                                delete [] dvals;
                            }
                            else if(strcmp(namenode,"IterationValues") == 0 ||
                                    strcmp(namenode,"Cycles") == 0
                                    )
                            {
                                int *ivals = new int[nstates+1];
                                if(cg_array_read_as(i+1, Integer, ivals) == CG_OK)
                                {
                                    createCycleStates = false;
                                    cycles.clear();
                                    debug4 << mName << "Cycles = {";
                                    for(int j = 0; j < nstates; ++j)
                                    {
                                        debug4 << ivals[j] << ", ";
                                        cycles.push_back(ivals[j]);
                                    }
                                    debug4 << "}" << endl;
                                }
                                else
                                {
                                    debug4 << mName << "Could not read the array: "
                                           << cg_get_error() << endl;
                                }
                                delete [] ivals;
                            }
                            else
                            {
                                debug5 << mName << namenode << " is not known" << endl;
                            }
                        }
                        else
                        {
                            debug4 << mName << "Could not read " << i+1 << " array info: "
                                   << cg_get_error() << endl;
                        }
                    }
                }
                else
                {
                    debug4 << mName << "Could not read narrays "
                           << "under BaseIterative node: "
                           << cg_get_error() << endl;
                }
            }
            else
            {
                debug4 << mName << "Could not go to BaseIterative node: "
                       << cg_get_error() << endl;
            }
        }
        else
        {
            debug4 << mName << "We can't determine the number of states. Assume 1"
                   << endl;
        }

        if(createTimeStates)
        {
            debug4 << mName << "Creating fake times." << endl;
            // Fake the times for now.
            for(int i = 0; i < nstates; ++i)
                times.push_back(double(i));
        }
        else
            cgnsTimesAccurate = true;

        if(createCycleStates)
        {
            debug4 << mName << "Creating fake cycles." << endl;
            // Fake the cycles for now.
            for(int i = 0; i < nstates; ++i)
                cycles.push_back(i);
        }
        else
           cgnsCyclesAccurate = true;

        timesRead = true;
        debug4 << mName << "End" << endl;
    }
    // make sure file handles are closed
    FreeUpResources();
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetTimes
//
// Purpose:
//   Returns the times for the database.
//
// Arguments:
//   t : The return vector for the times.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:34:21 PDT 2005
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::GetTimes(std::vector<double> &t)
{
    ReadTimes();
    t = times;
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetCycles
//
// Purpose:
//   Returns the cycles for the database.
//
// Arguments:
//   c : The return vector for the cycles.
//
// Programmer: Maxim Loginov
// Creation:   Mon Mar  3 18:18:32 NOVT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::GetCycles(std::vector<int> &c)
{
    // ncycles is equal to ntimes (CGNS MLL check this)
    ReadTimes();
    c = cycles;
}

// ****************************************************************************
// Method: avtCGNSFileReader::BaseContainsUnits
//
// Purpose:
//   Returns whether the base contains units.
//
// Arguments:
//   baes : The base to check for units.
//
// Returns:    True if the base contains units; false otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:18:33 PDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
avtCGNSFileReader::BaseContainsUnits(int base)
{
    const char *mName = "avtCGNSFileReader::BaseContainsUnits: ";
    bool baseContainsUnits = false;
    MassUnits_t        massU;
    LengthUnits_t      lengthU;
    TimeUnits_t        timeU;
    TemperatureUnits_t tempU;
    AngleUnits_t       angleU;
    bool unitError = false;
    if(cg_goto(GetFileHandle(), base, "end") == CG_OK)
    {
        DataClass_t dc = DataClassNull;
        if(cg_dataclass_read(&dc) == CG_OK)
        {
            baseContainsUnits = (dc == Dimensional);
            int nunits = 0;
            if(baseContainsUnits &&
               cg_nunits(&nunits) == CG_OK &&
               nunits > 0)
            {
                if(cg_units_read(&massU, &lengthU, &timeU,
                                 &tempU, &angleU) == CG_OK)
                {
                    debug4 << mName << "We read the units!" << endl;
                    baseContainsUnits = true;
                }
            }
            else
            {
                baseContainsUnits = false;
                unitError = true;
            }
        }
        else
            unitError = true;
    }
    else
        unitError = true;
    if(unitError)
        debug4 << mName << "No units: " << cg_get_error() << endl;
    else
    {
        debug4 << mName << "The base " << base << " has" << (baseContainsUnits?"":" no")
               << " units." << endl;
    }

    return baseContainsUnits;
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetVariablesForBase
//
// Purpose:
//   Iterates over a base and populates BaseInformation, which contains the
//   zone names, list of variables, and how each variable is mapped onto zones.
//
// Arguments:
//   base     : The base we're checking.
//   baseInfo : The base information that we're going to populate.
//
// Returns:    True on success; false on failure.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:19:18 PDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
avtCGNSFileReader::GetVariablesForBase(int base, avtCGNSFileReader::BaseInformation &baseInfo)
{
    const char *mName = "avtCGNSFileReader::GetVariablesForBase: ";

    bool retval = true;
    char namebase[33];
    if(cg_base_read(GetFileHandle(), base, namebase, &baseInfo.cellDim, &baseInfo.physicalDim) != CG_OK)
    {
        debug1 << "Could not read base " << base << endl;
        retval = false;
    }
    else
    {
        // Save the base name.
        baseInfo.name = namebase;
        debug4 << mName << "base " << namebase << ": " << endl;

        //
        // Determine the number of domains.
        //
        int nZones = 0;
        if(cg_nzones(GetFileHandle(), base, &nZones) != CG_OK)
        {
            debug4 << mName << "Could not get number of domains in base "
                   << base << ": " << cg_get_error() << endl;
            return false;
        }

        CGNSUnitsStack unitStack;
        bool baseUnits = BaseContainsUnits(base) &&
                         unitStack.PushUnits(GetFileHandle(), base);
        //
        // Iterate over the domains.
        //
        baseInfo.meshType = 0;
        for(int zone = 1; zone <= nZones; ++zone)
        {
            // Get information about the zone.
            char zonename[33];
            cgsize_t zsize[9];
            memset(zonename, 0, 33);
            memset(zsize, 0, 9 * sizeof(int));

            debug4 << "\tzone " << zone << endl;
            // Print the name and size.
            if(cg_zone_read(GetFileHandle(), base, zone, zonename, zsize) != CG_OK)
                debug4 << cg_get_error() << endl;
            else
            {
                debug4 << "\t\tname=" << zonename << endl;
                debug4 << "\t\tsize=[";
                for(int zi = 0; zi < 9; ++zi)
                {
                    if(zi > 0) { debug4 << ", "; }
                        debug4 << zsize[zi];
                }
                debug4 << "]" << endl;
            }
            // Save the domain name.
            baseInfo.zoneNames.push_back(zonename);

            // Get the zone type.
            ZoneType_t zt = ZoneTypeNull;
            if(cg_zone_type(GetFileHandle(), base, zone, &zt) != CG_OK)
                debug4 << cg_get_error() << endl;
            else
            {
                switch(zt)
                {
                case ZoneTypeNull:
                    debug4 << " type=ZoneTypeNull (NOT SUPPORTED!)";
                    baseInfo.meshType = -1;
                    break;
                case ZoneTypeUserDefined:
                    debug4 << " type=ZoneTypeUserDefined (NOT SUPPORTED!)";
                    baseInfo.meshType = -2;
                    break;
                case Structured:
                    debug4 << " type=Structured";
                    baseInfo.meshType |= 0;
                    break;
                case Unstructured:
                    debug4 << " type=Unstructured";
                    baseInfo.meshType |= 1;
                    break;
                }
            }

            // Get the units for the zone.
            bool zoneUnits = (zone == 1) && baseUnits &&
                              unitStack.PushUnits(GetFileHandle(), base, zone);

            int nsols = 0;
            if(cg_nsols(GetFileHandle(), base, zone, &nsols) != CG_OK)
            {
                debug4 << "Could not get number of solutions in zone "
                       << zone << endl;
                debug4 << cg_get_error() << endl;
                continue;
            }
            debug4 << "\t\tnsols=" << nsols << endl;

            //
            // Iterate over the solutions.
            //
            for(int sol = 1; sol <= nsols; ++sol)
            {
                char solname[33];
                GridLocation_t varcentering;
                if(cg_sol_info(GetFileHandle(), base, zone, sol, solname,
                               &varcentering) != CG_OK)
                {
                    debug4 << "Could not get solution " << sol << "'s info."
                           << endl;
                    debug4 << cg_get_error() << endl;
                    continue;
                }

                debug4 << "\t\t\t" << "solution[" << sol << "]" << solname;

                int nfields = 0;
                if(cg_nfields(GetFileHandle(), base, zone, sol, &nfields) != CG_OK)
                {
                    debug4 << "Could not get number of fields for solution "
                           << sol << endl;
                    debug4 << cg_get_error() << endl;
                    continue;
                }

                // Get the units for the solution.
                bool solUnits = (zone == 1) && baseUnits &&
                            unitStack.PushUnits(GetFileHandle(), base, zone, sol);

                debug4 << "\t\t\t\tnfields=" << nfields << endl;
                debug4 << "\t\t\t\tcentering=";
                int nodeCentering = 0;
                int cellCentering = 0;
                int badCentering = 1;
                switch(varcentering)
                {
                case GridLocationNull:
                     debug4 << "GridLocationNull";
                     break;
                case GridLocationUserDefined:
                     debug4 << "GridLocationUserDefined";
                     break;
                case Vertex:
                     debug4 << "Vertex";
                     nodeCentering = 1;
                     badCentering = 0;
                     break;
                case CellCenter:
                     debug4 << "CellCenter";
                     cellCentering = 1;
                     badCentering = 0;
                     break;
                case FaceCenter:
                     debug4 << "FaceCenter";
                     break;
                case IFaceCenter:
                     debug4 << "IFaceCenter";
                     break;
                case JFaceCenter:
                     debug4 << "JFaceCenter";
                     break;
                case KFaceCenter:
                     debug4 << "KFaceCenter";
                     break;
                case EdgeCenter:
                     debug4 << "EdgeCenter";
                     break;
                }
                debug4 << endl;

                for(int f = 1; f <= nfields; ++f)
                {
                    DataType_t dt;
                    char fieldname[33];

                    if(cg_field_info(GetFileHandle(), base, zone, sol, f, &dt, fieldname) != CG_OK)
                    {
                        debug4 << "Could not get number of fields for solution "
                               << sol << endl;
                        debug4 << cg_get_error() << endl;
                        continue;
                    }
                    debug4 << "\t\t\t\t\tfield[" << f << "]=" << fieldname << endl;

                    // Determine the units for the field.
                    bool fUnits = baseUnits &&
                                  unitStack.PushUnits(GetFileHandle(), base, zone, sol, f);
                    std::string fieldUnits;
                    bool fieldHasUnits = unitStack.GetUnits(fieldUnits);
                    if(fUnits)
                        unitStack.PopUnits();

                    // Now that we have a field in a solution in a zone, let's
                    // add information about it to the variable information that
                    // we're populating for the current base.
                    StringVarInfoMap::iterator pos = baseInfo.vars.find(fieldname);
                    if(pos == baseInfo.vars.end())
                    {
                        VarInfo info;
                        info.zoneList.push_back(zone);
                        info.cellCentering = cellCentering;
                        info.nodeCentering = nodeCentering;
                        info.badCentering = badCentering;
                        info.hasUnits = fieldHasUnits;
                        if(fieldHasUnits)
                            info.units = fieldUnits;
                        baseInfo.vars[fieldname] = info;
                    }
                    else if(sol == 1)
                    {
                        // We've already run across the variable in another zone
                        // so let's update what we know.
                        // This is done only for first iteration.
                        pos->second.zoneList.push_back(zone);
                        pos->second.cellCentering += cellCentering;
                        pos->second.nodeCentering += nodeCentering;
                        pos->second.badCentering += badCentering;
                        pos->second.hasUnits = fieldHasUnits;
                        if(fieldHasUnits)
                            pos->second.units = fieldUnits;
                    }
                } // for field

                // Pop the unit stack.
                if(solUnits)
                    unitStack.PopUnits();
            } // for sol

            // Pop the unit stack.
            if(zoneUnits)
                unitStack.PopUnits();
        } // for zone
    } // base read

    return retval;
}

// ****************************************************************************
// Method: avtCGNSFileReader::AddVectorExpression
//
// Purpose:
//   Adds a vector expression.
//
// Arguments:
//   md            : The metadata to which the expression will be added.
//   haveComponent : An array of 3 bools indicating whether we have certain
//                   vector components.
//   nBases        : The number of bases in the file.
//   baseName      : The current base name.
//   vecName       : The name of the vector.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:20:41 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::AddVectorExpression(avtDatabaseMetaData *md, bool *haveComponent,
    int nBases, const std::string &baseName, const std::string &vecName)
{
    char def[300];

    if(haveComponent[0] && haveComponent[1] && haveComponent[2])
    {
        Expression *e = new Expression;
        if(nBases > 1)
        {
            e->SetName(baseName + "/" + vecName);
            snprintf(def, 300, "{<%s/%sX>,<%s/%sY>,<%s/%sZ>}",
                baseName.c_str(), vecName.c_str(),
                baseName.c_str(), vecName.c_str(),
                baseName.c_str(), vecName.c_str());
            e->SetDefinition(def);
        }
        else
        {
            e->SetName(vecName);
            snprintf(def, 300, "{%sX,%sY,%sZ}",
                vecName.c_str(), vecName.c_str(), vecName.c_str());
            e->SetName(vecName);
            e->SetDefinition(def);
        }
        e->SetType(Expression::VectorMeshVar);
        md->AddExpression(e);
    }
    else if(haveComponent[0] && haveComponent[1])
    {
        Expression *e = new Expression;
        if(nBases > 1)
        {
            e->SetName(baseName + "/" + vecName);
            snprintf(def, 300, "{<%s/%sX>,<%s/%sY>}",
                baseName.c_str(), vecName.c_str(),
                baseName.c_str(), vecName.c_str());
            e->SetDefinition(def);
        }
        else
        {
            e->SetName(vecName);
            snprintf(def, 300, "{%sX,%sY}",
                vecName.c_str(), vecName.c_str());
            e->SetName(vecName);
            e->SetDefinition(def);
        }
        e->SetType(Expression::VectorMeshVar);
        md->AddExpression(e);
    }
}

// ****************************************************************************
// Method: avtCGNSFileReader::AddVectorExpressions
//
// Purpose:
//   Adds vector expressions for velocity and momentum.
//
// Arguments:
//   md            : The metadata to which the expression will be added.
//   haveVelocity  : An array of 3 bools indicating whether we have certain
//                   vector velocity components.
//   haveMomentum  : An array of 3 bools indicating whether we have certain
//                   vector momentum components.
//   nBases        : The number of bases in the file.
//   baseName      : The current base name.
//   vecName       : The name of the vector.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:22:32 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::AddVectorExpressions(avtDatabaseMetaData *md, bool *haveVelocity,
    bool *haveMomentum, int nBases, const std::string &baseName)
{
    AddVectorExpression(md, haveVelocity, nBases, baseName, "Velocity");
    AddVectorExpression(md, haveMomentum, nBases, baseName, "Momentum");
}

// ****************************************************************************
// Method: avtCGNSFileReader::AddReferenceStateExpressions
//
// Purpose:
//   Creates constant valued fields on the mesh based on values stored in
//   the reference state.
//
// Arguments:
//   md       : The metadata to which the expressions will be added.
//   base     : The current baes index.
//   nBases   : The number of bases in the file.
//   baseName : The name of the current base.
//   meshName : The name of the mesh on which we'll define the values.
//
// Returns:
//
// Note:
//
// Programmer: Maxim Loginov
// Creation:   Thu Apr 17 10:27:23 PDT 2008
//
// Modifications:
//   Brad Whitlock, Thu Apr 17 10:28:03 PDT 2008
//   Separated out into its own method, add variables as point_constant
//   expressions.
//
//    Jeremy Meredith, Thu Aug  7 15:55:54 EDT 2008
//    Some string comparisons were erroneously comparing char* pointers.
//    I converted them to use strcmp.
//
// ****************************************************************************

void
avtCGNSFileReader::AddReferenceStateExpressions(avtDatabaseMetaData *md,
    int base, int nBases, const std::string &baseName, const std::string &meshName)
{
    const char *mName = "avtCGNSFileReader::AddReferenceStateExpressions: ";

    // some constants from ReferenceState_t should be available as array mesh variable
    // TODO not finished yet!!!
    // Note: BJW - We don't have a good way to add constants in VisIt. I just add them
    //             to the 1st mesh as a node-centered constant using the point_constant
    //             expression.
    if(cg_goto(GetFileHandle(), base, "ReferenceState_t", 1, "end") == CG_OK)
    {
        int nrefstate = 0;
        if(cg_narrays(&nrefstate) == CG_OK)
        {
            debug5 << mName << nrefstate << " reference states found" << endl;
            for(int i = 0; i < nrefstate; ++i)
            {
                char namenode[33];
                int ndims = 1;
                cgsize_t dims[10];
                DataType_t dt;
                if(cg_array_info(i+1, namenode, &dt, &ndims, dims) == CG_OK)
                {
                    // there is only one value in each array
                    double dval;
                    char edef[100];
                    cg_array_read_as(i+1, RealDouble, &dval);
                    debug5 << mName << "Reference state: " << namenode
                           << " = " << dval << endl;
                    if(strcmp(namenode,"Mach")==0)
                    {
                        Expression *e = new Expression;
                        if(nBases > 1)
                            e->SetName(baseName + "/mach");
                        else
                            e->SetName("mach");
                        snprintf(edef, 100, "point_constant(%s, %g)", meshName.c_str(), dval);
                        e->SetDefinition(edef);
                        e->SetType(Expression::ScalarMeshVar);
                        md->AddExpression(e);
                    }
                    else if(strcmp(namenode, "SpecificHeatRatio")==0)
                    {
                        Expression *e = new Expression;
                        if(nBases > 1)
                            e->SetName(baseName + "/gamma");
                        else
                            e->SetName("gamma");
                        snprintf(edef, 100, "point_constant(%s, %g)", meshName.c_str(), dval);
                        e->SetDefinition(edef);
                        e->SetType(Expression::ScalarMeshVar);
                        md->AddExpression(e);
                    }
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: avtCGNSFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Maxim Loginov, Tue Mar  4 12:28:12 NOVT 2008
//    Some constants from ReferenceState_t should be available as array mesh
//    variables.
//
//    Brad Whitlock, Wed Apr 16 10:07:16 PDT 2008
//    Totally rewrote to support reading data from multiple bases. It's more
//    modular too.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Mark C. Miller, Mon Sep 21 14:17:47 PDT 2009
//    Adding missing calls to actually set the times/cycles in the metadata.
//
//    Eric Brugger, Thu Jul  2 10:56:36 PDT 2020
//    Corrected a bug that caused a crash when doing a Subset plot of "zones"
//    when reading data decomposed across multiple CGNS files.
//
// ****************************************************************************

void
avtCGNSFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    const char *mName = "avtCGNSFileReader::PopulateDatabaseMetaData: ";

    // Read the times if we have not read them yet.
    ReadTimes();
    md->SetTimesAreAccurate(cgnsTimesAccurate);
    if (cgnsTimesAccurate)
        md->SetTimes(times);
    md->SetCyclesAreAccurate(cgnsCyclesAccurate);
    if (cgnsCyclesAccurate)
        md->SetCycles(cycles);

    // Get the title
    char *refstate = 0;
    if(cg_goto(GetFileHandle(), 1, "end") == CG_OK)
    {
        if(cg_state_read(&refstate) == CG_OK)
        {
            md->SetDatabaseComment(refstate);
            debug4 << mName << "Reference string = " << refstate << endl;
            cg_free(refstate);
        }
        else
            debug4 << mName << cg_get_error() << endl;
    }
    else
        debug4 << mName << cg_get_error() << endl;

    // Read the number of bases
    int nbases = 0;
    if(cg_nbases(GetFileHandle(), &nbases) != CG_OK)
    {
        debug4 << cg_get_error() << endl;
        EXCEPTION1(InvalidFilesException, cgnsFileName);
    }
    debug4 << mName << "The file contains " << nbases << " bases." << endl;

    // Read the variables for each base.
    debug4 << "====================== READING FILE ======================" << endl;
    BaseInformationVector baseInfo;
    for(int base = 1; base < nbases+1; ++base)
    {
        BaseInformation info;
        if(GetVariablesForBase(base, info))
        {
            baseInfo.push_back(info);
            BaseNameToIndices[info.name] = base;
            debug4 << mName << "Associating name \"" << info.name.c_str()
                   << "\" with base " << base << endl;
        }
    }
    // Print the information that we read from the file.
    debug4 << "==================== BASE INFORMATION ====================" << endl;
    for(size_t bi = 0; bi < baseInfo.size(); ++bi)
    {
        if(DebugStream::Level4())
            PrintBaseInformation(DebugStream::Stream4(), baseInfo[bi]);
    }

    bool someInvalidCenterings = false;

    // Now that we have variables for each base, let's determine the
    // meshes that we need to create for each base. Let's use the base name
    // as the mesh name if there is more than one zone in a base. If there's
    debug4 << "=================== POPULATE VARIABLES ===================" << endl;
    for(size_t bi = 0; bi < baseInfo.size(); ++bi)
    {
        std::string baseName(baseInfo[bi].name);
        baseName = MakeSafeVariableName(baseName);
        VisItNameToCGNSName[baseName] = baseInfo[bi].name;

        //
        // STEP 1: Come up with a mesh name.
        //
        bool useBaseNameForMesh = baseInfo.size() > 1;
        bool domainResolution = baseInfo[bi].zoneNames.size() > 1;
        std::string meshName;
        if(useBaseNameForMesh || domainResolution)
            meshName = baseName;
        else
            meshName = baseInfo[bi].zoneNames[0];
        meshName = MakeSafeVariableName(meshName);
        debug4 << mName << "Step 1: meshName = " << meshName.c_str() << endl;

        //
        // STEP 2: Determine how many meshes are required based on the
        //         unique lists of domains that we have used in the
        //         variables.
        //
        std::map<intVector, std::string> meshDef;
        intVector allDomains;
        for(int i = 0; i < (int)baseInfo[bi].zoneNames.size(); ++i)
            allDomains.push_back(i+1);
        meshDef[allDomains] = meshName;
        debug4 << mName << "Step 2: Need mesh " << meshName.c_str() << endl;
        int meshCount = 0;
        for(StringVarInfoMap::const_iterator it = baseInfo[bi].vars.begin();
            it !=  baseInfo[bi].vars.end(); ++it)
        {
            std::map<intVector, std::string>::iterator meshIt =
                meshDef.find(it->second.zoneList);
            if(meshIt == meshDef.end())
            {
                ++meshCount;
                char tmp[100];
                snprintf(tmp, 100, "subgrid/%s%03d",
                         baseName.c_str(), meshCount);
                meshDef[it->second.zoneList] = std::string(tmp);
                debug4 << mName << "Step 2: Need mesh " << tmp << endl;
            }
        }

        //
        // STEP 3: Create mesh metadata for each mesh that we need.
        //
        for(std::map<intVector, std::string>::const_iterator it = meshDef.begin();
            it != meshDef.end(); ++it)
        {
            bool validVariable = true;
            avtMeshType mt = AVT_UNKNOWN_MESH;
            if(baseInfo[bi].meshType == 0)
                 mt = AVT_CURVILINEAR_MESH;
            else if(baseInfo[bi].meshType == 1)
                 mt = AVT_UNSTRUCTURED_MESH;
            else
                validVariable = false;

            avtMeshMetaData *mmd = new avtMeshMetaData(it->second,
                1, 1, 1, 0, baseInfo[bi].physicalDim, baseInfo[bi].cellDim, mt);

            if (cgnsIsMTMD)
            {
                //
                // The normal case.
                //
                stringVector domainNames;
                for(size_t di = 0; di < it->first.size(); ++di)
                {
                    int idx = it->first[di] - 1;
                    domainNames.push_back(baseInfo[bi].zoneNames[idx]);
                }
                mmd->blockNames = domainNames;
                mmd->numBlocks = (int)domainNames.size();
            }
            else
            {
                //
                // The parallel CGNS file case. Don't set the blockNames
                // and set the number of blocks to 1. This is so that
                // avtGenericDatabase::ReadDataset, when handling
                // AVT_DOMAIN_SUBSET doesn't seg fault.
                //
                mmd->numBlocks = 1;
            }
            mmd->blockOrigin = 1;
            mmd->groupOrigin = 1;
            mmd->cellOrigin = 1;
            mmd->blockPieceName = "zone";
            mmd->blockTitle = "zones";
            mmd->validVariable = validVariable;
            // Get the mesh coordinate units...
            md->Add(mmd);

            // Remember the list of zones that make up the mesh so we can use it
            // later in GetMesh.
            BaseAndZoneList bzl;
            bzl.base = (int)bi+1;
            bzl.zones = it->first;
            MeshDomainMapping[it->second] = bzl;

            // Print the entry we just created in MeshDomainMapping
            debug4 << mName << "Step 3: Creating mesh " << it->second.c_str() << " for base "
                   << bzl.base << " for zones [";
            for(size_t zi = 0; zi < it->first.size(); ++zi)
            {
                debug4 << it->first[zi];
                if(zi <  it->first.size()-1) {
                    debug4 << ", ";
                }
            }
            debug4 << "]" << endl;
        }

        //
        // STEP 4: Create scalar metadata for each variable in the current base.
        //
        bool haveVelocity[3] = {false, false, false};
        bool haveMomentum[3] = {false, false, false};
        for(StringVarInfoMap::const_iterator it = baseInfo[bi].vars.begin();
            it !=  baseInfo[bi].vars.end(); ++it)
        {
            std::string fieldName(MakeSafeVariableName(it->first));
            if(fieldName != it->first)
                VisItNameToCGNSName[fieldName] = it->first;

            // See if we have Velocity and Momentum components.
            haveVelocity[0] |= (fieldName == "VelocityX");
            haveVelocity[1] |= (fieldName == "VelocityY");
            haveVelocity[2] |= (fieldName == "VelocityZ");
            haveMomentum[0] |= (fieldName == "MomentumX");
            haveMomentum[1] |= (fieldName == "MomentumY");
            haveMomentum[2] |= (fieldName == "MomentumZ");

            // If there is more than 1 base, prepend the base name to the
            // field name to create the variable name.
            if(baseInfo.size() > 1)
                fieldName = baseName + "/" + fieldName;

            // Determine the centering
            avtCentering centering;
            bool validVariable = true;
            if(it->second.badCentering > 0)
            {
                centering = AVT_ZONECENT;
                validVariable = false;
                someInvalidCenterings = true;
            }
            else if(it->second.cellCentering > 0)
                centering = AVT_ZONECENT; // Force to zonecent if there is mixed centering
            else
                centering = AVT_NODECENT;

            // Get the name of the mesh to use for this variable.
            std::string varMesh(meshDef[it->second.zoneList]);

            // Create the scalar metadata.
            avtScalarMetaData *smd = new avtScalarMetaData(fieldName,
                        varMesh, centering);
            smd->validVariable = validVariable;
            smd->hasUnits = it->second.hasUnits;
            smd->units = it->second.units;
            md->Add(smd);

            debug4 << mName << "Step 4: Adding scalar " << fieldName.c_str()
                   << " on mesh " << varMesh.c_str() << endl;
        }

        //
        // STEP 5: Create Velocity and Momentum vectors if present.
        //
        AddVectorExpressions(md, haveVelocity, haveMomentum, (int)baseInfo.size(),
            baseName);

        //
        // STEP 6: Create expressions for reference state variables.
        //
        AddReferenceStateExpressions(md, (int)bi+1, (int)baseInfo.size(),
            baseName, meshName);
    }

    // If some variables had unsupported centerings then issue a warning.
    if(someInvalidCenterings)
    {
        avtCallback::IssueWarning("Some variables have been disabled because "
            "their grid locations (variable centerings) are not supported by "
            "VisIt. VisIt currently supports node and cell centered variables.");
    }

    // Indicate that we've initialized maps.
    initializedMaps = true;
}

// ****************************************************************************
// Method: avtCGNSFileReader::InitializeMaps
//
// Purpose:
//   Populate a dummy metadata with side-effect of initializing variable maps
//   that we need to read data.
//
// Arguments:
//   timeState : The time state.
//
// Returns:
//
// Note:       We call this method from GetMesh, GetVar so we can group files
//             since PopulateDatabaseMetaData gets skipped for grouped files.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 13 11:11:11 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::InitializeMaps(int timeState)
{
    if(!initializedMaps)
    {
        avtDatabaseMetaData md;
        PopulateDatabaseMetaData(&md, timeState);
    }
}

// ****************************************************************************
//  Method: avtCGNSFileReader::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Brad Whitlock, Wed Apr 16 10:06:46 PDT 2008
//    Changed how we search MeshDomainMapping.
//
//    Brad Whitlock, Thu Oct 13 11:13:30 PDT 2011
//    Call InitializeMaps so we can group files.
//
// ****************************************************************************

vtkDataSet *
avtCGNSFileReader::GetMesh(int timestate, int domain, const char *meshname)
{
    const char *mName = "avtCGNSFileReader::GetMesh: ";
    debug4 << mName << "ts=" << timestate << ", dom=" << domain
           << ", mesh=" << meshname << endl;

    InitializeMaps(timestate);

    //
    // See if this domain is turned off by default for this mesh.
    //
    std::map<std::string, BaseAndZoneList>::const_iterator pos =
        MeshDomainMapping.find(meshname);
    if(pos == MeshDomainMapping.end())
    {
        if (MeshDomainMapping.size() == 1)
        {
            pos = MeshDomainMapping.begin();
        }
        else
        {
            return 0;
        }
    }
    int base = pos->second.base;
    int zone = domain + 1;
    const intVector &zones = pos->second.zones;
    debug4 << mName << "Checking if zone " << zone << " is part of "
           << meshname << endl;
    debug4 << "zones = {";
    for(size_t i = 0; i < zones.size(); ++i)
        debug4 << zones[i] << ", ";
    debug4 << "}" << endl;
    if(std::find(zones.begin(), zones.end(), zone) == zones.end())
    {
        debug4 << mName << "No, the mesh does not contain zone " << zone << endl;
        return 0;
    }
    debug4 << mName << "Yes, the mesh contains zone " << zone << endl;
    debug4 << mName << "Mesh " << meshname << " exists in base " << base << endl;

    vtkDataSet *retval = 0;
    char zonename[33];
    cgsize_t zsize[9];
    memset(zonename, 0, 33);
    memset(zsize, 0, 9 * sizeof(int));

    //
    // Determine the topological and spatial dimensions.
    //
    char namebase[33];
    int cell_dim = 2, phys_dim = 2;
    if(cg_base_read(GetFileHandle(), base, namebase, &cell_dim, &phys_dim) != CG_OK)
    {
        debug4 << cg_get_error() << endl;
        EXCEPTION1(InvalidFilesException, cgnsFileName);
    }
    else
    {
        debug4 << mName << " name=" << namebase << " cell_dim=" << cell_dim
               << " phys_dim=" << phys_dim << endl;
    }

    if(cg_zone_read(GetFileHandle(), base, zone, zonename, zsize) != CG_OK)
    {
        debug4 << mName << cg_get_error() << endl;
        EXCEPTION1(InvalidVariableException, meshname);
    }
    else
    {
        // Print the zone info.
        debug4 << mName << " name=" << zonename << " size=[";
        for(int zi = 0; zi < 9; ++zi)
        {
            if(zi > 0) { debug4 << ", "; }
            debug4 << zsize[zi];
        }
        debug4 << "]\n";

        ZoneType_t zt = ZoneTypeNull;
        if(cg_zone_type(GetFileHandle(), base, zone, &zt) != CG_OK)
        {
            debug4 << mName << cg_get_error() << endl;
            EXCEPTION1(InvalidVariableException, meshname);
        }
        else
        {
            switch(zt)
            {
            case ZoneTypeNull:
                EXCEPTION1(InvalidVariableException,
                           "Meshes with ZoneTypeNull are not supported.");
                break;
            case ZoneTypeUserDefined:
                EXCEPTION1(InvalidVariableException,
                           "Meshes with ZoneTypeUserDefined are not supported.");
                break;
            case Structured:
                retval = GetCurvilinearMesh(timestate, base, zone, meshname, zsize, cell_dim, phys_dim);
                break;
            case Unstructured:
                retval = GetUnstructuredMesh(timestate, base, zone, meshname, zsize, cell_dim, phys_dim);
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetCoords
//
// Purpose:
//   Read the coordinates for the specified zone.
//
// Arguments:
//   base       : The base to use
//   zone       : The zone (mesh) that whose coordinates we want.
//   zsize      : Zone size information.
//   structured : Whether the mesh is structured.
//   coords     : Return array for the coordinates.
//
// Returns:    True if the coordinates were read; false otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 11:45:55 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Dec 11 09:28:52 PDT 2006
//   Prevent the coordinate arrays from being too large in the structured
//   1D, 2D cases.
//
//   Maxim Loginov, Thu Feb 28 13:36:46 PST 2008
//   Bugfix for too large arrays in the structured 1D, 2D case
//
//   Brad Whitlock, Mon Jun 18 15:21:02 PDT 2012
//   Don't pass out ncoords.
//
// ****************************************************************************

bool
avtCGNSFileReader::GetCoords(int timestate, int base, int zone, const cgsize_t *zsize,
    int cell_dim, int phys_dim, bool structured, float **coords)
{
    const char *mName = "avtCGNSFileReader::GetCoords: ";
    bool err = false;

    // Init the coord array
    coords[0] = 0;
    coords[1] = 0;
    coords[2] = 0;

    // Iterate through the coordinates and read them in.
    int ncoords = 0;
    if(cg_ncoords(GetFileHandle(), base, zone, &ncoords) != CG_OK)
    {
        debug4 << mName << "\t\tCould not get the number of coords" << endl;
        debug4 << mName << cg_get_error() << endl;
    }
    else
    {
        debug4 << mName << "ncoords = " << ncoords << endl;
        if(ncoords > 3)
            ncoords = 3;
        err = (ncoords != phys_dim);

        unsigned int nPts = 0;
        cgsize_t rmax[3] = {1,1,1};
        if(structured)
        {
            if(cell_dim == 1)
            {
                rmax[0] = zsize[0];
            }
            else if(cell_dim == 2)
            {
                rmax[0] = zsize[0];
                rmax[1] = zsize[1];
            }
            else
            {
                rmax[0] = zsize[0];
                rmax[1] = zsize[1];
                rmax[2] = zsize[2];
            }
            nPts = rmax[0] * rmax[1] * rmax[2];
        }
        else
        {
            rmax[0] = zsize[0];
            nPts = zsize[0];
        }

        // Check the number of grids stored in zone
        int ngrids = 0;
        if(cg_ngrids(GetFileHandle(), base, zone, &ngrids) != CG_OK)
        {
          debug4 << mName << "Could not get number of grids in zone "
                 << zone << endl;
          debug4 << cg_get_error() << endl;
        }
        // If the solution is unsteady but not the mesh, timestate will change but requiredgrid
        // should remain bounded.
        int requiredgrid = (timestate < ngrids) ? (timestate + 1) : ngrids;

        char GridCoordName[33];
        cg_grid_read(GetFileHandle(), base, zone, requiredgrid, GridCoordName);

        debug4 << "Reading mesh node " << GridCoordName << endl;
        if (cg_goto(GetFileHandle(), base, "Zone_t", zone, GridCoordName, 0, "end") != CG_OK)
        {
            debug4 << cg_get_error() << endl;
        }

        int narrays=0;
        cg_narrays(&narrays);
        if(narrays < ncoords)
        {
            debug4 << "Not enough coordinates in node " << GridCoordName << endl;
            err = true;
        }
        // Every grid is read through cg_array. However, "GridCoordinates" node should always be present
        // to describe reference state according to CGNS Grid Specification.
        for(int c = 1; c <= ncoords; ++c)
        {
            char coordname[33];
            DataType_t ct;
            if(err == true) break;
            if(cg_coord_info(GetFileHandle(), base, zone, c, &ct,
                coordname) != CG_OK)
            {
                debug4 << mName << cg_get_error() << endl;
            }
            else
            {
                debug5 << mName << "Array for " << coordname
                       << " has " << nPts << " points." << endl;
                coords[c-1] = new float[nPts];
                // Read the various coordinates as float
                debug4 << mName << "Reading " << coordname
                       << " as a float array." << endl;
                if(cg_array_read_as(c, RealSingle, (void*)coords[c-1] ) != CG_OK)
                {
                    debug4 << mName << cg_get_error() << endl;
                    err = true;
                }
            }
        }

        if(err)
        {
            delete [] coords[0];
            delete [] coords[1];
            delete [] coords[2];
            coords[0] = 0;
            coords[1] = 0;
            coords[2] = 0;
        }
    }

    return !err;
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetQuadGhostZones
//
// Purpose:
//   Add the ghost zone information to the specified data set.
//
// Arguments:
//   base       : The base to use
//   zone       : The zone (mesh) that whose rind data we want.
//   zsize      : The zone size information.
//   cell_dim   : The cell dimension.
//   ds         : The data set to add the ghost data to.
//
// Programmer: Eric Brugger
// Creation:   Tue Jul  6 10:27:03 PDT 2021
//
// Modifications:
//   Eric Brugger, Wed Jul 14 13:32:10 PDT 2021
//   Added a temporary hack to only generate ghost zones if the
//   environment variable CGNS_USE_RIND is set.
//
// ****************************************************************************

void
avtCGNSFileReader::GetQuadGhostZones(int base, int zone,
    const cgsize_t *zsize, int cell_dim, vtkDataSet *ds)
{
    const char *mName = "avtCGNSFileReader::GetQuadGhostZones: ";

    // Read the rind data.
    debug4 << "Reading rind node." << endl;
    int rind[6];
    if(cg_goto(GetFileHandle(), base, "Zone_t", zone, "FlowSolution_t", 1, "end") == CG_OK)
    {
        if (cg_rind_read(rind) == CG_OK)
        {
            debug4 << "rind=" << rind[0] << "," << rind[1] << ","
                              << rind[2] << "," << rind[3] << ","
                              << rind[4] << "," << rind[5] << endl;
        }
        else
        {
            debug4 << "No rind data." << endl;
            return;
        }
    }
    else
    {
        debug4 << "Error going to FlowSolution." << endl;
        return;
    }

    // Determine the size of the mesh.
    unsigned int ncells = 0;
    cgsize_t cdims[3] = {1,1,1};
    if(cell_dim == 1)
    {
        cdims[0] = zsize[0]-1;
    }
    else if(cell_dim == 2)
    {
        cdims[0] = zsize[0]-1;
        cdims[1] = zsize[1]-1;
    }
    else
    {
        cdims[0] = zsize[0]-1;
        cdims[1] = zsize[1]-1;
        cdims[2] = zsize[2]-1;
    }
    ncells = cdims[0] * cdims[1] * cdims[2];

    // Determine if the rind information is valid.
    cgsize_t first[3];
    cgsize_t last[3];
    bool ghostPresent = false;
    bool badIndex = false;
    for (int i = 0; i < cell_dim; i++)
    {
        first[i] = rind[i*2];
        last[i]  = cdims[i] - rind[i*2+1] - 1;

        if (first[i] < 0 || first[i] >= cdims[i])
        {
            debug1 << "bad rind value: rind[" << i*2 << "]=" << rind[i*2]
                   << endl;
            badIndex = true;
        }

        if (last[i] < 0 || last[i] >= cdims[i])
        {
            debug1 << "bad rind value: rind[" << i*2+1 << "]=" << rind[i*2+1]
                   << endl;
            badIndex = true;
        }

        if (first[i] != 0 || last[i] != cdims[i] - 1)
        {
            ghostPresent = true;
        }
    }

    // Temporary hack to only generate ghost zones if the environment
    // variable CGNS_USE_RIND is set. This is because there exist some
    // CGNS files that are single block with rind zones all around the
    // exterior. Adding ghost zones in this case generates a blank image.
    // The particular file is delta.cgns.
    if (getenv("CGNS_USE_RIND") == NULL)
    {
        ghostPresent = false;
        debug4 << "Disabling use of rind values." << endl;
        debug4 << "To enable setenv CGNS_USE_RIND." << endl;
    }

    //  Create the ghost zones array if necessary
    if (ghostPresent && !badIndex)
    {
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->Allocate(ncells);

        unsigned char realVal = 0;
        unsigned char ghostVal = 0;
        avtGhostData::AddGhostZoneType(ghostVal,
                                       DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        for (int i = 0; i < ncells; i++)
            ghostCells->InsertNextValue(ghostVal);

        unsigned char *gv = ghostCells->GetPointer(0);
        for (int k = first[2]; k <= last[2]; k++) {
            for (int j = first[1]; j <= last[1]; j++) {
                for (int i = first[0]; i <= last[0]; i++)
                {
                    int index = k*cdims[1]*cdims[0] + j*cdims[0] + i;
                    gv[index] = realVal;
                }
            }
        }

        ds->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->SetNumberOfValues(6);
        realDims->SetValue(0, first[0]);
        realDims->SetValue(1, last[0]+1);
        realDims->SetValue(2, first[1]);
        realDims->SetValue(3, last[1]+1);
        realDims->SetValue(4, first[2]);
        realDims->SetValue(5, last[2]+1);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();
    }

    ds->GetInformation()->Set(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetCurvilinearMesh
//
// Purpose:
//   Reads a curvilinear mesh from the file.
//
// Arguments:
//   base     : The CGNS base to use.
//   zone     : The CGNS zone to use (The domain number)
//   meshname : The name of the mesh to get. (unused currently)
//   zsize    : The size information associated with the zone.
//
// Returns:    A curvilinear mesh or 0 if we can't read it from the file.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:30:30 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Dec 11 09:42:35 PDT 2006
//   Corrected support for 2D.
//
//   Mickael Philit, Mon Jun 18 15:23:47 PDT 2012
//   Work with meshes whose tdim != sdim.
//
// ****************************************************************************

vtkDataSet *
avtCGNSFileReader::GetCurvilinearMesh(int timestate, int base, int zone, const char *meshname,
    const cgsize_t *zsize, int cell_dim, int phys_dim)
{
    vtkDataSet *retval = 0;

    // Get the coords
    float *coords[3] = {0,0,0};
    if(GetCoords(timestate, base, zone, zsize, cell_dim, phys_dim, true, coords))
    {
        // Create the curvilinear mesh.
        vtkStructuredGrid *sgrid   = vtkStructuredGrid::New();
        vtkPoints         *points  = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();

        // Populate the points array
        int dims[3];
        dims[0] = zsize[0];
        dims[1] = (cell_dim >= 2) ? zsize[1] : 1;
        dims[2] = (cell_dim == 3) ? zsize[2] : 1;
        sgrid->SetDimensions(dims);
        points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);
        float *pts = (float *) points->GetVoidPointer(0);
        float *xc = coords[0];
        float *yc = coords[1];
        if(phys_dim == 3)
        {
            float *zc = coords[2];
            for(int k = 0; k < dims[2]; ++k)
            {
                for(int j = 0; j < dims[1]; ++j)
                {
                    for(int i = 0; i < dims[0]; ++i)
                    {
                        *pts++ = *xc++;
                        *pts++ = *yc++;
                        *pts++ = *zc++;
                    }
                }
            }
        }
        else if(phys_dim == 2)
        {
            for(int j = 0; j < dims[1]; ++j)
            {
                for(int i = 0; i < dims[0]; ++i)
                {
                    *pts++ = *xc++;
                    *pts++ = *yc++;
                    *pts++ = 0.;
                }
            }
        }
        else if(phys_dim == 1)
        {
            for(int i = 0; i < dims[0]; ++i)
            {
                *pts++ = *xc++;
                *pts++ = 0.;
                *pts++ = 0.;
            }
        }
        retval = sgrid;

        delete [] coords[0];
        delete [] coords[1];
        delete [] coords[2];
    }
    else
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    GetQuadGhostZones(base, zone, zsize, cell_dim, retval);

    return retval;
}

// ****************************************************************************
// Method: avtCGNSFileReader::GetUnstructuredMesh
//
// Purpose:
//   Reads an unstructured mesh from the file.
//
// Arguments:
//   base     : The CGNS base to use.
//   zone     : The CGNS zone to use (The domain number)
//   meshname : The name of the mesh to get. (unused currently)
//   zsize    : The size information associated with the zone.
//
// Returns:    An unstructured mesh or 0 if we can't read it from the file.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:30:30 PDT 2005
//
// Modifications:
//   Kathleen Bonnell, Wed Feb  8 09:41:45 PST 2006
//   Don't retrieve zcoords if ncoords != 3.
//
//   Brad Whitlock, Wed Jun  4 14:34:17 PDT 2008
//   Iterate over all of the sections but skip those that set parent_flag>0
//   since they are probably boundary conditions or things we don't really
//   care about.
//
//   Jeremy Meredith, Thu Aug  7 14:14:00 EDT 2008
//   Added some missing cases for switch.
//
//   Mickael Philit, Mon Jun 18 15:25:55 PDT 2012
//   Pass in number of spatial dimensions.
//
//   Alister Maguire, Tue Mar  2 08:01:12 PST 2021
//   Refactored to handle NGon and NFace element types. I also moved
//   a large section of code used for handling named and mixed
//   sections to a new function, ReadMixedAndNamedElementSections.
//
// ****************************************************************************

vtkDataSet *
avtCGNSFileReader::GetUnstructuredMesh(int timestate, int base, int zone,
    const char *meshname, const cgsize_t *zsize, int cell_dim, int phys_dim)
{
    const char *mName = "avtCGNSFileReader::GetUnstructuredMesh: ";
    vtkDataSet *retval = 0;

    // Get the number of coords
    float *coords[3] = {0,0,0};
    if(GetCoords(timestate, base, zone, zsize, 0, phys_dim, false, coords))
    {
        // Read the number of sections, for the zone.
        int numSections = 0;
        if(cg_nsections(GetFileHandle(), base, zone, &numSections) != CG_OK)
        {
            debug4 << mName << cg_get_error() << endl;
            delete [] coords[0];
            delete [] coords[1];
            delete [] coords[2];
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // Populate the points array.
        unsigned int nPts = zsize[0];
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(nPts);

        const float *xc = coords[0];
        const float *yc = coords[1];
        const float *zc = NULL;

        if (phys_dim == 3)
        {
            zc = coords[2];
        }

        for (unsigned int i = 0; i < nPts; ++i)
        {
            float pt[3];
            pt[0] = *xc++;
            pt[1] = *yc++;
            if (phys_dim == 3)
                pt[2] = *zc++;
            else
                pt[2] = 0.;
            pts->SetPoint(i, pt);
        }

        delete [] coords[0];
        delete [] coords[1];
        delete [] coords[2];

        // Create an unstructured grid to contain the points.
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(pts);
        ugrid->Allocate(zsize[1]);
        pts->Delete();

        //
        // First pass:
        // We need to figure out what kind of data we're reading,
        // each of which requires different approaches to processing.
        //
        // Options are:
        //     1. NGON_n sections. This will contain point connectivity
        //        for faces. When used without NFACE_n, we construct 2D
        //        cells.
        //     2. NFACE_n sections. This will contain face connectivity
        //        for cells. This must be used in conjunction with NGON_n.
        //     3. Named sections. Each named section will explicity name
        //        the cell type and point count as CELLTYPE_COUNT.
        //     4. MIXED. Mixed sections contain multiple named sections.
        //        They should NOT contain NGON_n sections.
        //
        std::vector<int> nGonSections;
        std::vector<int> nFaceSections;
        std::vector<int> mixedAndNamedSections;

        //
        // NOTE: sections can be thought of as "collections" of zones.
        //
        for (int sec = 1; sec < numSections + 1; ++sec)
        {
            char sectionName[33];
            ElementType_t secElemType = ElementTypeNull;
            cgsize_t start            = 1;
            cgsize_t end              = 1;
            int bound                 = 0;
            int parentFlag            = 0;
            int elem_status           = CG_OK;

            if (cg_section_read(GetFileHandle(), base, zone, sec, sectionName,
                    &secElemType, &start, &end, &bound, &parentFlag) != CG_OK)
            {
                debug1 << mName << cg_get_error() << endl;
                continue;
            }

            switch (secElemType)
            {
                case (NGON_n):
                {
                    nGonSections.push_back(sec);
                    break;
                }
                case (NFACE_n):
                {
                    nFaceSections.push_back(sec);
                    break;
                }
                default:
                {
                    mixedAndNamedSections.push_back(sec);
                    break;
                }
            }
        }

        //
        // Mixed and named sections can be handled together.
        //
        if (mixedAndNamedSections.size() > 0)
        {
            ReadMixedAndNamedElementSections(ugrid, meshname,
                mixedAndNamedSections, base, zone, cell_dim, phys_dim);
        }

        //
        // Next, handle NGon and NFace cases.
        //
        bool haveNGonSections  = nGonSections.size() > 0;
        bool haveNFaceSections = nFaceSections.size() > 0;

        if (haveNGonSections && !haveNFaceSections)
        {
            //
            // Handle the 2D polygonal case.
            //
            ReadNGonSections(ugrid, meshname, nGonSections, base, zone,
                cell_dim, phys_dim);
        }
        else if (haveNGonSections && haveNFaceSections)
        {
            //
            // Handle the 3D polyhedra case.
            //
            ReadNGonAndNFaceSections(ugrid, meshname, nGonSections,
                nFaceSections, base, zone,
                cell_dim, phys_dim);
        }
        else if (haveNFaceSections)
        {
            debug1 << mName << "NFace elements MUST be used in conjunction "
                   << " with NGon elements, but only NFace were found." << endl;
            EXCEPTION1(InvalidVariableException, meshname);
        }

        retval = ugrid;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    return retval;
}

// ****************************************************************************
//  Method: avtCGNSFileReader::ReadMixedAndNamedElementSections
//
//  Purpose:
//
//      Read mixed and named element sections, and add the cells from
//      these sections to a given unstructured grid.
//
//      NOTE: the contents of this method were largely migrated from
//      GetUnstructuredMesh, which was originally authored by Brad Whitlock.
//
//  Arguments:
//      ugrid                   The unstructured grid to add our cells to.
//      meshName                The name of our mesh.
//      mixedAndNamedSections   An array of our mixed and named section indices.
//      base                    The CGNS base to use.
//      zone                    The CGNS zone (domain) to use.
//      cellDim                 Dimension of the cells; 3 for volume cells,
//                              2 for surface cells and 1 for line cells.
//      physDim                 Number of coordinates required to define a
//                              vector in the field.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Feb  2 09:20:04 PST 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::ReadMixedAndNamedElementSections(vtkUnstructuredGrid *ugrid,
    const char *meshName, std::vector<int> &mixedAndNamedSections, int base,
    int zone, int cellDim, int physDim)
{
    bool higherOrderWarning = false;
    const char *mName = "avtCGNSFileReader::ReadMixedAndNamedElementSections: ";

    //
    // Iterate over the mixed and named sections, and add each zone to
    // our mesh.
    //
    for(std::vector<int>::iterator secItr = mixedAndNamedSections.begin();
        secItr != mixedAndNamedSections.end(); ++secItr)
    {
        char sectionName[33];
        ElementType_t secElemType    = ElementTypeNull;
        int sec                      = *secItr;
        cgsize_t start               = 1;
        cgsize_t end                 = 1;
        cgsize_t elementSizeInterior = 0;
        int bound                    = 0;
        int parentFlag               = 0;
        int elem_status              = CG_OK;

        if (cg_section_read(GetFileHandle(), base, zone, sec, sectionName,
            &secElemType, &start, &end, &bound, &parentFlag) != CG_OK)
        {
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        if (parentFlag > 0)
        {
            debug4 << mName << "parentFlag = " << parentFlag << endl;
            continue;
        }

        elementSizeInterior = (end - start + 1);

        if (cellDim == physDim)
        {
            elementSizeInterior -= bound;
        }

        cgsize_t eDataSize = 0;

        if (cg_ElementDataSize(GetFileHandle(), base, zone, sec, &eDataSize)
            != CG_OK)
        {
            debug1 << mName << "Could not determine ElementDataSize\n";
            continue;
        }

        debug4 << "Element data size for sec " << sec << " is:"
               << eDataSize << endl;

        //
        // CGNS refers to the element connectivity as "elements". These
        // are actually indices to the nodes (or faces) of connectivity.
        //
        cgsize_t *elements = new cgsize_t[eDataSize];
        if (elements == 0)
        {
            debug4 << mName << "Could not allocate memory for connectivity\n";
            continue;
        }

        if (secElemType == MIXED)
        {
#if CGNS_VERSION >= 4000
            elem_status = cg_poly_elements_read(GetFileHandle(), base,
                zone, sec, elements, NULL, NULL);
#else
            elem_status = cg_elements_read(GetFileHandle(), base, zone,
                sec, elements, NULL);
#endif
        }
        else
        {
            elem_status = cg_elements_read(GetFileHandle(), base,
                zone, sec, elements, NULL);
        }

        if (elem_status != CG_OK)
        {
            delete [] elements;
            elements = 0;
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        debug4 << "section " << sec << ": secElemType=";
        PrintElementType(secElemType);
        debug4 << " start=" << start << " end=" << end << " bound=" << bound
               << " interior elements=" << elementSizeInterior
               << " parentFlag=" << parentFlag << endl;

        //
        // Iterate over the zones and insert them into ugrid.
        //
        vtkIdType verts[27];
        const cgsize_t *elem = elements;

        for (cgsize_t icell = 0; icell < elementSizeInterior; ++icell)
        {
            // If we're reading mixed elements then the element type
            // comes first.
            ElementType_t currentType = secElemType;

            if (currentType == MIXED)
            {
                currentType = (ElementType_t)(*elem++);

                //
                // Mixed types should not contain NGON/NFACE types.
                //
                if (currentType == NGON_n || currentType == NFACE_n)
                {
                    char msg[256];
                    sprintf(msg, "CGNS Mixed types should not contain "
                        "NGON_n or NFACE_n elements.");
                    EXCEPTION1(ImproperUseException, msg);
                }
            }

            // Process the connectivity information for the cell.
            switch (currentType)
            {
                case NODE:
                    verts[0] = elem[0]-1;
                    ugrid->InsertNextCell(VTK_VERTEX, 1, verts);
                    ++elem;
                    break;
                case BAR_2:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    ugrid->InsertNextCell(VTK_LINE, 2, verts);
                    elem += 2;
                    break;
                case BAR_3:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    ugrid->InsertNextCell(VTK_LINE, 2, verts);
                    higherOrderWarning = true;
                    elem += 3;
                    break;
                case TRI_3:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                    elem += 3;
                    break;
                case TRI_6:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    ugrid->InsertNextCell(VTK_LINE, 3, verts);
                    higherOrderWarning = true;
                    elem += 6;
                    break;
                case QUAD_4:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                    elem += 4;
                    break;
                case QUAD_8:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                    higherOrderWarning = true;
                    elem += 8;
                    break;
                case QUAD_9:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                    higherOrderWarning = true;
                    elem += 9;
                    break;
                case TETRA_4:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                    elem += 4;
                    break;
                case TETRA_10:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                    higherOrderWarning = true;
                    elem += 10;
                    break;
                case PYRA_5:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
                    elem += 5;
                    break;
                case PYRA_14:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
                    higherOrderWarning = true;
                    elem += 15;
                    break;
                case PENTA_6:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    verts[5] = elem[5]-1;
                    ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
                    elem += 6;
                    break;
                case PENTA_15:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    verts[5] = elem[5]-1;
                    ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
                    higherOrderWarning = true;
                    elem += 16;
                    break;
                case PENTA_18:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    verts[5] = elem[5]-1;
                    ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
                    higherOrderWarning = true;
                    elem += 18;
                    break;
                case HEXA_8:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    verts[5] = elem[5]-1;
                    verts[6] = elem[6]-1;
                    verts[7] = elem[7]-1;
                    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
                    elem += 8;
                    break;
                case HEXA_20:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    verts[5] = elem[5]-1;
                    verts[6] = elem[6]-1;
                    verts[7] = elem[7]-1;
                    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
                    higherOrderWarning = true;
                    elem += 20;
                    break;
                case HEXA_27:
                    verts[0] = elem[0]-1;
                    verts[1] = elem[1]-1;
                    verts[2] = elem[2]-1;
                    verts[3] = elem[3]-1;
                    verts[4] = elem[4]-1;
                    verts[5] = elem[5]-1;
                    verts[6] = elem[6]-1;
                    verts[7] = elem[7]-1;
                    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
                    higherOrderWarning = true;
                    elem += 27;
                    break;
                case NGON_n:
                    debug1 << mName << "Found NGON_n types while processing "
                           << "named sections... This shouldn't happen!"
                           << endl;
                    break;
                case NFACE_n:
                    debug1 << mName << "Found NFACE_n types while processing "
                           << "named sections... This shouldn't happen!"
                           << endl;
                    break;
                default:
                    delete [] elements;
                    elements = 0;
                    ugrid->Delete();
                    EXCEPTION1(InvalidVariableException, meshName);
                    break;
            }
        }

        debug4 << mName << "Done reading cell connectivity." << endl;
        delete [] elements;

        // Tell the user if we found any higher order elements.
        if (higherOrderWarning)
        {
            avtCallback::IssueWarning("VisIt found quadratic or cubic cells "
                "in the mesh and reduced them to linear cells. Contact "
                "https://visit-help.llnl.gov if you would like VisIt to natively "
                "process higher order elements.");
        }
    }
}

// ****************************************************************************
//  Method: avtCGNSFileReader::ReadNGonSections
//
//  Purpose:
//      Read the NGON_n sections, and add them to a given unstructured
//      grid. The NGON_n sections define point connectivity for faces.
//      This method assumes that NFACE_n is NOT defined for this mesh
//      and will add the elements as 2D surface cells.
//
//  Arguments:
//      ugrid                   The unstructured grid to add our cells to.
//      meshName                The name of our mesh.
//      nGonSections            A vector containing NGon_n section ids.
//      base                    The CGNS base to use.
//      zone                    The CGNS zone (domain) to use.
//      cellDim                 Dimension of the cells; 3 for volume cells,
//                              2 for surface cells and 1 for line cells.
//      physDim                 Number of coordinates required to define a
//                              vector in the field.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Feb  2 09:20:04 PST 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::ReadNGonSections(vtkUnstructuredGrid *ugrid,
    const char *meshName, std::vector<int> &nGonSections, int base,
    int zone, int cellDim, int physDim)
{
    const char *mName = "avtCGNSFileReader::ReadNGonSections: ";

    for (std::vector<int>::iterator nGonSecItr = nGonSections.begin();
         nGonSecItr != nGonSections.end(); ++nGonSecItr)
    {
        char sectionName[33];
        int sec                   = *nGonSecItr;
        ElementType_t secElemType = ElementTypeNull;
        cgsize_t numSectionZones  = 0;
        cgsize_t start            = 1;
        cgsize_t end              = 1;
        int bound                 = 0;
        int parentFlag            = 0;
        int status                = CG_OK;

        if (cg_section_read(GetFileHandle(), base, zone, sec, sectionName,
            &secElemType, &start, &end, &bound, &parentFlag) != CG_OK)
        {
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        numSectionZones = (end - start + 1);

        if (cellDim == physDim)
        {
            numSectionZones -= bound;
        }

        cgsize_t zonePointsSize = 0;
        if (cg_ElementDataSize(GetFileHandle(), base, zone, sec,
            &zonePointsSize) != CG_OK)
        {
            debug1 << mName << "Could not determine ElementDataSize\n";
            continue;
        }

        cgsize_t *zonePoints  = new cgsize_t[zonePointsSize];
        int connOffsetsSize   = numSectionZones + 1;
        cgsize_t *connOffsets = new cgsize_t[connOffsetsSize];

        for (int i = 0; i < zonePointsSize; ++i)
        {
            zonePoints[i] = 0;
        }

        for (int i = 0; i < connOffsetsSize; ++i)
        {
            connOffsets[i] = 0;
        }

#if CGNS_VERSION >= 4000
        status = cg_poly_elements_read(GetFileHandle(), base,
            zone, sec, zonePoints, connOffsets, NULL);
#else
        char msg[256];
        sprintf(msg, "CGNS version >= 4.0.0 is required for reading aribitrary "
            "polygons.")
        EXCEPTION1(ImproperUseException, msg);
#endif

        if (status != CG_OK)
        {
            delete [] zonePoints;
            zonePoints = NULL;
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        vtkIdType verts[27];
        const cgsize_t *zonePointsPtr = zonePoints;

        //
        // Iterate over our polygons, and add them to the mesh.
        //
        for (cgsize_t c = 0; c < numSectionZones; ++c)
        {
            //
            // The connectivity offsets have size numSectionZones + 1
            // so that we can grab the number of points for each zone.
            //
            int numVertices = connOffsets[c+1] - connOffsets[c];

            for (int v = 0; v < numVertices; ++v)
            {
                verts[v] = zonePointsPtr[v]-1;
            }

            ugrid->InsertNextCell(VTK_POLYGON, numVertices, verts);
            zonePointsPtr += numVertices;
        }

        delete [] zonePoints;
        delete [] connOffsets;
    }
}

// ****************************************************************************
//  Method: avtCGNSFileReader::ReadNGonAndNFaceSections
//
//  Purpose:
//      NFace_n sections are used to define arbitrary polyhedra, and they are
//      always used in conjunction with NGon_n. NFace_n "elements" define the
//      zone-to-face connectivity, and NGon_n "elements" define the
//      face-to-point connectivity.
//
//  Arguments:
//      ugrid                   The unstructured grid to add our cells to.
//      meshName                The name of our mesh.
//      nGonSections            A vector containing NGon_n section ids.
//      nFaceSections           A vector containing NFace_n section ids.
//      base                    The CGNS base to use.
//      zone                    The CGNS zone (domain) to use.
//      cellDim                 Dimension of the cells; 3 for volume cells,
//                              2 for surface cells and 1 for line cells.
//      physDim                 Number of coordinates required to define a
//                              vector in the field.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Feb  2 09:20:04 PST 2021
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::ReadNGonAndNFaceSections(vtkUnstructuredGrid *ugrid,
    const char *meshName, std::vector<int> &nGonSections,
    std::vector<int> &nFaceSections, int base,
    int zone, int cellDim, int physDim)
{
    const char *mName = "avtCGNSFileReader::ReadNGonAndNFaceSections: ";

    //
    // These sections should be sorted to begin with, but let's play it safe.
    //
    std::sort(nGonSections.begin(), nGonSections.end());
    std::sort(nFaceSections.begin(), nFaceSections.end());

    //
    // Establish variables needed for our section reads.
    //
    char tempSecName[33];
    int status               = CG_OK;
    int parentFlag           = 0;
    int curSec               = 0;
    int bound                = 0;
    ElementType_t elemType   = ElementTypeNull;

    long totalNumFacePoints  = 0;
    long totalNGonOffsetSize = 0;

    //
    // First pass through our nGonSections: count the total number
    // of face points and total size of our NGon offsets.
    //
    for (std::vector<int>::iterator nGonSecItr = nGonSections.begin();
         nGonSecItr != nGonSections.end(); ++nGonSecItr)
    {
        cgsize_t numSectionFaces = 0;
        cgsize_t faceStart       = 1;
        cgsize_t faceStop        = 1;
        curSec                   = *nGonSecItr;

        if (cg_section_read(GetFileHandle(), base, zone, curSec, tempSecName,
                &elemType, &faceStart, &faceStop, &bound, &parentFlag) != CG_OK)
        {
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        //
        // faceStart and faceStop are inclusive, which means that
        // we need to add 1 to account for the last face.
        //
        numSectionFaces = faceStop - faceStart + 1;
        if (cellDim == physDim)
        {
            numSectionFaces -= bound;
        }

        //
        // elementsSize is the size of the "elements" array, i.e. the size
        // of the connectivity array.
        //
        cgsize_t elementsSize = 0;
        if (cg_ElementDataSize(GetFileHandle(), base, zone,
                curSec, &elementsSize) != CG_OK)
        {
            debug1 << mName << "Could not determine ElementDataSize\n";
            continue;
        }

        //
        // NOTE: The offsets have an extra index on the end to allow us to
        // count the total number of points in the last face.
        //
        totalNGonOffsetSize += numSectionFaces + 1;

        totalNumFacePoints += elementsSize;
    }

    //
    // NFace sections don't really correspond to NGon sections, but NFace
    // elements will always map to the global NGon offsets. So, we need to pool
    // all of our NGon data (face points and offsets) into single containers
    // that we can reach into later.
    // totalFacePoints contains all mappings of faces to global node ids,
    // and totalNGonOffsets contains all offset information for reading this
    // face connectivity.
    //
    cgsize_t *totalFacePoints  = new cgsize_t[totalNumFacePoints];
    cgsize_t *totalNGonOffsets = new cgsize_t[totalNGonOffsetSize];

    for (int i = 0; i < totalNumFacePoints; ++i)
    {
        totalFacePoints[i] = 0;
    }

    for (int i = 0; i < totalNGonOffsetSize; ++i)
    {
        totalNGonOffsets[i] = 0;
    }

    //
    // Tricky business:
    // When reading the NGon offsets, CGNS will always return 1 more offset
    // at the end of the array to allow us to easily know the total number
    // of nodes in the last face of the current section. Unfortunately, that
    // last offset DOES NOT correspond to the start offset of the next
    // section... This means that we need to keep track of which section we're
    // on and offset appropriately to skip these "padded" values later on.
    //
    std::vector<cgsize_t> nGonOffsetIdxToSectionIdx;
    nGonOffsetIdxToSectionIdx.reserve(totalNGonOffsetSize);

    int nGonOffsetsIdx     = 0;
    int secIdx             = 0;
    int facePointIdx       = 0;
    cgsize_t sectionOffset = 0;

    //
    // Second pass through our nGonSections: read all NGon data into
    // our section-wide containers. NOTE: reading data from sections in
    // (incremental) order SHOULD give back data that is in the correct
    // global order (by sections).
    //
    for (std::vector<int>::iterator nGonSecItr = nGonSections.begin();
         nGonSecItr != nGonSections.end(); ++nGonSecItr)
    {
        curSec                   = *nGonSecItr;
        cgsize_t numSectionFaces = 0;
        cgsize_t faceStart       = 1;
        cgsize_t faceStop        = 1;

        if (cg_section_read(GetFileHandle(), base, zone, curSec, tempSecName,
                &elemType, &faceStart, &faceStop, &bound, &parentFlag) != CG_OK)
        {
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        numSectionFaces = faceStop - faceStart + 1;
        if (cellDim == physDim)
        {
            numSectionFaces -= bound;
        }

        int connOffsetsSize   = numSectionFaces + 1;
        cgsize_t *faceOffsets = new cgsize_t[connOffsetsSize];

        for (int i = 0; i < connOffsetsSize; ++i)
        {
            faceOffsets[i] = 0;
        }

        cgsize_t *sectionFacePtr = totalFacePoints + sectionOffset;

#if CGNS_VERSION >= 4000
        status = cg_poly_elements_read(GetFileHandle(), base,
            zone, curSec, sectionFacePtr, faceOffsets, NULL);
#else
        char msg[256];
        sprintf(msg, "CGNS version >= 4.0.0 is required for reading aribitrary "
            "polyhedra.")
        EXCEPTION1(ImproperUseException, msg);
#endif

        if (status != CG_OK)
        {
            delete [] faceOffsets;
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        //
        // We need to be careful when copying over our offsets since we're
        // now using section-wide containers, but this section read will
        // give us offsets for this section only.
        //
        for (int i = 0; i < connOffsetsSize; ++i)
        {
            //
            // The new offset will start at the section offset.
            //
            totalNGonOffsets[nGonOffsetsIdx++] = faceOffsets[i] + sectionOffset;

            //
            // Update our map. Remember that the last entry in the connectivity
            // array is not really part of the global offset indexing scheme in
            // CGNS. So, we want to pretend they don't exist (kind of).
            //
            if (i < connOffsetsSize - 1)
            {
                nGonOffsetIdxToSectionIdx.push_back(secIdx);
            }
        }

        //
        // Lastly, update our section offset.
        //
        cgsize_t sectionElementsSize = 0;
        if (cg_ElementDataSize(GetFileHandle(), base, zone,
                curSec, &sectionElementsSize) != CG_OK)
        {
            debug1 << mName << "Could not determine ElementDataSize\n";
            continue;
        }

        secIdx++;
        sectionOffset += sectionElementsSize;

        delete [] faceOffsets;
    }

    //
    // Single pass over nFaceSections: Now that we have our global arrays for
    // face-to-point connectivity, we can read in the zone-to-face connectivity
    // and construct our zones.
    //
    for (std::vector<int>::iterator nFaceSecItr = nFaceSections.begin();
         nFaceSecItr != nFaceSections.end(); ++nFaceSecItr)
    {
        curSec                   = *nFaceSecItr;
        cgsize_t numSectionZones = 0;
        cgsize_t offsetStart     = 1;
        cgsize_t offsetStop      = 1;

        if (cg_section_read(GetFileHandle(), base, zone, curSec, tempSecName,
               &elemType, &offsetStart, &offsetStop, &bound, &parentFlag)
               != CG_OK)
        {
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        numSectionZones = offsetStop - offsetStart + 1;
        if (cellDim == physDim)
        {
            numSectionZones -= bound;
        }

        cgsize_t faceElementsSize = 0;
        if (cg_ElementDataSize(GetFileHandle(), base, zone,
                curSec, &faceElementsSize) != CG_OK)
        {
            debug1 << mName << "Could not determine ElementDataSize\n";
            continue;
        }

        //
        // faceElements will contain a mapping from zones to faces. The
        // faces are actually 0 based indices into the NGon face-to-point
        // connectivity array. NOTE: this mapping is the GLOBAL mapping and
        // does not take sections into consideration.
        //
        cgsize_t *faceElements = new cgsize_t[faceElementsSize];
        int zoneOffsetsSize    = numSectionZones + 1;
        cgsize_t *zoneOffsets  = new cgsize_t[zoneOffsetsSize];

        for (int i = 0; i < faceElementsSize; ++i)
        {
            faceElements[i] = 0;
        }

        for (int i = 0; i < zoneOffsetsSize; ++i)
        {
            zoneOffsets[i] = 0;
        }

#if CGNS_VERSION >= 4000
        status = cg_poly_elements_read(GetFileHandle(), base,
            zone, curSec, faceElements, zoneOffsets, NULL);
#else
        char msg[256];
        sprintf(msg, "CGNS version >= 4.0.0 is required for reading aribitrary "
            "polyhedra.")
        EXCEPTION1(ImproperUseException, msg);
#endif

        if (status != CG_OK)
        {
            delete [] faceElements;
            delete [] zoneOffsets;
            debug1 << mName << cg_get_error() << endl;
            continue;
        }

        //
        // Finally, we can read in the zones!
        // NOTE: for all sections, the CGNS offset arrays are 0 based, and
        // the elements arrays are 1 based (Fortran).
        //
        for (cgsize_t c = 0; c < numSectionZones; ++c)
        {
            cgsize_t zoneStart = zoneOffsets[c];
            cgsize_t zoneStop  = zoneOffsets[c + 1];
            cgsize_t numFaces  = zoneStop - zoneStart;

            //
            // VTK defines the face stream as follows:
            // numFacePoints, pt1, pt2, ..., numFacePoints, pt1, pt2,...
            // With an entry for each face.
            //
            vtkIdType faceStream[1024];
            int faceStreamIdx = 0;

            for (cgsize_t zIdx = zoneStart; zIdx < zoneStop; ++zIdx)
            {
                //
                // This index will map back to our NGon section so that we
                // can read the connectivity for each face of the zone.
                // NOTE: CGNS will flip the sign of a face index to denote
                // normal directions.
                //
                cgsize_t faceOffsetIdx = abs(faceElements[zIdx]) - 1;

                //
                // Here's where we come back to the tricky business from
                // earlier. Our global offset array has padding after each
                // section to count the points in the last face of each
                // section. So, we need to account for this and ignore the
                // padding when reading sections > 0.
                //
                int curSecIdx = nGonOffsetIdxToSectionIdx[faceOffsetIdx];

                cgsize_t facePtStartIdx =
                    totalNGonOffsets[faceOffsetIdx + curSecIdx];
                cgsize_t facePtStopIdx  =
                    totalNGonOffsets[faceOffsetIdx + curSecIdx + 1];

                int numFacePts = facePtStopIdx - facePtStartIdx;

                faceStream[faceStreamIdx++] = numFacePts;

                for (int ptIdx = facePtStartIdx; ptIdx < facePtStopIdx;
                     ++ptIdx)
                {
                    faceStream[faceStreamIdx++] = totalFacePoints[ptIdx] - 1;
                }
            }

            ugrid->InsertNextCell(VTK_POLYHEDRON, numFaces, faceStream);
        }


        delete [] faceElements;
        delete [] zoneOffsets;
    }

    delete [] totalFacePoints;
    delete [] totalNGonOffsets;
}

// ****************************************************************************
//  Method: avtCGNSFileReader::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Brad Whitlock, Mon Dec 11 09:33:15 PDT 2006
//    Changed interpretation of zsize array for 1D, 2D cases so we allocate
//    and read the right number of values.
//
//    Maxim Loginov, Tue Mar  4 12:28:12 NOVT 2008
//    Read proper solution in accordance with ZoneIterativeData_t node
//
//    Brad Whitlock, Wed Apr 16 11:51:12 PDT 2008
//    Adjust how the base is selected since we can read data from multiple
//    bases now.
//
//    Jeremy Meredith, Thu Aug  7 15:56:52 EDT 2008
//    Added a default case for a switch.
//
//    Brad Whitlock, Thu Oct 13 11:13:30 PDT 2011
//    Call InitializeMaps so we can group files.
//
// ****************************************************************************

vtkDataArray *
avtCGNSFileReader::GetVar(int timestate, int domain, const char *varname)
{
    const char *mName = "avtCGNSFileReader::GetVar: ";
    debug4 << mName << "ts=" << timestate << ", dom=" << domain
           << ", var=" << varname << endl;

    InitializeMaps(timestate);

    // Look up the base that contains the variable.
    int base = 1;
    std::string sVarName(varname);
    std::string::size_type slashIndex = sVarName.find("/");
    if(slashIndex != std::string::npos)
    {
        std::string baseName = sVarName.substr(0,slashIndex);
        sVarName = sVarName.substr(slashIndex+1, sVarName.size() - slashIndex);
        baseName = VisItNameToCGNSName[baseName];
        if(BaseNameToIndices.find(baseName) == BaseNameToIndices.end())
        {
            debug4 << mName << "Basename " << baseName.c_str()
                   << " not found in the BaseNameToIndices map" << endl;
        }
        else
            base = BaseNameToIndices[baseName];
        debug4 << mName << "Using base " << base << " for the variable "
               << sVarName.c_str() << endl;
    }
    // Look up the real variable name in case it's been made safe for VisIt.
    if(VisItNameToCGNSName.find(sVarName) != VisItNameToCGNSName.end())
        sVarName = VisItNameToCGNSName[sVarName];

    vtkDataArray *retval = 0;
    int zone = domain + 1;
    char zonename[33];
    cgsize_t zsize[9];

    memset(zonename, 0, 33);
    memset(zsize, 0, 9 * sizeof(int));

    //
    // Determine the topological and spatial dimensions.
    //
    char namebase[33];
    int cell_dim = 2, phys_dim = 2;
    if(cg_base_read(GetFileHandle(), base, namebase, &cell_dim, &phys_dim) != CG_OK)
    {
        debug4 << cg_get_error() << endl;
        EXCEPTION1(InvalidFilesException, cgnsFileName);
    }
    else
    {
        debug4 << mName << " name=" << namebase << " cell_dim=" << cell_dim
               << " phys_dim=" << phys_dim << endl;
    }

    //
    // Read the zone information
    //
    if(cg_zone_read(GetFileHandle(), base, zone, zonename, zsize) != CG_OK)
    {
        debug4 << mName << cg_get_error() << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }
    else
    {
        // Print the zone info.
        debug4 << mName << " name=" << zonename << " size=[";
        for(int zi = 0; zi < 9; ++zi)
        {
            if(zi > 0) { debug4 << ", "; }
            debug4 << zsize[zi];
        }
        debug4 << "]\n";

        // Get the zone type because we need to know how many data points to
        // allocate using the zsize array and it is used differently for
        // structured vs. unstructured.
        ZoneType_t zt = ZoneTypeNull;
        if(cg_zone_type(GetFileHandle(), base, zone, &zt) != CG_OK)
        {
            debug4 << mName << cg_get_error() << endl;
            EXCEPTION1(InvalidVariableException, varname);
        }
        else
        {
            switch(zt)
            {
            case ZoneTypeNull:
                EXCEPTION1(InvalidVariableException,
                           "ZoneTypeNull is not supported.");
                break;
            case ZoneTypeUserDefined:
                EXCEPTION1(InvalidVariableException,
                           "ZoneTypeUserDefined is not supported.");
                break;
            default:
                // fall out
                break;
            }
        }

        int nsols = 0;
        if(cg_nsols(GetFileHandle(), base, zone, &nsols) != CG_OK)
        {
            debug4 << mName << "Could not get number of solutions in zone "
                   << zone << endl;
            debug4 << cg_get_error() << endl;
            EXCEPTION1(InvalidVariableException, varname);
        }

        debug5 << mName << "found " << nsols << " solutions in zone "
               << zone << endl;

        // find the requred solution from ZoneIterativeData_t
        // TODO currently we rely on the alphabetical sorting of the
        // solutions names in the zone and in FlowSolutionPointers,
        // e.g. the number of the required solution in the
        // FlowSolutionPointers array is the same as node number of
        // the solution, which is not necessarily true
        int requiredsol = (timestate < nsols) ? timestate + 1: nsols;

        // Iterate through the solutions until we find the variable that we're
        // looking for or required solution by number.
        bool fieldNotFound = true;
        for(int sol = 1; sol <= nsols && fieldNotFound; ++sol)
        {
            if(sol != requiredsol)
                continue;

            char solname[33];
            GridLocation_t varcentering;
            if(cg_sol_info(GetFileHandle(), base, zone, sol, solname,
                &varcentering) != CG_OK)
            {
                debug4 << "Could not get solution " << sol << "'s info."
                       << endl;
                debug4 << cg_get_error() << endl;
                continue;
            }

            debug5 << mName << "solution " << sol << " in zone "
                   << zone << " taken." << endl;

            if(varcentering == Vertex || varcentering == CellCenter)
            {
                int nfields = 0;
                if(cg_nfields(GetFileHandle(), base, zone, sol, &nfields) != CG_OK)
                {
                    debug4 << "Could not get number of fields for solution "
                           << sol << endl;
                    debug4 << cg_get_error() << endl;
                    continue;
                }

                for(int f = 1; f <= nfields && fieldNotFound; ++f)
                {
                    DataType_t dt;
                    char fieldname[33];

                    if(cg_field_info(GetFileHandle(), base, zone, sol, f, &dt, fieldname) != CG_OK)
                    {
                        debug4 << mName << "Could not get number of fields for "
                               << "solution " << sol << endl;
                        debug4 << cg_get_error() << endl;
                        continue;
                    }

                    //
                    // We found a matching field. Read it into a VTK data array.
                    if(sVarName == fieldname)
                    {
                        vtkDataArray *arr = 0;
                        switch(dt)
                        {
                        case DataTypeNull:
                        case DataTypeUserDefined:
                            debug4 << "Unsupported variable type: ";
                            PrintDataType(dt);
                            debug4 << endl;
                            break;
                        case Integer:
                            arr = vtkIntArray::New();
                            break;
                        case LongInteger:
                            arr = vtkLongArray::New();
                            break;
                        case RealSingle:
                            arr = vtkFloatArray::New();
                            break;
                        case RealDouble:
                            arr = vtkDoubleArray::New();
                            break;
                        case Character:
                            arr = vtkCharArray::New();
                            break;
                        }

                        if(arr != 0)
                        {
                            //
                            // Set up the number of tuples, etc. This code works
                            // for structured meshes but may need some alterations
                            // for unstructured meshes.
                            //
                            int nvals = 0;
                            cgsize_t rmin[3] = {1,1,1};
                            cgsize_t rmax[3] = {1,1,1};
                            if(zt == Structured)
                            {
                                if(varcentering == Vertex)
                                {
                                    if(cell_dim == 1)
                                    {
                                        rmax[0] = zsize[0];
                                    }
                                    else if(cell_dim == 2)
                                    {
                                        rmax[0] = zsize[0];
                                        rmax[1] = zsize[1];
                                    }
                                    else
                                    {
                                        rmax[0] = zsize[0];
                                        rmax[1] = zsize[1];
                                        rmax[2] = zsize[2];
                                    }
                                }
                                else
                                {
                                    if(cell_dim == 1)
                                    {
                                        rmax[0] = zsize[1];
                                    }
                                    else if(cell_dim == 2)
                                    {
                                        rmax[0] = zsize[2];
                                        rmax[1] = zsize[3];
                                    }
                                    else
                                    {
                                        rmax[0] = zsize[3];
                                        rmax[1] = zsize[4];
                                        rmax[2] = zsize[5];
                                    }
                                }
                                nvals = rmax[0] * rmax[1] * rmax[2];
                            }
                            else // Unstructured
                            {
                                if(varcentering == Vertex)
                                    nvals = zsize[0];
                                else
                                    nvals = zsize[1];
                                rmax[0] = nvals;
                            }
                            arr->SetNumberOfTuples(nvals);

                            if(cg_field_read(GetFileHandle(), base, zone, sol,
                               fieldname, dt, rmin, rmax,
                               (void*)arr->GetVoidPointer(0)) != CG_OK)
                            {
                                arr->Delete();
                                arr = 0;
                                debug4 << mName << "Could not read " << fieldname
                                       << ": " << cg_get_error() << endl;
                            }
                        }

                        retval = arr;
                        fieldNotFound = false;
                    }
                }
            }
        }

        // If we failed to read the variable, throw an exception.
        if(retval == 0)
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
    }

    return retval;
}


// ****************************************************************************
//  Method: avtCGNSFileReader::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
// ****************************************************************************

vtkDataArray *
avtCGNSFileReader::GetVectorVar(int timestate, int domain, const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}

// ****************************************************************************
// Method: avtCGNSFileReader::PrintVarInfo
//
// Purpose:
//   Prints variable information to a stream.
//
// Arguments:
//   out    : The stream to which we'll print.
//   var    : The var to print.
//   indent : The indentation level.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:23:39 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::PrintVarInfo(ostream &out, const avtCGNSFileReader::VarInfo &var, const char *indent)
{
    out << indent << "zoneList = {";
    for(size_t i = 0; i < var.zoneList.size(); ++i)
    {
        out << var.zoneList[i];
        if(i < var.zoneList.size()-1)
            out << ", ";
    }
    out << "}" << endl;
    out << indent << "cellCentering = " << var.cellCentering << endl;
    out << indent << "nodeCentering = " << var.nodeCentering << endl;
    out << indent << "badCentering = " << var.badCentering << endl;
    out << indent << "hasUnits = " << (var.hasUnits?"true":"false") << endl;
    out << indent << "units = \"" << var.units.c_str() << "\"" << endl;
}

// ****************************************************************************
// Method: avtCGNSFileReader::PrintStringVarInfoMap
//
// Purpose:
//   Prints all variable information to a stream.
//
// Arguments:
//   out    : The stream to which we'll print.
//   vars   : The vars to print.
//   indent : The indentation level.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:23:39 PDT 2008
//
// Modifications:
//
// ****************************************************************************


void
avtCGNSFileReader::PrintStringVarInfoMap(ostream &out, const avtCGNSFileReader::StringVarInfoMap &vars, const char *indent)
{
    std::string indent2(std::string(indent) + std::string(indent));
    for(StringVarInfoMap::const_iterator pos = vars.begin();
        pos != vars.end(); ++pos)
    {
        out << indent << pos->first.c_str() << " = {" << endl;
        PrintVarInfo(out, pos->second, indent2.c_str());
        out << indent << "}" << endl;
    }
}

// ****************************************************************************
// Method: avtCGNSFileReader::PrintBaseInformation
//
// Purpose:
//   Prints base information to a stream.
//
// Arguments:
//   out      : The stream to which we'll print.
//   baseInfo : The base information that we'll print.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 10:23:39 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtCGNSFileReader::PrintBaseInformation(ostream &out, const avtCGNSFileReader::BaseInformation &baseInfo)
{
    out << "name = " << baseInfo.name.c_str() << endl;
    out << "cellDim = " << baseInfo.cellDim << endl;
    out << "physicalDim = " << baseInfo.physicalDim << endl;
    out << "meshType = " << baseInfo.meshType << " 0=curv, 1=ucd, -1,-2=unsupported" << endl;
    out << "zoneNames = {";
    for(size_t i = 0; i < baseInfo.zoneNames.size(); ++i)
    {
        out << baseInfo.zoneNames[i];
        if(i < baseInfo.zoneNames.size()-1)
            out << ", ";
    }
    out << "}" << endl;
    out << "vars = {" << endl;
    PrintStringVarInfoMap(out, baseInfo.vars, "    ");
    out << "}" << endl;
}
