#include <PP_ZFileReader.h>
#include <MaterialEncoder.h>

#include <InvalidVariableException.h>
#include <InvalidCellTypeException.h>

#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkStructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>

#include <DebugStream.h>

//
// Make the revolved mesh be 40 zones in circumference.
//
const int PP_ZFileReader::revolutionSteps = 40 + 1;

// ****************************************************************************
// Method: PP_ZFileReader::PP_ZFileReader
//
// Purpose: 
//   Constructor for the PP_ZFileReader class.
//
// Arguments:
//   pdb : The pdb file to use.
//   c   : The variable cache to use for storing auxiliary data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:00:44 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 5 17:39:10 PST 2003
//   Added nMaterials, assumeMixedMaterialsPresent, nodalVars, and the new
//   cache pointer to the argument list.
//
// ****************************************************************************

PP_ZFileReader::PP_ZFileReader(PDBfile *pdb, avtVariableCache *c) :
    PDBReader(pdb, c), rtVar("rt"), ztVar("zt"), materialNames(), varStorage(),
    nodalVars()
{
    kmax = lmax = 0;
    meshDimensionsKnown = false;
    unstructuredCellCount = -1;

    cycles = 0;
    nCycles = 0;
    times = 0;
    nTimes = 0;
    assumeMixedMaterialsPresent = false;

    initialized = false;
}

// ****************************************************************************
// Method: PP_ZFileReader::~PP_ZFileReader
//
// Purpose: 
//   Destructor for the PP_ZFileReader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:03:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PP_ZFileReader::~PP_ZFileReader()
{
    delete [] cycles;
    delete [] times;

    // Delete the cached VariableData objects.
    for(VariableDataMap::iterator pos = varStorage.begin();
        pos != varStorage.end(); ++pos)
    {
        delete pos->second;
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::Identify
//
// Purpose: 
//   Identifies the file as a PP Z file.
//
// Returns:    True if the PDB file is a PP Z file; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:05:44 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 8 10:50:01 PDT 2003
//   Made it able to operate on files that lack kmax, lmax.
//
// ****************************************************************************

bool
PP_ZFileReader::Identify()
{
    bool have_kmax, have_lmax, validFile;

    // Make sure the file has kmax,
    if((have_kmax = GetInteger("kmax@value", &kmax)) == false)
    {
        have_kmax = GetInteger("kmax@las", &kmax);
    }

    // Make sure the file has lmax,
    if((have_lmax = GetInteger("lmax@value", &lmax)) == false)
    {
        have_lmax = GetInteger("lmax@las", &lmax);
    }

    debug4 << "PP_ZFileReader::Identify()" << endl;
    validFile = have_kmax && have_lmax;
    if(validFile)
    {
        debug4 << "\tkmax@value=" << kmax << endl;
        debug4 << "\tlmax@value=" << lmax << endl;
        meshDimensionsKnown = true;
    }
    else
    {
        bool haveCycleVariable = SymbolExists("cycle_variable@value");
        bool haveTimeVariable = SymbolExists("time_variable@value");
        validFile = haveCycleVariable && haveTimeVariable;
        meshDimensionsKnown = false;
    }
    debug4 << "\t meshDimensionsKnown="
           << (meshDimensionsKnown?"true":"false") << endl;
    debug4 << "\t validFile=" << (validFile?"true":"false") << endl;

    return validFile;
}

// ****************************************************************************
// Method: PP_ZFileReader::Initialize
//
// Purpose: 
//   Reads in the cycles and times.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:06:33 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Aug 7 15:15:54 PST 2003
//   Added a list of nodal variables. Only variables in that list are
//   considered to be nodal. Added code to figure out the mesh dimensions
//   if they were never given.
//
// ****************************************************************************

void
PP_ZFileReader::Initialize()
{
    if(!initialized)
    {
        //
        // Read the cycles
        //
        char *realName = 0;
        if(GetString("cycle_variable@value", &realName))
        {
            debug4 << "Cycle array name: " << realName << endl;

            if(GetIntegerArray(realName, &cycles, &nCycles))
            {
                debug4 << "Cycles = (";
                for(int i = 0; i < nCycles; ++i)
                {
                    debug4 << cycles[i];
                    if(i < nCycles - 1)
                        debug4 << ", ";
                }
                debug4 << ")" << endl;
            }

            delete [] realName;
        }
        else
        {
            debug4 << "Could not read the cycle array!" << endl;
            cycles = new int[1];
            cycles[0] = 0;
            nCycles = 1;
        }

        //
        // Read the time array.
        //
        if(GetString("time_variable@value", &realName))
        {
            debug4 << "Time array name: " << realName << endl;

            if(GetDoubleArray(realName, &times, &nTimes))
            {
                debug4 << "Times = (";
                for(int i = 0; i < nTimes; ++i)
                {
                    debug4 << times[i];
                    if(i < nTimes - 1)
                        debug4 << ", ";
                }
                debug4 << ")" << endl;
            }

            delete [] realName;
        }
        else
        {
            debug4 << "Could not read the time array!" << endl;
            times = new double[1];
            times[0] = 0.;
            nTimes = 1;
        }

        //
        // Read the list of nodal variables.
        //
        char *pc_list = 0;
        int   pc_list_len = 0;
        if(GetString("pc_list@global", &pc_list, &pc_list_len))
        {
            for(int i = 0; i < pc_list_len; i += 2)
            {
                char nodalVar[3] = {0,0,0};
                nodalVar[0] = pc_list[i];
                nodalVar[1] = (pc_list[i+1] != ' ') ? pc_list[i+1] : 0;
                nodalVars.push_back(nodalVar);
                debug4 << "Added " << nodalVar << " to the list of nodal vars."
                       << endl;
            }
            free_void_mem((void *)pc_list, CHARARRAY_TYPE);
        }
        else
        {
            debug4 << "Could not read pc_list@global nodal var list." << endl;
        }

        //
        // Figure out the mesh dimensions if they were not given in the file.
        //
        if(!meshDimensionsKnown)
        {
            int *dimensions = 0;
            int nDims = 0;
            TypeEnum t = NO_TYPE;
            int nTotalElements = 0;

            if(SymbolExists("ireg@history", &t, &nTotalElements, &dimensions,
                            &nDims))
            {
                if(nDims >= 2)
                {
                    meshDimensionsKnown = true;
                    kmax = dimensions[0];
                    lmax = dimensions[1];
                    debug4 << "kmax determined to be: " << kmax << endl;
                    debug4 << "lmax determined to be: " << lmax << endl;
                }

                delete [] dimensions;
            }
        }

        initialized = true;
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::VariableIsNodal
//
// Purpose: 
//   Returns whether or not the variable is nodal.
//
// Arguments:
//   var : The name of the variable.
//
// Returns:    True if the variable is nodal; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 7 15:25:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PP_ZFileReader::VariableIsNodal(const std::string &var) const
{
    for(int i = 0; i < nodalVars.size(); ++i)
    {
        if(var == nodalVars[i])
            return true;
    }

    return false;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetTimeVaryingInformation
//
// Purpose: 
//   Stores time varying information like the cycles and times in the
//   database metadata.
//
// Arguments:
//   state : The state that we're interested in.
//   md    : The database metadata that we're populating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:09:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::GetTimeVaryingInformation(int state, avtDatabaseMetaData *md)
{
    debug4 << "PP_ZFileReader::GetTimeVaryingInformation: state=" << state << endl;

    //
    // Make sure everything is initialized.
    //
    Initialize();

    // Put the cycles into the metadata.
    if(md->GetCycles().size() < nCycles)
    {
        intVector c; c.reserve(nCycles);
        for(int i = 0; i < nCycles; ++i)
            c.push_back(cycles[i]);

        // Set all of the cycles at once.
        md->SetCycles(c);
        md->SetCyclesAreAccurate(true);
    }

    // Put the times into the metadata.
    if(md->GetTimes().size() < nTimes)
    {
        doubleVector d; d.reserve(nTimes);
        for(int i = 0; i < nTimes; ++i)
            d.push_back(times[i]);

        // Set all of the cycles at once.
        md->SetTimes(d);
        md->SetTimesAreAccurate(true);

        if(nTimes > 0)
            md->SetTemporalExtents(times[0], times[nTimes-1]);
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::PopulateMaterialNames
//
// Purpose: 
//   Populates the materialNames object using the variables in the file.
//
// Returns:    True if materials were found.
//
// Note:       It's possible that no materials are found.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 12 13:46:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PP_ZFileReader::PopulateMaterialNames()
{
    bool haveMaterials = false;

    //
    // Look for evidence of materials in the file.
    //
    int nMaterials = 0;
    if(GetInteger("nreg@las", &nMaterials))
    {
        debug4 << "We found evidence of " << nMaterials 
               << " materials in the file." << endl;

        for(int i = 0; i < nMaterials; ++i)
        {
            char tmp[20];
            sprintf(tmp, "%d", i + 1);
            materialNames.push_back(tmp);
        }
        haveMaterials = true;
    }
    else if(varStorage.find("ireg") != varStorage.end())
    {
        debug4 << "We didn't find obvious evidence of materials in the file "
                  "but we can read the ireg array to determine the materials."
               << endl;

        TRY
        {
            ReadVariable("ireg");

            VariableData *ireg_data = varStorage["ireg"];
            if(ireg_data->dataType == INTEGERARRAY_TYPE)
            {
                const int *ireg = (const int *)ireg_data->data;
                int i;
#define MAX_MATERIALS 1000
                bool *mats = new bool[MAX_MATERIALS];
                for(i = 0; i < MAX_MATERIALS; ++i)
                    mats[i] = false;

                //
                // Search through the ireg array and look for all unique
                // material numbers.
                //
                for(i = 0; i < ireg_data->nTotalElements; ++i)
                {
                    if(ireg[i] > 0 && ireg[i] < MAX_MATERIALS)
                        mats[ireg[i]] = true;
                }

                //
                // Look through the list of material numbers and use it
                // to add to the material names vector.
                //
                for(i = 1; i < MAX_MATERIALS; ++i)
                {
                    if(mats[i])
                    {
                        char tmp[20];
                        sprintf(tmp, "%d", i);
                        materialNames.push_back(tmp);
                    }
                }
                delete [] mats;
#undef MAX_MATERIALS

                //
                // If we have material names, return that we have materials.
                //
                if(materialNames.size() > 0)
                {
                    debug4 << "We found materials by inspecting the ireg "
                              "array." << endl;
                    debug4 << "Materials={";
                    for(i = 0; i < materialNames.size(); ++i)
                        debug4 << materialNames[i] << ", ";
                    debug4 << "}" << endl;
                    haveMaterials = true;
                }
            }
        }
        CATCH(InvalidVariableException)
        {
            debug4 << "We can't read the ireg array so we'll say that the "
                      "database has no materials." << endl;
        }
        ENDTRY
    }

    return haveMaterials;
}

// ****************************************************************************
// Method: PP_ZFileReader::PopulateDatabaseMetaData
//
// Purpose: 
//   Reads in the list of variables from the file and puts that list into
//   the database metadata.
//
// Arguments:
//   md : The database metadata that we're populating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:11:20 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 5 17:39:41 PST 2003
//   Added support for materials, a revolved mesh, and zonal variables. Made
//   it work on files that are missing some crucial information.
//
// ****************************************************************************

void
PP_ZFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "PP_ZFileReader::PopulateDatabaseMetaData: start" << endl;

    //
    // Make sure that everything is initialized.
    //
    Initialize();

    int cellOrigin = 1;
    int ndims = 2;

    // Add the logical mesh
    avtMeshMetaData *mmd = new avtMeshMetaData(
        "logical_mesh", 1, 0, cellOrigin, ndims, ndims, AVT_RECTILINEAR_MESH);
    mmd->hasSpatialExtents = false;
    mmd->cellOrigin = 1;
    md->Add(mmd);

    // Add the mesh.
    mmd = new avtMeshMetaData(
        "mesh", 1, 0, cellOrigin, ndims, ndims, AVT_CURVILINEAR_MESH);
    mmd->hasSpatialExtents = false;
    mmd->cellOrigin = 1;
    md->Add(mmd);

    // Add a revolved mesh.
    mmd = new avtMeshMetaData(
        "revolved_mesh", 1, 0, cellOrigin, 3, 3, AVT_UNSTRUCTURED_MESH);
    mmd->hasSpatialExtents = false;
    mmd->cellOrigin = 1;
    md->Add(mmd);

    // Determine the size of the problem.
    int problemSize = kmax * lmax * nCycles;
    debug4 << "problemSize = " << problemSize << endl;

    //
    // Read all variables of the specified type.
    //
    int numVars = 0;
    char **varList = PD_ls(pdb, NULL /*path*/, NULL /*pattern*/, &numVars);

    //
    // If we got any variable names, see if any are the size that we think
    // the problem should be.
    //
    if(varList != NULL)
    {
        for(int j = 0; j < numVars; ++j)
        {
            bool problemSized = false;
            int length = 1;
            syment *ep = 0;

            // Check to see if the  variable is problem sized.
            if((ep = PD_inquire_entry(pdb, varList[j], 0, NULL)) != NULL)
            {
                // Figure out the number of dimensions and the number of
                // elements that are in the entire array.
                dimdes *dimptr = PD_entry_dimensions(ep);
                while(dimptr != 0)
                {
                    length *= dimptr->number;
                    dimptr = dimptr->next;
                }

                problemSized = (length == problemSize);
            }

            //
            // If the variable is problem sized then consider it as a
            // candidate for being a plottable variable.
            //
            if(problemSized)
            {
                debug4 << "\t" << varList[j] << " (problem sized)";
                char *s = strstr(varList[j], "@");
                if(s != 0)
                {
                    int len = s - varList[j];
                    char *newCStr = new char[len + 1];
                    strncpy(newCStr, varList[j], len);
                    newCStr[len] = '\0';
                    std::string newStr(newCStr);

                    avtCentering centering;
                    if(VariableIsNodal(newStr))
                    {
                        centering = AVT_NODECENT;
                        debug4 << "(nodal)";
                    }
                    else
                    {
                        centering = AVT_ZONECENT;
                        debug4 << "(zonal)";
                    }

                    debug4 << " added as " << newCStr << endl;

                    // Add the variable over the logical mesh to the metadata.
                    std::string logicalMeshVar(std::string("logical_mesh/") +
                                               newStr);
                    avtScalarMetaData *smd = new avtScalarMetaData(
                        logicalMeshVar, "logical_mesh", centering);
                    md->Add(smd);

                    // Add the variable over the revolved mesh.
                    std::string revolvedMeshVar(std::string("revolved_mesh/") +
                                                newStr);
                    smd = new avtScalarMetaData(revolvedMeshVar,
                        "revolved_mesh", centering);
                    md->Add(smd);

                    // Add the variable over the mesh to the metadata.
                    std::string meshVar(std::string("mesh/") + newStr);
                    smd = new avtScalarMetaData(meshVar, "mesh", centering);
                    md->Add(smd);

                    // Add the variable to the variable map.
                    VariableData *v = new VariableData(varList[j]);
                    varStorage[newStr] = v;

                    delete [] newCStr;
                }
            }
        }

        debug4 << endl << endl;
        SFREE(varList);
    }

    //
    // If we have material names, add materials to the metadata.
    //
    if(PopulateMaterialNames())
    {
        //
        // Add material metadata to the metadata.
        //
        avtMaterialMetaData *mmd = new avtMaterialMetaData("material",
            "mesh", materialNames.size(), materialNames);
        md->Add(mmd);
        mmd = new avtMaterialMetaData("material2",
            "revolved_mesh", materialNames.size(), materialNames);
        md->Add(mmd);

        //
        // Look for evidence of mixed materials.
        //
        int nszmmt = 0;
        GetInteger("nszmmt@las", &nszmmt);
        if(nszmmt > 0)
        {
            //
            // Add the mixed material arrays to the cache so we can read them
            // later and have them be cached.
            //
            VariableData *v = new VariableData("iregmm@las");
            varStorage["iregmm"] = v;
            v = new VariableData("volfmm@las");
            varStorage["volfmm"] = v;
            v = new VariableData("ilamm@las");
            varStorage["ilamm"] = v;
            if(varStorage.find("nummm") == varStorage.end())
            {
                v = new VariableData("nummm@las");
                varStorage["nummm"] = v;
            }
 
            assumeMixedMaterialsPresent = true;
        }
    }

    //
    // Figure out the names that should be used for the rt,zt mesh arrays.
    //
    bool haveRt = true;
    if(varStorage.find(rtVar) == varStorage.end())
    {
        if(varStorage.find("r") != varStorage.end())
            rtVar = "r";
        else
            haveRt = false;
    }
    bool haveZt = true;
    if(varStorage.find(ztVar) == varStorage.end())
    {
        if(varStorage.find("z") != varStorage.end())
            ztVar = "z";
        else
            haveZt = false;
    }
    if(!haveRt || !haveZt)
    {
        debug4 << "We don't have r or z coordinate fields! We need to "
                   "disable most of the variables in the metadata." << endl;
    }
    else
    {
        debug4 << "Using " << rtVar.c_str() << " for r coordinate field."
               << endl;
        debug4 << "Using " << ztVar.c_str() << " for z coordinate field."
               << endl;
    }

    debug4 << "PP_ZFileReader::PopulateDatabaseMetaData: end" << endl;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetCycles
//
// Purpose: 
//   Adds the cycle numbers to the specified vector of ints.
//
// Arguments:
//   c : The int vector to which the cycles are added.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:51:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::GetCycles(std::vector<int> &c)
{
    Initialize();
    c.clear();

    debug4 << "PP_ZFileReader::GetCycles: cycles=(";
    for(int i = 0; i < nCycles; ++i)
    {
        c.push_back(cycles[i]);
        debug4 << ", " << cycles[i];
    }
    debug4 << ")" << endl;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetNTimesteps
//
// Purpose: 
//   Returns the number of timesteps in the database.
//
// Returns:    The number of timesteps.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:52:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PP_ZFileReader::GetNTimesteps()
{
    Initialize();
    return nTimes;
}

// ****************************************************************************
// Method: PP_ZFileReader::CreateGhostZones
//
// Purpose: 
//   Marks all of the cells in the input mesh where ireg <= 0 as ghsot cells.
//
// Arguments:
//   ireg : The ireg array used to determine if cells are ghost cells.
//   ds   : The input dataset.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 16:11:08 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::CreateGhostZones(const int *ireg, vtkDataSet *ds)
{
    if(ireg)
    {
        unsigned char realVal = 0, ghostVal = 1;
        int nCells = ds->GetNumberOfCells();
        vtkIdList *ptIds = vtkIdList::New();
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("vtkGhostLevels");
        ghostCells->Allocate(nCells);

        for(int i = 0; i < nCells; ++i)
        {
            ds->GetCellPoints(i, ptIds);
            int cellRow = i / (kmax - 1);
            int cellCol = i % (kmax - 1);
            // Get the node index for the upper right node in the cell.
            int nodeIndex = (cellRow + 1) * kmax + cellCol + 1;

            if(ireg[nodeIndex] <= 0)
                ghostCells->InsertNextValue(ghostVal);
            else
                ghostCells->InsertNextValue(realVal);
        }

        ds->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();
        ptIds->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->Allocate(6);
        realDims->SetValue(0, 0);
        realDims->SetValue(1, kmax);
        realDims->SetValue(2, 0);
        realDims->SetValue(3, lmax);
        realDims->SetValue(4, 0);
        realDims->SetValue(5, 0);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();
        ds->SetUpdateGhostLevel(0);
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetIreg
//
// Purpose: 
//   Returns the ireg variable, which is commonly used to identify ghost cells
//   and material numbers.
//
// Arguments:
//   state : The state to use when returning ireg.
//
// Returns:    A const pointer to the ireg array at the specified state.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:16:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const int *
PP_ZFileReader::GetIreg(int state)
{
    const int *ireg = 0;
    int nnodes = kmax * lmax;

    TRY
    {
        ReadVariable("ireg");

        VariableData *iregData = varStorage["ireg"];
        if(iregData->dataType == INTEGERARRAY_TYPE)
        {
            //
            // Go to the right time state in ireg
            //
            ireg = (const int *)iregData->data;
            ireg += ((state < nCycles) ? (state * nnodes) : 0);
        }
    }
    CATCH(InvalidVariableException)
    {
        debug4 << "ireg could not be read in." << endl;
    }
    ENDTRY


    return ireg;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetMesh
//
// Purpose: 
//   Returns the mesh.
//
// Arguments:
//   state : The time state for which we want the mesh.
//   var   : The name of the mesh that we want.
//
// Returns:    A vtkDataSet object containing the mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:58:50 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Jul 16 16:14:12 PST 2003
//   I added a curvilinear mesh, which is what they really want to look at,
//   and I made cells be marked as ghost zones according to the ireg array.
//
//   Brad Whitlock, Wed Aug 6 11:50:54 PDT 2003
//   I added a revolved (unstructured) version of their curvilinear mesh.
//
// ****************************************************************************

vtkDataSet *
PP_ZFileReader::GetMesh(int state, const char *var)
{
    debug4 << "PP_ZFileReader::GetMesh: state=" << state
           << ", var=" << var << endl;

    //
    // Make sure that everything is initialized.
    //
    Initialize();

    //
    // Read the ireg variable, which tells us which are ghost zones.
    //
    int i, j;
    int nnodes = kmax * lmax;
    const int *ireg = GetIreg(state);

    //
    // If we're asking for the logical mesh, return it.
    //
    if(strcmp(var, "logical_mesh") == 0)
    {
        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
        int dims[3] = {kmax, lmax, 1};
        int ndims = 2;
        vtkFloatArray *coords[3];

        for (i = 0 ; i < 3 ; i++)
        {
            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();

            if (i < ndims)
            {
                coords[i]->SetNumberOfTuples(dims[i]);
                for (int j = 0; j < dims[i]; j++)
                    coords[i]->SetComponent(j, 0, j);
            }
            else
            {
                dims[i] = 1;
                coords[i]->SetNumberOfTuples(1);
                coords[i]->SetComponent(0, 0, 0.);
            }
        }

        rgrid->SetDimensions(dims);
        rgrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rgrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rgrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        //
        // Mark some cells as ghost zones so they don't show up.
        //    
        CreateGhostZones(ireg, rgrid);

        return rgrid;
    }

    //
    // Read the variables that give the positions of the nodes.
    //
    TRY
    {
        ReadVariable(rtVar);
        ReadVariable(ztVar);
    }
    CATCH(InvalidVariableException)
    {
        EXCEPTION1(InvalidVariableException, var);
    }
    ENDTRY

    VariableData *rtData = varStorage[rtVar];
    VariableData *ztData = varStorage[ztVar];
    if(rtData->dataType != DOUBLEARRAY_TYPE ||
       ztData->dataType != DOUBLEARRAY_TYPE)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // Go to the right time state
    //
    const double *rt = (const double *)rtData->data;
    rt += ((state < nCycles) ? (state * nnodes) : 0);
    const double *zt = (const double *)ztData->data;
    zt += ((state < nCycles) ? (state * nnodes) : 0);

    //
    // Both the unstructured grid and the structured grid need the points
    // populated in the same way so do it beforehand.
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);

    //
    // The minimum value for R is way off the charts so we should take care
    // to not actually use it as a node coordinate since it would make the
    // extents be wrong. Find the maximum R value and use that everywhere we
    // get the minR value.
    //
    const double *rt2 = rt;
    const double *zt2 = zt;
    const double minR = -1e8;
    float maxR = float(minR);
    for(int k = 0; k < nnodes; ++k, ++rt2)
        if(*rt2 > maxR) maxR = *rt2;
    rt2 = rt;

    //
    // Store the points for the mesh.
    //
    float *ptr = (float *)points->GetVoidPointer(0);
    for(j = 0; j < lmax; ++j)
    { 
        for(i = 0; i < kmax; ++i)
        {
            *ptr++ = float(*zt2++);
            if(*rt2 == minR)
                *ptr++ = maxR;
            else
                *ptr++ = float(*rt2);
            ++rt2;
            *ptr++ = 0.f;
        }
    }

    vtkDataSet *retval = 0;
    if(strcmp(var, "revolved_mesh") == 0)
    {
        //
        // We want a revolved mesh so create an unstructured grid and
        // revolve it. We can't just revolve the structured grid because
        // it contains so many ghost zones that we don't get all of the
        // faces that we need.
        //
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(points);
        points->Delete();
        int nzones = (kmax - 1) * (lmax - 1);
        ugrid->Allocate(nzones);

        //
        // Add the zones that are not ghost zones.
        //
        for(j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            for(i = 1; i < kmax; ++i)
            {
                // Add it if it's a real zone.
                if(ireg_row[i] > 0)
                {
                    vtkIdType quad[4];
                    quad[0] = j * kmax + i;
                    quad[1] = j * kmax + i - 1;
                    quad[2] = (j-1) * kmax + i - 1;
                    quad[3] = (j-1) * kmax + i;
                    ugrid->InsertNextCell(VTK_QUAD, 4, quad);
                }
            }
        }

        //
        // Revolve the unstructured grid.
        //
        double axis[3] = {1., 0., 0.};
        retval = RevolveDataSet(ugrid, axis, 0., 360., revolutionSteps);
        ugrid->Delete();
    }
    else
    {
        //
        // Create a structured grid that uses the points array.
        //
        vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
        sgrid->SetPoints(points);
        points->Delete();

        //
        // Tell the grid what its dimensions are.
        //
        int dims[3];
        dims[0] = kmax;
        dims[1] = lmax;
        dims[2] = 1;
        sgrid->SetDimensions(dims);

        //
        // Mark some cells as ghost zones so they don't show up.
        //    
        CreateGhostZones(ireg, sgrid);
        retval = sgrid;
    }

    return retval;
}

// ****************************************************************************
// Method: PP_ZFileReader::ReadVariable
//
// Purpose: 
//   Reads in a variable and caches it in the varStorage map.
//
// Arguments:
//   varName : The name of the variable to be read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 16:06:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::ReadVariable(const std::string &varStr)
{
    VariableDataMap::iterator pos;

    if((pos = varStorage.find(varStr)) != varStorage.end())
    {
        if(!pos->second->ReadValues(this))
        {
            delete pos->second;
            varStorage.erase(pos);
            EXCEPTION1(InvalidVariableException, varStr);
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varStr);
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::ReadMixvarAndCache
//
// Purpose: 
//   Reads in a mixvar for the specified variable and adds it to the generic
//   database's variable cache.
//
// Arguments:
//   varStr : The name of the variable for which we want a mixed var array.
//   state  : The timestate that we want.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 7 16:41:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_ZFileReader::ReadMixvarAndCache(const std::string &varStr, int state)
{
// Disable mixvars for now.
#if 0
    VariableData *data = varStorage[varStr];

    std::string suffix(data->varName.substr(varStr.size()));
    std::string mixedVar(varStr + "mm");
    std::string mixedVarWithSuffix(mixedVar + suffix);

    if(SymbolExists(mixedVarWithSuffix.c_str()))
    {
        //
        // If the mixedVar is not cached, add an empty cache entry.
        //
        if(varStorage.find(mixedVar) == varStorage.end())
        {
            VariableData *v = new VariableData(mixedVarWithSuffix);
            varStorage[mixedVar] = v;
        }

        TRY
        {
            //
            // Try and read the mixed variable array.
            //
            ReadVariable(mixedVar);

            //
            // Translate the variable into a float array.
            //
            data = varStorage[mixedVar];
            float *mixvar = new float[data->nTotalElements];
            if(data->dataType == INTEGERARRAY_TYPE)
            {
                const int *iptr = (const int *)data->data;
                for(int i = 0; i < data->nTotalElements; ++i)
                    mixvar[i] = float(iptr[i]);
            }
            else if(data->dataType == DOUBLEARRAY_TYPE)
            {
                const double *iptr = (const double *)data->data;
                for(int i = 0; i < data->nTotalElements; ++i)
                    mixvar[i] = float(iptr[i]);
            }
            else
            {
                debug4 << "PP_ZFileReader::ReadMixvarAndCache:"
                       << "Unsupported array type!" << endl;
                delete [] mixvar;
                mixvar = 0;
            }

            //
            // If we have a mixed variable array by this point, add it to
            // the generic database's cache.
            //
            if(mixvar)
            {
                avtMixedVariable *mv = new avtMixedVariable(mixvar,
                    data->nTotalElements, varStr);
                void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
                cache->CacheVoidRef(varStr.c_str(),
                    AUXILIARY_DATA_MIXED_VARIABLE, state, 0, vr);

                delete [] mixvar;
            }
        }
        CATCH(InvalidVariableException)
        {
            debug4 << "The mixvar array: " << mixedVarWithSuffix << " could "
                   << "not be read!" << endl;
        }
        ENDTRY
    }
    else
    {
        debug4 << "The mixvar array: " << mixedVarWithSuffix << " was not"
               << "found in the file." << endl;
    }
#endif
}

// ****************************************************************************
// Function: CopyVariableData
//
// Purpose:
//   This template function copies data from one destination array to memory
//   that is owned by a VTK object.
//
// Arguments:
//   ptr         : The destination array.
//   dataPointer : The source data array.
//   centering   : The variable centering, which indicates how it should be
//                 copied from the src to the destination array.
//   kmax        : Number of nodes in X.
//   lmax        : Number of nodes in Y.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:20:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
CopyVariableData(T *ptr, const T *dataPointer, avtCentering centering,
    int kmax, int lmax)
{
    int nnodes = kmax * lmax;

    if(centering == AVT_NODECENT)
    {
        // Nodal copy
        memcpy(ptr, dataPointer, sizeof(T) * nnodes);
    }
    else
    {
        // Zonal copy
        for(int j = 1; j < lmax; ++j)
        {
            const T *var_row = dataPointer + j * kmax;
            for(int i = 1; i < kmax; ++i)
                *ptr++ = var_row[i];
        }
    }
}

// ****************************************************************************
// Function: CopyRevolvedVariableData
//
// Purpose:
//   This template function copies data from one destination array to memory
//   that is owned by a VTK object.
//
// Arguments:
//   ptr         : The destination array.
//   dataPointer : The source data array.
//   ireg        : The file's ireg array.
//   centering   : The variable centering, which indicates how it should be
//                 copied from the src to the destination array.
//   kmax        : Number of nodes in X.
//   lmax        : Number of nodes in Y.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:20:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
CopyRevolvedVariableData(T *ptr, const T *dataPointer, const int *ireg,
    avtCentering centering, int kmax, int lmax, int revolutionSteps)
{
    int nnodes = kmax * lmax;

    if(centering == AVT_NODECENT)
    {
        // Nodal copy
        for(int k = 0; k < revolutionSteps; ++k)
        {
            memcpy(ptr, dataPointer, sizeof(T) * nnodes);
            ptr += nnodes;
        }
    }
    else
    {
        // Zonal copy
        for(int j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            const T *var_row = dataPointer + j * kmax;
            for(int i = 1; i < kmax; ++i)
            {
                if(ireg_row[i] > 0)
                {
                    for(int k = 0; k < revolutionSteps - 1; ++k)
                        *ptr++ = var_row[i];
                }
            }
        }
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetUnstructuredCellCount
//
// Purpose: 
//   Returns the number of cells that would be in an unstructured version of
//   the mesh.
//
// Returns:    The number of cells that would be in an unstructured version of
//             the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 8 16:30:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PP_ZFileReader::GetUnstructuredCellCount()
{
    if(unstructuredCellCount < 0)
    {
        const int *ireg = GetIreg(0);
        if(ireg)
        {
            unstructuredCellCount = 0;
            for(int j = 1; j < lmax; ++j)
            {
                const int *ireg_row = ireg + j * kmax;
                for(int i = 1; i < kmax; ++i)
                {
                    if(ireg_row[i] > 0)
                        ++unstructuredCellCount;
                }
            }
        }
    }

    return unstructuredCellCount;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetVar
//
// Purpose: 
//   Returns data for the specified variable and time state.
//
// Arguments:
//   state : The time state.
//   var   : The variable that we want to read.
//
// Returns:    A data array containing the data that we want.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 16:01:18 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 8 16:08:58 PST 2003
//   Added support for zonal variables and variables defined on the new
//   revolved mesh.
//
// ****************************************************************************

vtkDataArray *
PP_ZFileReader::GetVar(int state, const char *var)
{
    debug4 << "PP_ZFileReader::GetVar: state=" << state
           << ", var=" << var << endl;

    //
    // Make sure that everything is initialized.
    //
    Initialize();

    //
    // If we're asking for the variable over the logical mesh, it's really
    // the same variable so strip off the logical_mesh part of the name.
    //
    std::string varStr(var);
    std::string meshName("mesh/");
    std::string logical("logical_mesh/");
    std::string revolved("revolved_mesh/");
    bool wantRevolvedMesh = false;
    if(varStr.substr(0, meshName.size()) == meshName)
        varStr = varStr.substr(meshName.size());
    else if(varStr.substr(0, logical.size()) == logical)
        varStr = varStr.substr(logical.size());
    else if(varStr.substr(0, revolved.size()) == revolved)
    {
        varStr = varStr.substr(revolved.size());
        wantRevolvedMesh = true;
    }

    //
    // Read in the data for the variable if it hasn't been read yet.
    //
    ReadVariable(varStr);

    //
    // Determine the variable centering and use that to figure out how
    // large to allocate the data array.
    //
    int nels;
    int nnodes = kmax * lmax;
    int nzones = (kmax - 1) * (lmax - 1);
    avtCentering centering;
    const int *ireg = 0;
    if(VariableIsNodal(varStr))
    {
        nels = wantRevolvedMesh ? (nnodes * revolutionSteps) : nnodes;
        centering = AVT_NODECENT;
    }
    else
    {
        centering = AVT_ZONECENT;
        if(wantRevolvedMesh)
        {
            int size = GetUnstructuredCellCount();
            if(size < 0)
                EXCEPTION1(InvalidVariableException, varStr);

            ireg = GetIreg(state);
            nels = size * (revolutionSteps - 1);
        }
        else
            nels = nzones;
    }

    //
    // Now that the variable is in memory, put it in a VTK data structure.
    //
    vtkDataArray *scalars = 0;
    VariableData *data = varStorage[varStr];

    if(data->dataType == INTEGERARRAY_TYPE)
    {
        vtkIntArray *iscalars = vtkIntArray::New();
        iscalars->SetNumberOfTuples(nels);
        scalars = iscalars;
        int *ptr = (int *) iscalars->GetVoidPointer(0);

        // Get a pointer to the start of the VariableData object's data array.
        const int *dataPointer = (const int *)data->data;
        dataPointer += ((state < nCycles) ? (state * nnodes) : 0);

        //
        // Copy the data appropriately.
        //
        if(wantRevolvedMesh)
        {
            CopyRevolvedVariableData(ptr, dataPointer, ireg, centering,
                                     kmax, lmax, revolutionSteps);
        }
        else
            CopyVariableData(ptr, dataPointer, centering, kmax, lmax);

        ReadMixvarAndCache(varStr, state);
    }
    else if(data->dataType == DOUBLEARRAY_TYPE)
    {
        vtkDoubleArray *dscalars = vtkDoubleArray::New();
        dscalars->SetNumberOfTuples(nels);
        scalars = dscalars;
        double *ptr = (double *) dscalars->GetVoidPointer(0);

        // Get a pointer to the start of the VariableData object's data array.
        const double *dataPointer = (const double *)data->data;
        dataPointer += ((state < nCycles) ? (state * nnodes) : 0);

        //
        // Copy the data appropriately.
        //
        if(wantRevolvedMesh)
        {
            CopyRevolvedVariableData(ptr, dataPointer, ireg, centering,
                                     kmax, lmax, revolutionSteps);
        }
        else
            CopyVariableData(ptr, dataPointer, centering, kmax, lmax);

        ReadMixvarAndCache(varStr, state);
    }
    else
    {
        debug4 << "GetVar: Unsupported data type: "
               << int(data->dataType) << endl;
    }

    return scalars;
}

// ****************************************************************************
// Method: PP_ZFileReader::GetVectorVar
//
// Purpose: 
//   Returns data for vector variables.
//
// Arguments:
//   state : The time state.
//   var   : The name of the variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 16:17:46 PST 2003
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
PP_ZFileReader::GetVectorVar(int state, const char *var)
{
    debug4 << "PP_ZFileReader::GetVectorVar: state=" << state
           << ", var=" << var << endl;

    Initialize();

    return NULL;
}

// ****************************************************************************
// Function: AddCleanMaterials
//
// Purpose:
//   Adds clean materials to the material encoder for the regular 2d mesh or
//   for the revolved mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 11 13:56:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

static void
AddCleanMaterials(MaterialEncoder &mats, const int *ireg, int kmax, int lmax,
    bool wantRevolvedMaterial, int nSteps)
{
    //
    // Add materials for all zones in a way that matches the mesh.
    //
    if(wantRevolvedMaterial)
    {
        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            for(int i = 1; i < kmax; ++i)
            {
                if(ireg_row[i] > 0)
                {
                    for(int k = 0; k < nSteps; ++k)
                        mats.AddClean(zoneId++, ireg_row[i]);
                }
            }
        }
    }
    else
    {
        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
            const int *ireg_row = ireg + j * kmax;
            for(int i = 1; i < kmax; ++i, ++zoneId)
                mats.AddClean(zoneId, ireg_row[i]);
        }
    }
}

// ****************************************************************************
// Function: AddMixedMaterials
//
// Purpose:
//   Adds mixed materials to the material encoder for the regular 2d mesh or
//   for the revolved mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 11 13:56:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

static void
AddMixedMaterials(MaterialEncoder &mats, const int *ireg, const int *iregmm,
    const int *nummm, const int *ilamm, const double *volfmm, int kmax,
    int lmax, bool wantRevolvedMaterial, int nSteps)
{
    //
    // Add materials for all zones in a way that matches the mesh.
    //
    if(wantRevolvedMaterial)
    {
        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
             const int *nummm_row = nummm + j * kmax;
             const int *ireg_row = ireg + j * kmax;
             const int *ilamm_row = ilamm + j * 2 * kmax;
             for(int i = 1; i < kmax; ++i)
             {
                 if(ireg_row[i] > 0)
                 {
                     if(nummm_row[i] == 0)
                     {
                         for(int k = 0; k < nSteps; ++k)
                             mats.AddClean(zoneId++, ireg_row[i]);
                     }
                     else
                     {
                         int minIndex = ilamm_row[2 * i];
                         for(int k = 0; k < nSteps; ++k)
                             mats.AddMixed(zoneId++,
                                           iregmm + minIndex,
                                           volfmm + minIndex,
                                           nummm_row[i]);
                     }
                 }
             }
        }
    }
    else
    {
        int zoneId = 0;
        for(int j = 1; j < lmax; ++j)
        {
             const int *nummm_row = nummm + j * kmax;
             const int *ireg_row = ireg + j * kmax;
             const int *ilamm_row = ilamm + j * 2 * kmax;
             for(int i = 1; i < kmax; ++i, ++zoneId)
             {
                 if(nummm_row[i] == 0)
                     mats.AddClean(zoneId, ireg_row[i]);
                 else
                 {
                     int minIndex = ilamm_row[2 * i];
                     mats.AddMixed(zoneId,
                                   iregmm + minIndex,
                                   volfmm + minIndex,
                                   nummm_row[i]);
                 }
             }
        }
    }
}

// ****************************************************************************
// Method: PP_ZFileReader::GetAuxiliaryData
//
// Purpose: 
//   Returns auxiliary data such as materials.
//
// Arguments:
//   var   : The name of the variable for which to return auxiliary data.
//   state : The time state that we're interested in.
//   type  : The type of aux data that we want.
//   args  : ?
//   df    : The function to use when destroying the returned data.
//
// Returns:    An object that contains auxiliary data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 5 17:35:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void  *
PP_ZFileReader::GetAuxiliaryData(const char *var, int state, const char *type,
    void *args, DestructorFunction &df)
{
    avtMaterial *retval = 0;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        df = avtMaterial::Destruct;

        //
        // Initialize some variables based on the variable name.
        //
        int  nnodes = kmax * lmax;
        int  nzones = (kmax-1)*(lmax-1);
        bool validVariable = false;
        bool wantRevolvedMaterial = false;
        int  nSteps = 1;
        int  nCleanZones = nzones;
        if(strcmp(var, "material") == 0)
        {
            validVariable = true;
        }
        else if(strcmp(var, "material2") == 0)
        {
            wantRevolvedMaterial = true;
            validVariable = true;
            nSteps = revolutionSteps - 1;
            nCleanZones = GetUnstructuredCellCount() * nSteps;
        }
           
        if(validVariable)
        {
            std::string varStr("ireg");
            VariableData *data = varStorage[varStr];
            const int *ireg = 0;
            if(data->dataType == INTEGERARRAY_TYPE)
            {

                // Get a pointer to ireg's data at the right time state.
                ireg = (const int *)data->data;
                ireg += ((state < nCycles) ? (state * nnodes) : 0);

                //
                // Try and read the mixed material variables.
                //
                if(assumeMixedMaterialsPresent)
                {
                    TRY
                    {
                        ReadVariable("nummm");
                        ReadVariable("ilamm");
                        ReadVariable("volfmm");
                        ReadVariable("iregmm");
                    }
                    CATCH(InvalidVariableException)
                    {
                        assumeMixedMaterialsPresent = false; 
                    }
                    ENDTRY
                }

                //
                // Create a MaterialEncoder object and define materials.
                //
                MaterialEncoder mats;
                mats.AllocClean(nCleanZones);
                for(int i = 0; i < materialNames.size(); ++i)
                    mats.AddMaterial(materialNames[i]);

                //
                // Populate the MaterialEncoder object and define materials.
                //
                if(assumeMixedMaterialsPresent)
                {
                    VariableData *nummm_data  = varStorage["nummm"];
                    VariableData *ilamm_data  = varStorage["ilamm"];
                    VariableData *volfmm_data = varStorage["volfmm"];
                    VariableData *iregmm_data = varStorage["iregmm"];

                    if(nummm_data->dataType == INTEGERARRAY_TYPE &&
                       ilamm_data->dataType == INTEGERARRAY_TYPE &&
                       volfmm_data->dataType == DOUBLEARRAY_TYPE &&
                       iregmm_data->dataType == INTEGERARRAY_TYPE)
                    {
                        const int *nummm = (const int *)nummm_data->data;
                        nummm += ((state < nCycles) ? (state * nnodes) : 0);

                        const int *ilamm = (const int *)ilamm_data->data;
                        ilamm += ((state < nCycles) ? (state * 2 * nnodes) : 0);

                        const double *volfmm = (const double *)volfmm_data->data;
                        const int *iregmm = (const int *)iregmm_data->data;

                        //
                        // Add materials for all of the zones. If the nummm
                        // value for the zone is zero, then there are no
                        // mixed materials in the zone. Otherwise, there are
                        // mixed materials and we use the ilamm value for the
                        // zone to index into the volfmm and iregmm arrays
                        // to get the volume fraction and material numbers.
                        //
                        AddMixedMaterials(mats, ireg, iregmm, nummm, ilamm,
                                          volfmm, kmax, lmax,
                                          wantRevolvedMaterial, nSteps);
                    }
                    else
                    {
                        debug4 << "PP_ZFileReader::GetAuxiliaryData: "
                               << "Some of the mixed material arrays had the "
                               << "wrong types. Making all zones clean."
                               << endl;

                        //
                        // Add materials for all of the zones but make them
                        // all clean.
                        //
                        AddCleanMaterials(mats, ireg, kmax, lmax,
                                          wantRevolvedMaterial, nSteps);
                    }
                }
                else
                {
                    //
                    // Add materials for all of the zones but make them
                    // all clean.
                    //
                    AddCleanMaterials(mats, ireg, kmax, lmax,
                                      wantRevolvedMaterial, nSteps);
                }

                //
                // Create an avtMaterial using the MaterialEncoder object.
                //
                if(wantRevolvedMaterial)
                {
                    int dims[3] = {nCleanZones, 1, 1};
                    retval = mats.CreateMaterial(dims, 3);
                }
                else
                {
                    int dims[2] = {kmax-1, lmax-1};
                    retval = mats.CreateMaterial(dims, 2);
                }
            }
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: PP_ZFileReader::RevolveDataSet
//
//  Purpose:
//      Sends the specified input and output through the Revolve filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Wed Dec 11 11:31:52 PDT 2002
//
//  Modifications:
//     Brad Whitlock, Wed Aug 6 12:14:51 PDT 2003
//     I stole this code from avtRevolveFilter in the Revolve operator.
//
// ****************************************************************************

vtkDataSet *
PP_ZFileReader::RevolveDataSet(vtkDataSet *in_ds, const double *axis,
    double start_angle, double stop_angle, int nsteps)
{
    int   i, j;

    //
    // Set up our VTK structures.
    //
    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    int npts = in_ds->GetNumberOfPoints();
    int ncells = in_ds->GetNumberOfCells();
    int n_out_pts = npts*nsteps;
    pts->SetNumberOfPoints(n_out_pts);
    ugrid->SetPoints(pts);

    //
    // Create the points for each timestep.
    //
    int niter = (fabs(stop_angle-start_angle-360.) < 0.001 ? nsteps-1 : nsteps);
    float *ptr = (float *) pts->GetVoidPointer(0);
    for (i = 0 ; i < niter ; i++)
    {
        double angle = ((stop_angle-start_angle)*i)/(nsteps-1) + start_angle;
        GetRotationMatrix(angle, axis, mat);
        for (j = 0 ; j < npts ; j++)
        {
            float pt[4];
            in_ds->GetPoint(j, pt);
            pt[3] = 1.;
            float outpt[4];
            mat->MultiplyPoint(pt, outpt);
            ptr[0] = outpt[0];
            ptr[1] = outpt[1];
            ptr[2] = outpt[2];
            ptr += 3;
        }
    }

    //
    // Now set up the connectivity.  The output will consist of revolved
    // quads (-> hexes) and revolved triangles (-> wedges).  No special care is
    // given to the case where an edge of a cell lies directly on the axis of
    // revolution (ie: you get a degenerate hex, not a wedge).
    //
    int n_out_cells = ncells*(nsteps-1);
    ugrid->Allocate(8*n_out_cells);
    bool overlap_ends = (fabs(stop_angle-start_angle-360.) < 0.001);
    for (i = 0 ; i < ncells ; i++)
    {
         vtkCell *cell = in_ds->GetCell(i);
         int c = cell->GetCellType();
         if (c != VTK_QUAD && c != VTK_TRIANGLE && c != VTK_PIXEL)
         {
             EXCEPTION1(InvalidCellTypeException, "anything but quads and"
                                                  " tris.");
         }
         vtkIdList *list = cell->GetPointIds();

         if (c == VTK_TRIANGLE)
         {
             int pt0 = list->GetId(0);
             int pt1 = list->GetId(1);
             int pt2 = list->GetId(2);
             for (j = 0 ; j < nsteps-1 ; j++)
             {
                 vtkIdType wedge[6];
                 wedge[0] = npts*j + pt0;
                 wedge[1] = npts*j + pt1;
                 wedge[2] = npts*j + pt2;
                 wedge[3] = npts*(j+1) + pt0;
                 wedge[4] = npts*(j+1) + pt1;
                 wedge[5] = npts*(j+1) + pt2;
                 if (j == nsteps-2 && overlap_ends)
                 {
                     wedge[3] = pt0;
                     wedge[4] = pt1;
                     wedge[5] = pt2;
                 }
                 ugrid->InsertNextCell(VTK_WEDGE, 6, wedge);
             }
         }
         else
         {
             int pt0 = list->GetId(0);
             int pt1 = list->GetId(1);
             int pt2 = list->GetId(2);
             int pt3 = list->GetId(3);
             if (c == VTK_PIXEL)
             {
                 pt2 = list->GetId(3);
                 pt3 = list->GetId(2);
             }
             for (j = 0 ; j < nsteps-1 ; j++)
             {
                 vtkIdType hex[6];
                 hex[0] = npts*j + pt0;
                 hex[1] = npts*j + pt1;
                 hex[2] = npts*j + pt2;
                 hex[3] = npts*j + pt3;
                 hex[4] = npts*(j+1) + pt0;
                 hex[5] = npts*(j+1) + pt1;
                 hex[6] = npts*(j+1) + pt2;
                 hex[7] = npts*(j+1) + pt3;
                 if (j == nsteps-2 && overlap_ends)
                 {
                     hex[4] = pt0;
                     hex[5] = pt1;
                     hex[6] = pt2;
                     hex[7] = pt3;
                 }
                 ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, hex);
             }
         }
    }

    vtkCellData *incd   = in_ds->GetCellData();
    vtkCellData *outcd  = ugrid->GetCellData();
    outcd->CopyAllocate(incd, n_out_cells);
    for (i = 0 ; i < n_out_cells ; i++)
    {
        outcd->CopyData(incd, i/(nsteps-1), i);
    }
    
    vtkPointData *inpd  = in_ds->GetPointData();
    vtkPointData *outpd = ugrid->GetPointData();
    outpd->CopyAllocate(inpd, n_out_pts);
    for (i = 0 ; i < n_out_pts ; i++)
    {
        outpd->CopyData(inpd, i%npts, i);
    }
    
    //
    // Clean up.
    //
    mat->Delete();
    pts->Delete();

    return ugrid;
}

// ****************************************************************************
//  Function: GetRotationMatrix
//
//  Purpose:
//      Given an angle and an axis, this creates a matrix that will rotate a
//      point around that axis.
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Aug 6 12:16:01 PDT 2003
//    I stole this code from avtRevolveFilter in the Revolve operator.
//
// ****************************************************************************

void
PP_ZFileReader::GetRotationMatrix(double angle, const double axis[3],
    vtkMatrix4x4 *mat)
{
    //
    // The game plan is to transform into a coordinate space that we are
    // familiar with, perform the rotation there, and then rotate back.
    //

    //
    // First rotate to the yz plane.  We will do this by rotating by theta
    // around the y-axis, where theta is arctan(axis[0] / axis[2]).
    //
    vtkMatrix4x4 *rot1 = vtkMatrix4x4::New();
    rot1->Identity();
    vtkMatrix4x4 *rot5 = vtkMatrix4x4::New();
    rot5->Identity();
    if (axis[0] != 0.)
    {
        double theta = atan2(axis[0], axis[2]);
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);
        rot1->SetElement(0, 0, cos_theta);
        rot1->SetElement(2, 0, -sin_theta);
        rot1->SetElement(0, 2, sin_theta);
        rot1->SetElement(2, 2, cos_theta);

        //
        // Rot 5 will be a rotation around -theta.
        //
        double cos_minus_theta = cos_theta;
        double sin_minus_theta = -sin_theta;
        rot5->SetElement(0, 0, cos_minus_theta);
        rot5->SetElement(2, 0, -sin_minus_theta);
        rot5->SetElement(0, 2, sin_minus_theta);
        rot5->SetElement(2, 2, cos_minus_theta);
    }

    //
    // Now rotate around the x-axis until we get the to the z-axis.
    //
    vtkMatrix4x4 *rot2 = vtkMatrix4x4::New();
    rot2->Identity();
    vtkMatrix4x4 *rot4 = vtkMatrix4x4::New();
    rot4->Identity();
    if (axis[1] != 0.)
    {
        double theta = atan2(axis[1], sqrt(axis[0]*axis[0] + axis[2]*axis[2]));
        double cos_theta = cos(theta);
        double sin_theta = sin(theta);
        rot2->SetElement(1, 1, cos_theta);
        rot2->SetElement(2, 1, sin_theta);
        rot2->SetElement(1, 2, -sin_theta);
        rot2->SetElement(2, 2, cos_theta);

        //
        // Rot 4 will be a rotation around -theta.
        //
        double cos_minus_theta = cos_theta;
        double sin_minus_theta = -sin_theta;
        rot4->SetElement(1, 1, cos_minus_theta);
        rot4->SetElement(2, 1, sin_minus_theta);
        rot4->SetElement(1, 2, -sin_minus_theta);
        rot4->SetElement(2, 2, cos_minus_theta);
    }

    //
    // Now we can do the easy rotation around the z-axis.
    //
    double angle_rad = (angle / 360. * 2. * M_PI);
    vtkMatrix4x4 *rot3 = vtkMatrix4x4::New();
    rot3->Identity();
    double cos_angle = cos(angle_rad);
    double sin_angle = sin(angle_rad);
    rot3->SetElement(0, 0, cos_angle);
    rot3->SetElement(1, 0, sin_angle);
    rot3->SetElement(0, 1, -sin_angle);
    rot3->SetElement(1, 1, cos_angle);

    //
    // Now set up our matrix.
    //
    vtkMatrix4x4 *tmp  = vtkMatrix4x4::New();
    vtkMatrix4x4 *tmp2 = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(rot5, rot4, tmp);
    vtkMatrix4x4::Multiply4x4(tmp, rot3, tmp2);
    vtkMatrix4x4::Multiply4x4(tmp2, rot2, tmp);
    vtkMatrix4x4::Multiply4x4(tmp, rot1, mat);

    tmp->Delete();
    tmp2->Delete();
    rot1->Delete();
    rot2->Delete();
    rot3->Delete();
    rot4->Delete();
    rot5->Delete();
}


