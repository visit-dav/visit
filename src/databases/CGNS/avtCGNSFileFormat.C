/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtCGNSFileFormat.C                            //
// ************************************************************************* //

#include <avtCGNSFileFormat.h>
#include <cgnslib.h>

#include <algorithm>
#include <string>

#include <vtkCellTypes.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <Expression.h>
#include <DebugStream.h>
#include <snprintf.h>

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
        for(int ci = 0; ci < var.size(); ++ci)
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
//  Method: avtCGNSFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Brad Whitlock, Tue Apr 15 16:00:52 PDT 2008
//    Added cgnsFileName, BaseNameToIndices, VisItNameToCGNSName.
//
// ****************************************************************************

avtCGNSFileFormat::avtCGNSFileFormat(const char *filename)
    : avtMTMDFileFormat(filename), times(), MeshDomainMapping(), BaseNameToIndices(),
    VisItNameToCGNSName()
{
    cgnsFileName = new char[strlen(filename) + 1];
    strcpy(cgnsFileName, filename);

    debug1 << "avtCGNSFileFormat::avtCGNSFileFormat: filename=" << cgnsFileName << endl;
    fn = INVALID_FILE_HANDLE;
    timesRead = false;
    cgnsCyclesAccurate = false;
    cgnsTimesAccurate = false;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::~avtCGNSFileFormat
//
// Purpose: 
//   Destructor for the avtCGNSFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 30 16:12:20 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtCGNSFileFormat::~avtCGNSFileFormat()
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
avtCGNSFileFormat::GetNTimesteps(void)
{
    ReadTimes();

    return times.size();
}

// ****************************************************************************
//  Method: avtCGNSFileFormat::FreeUpResources
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
avtCGNSFileFormat::FreeUpResources(void)
{
    if(fn != INVALID_FILE_HANDLE)
    {
        cg_close(fn);
        fn = INVALID_FILE_HANDLE;
    }
}

// ****************************************************************************
// Method: avtCGNSFileFormat::GetFileHandle
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
avtCGNSFileFormat::GetFileHandle()
{
    if(fn == INVALID_FILE_HANDLE)
    {
        if(cg_open(cgnsFileName, MODE_READ, &fn) != CG_OK)
        {
            debug4 << cg_get_error() << endl;
            EXCEPTION1(InvalidFilesException, cgnsFileName);
        }
    }

    return fn;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::ReadTimes
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
// ****************************************************************************

void
avtCGNSFileFormat::ReadTimes()
{
    const char *mName = "avtCGNSFileFormat::ReadTimes: ";

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
                        int dims[10];
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
                    debug4 << mName << "Could not read narrays under BaseIterative node: "
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
}

// ****************************************************************************
// Method: avtCGNSFileFormat::GetTimes
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
avtCGNSFileFormat::GetTimes(std::vector<double> &t)
{
    ReadTimes();
    t = times;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::GetCycles
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
avtCGNSFileFormat::GetCycles(std::vector<int> &c)
{
    // ncycles is equal to ntimes (CGNS MLL check this)
    ReadTimes();
    c = cycles;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::BaseContainsUnits
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
avtCGNSFileFormat::BaseContainsUnits(int base)
{
    const char *mName = "avtCGNSFileFormat::BaseContainsUnits: ";
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
// Method: avtCGNSFileFormat::GetVariablesForBase
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
avtCGNSFileFormat::GetVariablesForBase(int base, avtCGNSFileFormat::BaseInformation &baseInfo)
{
    const char *mName = "avtCGNSFileFormat::GetVariablesForBase: ";

    bool retval = true;
    char namebase[33];
    int cell_dim = 2, phys_dim = 2;
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
            int zsize[9];
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
                    if(zi > 0) debug4 << ", ";
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
                    else
                    {
                        // We've already run across the variable in another zone
                        // so let's update what we know.
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
// Method: avtCGNSFileFormat::AddVectorExpression
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
avtCGNSFileFormat::AddVectorExpression(avtDatabaseMetaData *md, bool *haveComponent,
    int nBases, const std::string &baseName, const std::string &vecName)
{
    char def[300];

    if(haveComponent[0] && haveComponent[1] && haveComponent[2])
    {
        Expression *e = new Expression;
        if(nBases > 1)
        {
            e->SetName(baseName + "/" + vecName);
            SNPRINTF(def, 300, "{<%s/%sX>,<%s/%sY>,<%s/%sZ>}",
                baseName.c_str(), vecName.c_str(),
                baseName.c_str(), vecName.c_str(),
                baseName.c_str(), vecName.c_str());
            e->SetDefinition(def);
        }
        else
        {
            e->SetName(vecName);
            SNPRINTF(def, 300, "{%sX,%sY,%sZ}",
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
            SNPRINTF(def, 300, "{<%s/%sX>,<%s/%sY>}",
                baseName.c_str(), vecName.c_str(),
                baseName.c_str(), vecName.c_str());
            e->SetDefinition(def);
        }
        else
        {
            e->SetName(vecName);
            SNPRINTF(def, 300, "{%sX,%sY}",
                vecName.c_str(), vecName.c_str());
            e->SetName(vecName);
            e->SetDefinition(def);
        }
        e->SetType(Expression::VectorMeshVar);
        md->AddExpression(e);
    }
}

// ****************************************************************************
// Method: avtCGNSFileFormat::AddVectorExpressions
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
avtCGNSFileFormat::AddVectorExpressions(avtDatabaseMetaData *md, bool *haveVelocity,
    bool *haveMomentum, int nBases, const std::string &baseName)
{
    AddVectorExpression(md, haveVelocity, nBases, baseName, "Velocity");
    AddVectorExpression(md, haveMomentum, nBases, baseName, "Momentum");
}

// ****************************************************************************
// Method: avtCGNSFileFormat::AddReferenceStateExpressions
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
avtCGNSFileFormat::AddReferenceStateExpressions(avtDatabaseMetaData *md,
    int base, int nBases, const std::string &baseName, const std::string &meshName)
{
    const char *mName = "avtCGNSFileFormat::AddReferenceStateExpressions: ";

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
                int dims[10];
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
                        SNPRINTF(edef, 100, "point_constant(%s, %lg)", meshName.c_str(), dval);
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
                        SNPRINTF(edef, 100, "point_constant(%s, %lg)", meshName.c_str(), dval);
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
//  Method: avtCGNSFileFormat::PopulateDatabaseMetaData
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
//   Maxim Loginov, Tue Mar  4 12:28:12 NOVT 2008
//   Some constants from ReferenceState_t should be available as array mesh 
//   variables.
//
//   Brad Whitlock, Wed Apr 16 10:07:16 PDT 2008
//   Totally rewrote to support reading data from multiple bases. It's more
//   modular too.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Mark C. Miller, Mon Sep 21 14:17:47 PDT 2009
//   Adding missing calls to actually set the times/cycles in the metadata.
// ****************************************************************************

void
avtCGNSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    const char *mName = "avtCGNSFileFormat::PopulateDatabaseMetaData: ";

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
            free(refstate);
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
    for(int bi = 0; bi < baseInfo.size(); ++bi)
    {
        if(DebugStream::Level4())
            PrintBaseInformation(DebugStream::Stream4(), baseInfo[bi]);
    }

    bool someInvalidCenterings = false;

    // Now that we have variables for each base, let's determine the
    // meshes that we need to create for each base. Let's use the base name
    // as the mesh name if there is more than one zone in a base. If there's
    debug4 << "=================== POPULATE VARIABLES ===================" << endl;
    for(int bi = 0; bi < baseInfo.size(); ++bi)
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
        for(int i = 0; i < baseInfo[bi].zoneNames.size(); ++i)
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
                SNPRINTF(tmp, 100, "subgrid/%s%03d", 
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

            stringVector domainNames;
            for(int di = 0; di < it->first.size(); ++di)
            {
                int idx = it->first[di] - 1;
                domainNames.push_back(baseInfo[bi].zoneNames[idx]);
            }
            mmd->blockNames = domainNames;
            mmd->numBlocks = domainNames.size();
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
            bzl.base = bi+1;
            bzl.zones = it->first;
            MeshDomainMapping[it->second] = bzl;

            // Print the entry we just created in MeshDomainMapping 
            debug4 << mName << "Step 3: Creating mesh " << it->second.c_str() << " for base "
                   << bzl.base << " for zones [";
            for(int zi = 0; zi < it->first.size(); ++zi)
            {
                debug4 << it->first[zi];
                if(zi <  it->first.size()-1)
                    debug4 << ", ";
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
        AddVectorExpressions(md, haveVelocity, haveMomentum, baseInfo.size(), 
            baseName);

        //
        // STEP 6: Create expressions for reference state variables.
        //
        AddReferenceStateExpressions(md, bi+1, baseInfo.size(), 
            baseName, meshName);
    }

    // If some variables had unsupported centerings then issue a warning.
    if(someInvalidCenterings)
    {
        avtCallback::IssueWarning("Some variables have been disabled because "
            "their grid locations (variable centerings) are not supported by "
            "VisIt. VisIt currently supports node and cell centered variables.");
    }
}

// ****************************************************************************
//  Method: avtCGNSFileFormat::GetMesh
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
// ****************************************************************************

vtkDataSet *
avtCGNSFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    const char *mName = "avtCGNSFileFormat::GetMesh: ";
    debug4 << mName << "ts=" << timestate << ", dom=" << domain
           << ", mesh=" << meshname << endl;

    //
    // See if this domain is turned off by default for this mesh.
    //
    std::map<std::string, BaseAndZoneList>::const_iterator pos = 
        MeshDomainMapping.find(meshname);
    if(pos == MeshDomainMapping.end())
        return 0;
    int base = pos->second.base;
    int zone = domain + 1;
    const intVector &zones = pos->second.zones;
    debug4 << mName << "Checking if zone " << zone << " is part of "
           << meshname << endl;
    debug4 << "zones = {";
    for(int i = 0; i < zones.size(); ++i)
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
    int zsize[9];
    memset(zonename, 0, 33);
    memset(zsize, 0, 9 * sizeof(int));

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
            if(zi > 0) debug4 << ", ";
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
                retval = GetCurvilinearMesh(base, zone, meshname, zsize);
                break;
            case Unstructured:
                retval = GetUnstructuredMesh(base, zone, meshname, zsize);
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::GetCoords
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
//   ncoords    : The number of pointers in the coords array.
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
// ****************************************************************************

bool
avtCGNSFileFormat::GetCoords(int base, int zone, const int *zsize,
    bool structured, float **coords, int *ncoords)
{
    const char *mName = "avtCGNSFileFormat::GetCoords: ";
    bool err = false;

    // Init the coord array
    coords[0] = 0;
    coords[1] = 0;
    coords[2] = 0;

    // Iterate through the coordinates and read them in.
    if(cg_ncoords(GetFileHandle(), base, zone, ncoords) != CG_OK)
    {
        debug4 << mName << "\t\tCould not get the number of coords" << endl;
        debug4 << mName << cg_get_error() << endl;
    }
    else
    {
        debug4 << mName << "ncoords = " << *ncoords << endl;
        if(*ncoords > 3)
            *ncoords = 3;
        err = *ncoords != 2 && *ncoords != 3;
        
        unsigned int nPts = 0;
        int rmin[3] = {1,1,1};
        int rmax[3] = {1,1,1};
        if(structured)
        {
            if(*ncoords == 1)
            {
                rmax[0] = zsize[0];
            }
            else if(*ncoords == 2)
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

        for(int c = 1; c <= *ncoords; ++c)
        {
            char coordname[33];
            DataType_t ct;
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
                if(cg_coord_read(GetFileHandle(), base, zone, coordname,
                   RealSingle, rmin, rmax, (void*)coords[c-1]) != CG_OK)
                {
                    debug4 << mName << cg_get_error() << endl;
                    err = true;
                    break;
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
            *ncoords = 0;
        }
    }

    return !err;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::GetCurvilinearMesh
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
// ****************************************************************************

vtkDataSet *
avtCGNSFileFormat::GetCurvilinearMesh(int base, int zone, const char *meshname,
    const int *zsize)
{
    const char *mName = "avtCGNSFileFormat::GetCurvilinearMesh: ";
    vtkDataSet *retval = 0;

    // Get the number of coords
    int ncoords = 0;
    float *coords[3] = {0,0,0};
    if(GetCoords(base, zone, zsize, true, coords, &ncoords))
    {
        // Create the curvilinear mesh.
        vtkStructuredGrid *sgrid   = vtkStructuredGrid::New(); 
        vtkPoints         *points  = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();

        // Populate the points array
        int dims[3];
        dims[0] = zsize[0];
        dims[1] = (ncoords >= 2) ? zsize[1] : 1;
        dims[2] = (ncoords == 3) ? zsize[2] : 1;
        sgrid->SetDimensions(dims);
        points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);
        float *pts = (float *) points->GetVoidPointer(0);
        float *xc = coords[0];
        float *yc = coords[1];
        if(ncoords == 3)
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
        else if(ncoords == 2)
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
        else if(ncoords == 1)
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

    return retval;
}

// ****************************************************************************
// Method: avtCGNSFileFormat::GetUnstructuredMesh
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
// ****************************************************************************

vtkDataSet *
avtCGNSFileFormat::GetUnstructuredMesh(int base, int zone, const char *meshname,
    const int *zsize)
{
    const char *mName = "avtCGNSFileFormat::GetUnstructuredMesh: ";
    vtkDataSet *retval = 0;

    // Get the number of coords
    int ncoords = 0;
    float *coords[3] = {0,0,0};
    if(GetCoords(base, zone, zsize, false, coords, &ncoords))
    {
        // Read the number of sections, for the zone.
        int nsections = 0;
        if(cg_nsections(GetFileHandle(), base, zone, &nsections) != CG_OK)
        {
            debug4 << mName << cg_get_error() << endl;
            delete [] coords[0];
            delete [] coords[1];
            delete [] coords[2];
            EXCEPTION1(InvalidVariableException, meshname);
        }
        else
        {
            // Populate the points array.
            unsigned int nPts = zsize[0];
            vtkPoints *pts = vtkPoints::New();
            pts->SetNumberOfPoints(nPts);
            const float *xc = coords[0];
            const float *yc = coords[1];
            const float *zc = NULL; 
            if (ncoords == 3)
            {
                zc = coords[2];
            }
            for(unsigned int i = 0; i < nPts; ++i)
            {
                float pt[3];
                pt[0] = *xc++;
                pt[1] = *yc++;
                if (ncoords == 3)
                    pt[2] = *zc++;
                else 
                    pt[2] = 0.;
                pts->SetPoint(i, pt);
            }

            // Create an unstructured grid to contain the points.
            vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
            ugrid->SetPoints(pts);
            ugrid->Allocate(zsize[1]);
            pts->Delete();
            bool higherOrderWarning = false;

            // Iterate over each of the sections.
            for(int sec = 1; sec <= nsections; ++sec)
            {
                char sectionname[33];
                ElementType_t et = ElementTypeNull;
                int start = 1, end = 1, bound = 0, parent_flag = 0;
                if(cg_section_read(GetFileHandle(), base, zone, sec, sectionname, &et,
                    &start, &end, &bound, &parent_flag) != CG_OK)
                {
                    debug4 << mName << cg_get_error() << endl;
                    continue;
                }

                if(parent_flag > 0)
                {
                    debug4 << mName << "parent_flag = " << parent_flag << endl;
                    continue;
                }

                int eDataSize = 0;
                if(cg_ElementDataSize(GetFileHandle(), base, zone, sec, &eDataSize) != CG_OK)
                {
                    debug4 << mName << "Could not determine ElementDataSize\n";
                    continue;
                }
                debug4 << "Element data size for sec " << sec << " is:" << eDataSize << endl;

                int *elements = new int[eDataSize];
                if(elements == 0)
                {
                    debug4 << mName << "Could not allocate memory for connectivity\n";
                    continue;
                }
                
                if(cg_elements_read(GetFileHandle(), base, zone, sec, elements, NULL)
                   != CG_OK)
                {
                    delete [] elements;
                    elements = 0;
                    debug4 << mName << cg_get_error() << endl;
                    continue;
                }

                debug4 << "section " << sec << ": elementType=";
                PrintElementType(et);
                debug4 << " start=" << start << " end=" << end << " bound=" << bound
                       << " parent_flag=" << parent_flag << endl;

                //
                // Iterate over the elements and insert them into ugrid.
                //
                vtkIdType verts[27];
                const int *elem = elements;
                for(unsigned int icell = 0; icell < (end-start+1); ++icell)
                {
                    // If we're reading mixed elements then the element type 
                    // comes first.
                    ElementType_t currentType = et;
                    if(currentType == MIXED)
                    {
                        currentType = (ElementType_t)(*elem++);
#if 0
                        debug4 << "et[" << icell << "] = ";
                        PrintElementType(currentType);
                        debug4 << endl;
#endif
                    }

                    // Process the connectivity information for the cell.
                    switch(currentType)
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
                        //debug4 << "TRI3 [" << verts[0] << ", " << verts[1] << ", "
                        //       << verts[2] << "]" << endl;
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
                    case ElementTypeUserDefined:
                    case NGON_n:
                        delete [] coords[0];
                        delete [] coords[1];
                        delete [] coords[2];
                        delete [] elements;
                        elements = 0;
                        ugrid->Delete();
                        EXCEPTION1(InvalidVariableException, meshname);
                        break;
                    case ElementTypeNull:
                    case MIXED:
                        // What to do here?
                        break;
                    }
                }

                debug4 << mName << "Done reading cell connectivity." << endl;
                delete [] elements;
            }

            // Tell the user if we found any higher order elements.
            if(higherOrderWarning)
            {
                avtCallback::IssueWarning("VisIt found quadratic or cubic cells "
                    "in the mesh and reduced them to linear cells. Contact "
                    "visit-users@ornl.gov if you would like VisIt to natively "
                    "process higher order elements.");
            }

            retval = ugrid;
        }

        delete [] coords[0];
        delete [] coords[1];
        delete [] coords[2];
    }
    else
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    return retval;
}

// ****************************************************************************
//  Method: avtCGNSFileFormat::GetVar
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
// ****************************************************************************

vtkDataArray *
avtCGNSFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    const char *mName = "avtCGNSFileFormat::GetVar: ";
    debug4 << mName << "ts=" << timestate << ", dom=" << domain
           << ", var=" << varname << endl;

    // Look up the base that contains the variable.
    int base = 1;
    std::string sVarName(varname);
    int slashIndex = sVarName.find("/");
    if(slashIndex != -1)
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
    int zsize[9];

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
            if(zi > 0) debug4 << ", ";
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
        int requiredsol = timestate + 1;

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
                            debug4 << "Unsupported variable type" << endl;
                            break;
                        case Integer:
                            arr = vtkIntArray::New();
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
                            int rmin[3] = {1,1,1};
                            int rmax[3] = {1,1,1};
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
//  Method: avtCGNSFileFormat::GetVectorVar
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
avtCGNSFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}

// ****************************************************************************
// Method: avtCGNSFileFormat::PrintVarInfo
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
avtCGNSFileFormat::PrintVarInfo(ostream &out, const avtCGNSFileFormat::VarInfo &var, const char *indent)
{
    out << indent << "zoneList = {";
    for(int i = 0; i < var.zoneList.size(); ++i)
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
// Method: avtCGNSFileFormat::PrintStringVarInfoMap
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
avtCGNSFileFormat::PrintStringVarInfoMap(ostream &out, const avtCGNSFileFormat::StringVarInfoMap &vars, const char *indent)
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
// Method: avtCGNSFileFormat::PrintBaseInformation
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
avtCGNSFileFormat::PrintBaseInformation(ostream &out, const avtCGNSFileFormat::BaseInformation &baseInfo)
{
    out << "name = " << baseInfo.name.c_str() << endl;
    out << "cellDim = " << baseInfo.cellDim << endl;
    out << "physicalDim = " << baseInfo.physicalDim << endl;
    out << "meshType = " << baseInfo.meshType << " 0=curv, 1=ucd, -1,-2=unsupported" << endl;
    out << "zoneNames = {";
    for(int i = 0; i < baseInfo.zoneNames.size(); ++i)
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
