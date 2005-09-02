// ************************************************************************* //
//                            avtCGNSFileFormat.C                            //
// ************************************************************************* //

#include <avtCGNSFileFormat.h>
#include <cgnslib.h>

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
#define USE_BASES_FOR_TIMESTATES

// Include more source code.
#include <CGNSHelpers.C>
#include <CGNSUnitsStack.C>

// ****************************************************************************
// Class: VarInfo_t
//
// Purpose:
//   Helper class that contains a scalar metadata and a list of on/off values
//   for different domains.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 2 09:12:21 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class VarInfo_t
{
public:
    VarInfo_t() : domainsPresent()
    {
        smd = 0;
    }

    VarInfo_t(int nz) : domainsPresent()
    {
        smd = 0;
        for(int i = 0; i < nz; ++i)
            domainsPresent.push_back(0);
    }

    VarInfo_t(const VarInfo_t &obj) : domainsPresent(obj.domainsPresent)
    {
        smd = obj.smd;
    }

    ~VarInfo_t()
    {
    }

    avtScalarMetaData *smd;
    intVector          domainsPresent;
};

typedef std::map<std::string, VarInfo_t> VarInfoMap;


// ****************************************************************************
//  Method: avtCGNSFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
// ****************************************************************************

avtCGNSFileFormat::avtCGNSFileFormat(const char *filename)
    : avtMTMDFileFormat(filename), times(), MeshDomainMapping()
{
    fn = INVALID_FILE_HANDLE;
    timesRead = false;
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
//   
// ****************************************************************************

int
avtCGNSFileFormat::GetFileHandle()
{
    if(fn == INVALID_FILE_HANDLE)
    {
        if(cg_open(filename, MODE_READ, &fn) != CG_OK)
        {
            debug4 << cg_get_error() << endl;
            EXCEPTION1(InvalidFilesException, filename);
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
//   
// ****************************************************************************

void
avtCGNSFileFormat::ReadTimes()
{
    const char *mName = "avtCGNSFileFormat::ReadTimes: ";

    if(!timesRead)
    {
        // Read the number of bases and use that for the time step.
        int nbases = 0;
        if(cg_nbases(GetFileHandle(), &nbases) != CG_OK)
        {
            debug4 << cg_get_error() << endl;
            EXCEPTION1(InvalidFilesException, filename);
        }

        bool createTimeStates = true;
#ifdef USE_BASES_FOR_TIMESTATES
        int nstates = nbases;
#else
        int base = 1;
        int nstates = 1;
        if(cg_biter_read(GetFileHandle(), base, "TimeIterValues", &nstates) == CG_OK)
        {
            if(cg_goto(GetFileHandle(), base, "BaseIterativeData_t", 1, "end") == CG_OK)
            {
                int ndims = 1;
                int dims[10];
                DataType_t dt;
                if(cg_array_info(1, "TimeValues", &dt, &ndims, dims) == CG_OK)
                {
                    for(int i = 0; i < ndims; ++i)
                        nstates *= dims[i];
                    debug4 << mName << "nstates determined to be: " << nstates << endl;

                    double *dvals = new double[nstates+1];
                    if(cg_array_read_as(1, RealDouble, dvals) == CG_OK)
                    {
                        createTimeStates = false;
                        for(int j = 0; j < nstates; ++j)
                            times.push_back(dvals[j]);
                    }
                    else
                    {
                        debug4 << mName << "Could not read the time states: "
                               << cg_get_error() << endl;
                    }

                    delete [] dvals;
                }
                else
                {
                    debug4 << mName << "Could not read the time state array info: "
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
#endif
        if(createTimeStates)
        {
            // Fake the times for now.
            for(int i = 0; i < nstates; ++i)
                times.push_back(double(i));
        }

        timesRead = true;
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
//
// ****************************************************************************

void
avtCGNSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    const char *mName = "avtCGNSFileFormat::PopulateDatabaseMetaData: ";

    // Read the times if we have not read them yet.
    ReadTimes();

    int base = 1;

    // Get the title
    char *refstate = 0;
    if(cg_goto(GetFileHandle(), base, "end") == CG_OK)
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

    //
    // Determine the topological and spatial dimensions.
    //
    char namebase[33];
    int cell_dim = 2, phys_dim = 2;
    if(cg_base_read(GetFileHandle(), base, namebase, &cell_dim, &phys_dim) != CG_OK)
    {
        debug4 << cg_get_error() << endl;
        EXCEPTION1(InvalidFilesException, filename);
    }
    else
    {
        debug4 << mName << " name=" << namebase << " cell_dim=" << cell_dim
               << " phys_dim=" << phys_dim << endl;
    }

    //
    // See if the file will contain units.
    //
    bool fileContainsUnits = false;
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
            fileContainsUnits = (dc == Dimensional);
            int nunits = 0;
            if(fileContainsUnits &&
               cg_nunits(&nunits) == CG_OK &&
               nunits > 0)
            {
                if(cg_units_read(&massU, &lengthU, &timeU,
                                 &tempU, &angleU) == CG_OK)
                {
                    debug4 << mName << "We read the units!" << endl;
                    fileContainsUnits = true;
                }
            }
            else
            {
                fileContainsUnits = false;
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
        debug4 << mName << "The file has" << (fileContainsUnits?"":" no")
               << " units." << endl;
    }
    CGNSUnitsStack unitStack;
    bool baseUnits = fileContainsUnits &&
                     unitStack.PushUnits(GetFileHandle(), base);

    //
    // Determine the number of domains.
    //
    int nZones = 0;
    if(cg_nzones(GetFileHandle(), base, &nZones) != CG_OK)
    {
        debug4 << mName << "Could not get number of domains in base "
               << base << ": " << cg_get_error() << endl;
    }

    //
    // Iterate over the domains and learn what variables are in the file.
    //
    VarInfoMap   varInfo;
    stringVector domainNames;
    avtMeshType  mt = AVT_CURVILINEAR_MESH;
    bool         someInvalidCenterings = false;
    bool haveVelocityX = false, haveVelocityY = false, haveVelocityZ = false;
    bool haveMomentumX = false, haveMomentumY = false, haveMomentumZ = false;
    for(int zone = 1; zone <= nZones; ++zone)
    {
        // Get information about the zone.
        char zonename[33];
        int zsize[9];
        memset(zonename, 0, 33);
        memset(zsize, 0, 9 * sizeof(int));

        debug4 << "\tzone " << zone;

        // Print the name and size.
        if(cg_zone_read(GetFileHandle(), base, zone, zonename, zsize) != CG_OK)
            debug4 << cg_get_error() << endl;
        else
        {
            debug4 << " name=" << zonename << " size=[";
            for(int zi = 0; zi < 9; ++zi)
            {
                if(zi > 0) debug4 << ", ";
                debug4 << zsize[zi];
            }
            debug4 << "]";
        }
        domainNames.push_back(zonename);

        // Get the zone type.
        ZoneType_t zt = ZoneTypeNull;
        if(cg_zone_type(GetFileHandle(), base, zone, &zt) != CG_OK)
            debug4 << cg_get_error() << endl;
        else
        {
            switch(zt)
            {
            case ZoneTypeNull:
                debug4 << " type=ZoneTypeNull";
                EXCEPTION1(ImproperUseException, 
                           "ZoneTypeNull is not supported.");
                break;
            case ZoneTypeUserDefined:
                debug4 << " type=ZoneTypeUserDefined";
                EXCEPTION1(ImproperUseException, 
                           "ZoneTypeUserDefined is not supported.");
                break;
            case Structured:
                debug4 << " type=Structured";
                mt = AVT_CURVILINEAR_MESH;
                break;
            case Unstructured:
                debug4 << " type=Unstructured";
                mt = AVT_UNSTRUCTURED_MESH;
                break;
            }
        }

        // Get the zone id
        double zone_id = 0.;
        if(cg_zone_id(GetFileHandle(), base, zone, &zone_id) != CG_OK)
            debug4 << cg_get_error() << endl;
        else
            debug4 << " zone_id=" << zone_id;

        int nsols = 0;
        if(cg_nsols(GetFileHandle(), base, zone, &nsols) != CG_OK)
        {
            debug4 << "Could not get number of solutions in zone "
                 << zone << endl;
            debug4 << cg_get_error() << endl;
            continue;
        }

        debug4 << "  nsols=" << nsols << endl;

        // Get the number of grids
        int ngrids = 0;
        if(cg_ngrids(GetFileHandle(), base, zone, &ngrids) != CG_OK)
        {
            debug4 << "\t\tCould not get the number of grids" << endl;
            debug4 << cg_get_error() << endl;
        }
        else
        {
            debug4 << "\t\tgrids: " << ngrids << endl;
            for(int gridno = 1; gridno <= ngrids; ++gridno)
            {
                char gridname[33];
                if(cg_grid_read(GetFileHandle(), base, zone, gridno, gridname) != CG_OK)
                {
                    debug4 << "\t\t\tgrid[" << gridno << "] = ? "
                         << cg_get_error() << endl;
                }
                else
                    debug4 << "\t\t\tgrid[" << gridno << "] = " << gridname;
                debug4 << endl;
            }
        }

        // Get the number of coords
        int ncoords = 0;
        if(cg_ncoords(GetFileHandle(), base, zone, &ncoords) != CG_OK)
        {
            debug4 << "\t\tCould not get the number of coords" << endl;
            debug4 << cg_get_error() << endl;
        }
        else
        {
            debug4 << "\t\tcoords: " << ncoords << endl;
            for(int c = 1; c <= ncoords; ++c)
            {
                char coordname[33];
                DataType_t ct;
                if(cg_coord_info(GetFileHandle(), base, zone, c, &ct, coordname) != CG_OK)
                {
                    debug4 << "\t\t\tcoord[" << c << "] = ? "
                         << cg_get_error() << endl;
                }
                else
                {
                    debug4 << "\t\t\tcoord[" << c << "] = " << coordname
                         << " type=";
                    PrintDataType(ct);
                }
                debug4 << endl;
            }
        }

        // Get the units for the zone.
        bool zoneUnits = (zone == 1) && fileContainsUnits &&
                          unitStack.PushUnits(GetFileHandle(), base, zone);

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

            debug4 << "\t\t" << "solution: " << solname;

            int nfields = 0;
            if(cg_nfields(GetFileHandle(), base, zone, sol, &nfields) != CG_OK)
            {
                debug4 << "Could not get number of fields for solution "
                     << sol << endl;
                debug4 << cg_get_error() << endl;
                continue;
            }

            debug4 << ": nfields=" << nfields << " centering=";
            avtCentering centering = AVT_UNKNOWN_CENT;
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
                 centering = AVT_NODECENT;
                 break;
            case CellCenter:
                 debug4 << "CellCenter";
                 centering = AVT_ZONECENT;
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

            // Get the units for the solution.
            bool solUnits = (zone == 1) && fileContainsUnits &&
                            unitStack.PushUnits(GetFileHandle(), base, zone, sol);

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

                // Print the field name and type.
                debug4 << "\t\t\t" << "field: " << fieldname << " type=";
                PrintDataType(dt);
                debug4 << endl;

                // Create a scalar metadata for the field.
                VarInfoMap::iterator pos = varInfo.find(fieldname);
                if(pos == varInfo.end())
                {
                    avtScalarMetaData *smd = new avtScalarMetaData(fieldname,
                        "grid", centering);
                    smd->validVariable = centering == AVT_NODECENT ||
                                         centering == AVT_ZONECENT;
                    // Get the units
                    if(smd->validVariable)
                    {
                        bool fUnits = fileContainsUnits &&
                            unitStack.PushUnits(GetFileHandle(), base, zone, sol, f);
                        smd->hasUnits = unitStack.GetUnits(smd->units);
                        if(fUnits)
                            unitStack.PopUnits();
                    
                        // See if we have Volicity and Momentum components.
                        haveVelocityX |= (strcmp(fieldname, "VelocityX") == 0);
                        haveVelocityY |= (strcmp(fieldname, "VelocityY") == 0);
                        haveVelocityZ |= (strcmp(fieldname, "VelocityZ") == 0);
                        haveMomentumX |= (strcmp(fieldname, "MomentumX") == 0);
                        haveMomentumY |= (strcmp(fieldname, "MomentumY") == 0);
                        haveMomentumZ |= (strcmp(fieldname, "MomentumZ") == 0);
                    }

                    VarInfo_t currentVar(nZones);
                    currentVar.smd = smd;
                    currentVar.domainsPresent[zone-1] = 1;
                    varInfo[fieldname] = currentVar;

                    someInvalidCenterings |= (!smd->validVariable);
                }
                else
                    pos->second.domainsPresent[zone-1] = 1;
            }

            // Pop the unit stack.
            if(solUnits)
                unitStack.PopUnits();
        }

        // Pop the unit stack.
        if(zoneUnits)
            unitStack.PopUnits();
    }

    if(someInvalidCenterings)
    {
        avtCallback::IssueWarning("Some variables have been disabled because "
            "their grid locations (variable centerings) are not supported by "
            "VisIt. VisIt currently supports node and cell centered variables.");
    }

    //
    // Determine how many meshes are required based on the unique lists of
    // domains that we have.
    //
    std::map<intVector, std::string> meshDef;
    int meshCount = 0;
    for(VarInfoMap::const_iterator pos = varInfo.begin();
        pos != varInfo.end(); ++pos)
    {
        std::map<intVector, std::string>::iterator meshIt;
        meshIt = meshDef.find(pos->second.domainsPresent);
        if(meshIt == meshDef.end())
        {
            ++meshCount;

            char tmp[100];
            SNPRINTF(tmp, 100, "subgrid/grid%03d", meshCount);
            meshDef[pos->second.domainsPresent] = std::string(tmp);
        }
    }

    //
    // Use the mesh definitions to date to create the list of meshes and
    // populate the MeshDomainMapping map, which contains a list of active
    // domains for each mesh.
    //
    if(meshCount < 2)
    {
        intVector domainsPresent;
        for(int i = 0; i < nZones; ++i)
            domainsPresent.push_back(1);
        MeshDomainMapping["grid"] = domainsPresent;
    }
    else
    {
        intVector alldoms;
        for(int i = 0; i < nZones; ++i)
            alldoms.push_back(1);
        bool alldomsRepresented = false;

        // Iterate over the mesh definitions and put their definitions
        // in the MeshDomainMapping.
        std::map<intVector, std::string>::iterator meshIt;
        for(meshIt = meshDef.begin(); meshIt != meshDef.end(); ++meshIt)
        {
            // See if the mesh with "all domains" is in the list.
            bool equalsAllDoms = (alldoms == meshIt->first);
            if(equalsAllDoms)
            {
                MeshDomainMapping["grid"] = meshIt->first;
                meshIt->second = "grid";
                alldomsRepresented = true;
            }
            else
                MeshDomainMapping[meshIt->second] = meshIt->first;
        }

        if(!alldomsRepresented)
        {
            MeshDomainMapping["grid"] = alldoms;
        }

        // Iterate over the variables and assign them to a mesh.
        for(VarInfoMap::iterator pos = varInfo.begin();
            pos != varInfo.end(); ++pos)
        {
            meshIt = meshDef.find(pos->second.domainsPresent);

            // Tell the scalar metadata which mesh it will be on.
            pos->second.smd->meshName = meshIt->second;
        }
    }

    // Iterate over the variables and them to the metadata.
    for(VarInfoMap::iterator pos = varInfo.begin();
        pos != varInfo.end(); ++pos)
    {
        md->Add(pos->second.smd);
    }

    //
    // Add the mesh(es)
    //
    std::map<std::string, intVector>::const_iterator meshIt;
    for(meshIt = MeshDomainMapping.begin();
        meshIt != MeshDomainMapping.end();
        ++meshIt)
    {
        avtMeshMetaData *mmd = new avtMeshMetaData(meshIt->first, 
            1, 1, 1, 0, phys_dim, cell_dim, mt);
        mmd->blockNames = domainNames;
        mmd->numBlocks = domainNames.size();
        mmd->blockOrigin = 1;
        mmd->groupOrigin = 1;
        mmd->cellOrigin = 1;
        // Get the mesh coordinate units...
        md->Add(mmd);
    }

    // Add some expressions.
    if(haveVelocityX && haveVelocityY && haveVelocityZ)
    {
        Expression *e = new Expression;
        e->SetName("Velocity");
        e->SetDefinition("{VelocityX,VelocityY,VelocityZ}");
        e->SetType(Expression::VectorMeshVar);
        md->AddExpression(e);
    }
    if(haveMomentumX && haveMomentumY && haveMomentumZ)
    {
        Expression *e = new Expression;
        e->SetName("Momentum");
        e->SetDefinition("{MomentumX,MomentumY,MomentumZ}");
        e->SetType(Expression::VectorMeshVar);
        md->AddExpression(e);
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
// ****************************************************************************

vtkDataSet *
avtCGNSFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    const char *mName = "avtCGNSFileFormat::GetMesh: ";
    debug4 << mName << "ts=" << timestate << ", dom=" << domain
           << ", mesh=" << meshname << endl;

    // See if this domain is turned off by default for this mesh.
    std::map<std::string, intVector>::const_iterator pos = 
        MeshDomainMapping.find(meshname);
    if(pos != MeshDomainMapping.end() && pos->second[domain] == 0)
        return 0;

    vtkDataSet *retval = 0;
#ifdef USE_BASES_FOR_TIMESTATES
    int base = timestate + 1;
#else
    int base = 1;
#endif
    int zone = domain + 1;
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
            rmax[0] = zsize[0];
            rmax[1] = zsize[1];
            rmax[2] = zsize[2];
            nPts = zsize[0] * zsize[1] * zsize[2];
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
        dims[1] = zsize[1];
        dims[2] = zsize[2];
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
            const float *zc = coords[2];
            for(unsigned int i = 0; i < nPts; ++i)
            {
                float pt[3];
                pt[0] = *xc++;
                pt[1] = *yc++;
                pt[2] = *zc++;
                pts->SetPoint(i, pt);
            }

            // Create an unstructured grid to contain the points.
            vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
            ugrid->SetPoints(pts);
            ugrid->Allocate(zsize[1]);
            pts->Delete();
            bool higherOrderWarning = false;

            // Iterate over each of the sections.
#if 1
            nsections = 1;
#endif
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
                int *pdata = new int[eDataSize];
                if(pdata == 0)
                { 
                    delete [] elements; elements = 0;
                    debug4 << mName << "Could not allocate memory for pdata\n";
                    continue;
                }
                
                if(cg_elements_read(GetFileHandle(), base, zone, sec, elements, pdata)
                   != CG_OK)
                {
                    delete [] elements;
                    elements = 0;
                    delete [] pdata;
                    pdata = 0;
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
                        delete [] pdata;
                        pdata = 0;
                        ugrid->Delete();
                        EXCEPTION1(InvalidVariableException, meshname);
                        break;
                    }
                }

                debug4 << mName << "Done reading cell connectivity." << endl;
                delete [] elements;
                delete [] pdata;
            }

            // Tell the user if we found any higher order elements.
            if(higherOrderWarning)
            {
                avtCallback::IssueWarning("VisIt found quadratic or cubic cells "
                    "in the mesh and reduced them to linear cells. Contact "
                    "visit-help@llnl.gov if you would like VisIt to natively "
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
//
// ****************************************************************************

vtkDataArray *
avtCGNSFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    const char *mName = "avtCGNSFileFormat::GetVar: ";
    debug4 << mName << "ts=" << timestate << ", dom=" << domain
           << ", var=" << varname << endl;

    vtkDataArray *retval = 0;
#ifdef USE_BASES_FOR_TIMESTATES
    int base = timestate + 1;
#else
    int base = 1;
#endif
    int zone = domain + 1;
    char zonename[33];
    int zsize[9];

    memset(zonename, 0, 33);
    memset(zsize, 0, 9 * sizeof(int));

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

        // Iterate through the solutions until we find the variable that we're
        // looking for.
        bool fieldNotFound = true;
        for(int sol = 1; sol <= nsols && fieldNotFound; ++sol)
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
                    if(strcmp(fieldname, varname) == 0)
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
                                    rmax[0] = zsize[0];
                                    rmax[1] = zsize[1];
                                    rmax[2] = zsize[2];
                                }
                                else
                                {
                                    rmax[0] = zsize[3];
                                    rmax[1] = zsize[4];
                                    rmax[2] = zsize[5];
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
