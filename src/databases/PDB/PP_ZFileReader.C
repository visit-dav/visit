#include <PP_ZFileReader.h>
#include <InvalidVariableException.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <avtDatabaseMetaData.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: PP_ZFileReader::PP_ZFileReader
//
// Purpose: 
//   Constructor for the PP_ZFileReader class.
//
// Arguments:
//   pdb : The pdb file to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:00:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PP_ZFileReader::PP_ZFileReader(PDBfile *pdb) : PDBReader(pdb), cycles(),
    varStorage()
{
    kmax = lmax = 0;

    cycles = 0;
    nCycles = 0;
    times = 0;
    nTimes = 0;

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

    validFile = have_kmax && have_lmax;

    debug4 << "PP_ZFileReader::Identify()" << endl;
    debug4 << "\tkmax@value=" << kmax << endl;
    debug4 << "\tlmax@value=" << lmax << endl;
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
//   
// ****************************************************************************

void
PP_ZFileReader::Initialize()
{
    if(!initialized)
    {
        // Read the cycles
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

        // Read the time array.
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

        initialized = true;
    }
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

    float extents[4];
    int cellOrigin = 1;
    int ndims = 2;

    // Make up some extents since the file does not have them.
    extents[0] = 0.;
    extents[1] = float(kmax);
    extents[2] = 0.;
    extents[3] = float(lmax);

    // Create a mesh to use for the data.
    avtMeshMetaData *mmd = new avtMeshMetaData(extents,
        "mesh", 1, 0, cellOrigin, ndims, ndims, AVT_RECTILINEAR_MESH);
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
                    debug4 << " added as " << newCStr << endl;

                    // Add the variable to the metadata.
                    avtScalarMetaData *smd = new avtScalarMetaData(
                        newStr, "mesh", AVT_ZONECENT);
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
// Method: PP_ZFileReader::GetMesh
//
// Purpose: 
//   Returns the mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:58:50 PST 2003
//
// Modifications:
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
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
    int dims[3] = {kmax + 1, lmax + 1, 1};
    int ndims = 2;
    vtkFloatArray *coords[3];

    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();

        if (i < ndims)
        {
            coords[i]->SetNumberOfTuples(dims[i]);
            for (int j = 0; j < dims[i]; j++)
            {
                coords[i]->SetComponent(j, 0, j);
            }
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

    return rgrid;
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
    // Read in the data for the variable if it hasn't been read yet.
    //
    std::string varStr(var);
    ReadVariable(varStr);

    //
    // Now that the variable is in memory, put it in a VTK data structure.
    //
    int nels = kmax * lmax;
    vtkDataArray *scalars = 0;
    VariableData *data = varStorage[varStr];

    if(data->dataType == INTEGERARRAY_TYPE)
    {
        vtkIntArray *iscalars = vtkIntArray::New();
        iscalars->SetNumberOfTuples(nels);

        // Get a pointer to the start of the VariableData object's data array.
        const int *dataPointer = (const int *)data->data;

        // Advance the pointer to the proper time state.
        dataPointer += ((state < nCycles) ? (state * nels) : 0);

        // Copy the data from the VariableData object's data array into the
        // vtkIntArray object's data array.
        int *ptr = (int *) iscalars->GetVoidPointer(0);
        memcpy(ptr, dataPointer, sizeof(int) * nels);
        scalars = iscalars;
    }
    else if(data->dataType == DOUBLEARRAY_TYPE)
    {
        vtkDoubleArray *dscalars = vtkDoubleArray::New();
        dscalars->SetNumberOfTuples(nels);

        // Get a pointer to the start of the VariableData object's data array.
        const double *dataPointer = (const double *)data->data;

        // Advance the pointer to the proper time state.
        dataPointer += ((state < nCycles) ? (state * nels) : 0);

        // Copy the data from the VariableData object's data array into the
        // vtkIntArray object's data array.
        double *ptr = (double *) dscalars->GetVoidPointer(0);
        memcpy(ptr, dataPointer, sizeof(double) * nels);
        scalars = dscalars;
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
