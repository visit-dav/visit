// ***************************************************************************
//                           avtCosmosFileFormat.C                           
// ***************************************************************************


#include <avtCosmosFileFormat.h>

#include <ctype.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <BadDomainException.h>
#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>
#include <InvalidVariableException.h>

#include <hdf.h>
#include <mfhdf.h>

using std::vector;
using std::string;

static string GetDirName(const char *path);

//
// A certain number of ghost zones are added in each dimension.
// We only use some of them, and only one layer, whereas COSMOS
// has two layers in each dimension, reguardless of use.
//

const int GHOST_PADDING = 2;
const int THROWOUT_DIM = (2 * GHOST_PADDING);

// ****************************************************************************
//  Constructor:  avtCosmosFileFormat::avtCosmosFileFormat
//
//  Arguments:
//    fname      the file name of the master Cosmos file
//
//  Programmer:  Akira Haddox
//  Creation:    June 4, 2003
//
//  Modifications:
//    Akira Haddox, Fri Jun 13 11:16:41 PDT 2003
//    Added in meshInfo structures.
//
//    Akira Haddox, Mon Jun 16 12:36:11 PDT 2003
//    Added in for cylindrical coordinates, and other 2D configurations.
//    Added in ghostZone support.
//
// ****************************************************************************

avtCosmosFileFormat::avtCosmosFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    // This value stays -1 if rank == 3
    dropDimension = -1;
    
    dirname = GetDirName(fname);

    ifstream ifile(fname);

    if (ifile.fail())
    {
        EXCEPTION1(InvalidDBTypeException, "Unable to open cosmos file.");
    }
    
    ntimesteps = ndomains = dimensions[0] = -1;

    existGhostZones = false;
    readInTimes = false;
    
    int i;

    //
    // Parse the master file to find out the information that we need.
    // We're not too picky about the format as long as the necessary
    // parts are there, and that #DOMAINS is defined before the
    // grid and variable files.
    //

    for (;;)
    {
        string key;
        ReadString(ifile, key);

        if (ifile.eof() || ifile.fail())
                break;
        
        if (key == "#COORDINATES")
        {
            string value;
            ReadString(ifile, value);
            if (value == "spherical")
                coordType = spherical;
            else if (value == "cylindrical")
                coordType = cylindrical;
            else
                coordType = cartesian;
        }
        else if (key == "#DOMAINS")
        {
            ifile >> ndomains;
            gridFileNames.resize(ndomains);
            scalarFileNames.resize(ndomains);
            vectorFileNames.resize(ndomains);
            ghostSet.resize(ndomains);
            
            for (i = 0; i < ndomains; ++i)
                ghostSet[i].resize(6, false);
        }
        else if (key == "#TIMESTEPS")
        {
            ifile >> ntimesteps;
        }
        else if (key == "#TIMEFILE")
        {
            ReadString(ifile, timeFileName);
            timeFileName = dirname + timeFileName;
        }
        else if (key == "#DIMENSIONS")
        {
            ifile >> dimensions[0] >> dimensions[1] >> dimensions[2]; 
            
            rank = 3;
            
            for (i = 0; i < 3; ++i)
            {
                if (dimensions[i] == 1)
                {
                    dropDimension = i;
                    rank = 2;
                }
                else
                    dimensions[i] -= THROWOUT_DIM;
            }
        }
        else if (key == "#GRID")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            string value;
            for (i = 0; i < ndomains; ++i)
            {
                ReadString(ifile, value);
                gridFileNames[i] = dirname + value;
            }
        }
        else if (key == "#SCALAR")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            // Strip out leading white space.
            while (isspace(ifile.peek()))
                ifile.get();

            string value;
            char line[1024];
            ifile.getline(line, 1024);
            value = line;

            scalarVarNames.push_back(value);

            for (i = 0; i < ndomains; ++i)
            {
                ReadString(ifile, value);
                scalarFileNames[i].push_back(dirname + value);
            }
        }
        else if (key == "#VECTOR")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            // Strip out leading white space.
            while(isspace(ifile.peek()))
                ifile.get();

            string value;
            char line[1024];
            ifile.getline(line, 1024);
            value = line;

            int index = vectorVarNames.size();
            vectorVarNames.push_back(value);
            
            
            for (i = 0; i < ndomains; ++i)
            {
                vectorFileNames[i].push_back(TripleString());
                ReadString(ifile, value);
                vectorFileNames[i][index].x = dirname + value;
            }
            for (i = 0; i < ndomains; ++i)
            {
                ReadString(ifile, value);
                vectorFileNames[i][index].y = dirname + value;
            }

            if (rank == 3)
            { 
                for (i = 0; i < ndomains; ++i)
                {
                    ReadString(ifile, value);
                    vectorFileNames[i][index].z = dirname + value;
                }
            }
        }
        else if (key == "#GHOSTZONES")
        {
            for (i = 0; i < ndomains; ++i)
            {
                int j;
                for (j = 0; j < 6; ++j)
                {
                    int b;
                    ifile >> b;
                    ghostSet[i][j] = (b != 0);
                    if (ghostSet[i][j])
                        existGhostZones = true;
                }
            }

            //
            // Right now, we only support ghost zones in 3D.
            // 
            if (rank != 3)
                existGhostZones = false;
        }
        else
            EXCEPTION1(InvalidDBTypeException, "Badly formed cosmos file.");
    }

    ifile.close();
    
    if (ntimesteps == -1)
        EXCEPTION1(InvalidDBTypeException, "Number of timesteps not defined.");
    if (ndomains == -1)
        EXCEPTION1(InvalidDBTypeException, "Number of domains not defined.");
    if (dimensions[0] == -1)
        EXCEPTION1(InvalidDBTypeException, "Dimensions not defined.");

    //
    // Check for incompatible configurations.
    //
    if (coordType == spherical && dropDimension == 0)
        EXCEPTION1(InvalidDBTypeException, "Bad spherical configuration.");
    if (coordType == cylindrical && rank == 3)
        EXCEPTION1(InvalidDBTypeException, "Bad cylindrical configuration.");

    nscalars = scalarVarNames.size();
    nvectors = vectorVarNames.size();

    meshes = new vtkDataSet *[ndomains];
    for (i = 0; i < ndomains; ++i)
        meshes[i] = NULL;
    meshInfo.resize(ndomains);
    meshInfoLoaded.resize(ndomains, false);
}


// ****************************************************************************
//  Destructor:  avtCosmosFileFormat::~avtCosmosFileFormat
//
//  Programmer:  Akira Haddox
//  Creation:    June  4, 2003
//
// ****************************************************************************

avtCosmosFileFormat::~avtCosmosFileFormat()
{
    if (meshes)
    {
        int i;
        for (i = 0; i < ndomains; ++i)
            if (meshes[i])
                meshes[i]->Delete();
        delete[] meshes;
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given time step and
//    domain.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Akira Haddox
//  Creation:    June 4, 2003
//
//  Modifications:
//    Akira Haddox, Wed Jul 23 08:13:32 PDT 2003
//    Added in call to read in times.
//
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    if (!readInTimes)
        ReadInTimes();
    
    if (strcmp(mesh, "mesh"))
        EXCEPTION1(InvalidVariableException, mesh);

    if (ts < 0 || ts >= ntimesteps)
        EXCEPTION2(InvalidTimeStepException, ts, ntimesteps);
    
    if (dom < 0 || dom >= ndomains)
        EXCEPTION2(BadDomainException, dom, ndomains);

    if (meshes[dom] == NULL)
        ReadMesh(dom);
    
    meshes[dom]->Register(NULL);
    return meshes[dom];
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain. 
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Akira Haddox
//  Creation:    June 4, 2003
//
//  Modifications:
//    Akira Haddox, Wed Jul 16 15:33:48 PDT 2003
//    Added in ghostZone support.
//
// ****************************************************************************

vtkDataArray *
avtCosmosFileFormat::GetVar(int ts, int dom, const char *name)
{
    int varfId;
    for (varfId = 0; varfId < scalarVarNames.size(); ++varfId)
        if (scalarVarNames[varfId] == name)
            break;
    if (varfId >= scalarVarNames.size())
        EXCEPTION1(InvalidVariableException, name);

    debug5 << "Reading in var " << name << " for domain/ts : " << dom << ','
           << ts << endl;
    
    string fname = scalarFileNames[dom][varfId];
    
    int32   file_handle = SDstart(fname.c_str(), DFACC_READ);
    if (file_handle < 0)
    {
        EXCEPTION1(InvalidFilesException, fname.c_str());
    }

    //
    // Datasets begin with names counting at 2, and increment as
    // the timesteps go on.
    //
    
    char dsName[32];
    sprintf(dsName, "Data-Set-%d", ts + 2);

    int32 sds_index = SDnametoindex(file_handle, dsName);
    
    if (sds_index < 0)
    {
        SDend(file_handle);
        EXCEPTION1(InvalidFilesException, fname.c_str());
    }
    
    int32 var_handle = SDselect(file_handle, sds_index);
    if (var_handle < 0)
    {
        SDend(file_handle);
        EXCEPTION1(InvalidFilesException, fname.c_str());
    }

    //
    // Calculate the dimensions, considering the ghostZones that
    // we want to keep.
    //
    int dim[3];
    int i;
    for (i = 0; i < 3; ++i)
    {
        dim[i] = dimensions[i];
        if (existGhostZones)
        {
            if (ghostSet[dom][i * 2])
                ++(dim[i]);
            if (ghostSet[dom][i * 2 + 1])
                ++(dim[i]);
        }
    }
 
    //
    // Allocate the array that we will be returning.
    //
    
    vtkFloatArray * fa = vtkFloatArray::New();
    fa->SetNumberOfTuples(dim[0] * dim[1] * dim[2]);
    float *vars = fa->GetPointer(0);

    //
    // The dimensions of the data that we want is bigger than the dimensions
    // that we are using. So we need to read in and convert.
    //
    
    int32 start[3];
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    int32 realDims[3];
    realDims[0] = dimensions[0] + THROWOUT_DIM;
    realDims[1] = dimensions[1] + THROWOUT_DIM;
    realDims[2] = dimensions[2] + THROWOUT_DIM;

    if (rank == 2)
        realDims[dropDimension] = 1;

    //
    // HDF4 takes in the dimensions backwards from how we store them:
    // taking the most significant dimension first (z / phi).
    // 2D meshes are still stored in a 3D-array, with the most
    // significant dimension as 1.
    //
    
    int32 transDims[3];
    transDims[0] = realDims[2]; 
    transDims[1] = realDims[1]; 
    transDims[2] = realDims[0]; 
    
    float *data = new float[realDims[0] * realDims[1] * realDims[2]];

    SDreaddata(var_handle, start, NULL, transDims, data);

    //
    // Go through the data array and pull out the earlier dimensions
    // into our result array.
    //
    
    int x,y,z;

    int startI[3];
    int endI[3];

    for (i = 0; i < 3; ++i)
    {
        if (i == dropDimension)
        {
            startI[i] = 0;
            endI[i] = 1;
        }
        else
        {
            if (existGhostZones && ghostSet[dom][i * 2])
                startI[i] = 1;
            else
                startI[i] = GHOST_PADDING;

            if (existGhostZones && ghostSet[dom][i * 2 + 1])
                endI[i] = dimensions[i] + GHOST_PADDING + 1;
            else
                endI[i] = dimensions[i] + GHOST_PADDING;
        }
    }
    
    for (z = startI[2]; z < endI[2]; ++z)
    {
        for (y = startI[1]; y < endI[1]; ++y)
        {
            for (x = startI[0]; x < endI[0]; ++x)
            {
                *(vars++) = data[x + y * realDims[0]
                                   + z * realDims[0] * realDims[1]];
            }
        }
    }
    
    delete[] data;
    
    SDendaccess(var_handle);
    SDend(file_handle);
 
    return fa;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::GetVectorVar
//
//  Purpose:
//    Reads the vector variable with the given name for the given time step and
//    domain. 
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Akira Haddox
//  Creation:    June 4, 2003
//
//  Modifications:
//    Akira Haddox, Fri Jun 13 11:14:47 PDT 2003
//    Fixed dealing with spherical coordinate vectors.
//
//    Akira Haddox, Wed Jul 16 15:33:48 PDT 2003
//    Added in ghostZone support. Split reading in for different configuration
//    types into seperate methods.
//
// ****************************************************************************

vtkDataArray *
avtCosmosFileFormat::GetVectorVar(int ts, int dom, const char *name)
{
    int varfId;
    for (varfId = 0; varfId < scalarVarNames.size(); ++varfId)
        if(vectorVarNames[varfId] == name)
            break;
    if (varfId >= vectorVarNames.size())
        EXCEPTION1(InvalidVariableException, name);

    debug5 << "Reading in vector var " << name << " for domain/ts : "
           << dom << ',' << ts << endl;
    
    string fname[3];
    fname[0] = vectorFileNames[dom][varfId].x;
    fname[1] = vectorFileNames[dom][varfId].y;
    fname[2] = vectorFileNames[dom][varfId].z;
    
    int dim[3];
    int i;
    
    int startI[3];
    int endI[3];

    for (i = 0; i < 3; ++i)
    {
        if (i == dropDimension)
        {
            startI[i] = 0;
            endI[i] = 1;
            dim[i] = 1;
        }
        else
        {
            dim[i] = dimensions[i];
            if (existGhostZones && ghostSet[dom][i * 2])
            {
                ++(dim[i]);
                startI[i] = 1;
            }
            else
                startI[i] = GHOST_PADDING;

            if (existGhostZones && ghostSet[dom][i * 2 + 1])
            {
                ++(dim[i]);
                endI[i] = dimensions[i] + GHOST_PADDING + 1;
            }
            else
                endI[i] = dimensions[i] + GHOST_PADDING;
        }
    }
    
    
    float *values[3];
    values[0] = new float[dim[0] * dim[1] * dim[2]];
    values[1] = new float[dim[0] * dim[1] * dim[2]];
    if (rank == 3)
        values[2] = new float[dim[0] * dim[1] * dim[2]];

    //
    // Read in the xyz components.
    //

    for (i = 0; i < rank; ++i)
    {
        int32   file_handle = SDstart(fname[i].c_str(), DFACC_READ);
        if (file_handle < 0)
        {
            delete[] values[0];
            delete[] values[1];
            delete[] values[2];
            EXCEPTION1(InvalidFilesException, fname[i].c_str());
        }

        char dsName[32];
        sprintf(dsName, "Data-Set-%d", ts + 2);

        int32 sds_index = SDnametoindex(file_handle, dsName);
    
        if (sds_index < 0)
        {
            SDend(file_handle);
            delete[] values[0];
            delete[] values[1];
            delete[] values[2];
            EXCEPTION1(InvalidFilesException, fname[i].c_str());
        }
        
        int32 var_handle = SDselect(file_handle, sds_index);
        if (var_handle < 0)
        {
            SDend(file_handle);
            delete[] values[0];
            delete[] values[1];
            delete[] values[2];
            EXCEPTION1(InvalidFilesException, fname[i].c_str());
        }

        //
        // Once again the data that is stored is bigger in dimension
        // than the cells we use. We need to read them in and then
        // pull out only the data we use.
        //
        
        int32 start[3];
        start[0] = 0;
        start[1] = 0;
        start[2] = 0;

        int32 realDims[3];
        realDims[0] = dimensions[0] + THROWOUT_DIM;
        realDims[1] = dimensions[1] + THROWOUT_DIM;
        realDims[2] = dimensions[2] + THROWOUT_DIM;

        if (rank == 2)
            realDims[dropDimension] = 1;

        //
        // HDF4 takes in the dimensions backwards from how we store them:
        // taking the most significant dimension first (z / phi).
        // 2D meshes are still stored in a 3D-array, with the most
        // significant dimension as 1.
        //
        
        int32 transDims[3];
        transDims[0] = realDims[2]; 
        transDims[1] = realDims[1]; 
        transDims[2] = realDims[0];
        
        float *data = new float[realDims[0] * realDims[1] * realDims[2]];
        
        SDreaddata(var_handle, start, NULL, transDims, data);

        float *vars = values[i];
        int x,y,z;
        for (z = startI[2]; z < endI[2]; ++z)
        {
            for (y = startI[1]; y < endI[1]; ++y)
            {
                for (x = startI[0]; x < endI[0]; ++x)
                {
                    *(vars++) = data[x + y * realDims[0]
                                       + z * realDims[0] * realDims[1]];
                }
            }
        }

        delete[] data;

        SDendaccess(var_handle);
        SDend(file_handle);
    }
 
    int nCells = dim[0] * dim[1] * dim[2];
    vtkFloatArray * fa = vtkFloatArray::New();
    fa->SetNumberOfComponents(3);
    fa->SetNumberOfTuples(nCells);
    float *ptr = fa->GetPointer(0);

    double start[3];
    double delta[3];
    if (coordType != cartesian)
    {
        //
        // For this, we need information about the mesh.
        //
        if (!meshInfoLoaded[dom])
            ReadMeshInfo(dom);

        start[0] = meshInfo[dom][0];
        start[1] = meshInfo[dom][1];
        start[2] = meshInfo[dom][2];
        delta[0] = meshInfo[dom][3];
        delta[1] = meshInfo[dom][4];
        delta[2] = meshInfo[dom][5];

        //
        // Convert from node positions to cell positions.
        //
        start[0] += delta[0] / 2.0;
        start[1] += delta[1] / 2.0;
        start[2] += delta[2] / 2.0;
    }

    switch (coordType)
    {
        case cartesian:
            if (rank == 3)
                FillVectorVar3DCartesian(ptr, values, nCells); 
            else
                FillVectorVar2DCartesian(ptr, values, nCells); 
            break;
        case spherical:
            if (rank == 3)
                FillVectorVar3DSpherical(ptr, values, start, delta, dim); 
            else
                FillVectorVar2DSpherical(ptr, values, start, delta); 
            break;
        case cylindrical:
            FillVectorVar2DCylindrical(ptr, values, start, delta); 
            break;
    }
    
    delete[] values[0];
    delete[] values[1];
    if (rank == 3)
        delete[] values[2];
    
    return fa;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::GetCycles
//
//  Purpose:
//    Returns the actual cycle numbers for each time step.
//
//  Arguments:
//   cycles      the output vector of cycle numbers 
//
//  Programmer:  Akira Haddox
//  Creation:    June 4, 2003
//
// ****************************************************************************


void
avtCosmosFileFormat::GetCycles(vector<int> &cycles)
{
    int nTimesteps = GetNTimesteps();

    cycles.resize(nTimesteps);
    for (int i = 0 ; i < nTimesteps ; i++)
    {
        cycles[i] = i;
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::GetNTimesteps
//
//  Purpose:
//    Returns the number of timesteps
//
//  Arguments:
//    none
//
//  Programmer:  Akira Haddox
//  Creation:    June 4, 2003
//
// ****************************************************************************

int
avtCosmosFileFormat::GetNTimesteps()
{
    return ntimesteps;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//
//  Programmer:  Akira Haddox
//  Creation:    June  4, 2003
//
//  Modifications:
//    Akira Haddox, Wed Jul 16 15:33:48 PDT 2003
//    Added in renaming of axes for xz, yz 2D cartesian plots.
//
//    Akira Haddox, Wed Jul 23 08:14:56 PDT 2003
//    Moved time reading code to seperate function called by GetMesh.
//
// ****************************************************************************

void
avtCosmosFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i;

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    if (coordType == cartesian)
        mesh->meshType = AVT_RECTILINEAR_MESH;
    else
        mesh->meshType = AVT_CURVILINEAR_MESH;
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = rank;
    mesh->topologicalDimension = rank;
    mesh->hasSpatialExtents = false;

    //
    // If we're a 2D cartesian plot, we might have projected the axes.
    //

    if (rank == 2 && coordType == cartesian)
    {
        if (dropDimension == 0)
        {
            mesh->xUnits = "Z-Axis";
            mesh->yUnits = "Y-Axis";
            mesh->zUnits = "unused";
        }
        if (dropDimension == 1)
        {
            mesh->xUnits = "X-Axis";
            mesh->yUnits = "Z-Axis";
            mesh->zUnits = "unused";
        }
    }
    
    md->Add(mesh);
    
    for (i = 0; i < nscalars; i++)
        md->Add(new avtScalarMetaData(scalarVarNames[i], "mesh", AVT_ZONECENT));
    for (i = 0; i < nvectors; i++)
        md->Add(new avtVectorMetaData(vectorVarNames[i], "mesh", 
                                      AVT_ZONECENT, 3));
}

// ****************************************************************************
//  Method:  avtCosmosFileFormat::ReadInTimes
//
//  Purpose:
//      Read in the time information.
//
//  Programmer:  Akira Haddox
//  Creation:    July 22, 2003
//  
// ****************************************************************************

void
avtCosmosFileFormat::ReadInTimes()
{
    readInTimes = true;

    ifstream tf;
    tf.open(timeFileName.c_str());
    if (tf.fail())
        EXCEPTION1(InvalidFilesException, timeFileName.c_str());

    int cycle, junk;
    double time;

    // 
    // The first 6 numbers we don't use.
    // 
    int i;
    for (i = 0; i < 6; ++i)
        tf >> junk;

    for (i = 0; i < ntimesteps; ++i)
    {
        tf >> cycle >> time;
        metadata->SetTime(i, time);
    }
    
    tf.close(); 
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::ReadMesh
//
//  Purpose:
//      Read in information to create a mesh for a particular domain.
//      Calculated mesh is stored in meshes[domain]. This method
//      allocates the vtkDataSet.
//
//  Arguments:
//    domain    The domain to read. (Not bounds checked).
//
//  Programmer:  Akira Haddox
//  Creation:    June  4, 2003
//
//  Modifications:
//    Akira Haddox, Fri Jun 13 10:09:26 PDT 2003
//    Moved reading mesh info into ReadMeshInfo call.
//      
//    Akira Haddox, Mon Jun 16 12:41:15 PDT 2003
//    Added in for cylindrical and other 2D configurations.
//    Moved mesh generation into seperate member functions.
//    Added ghostZone support.
//
// ****************************************************************************

void
avtCosmosFileFormat::ReadMesh(int domain)
{
    debug5 << "Reading in mesh for domain: " << domain << endl;
    if (!meshInfoLoaded[domain])
        ReadMeshInfo(domain);

    vtkDataSet *rv;

    int ptDim[3];
    int i;
    
    ptDim[0] = dimensions[0] + 1;
    ptDim[1] = dimensions[1] + 1;
    ptDim[2] = dimensions[2] + 1;

    if (existGhostZones)
    {
        for (i = 0; i < 3; ++i)
        {
            if (ghostSet[domain][i * 2])
                ++(ptDim[i]);
            if (ghostSet[domain][i * 2 + 1])
                ++(ptDim[i]);
        }
    }
    
    if (rank == 2)
        ptDim[dropDimension] = 1;
    
    double start[3];
    double delta[3];
    
    start[0] = meshInfo[domain][0];
    start[1] = meshInfo[domain][1];
    start[2] = meshInfo[domain][2];
    delta[0] = meshInfo[domain][3];
    delta[1] = meshInfo[domain][4];
    delta[2] = meshInfo[domain][5];
    
    switch (coordType)
    {
        case cartesian:
            if (rank == 3)
                rv = CalculateMesh3DCartesian(start, delta, ptDim); 
            else
                rv = CalculateMesh2DCartesian(start, delta, ptDim); 
            break;
        case spherical:
            if (rank == 3)
                rv = CalculateMesh3DSpherical(start, delta, ptDim); 
            else
                rv = CalculateMesh2DSpherical(start, delta, ptDim); 
            break;
        case cylindrical:
            rv = CalculateMesh2DCylindrical(start, delta, ptDim); 
            break;
    }

    // Add in the ghost zones
    if (existGhostZones)
    {
        int cDims[3];
        cDims[0] = ptDim[0] - 1;
        cDims[1] = ptDim[1] - 1;
        cDims[2] = ptDim[2] - 1;
        
        vtkUnsignedCharArray *ghostLevels = vtkUnsignedCharArray::New();
        ghostLevels->SetNumberOfTuples(cDims[0] * cDims[1] * cDims[2]);
        unsigned char *p = ghostLevels->GetPointer(0);

        int x,y,z;
        for (z = 0; z < cDims[2]; ++z)
        {
            for (y = 0; y < cDims[1]; ++y)
            {
                for (x = 0; x < cDims[0]; ++x)
                {
                    // Start X
                    if (ghostSet[domain][0] && x == 0)
                        *p = 1;
                    else if (ghostSet[domain][1] && x == cDims[0] - 1)
                        *p = 1;
                    else if (ghostSet[domain][2] && y == 0)
                        *p = 1;
                    else if (ghostSet[domain][3] && y == cDims[1] - 1)
                        *p = 1;
                    else if (ghostSet[domain][4] && z == 0)
                        *p = 1;
                    else if (ghostSet[domain][5] && z == cDims[2] - 1)
                        *p = 1;
                    else
                        *p = 0;
                    ++p;
                }
            }
        }

        ghostLevels->SetName("vtkGhostLevels");
        rv->GetCellData()->AddArray(ghostLevels);
        
        int first[3];
        int last[3];

        first[0] = ghostSet[domain][0] ? 1 : 0;
        first[1] = ghostSet[domain][2] ? 1 : 0;
        first[2] = ghostSet[domain][4] ? 1 : 0;

        last[0] = first[0] + dimensions[0];
        last[1] = first[1] + dimensions[1];
        last[2] = first[2] + dimensions[2];

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->SetNumberOfValues(6);
        realDims->SetValue(0, first[0]);
        realDims->SetValue(1, last[0]);
        realDims->SetValue(2, first[1]);
        realDims->SetValue(3, last[1]);
        realDims->SetValue(4, first[2]);
        realDims->SetValue(5, last[2]);
        rv->GetFieldData()->AddArray(realDims);
        rv->GetFieldData()->CopyFieldOn("avtRealDims");

        realDims->Delete();
        ghostLevels->Delete();
        
        rv->SetUpdateGhostLevel(0);
    } 
    
    meshes[domain] = rv;
}

// ****************************************************************************
//  Method:  avtCosmosFileFormat::ReadMeshInfo
//
//  Purpose:
//      Read in information to create a mesh for a particular domain.
//
//  Arguments:
//    domain    The domain to read. (Not bounds checked).
//
//  Programmer:  Akira Haddox
//  Creation:    June 13, 2003
//
//  Modifications:
//    Akira Haddox, Wed Jul 16 15:33:48 PDT 2003
//    Added in ghostZone support.
//
// ****************************************************************************

void
avtCosmosFileFormat::ReadMeshInfo(int domain)
{
    meshInfoLoaded[domain] = true;
    ifstream file;
    file.open(gridFileNames[domain].c_str());
    if (file.fail())
        EXCEPTION1(InvalidFilesException, gridFileNames[domain].c_str());

    meshInfo[domain].resize(6);
    
    double start[3];
    double end[3];
    double delta[3];

    int i;
    for (i = 0; i < 3; ++i)
    {
        file >> start[i] >> end[i] >> delta[i];
        start[i] -= delta[i] / 2.0;

        if (ghostSet[domain][i * 2])
            start[i] += delta[i];
        else
            start[i] += delta[i] * 2;
    }

    meshInfo[domain][0] = start[0];
    meshInfo[domain][1] = start[1];
    meshInfo[domain][2] = start[2];
    meshInfo[domain][3] = delta[0];
    meshInfo[domain][4] = delta[1];
    meshInfo[domain][5] = delta[2];
    
    file.close();
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::CalculateMesh3DCartesian
//
//  Purpose:
//      Calculate the mesh points for a 3D cartesian coordinate mesh.
//
//  Arguments:
//    start     The starting array for the coordinates.
//    delta     The delta array for the coordinates.
//    ptDim     The dimensions of the points.
//
//  Returns:    The calculated dataset.
//  
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::CalculateMesh3DCartesian(double start[3], double delta[3],
                                              int ptDim[3])
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();

    rg->SetDimensions(ptDim);

    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(ptDim[0]);
    ycoord->SetNumberOfTuples(ptDim[1]);
    zcoord->SetNumberOfTuples(ptDim[2]);

    int i;
    float *ptr = xcoord->GetPointer(0);
    for (i = 0; i < ptDim[0]; ++i)
        ptr[i] = (start[0] + i * delta[0]);

    ptr = ycoord->GetPointer(0);
    for (i = 0; i < ptDim[1]; ++i)
        ptr[i] = (start[1] + i * delta[1]);

    ptr = zcoord->GetPointer(0);
    for (i = 0; i < ptDim[2]; ++i)
        ptr[i] = (start[2] + i * delta[2]);

    rg->SetXCoordinates(xcoord);
    rg->SetYCoordinates(ycoord);
    rg->SetZCoordinates(zcoord);

    xcoord->Delete();
    ycoord->Delete();
    zcoord->Delete();

    return rg;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::CalculateMesh3DSpherical
//
//  Purpose:
//      Calculate the mesh points for a 3D spherical coordinate mesh.
//
//  Arguments:
//    start     The starting array for the coordinates.
//    delta     The delta array for the coordinates.
//    ptDim     The dimensions of the points.
//
//  Returns:    The calculated dataset.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::CalculateMesh3DSpherical(double start[3], double delta[3],
                                              int ptDim[3])
{
    vtkStructuredGrid *sg = vtkStructuredGrid::New();

    sg->SetDimensions(ptDim);
    vtkPoints *pts = vtkPoints::New();
    sg->SetPoints(pts);

    int npoints = ptDim[0] * ptDim[1] * ptDim[2];
    pts->SetNumberOfPoints(npoints);

    int r,t,p;
    int i = -1;
    double x,y,z;
    for (p = 0; p < ptDim[2]; ++p)
    {
        double phi = start[2] + p * delta[2];
        for (t = 0; t < ptDim[1]; ++t)
        {
            double theta = start[1] + t * delta[1];
            for (r = 0; r < ptDim[0]; ++r)
            {
                double rho = start[0] + r * delta[0];
    
                x = rho * sin(theta) * cos(phi);
                y = rho * sin(theta) * sin(phi);
                z = rho * cos(theta);
                
                pts->SetPoint(++i, x, y, z);
            }
        }
    }

    return sg;
}

// ****************************************************************************
//  Method:  avtCosmosFileFormat::CalculateMesh2DCartesian
//
//  Purpose:
//      Calculate the mesh points for a 2D cartesian coordinate mesh.
//
//  Arguments:
//    start     The starting array for the coordinates.
//    delta     The delta array for the coordinates.
//    ptDim     The dimensions of the points.
//
//  Returns:    The calculated dataset.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::CalculateMesh2DCartesian(double start[3], double delta[3],
                                              int ptDim[3])
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();

    //
    // Now the dataset could be in any three of the axes planes. However,
    // we need to translate them into the XY plane. Since we ignore the
    // third component on each of these vectors, we can just drop it
    // in the translation.
    //

    //
    // Dataset is in the YZ plane.
    //  Z becomes X.
    //
    if (dropDimension == 0)
    {
        ptDim[0] = ptDim[2];
        start[0] = start[2];
        delta[0] = delta[2];
    }

    //
    // Dataset is in the XZ plane.
    //  Z becomes Y.
    //
    if (dropDimension == 1)
    {
        ptDim[1] = ptDim[2];
        start[1] = start[2];
        delta[1] = delta[2];
    }
    
    rg->SetDimensions(ptDim);
    
    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(ptDim[0]);
    ycoord->SetNumberOfTuples(ptDim[1]);
    zcoord->SetNumberOfTuples(1);

    int i;
    float *ptr = xcoord->GetPointer(0);
    for (i = 0; i < ptDim[0]; ++i)
        ptr[i] = (start[0] + i * delta[0]);
    
    ptr = ycoord->GetPointer(0);
    for (i = 0; i < ptDim[1]; ++i)
        ptr[i] = (start[1] + i * delta[1]);
    
    ptr = zcoord->GetPointer(0);
    *ptr = 0;

    rg->SetXCoordinates(xcoord);
    rg->SetYCoordinates(ycoord);
    rg->SetZCoordinates(zcoord);
    
    xcoord->Delete();
    ycoord->Delete();
    zcoord->Delete();

    return rg;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::CalculateMesh2DSpherical
//
//  Purpose:
//      Calculate the mesh points for a 2D spherical coordinate mesh.
//
//  Arguments:
//    start     The starting array for the coordinates.
//    delta     The delta array for the coordinates.
//    ptDim     The dimensions of the points.
//
//  Returns:    The calculated dataset.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::CalculateMesh2DSpherical(double start[3], double delta[3],
                                              int ptDim[3])
{
    vtkStructuredGrid *sg = vtkStructuredGrid::New();

    int gridDim[3];
    if (dropDimension == 2)
    {
        gridDim[0] = ptDim[0];
        gridDim[1] = ptDim[1];
        gridDim[2] = ptDim[2];
    }
    else
    {
        gridDim[0] = ptDim[0];
        gridDim[1] = ptDim[2];
        gridDim[2] = 1;
    }
    
    sg->SetDimensions(gridDim);
    vtkPoints *pts = vtkPoints::New();
    sg->SetPoints(pts);

    int npoints = ptDim[0] * ptDim[1] * ptDim[2];
    pts->SetNumberOfPoints(npoints);
    
    int r,t,p;
    int i = -1;
    double x,y;

    //
    // A rho - theta slice.
    //
    if (dropDimension == 2)
    {
        for (t = 0; t < ptDim[1]; ++t)
        {
            double theta = start[1] + t * delta[1];
            for (r = 0; r < ptDim[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                x = rho * sin(theta);
                y = rho * cos(theta);
            
                pts->SetPoint(++i, x, y, 0.);
            }
        }
    }
    //
    // A rho - phi slice.
    //
    else
    {
        for (p = 0; p < ptDim[2]; ++p)
        {
            double phi = start[2] + p * delta[2];
            for (r = 0; r < ptDim[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                x = rho * cos(phi);
                y = rho * sin(phi);

                pts->SetPoint(++i, x, y, 0.);
            }
        }
    }

    return sg;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::CalculateMesh2DCylindrical
//
//  Purpose:
//      Calculate the mesh points for a 2D cylindrical coordinate mesh.
//
//  Arguments:
//    start     The starting array for the coordinates.
//    delta     The delta array for the coordinates.
//    ptDim     The dimensions of the points.
//
//  Returns:    The calculated dataset.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::CalculateMesh2DCylindrical(double start[3], 
                                                double delta[3], int ptDim[3])
{
    vtkStructuredGrid *sg = vtkStructuredGrid::New();

    int gridDim[3];

    if (dropDimension == 2)
    {
        gridDim[0] = ptDim[0];
        gridDim[1] = ptDim[1];
        gridDim[2] = ptDim[2];
    }
    else if (dropDimension == 1)
    {
        gridDim[0] = ptDim[0];
        gridDim[1] = ptDim[2];
        gridDim[2] = 1;
    }
    else if (dropDimension == 0)
    {
        gridDim[0] = ptDim[1];
        gridDim[1] = ptDim[2];
        gridDim[2] = 1;
    }
    
    sg->SetDimensions(gridDim);
    vtkPoints *pts = vtkPoints::New();
    sg->SetPoints(pts);

    int npoints = ptDim[0] * ptDim[1] * ptDim[2];
    pts->SetNumberOfPoints(npoints);
    
    int r,p,z;
    int i = -1;
    double x,y;

    //
    // A rho - phi slice.
    //
    if (dropDimension == 2)
    {
        for (p = 0; p < ptDim[1]; ++p)
        {
            double phi = start[1] + p * delta[1];
            for (r = 0; r < ptDim[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                //
                // Yes, the sin/cos are reverse from spherical coordinates.
                //
                x = rho * cos(phi);
                y = rho * sin(phi);
            
                pts->SetPoint(++i, x, y, 0.);
            }
        }
    }
    //
    // A rho - z slice.
    //
    else if (dropDimension == 1)
    {
        for (z = 0; z < ptDim[2]; ++z)
        {
            double zee = start[2] + z * delta[2];
            for (r = 0; r < ptDim[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                x = rho;
                y = zee;

                pts->SetPoint(++i, x, y, 0.);
            }
        }
    }
    //
    // A phi - z slice. (Rare)
    //
    else
    {
        for (z = 0; z < ptDim[2]; ++z)
        {
            double zee = start[2] + z * delta[2];
            for (p = 0; p < ptDim[1]; ++p)
            {
                double phi = start[1] + p * delta[1];

                x = phi;
                y = zee;

                pts->SetPoint(++i, x, y, 0.);
            }
        }
    }

    return sg;
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::FillVectorVar3DCartesian
//
//  Purpose:
//      Fill in the vector variable for a 3D cartesian dataset.
//
//  Arguments:
//    ptr       Pointer to the first element of the vector variable.
//    values    The read in values.
//    nCells    The number of cells in this domain.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

void
avtCosmosFileFormat::FillVectorVar3DCartesian(float *ptr, float *values[3],
                                              int nCells)
{
    int i, j;
    for (i = 0; i < nCells; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            *(ptr++) = values[j][i];
        }
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::FillVectorVar3DSpherical
//
//  Purpose:
//      Fill in the vector variable for a 3D spherical dataset.
//
//  Arguments:
//    ptr       Pointer to the first element of the vector variable.
//    values    The read in values.
//    start     The starting coordinates for the mesh.
//    delta     The change in coordinates for the mesh.
//    dims      The dimensions to fill according to.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

void
avtCosmosFileFormat::FillVectorVar3DSpherical(float *ptr, float *values[3],
                                              double start[3], double delta[3],
                                              int dims[3])
{
    //
    // Calculating vectors for spherical coordinates is calculated for
    // arbitrary coordinate: m (one of: x,y,z) as:
    //
    // component m = dm/dr * vr + dm/dt * vt + dm/dp * vp
    //
   
    int r,t,p;
    int i = 0;
    for (p = 0; p < dims[2]; ++p)
    {
        double phi = start[2] + p * delta[2];
        for (t = 0; t < dims[1]; ++t)
        {
            double theta = start[1] + t * delta[1];
            for (r = 0; r < dims[0]; ++r)
            {
                double rho = start[0] + r * delta[0];
                
                double vr = values[0][i];
                double vt = values[1][i];
                double vp = values[2][i];
                ++i;
                
                *(ptr++) =   sin(theta) * cos(phi) * vr
                           + rho * cos(theta) * cos(phi) * vt
                           - rho * sin(theta) * sin(phi) * vp;
                *(ptr++) =   sin(theta) * sin(phi) * vr
                           + rho * cos(theta) * sin(phi) * vt
                           + rho * sin(theta) * cos(phi) * vp;
                *(ptr++) =   cos(theta) * vr
                           - rho * sin(theta) * vt;
            }
        }
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::FillVectorVar2DCartesian
//
//  Purpose:
//      Fill in the vector variable for a 2D cartesian dataset.
//
//  Arguments:
//    ptr       Pointer to the first element of the vector variable.
//    values    The read in values.
//    nCells    The number of cells in this domain.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

void
avtCosmosFileFormat::FillVectorVar2DCartesian(float *ptr, float *values[3],
                                              int nCells)
{
    float *xVal, *yVal;
    //
    // Find which two sets of values we're using.
    //
    switch (dropDimension)
    {
        case 0:
            xVal = values[2];
            yVal = values[1];
            break;
        case 1:
            xVal = values[0];
            yVal = values[2];
            break;
        case 2:
            xVal = values[0];
            yVal = values[1];
            break;
        default:
            break;
    }
    
    int i;
    for (i = 0; i < nCells; ++i)
    {
        *(ptr++) = xVal[i];
        *(ptr++) = yVal[i];
        *(ptr++) = 0.;
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::FillVectorVar2DSpherical
//
//  Purpose:
//      Fill in the vector variable for a 2D spherical dataset.
//
//  Arguments:
//    ptr       Pointer to the first element of the vector variable.
//    values    The read in values.
//    start     The starting coordinates for the mesh.
//    delta     The change in coordinates for the mesh.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

void
avtCosmosFileFormat::FillVectorVar2DSpherical(float *ptr, float *values[3],
                                              double start[3], double delta[3])
{
    int r,t,p;
    int i = 0;

    if (dropDimension == 2)
    {
        for (t = 0; t < dimensions[1]; ++t)
        {
            double theta = start[1] + t * delta[1];
            for (r = 0; r < dimensions[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                double vr = values[0][i];
                double vt = values[1][i];
                ++i;

                *(ptr++) = sin(theta) * vr + rho * cos(theta) * vt;
                *(ptr++) = cos(theta) * vr - rho * sin(theta) * vt;
                *(ptr++) = 0;
            }
        }
    }
    else
    {
        for (p = 0; p < dimensions[2]; ++p)
        {
            double phi = start[2] + p * delta[2];
            for (r = 0; r < dimensions[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                double vr = values[0][i];
                double vp = values[1][i];
                ++i;

                *(ptr++) = cos(phi) * vr - rho * sin(phi) * vp;
                *(ptr++) = sin(phi) * vr + rho * cos(phi) * vp;
                *(ptr++) = 0;
            }
        }
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::FillVectorVar2DCylindrical
//
//  Purpose:
//      Fill in the vector variable for a 2D cylindrical dataset.
//
//  Arguments:
//    ptr       Pointer to the first element of the vector variable.
//    values    The read in values.
//    start     The starting coordinates for the mesh.
//    delta     The change in coordinates for the mesh.
//
//  Programmer:  Akira Haddox
//  Creation:    June 16, 2003
//
// ****************************************************************************

void
avtCosmosFileFormat::FillVectorVar2DCylindrical(float *ptr, float *values[3],
                                                double start[3],
                                                double delta[3])
{
    int r,p;
    int i = 0;

    if (dropDimension == 2)
    {
        for (p = 0; p < dimensions[1]; ++p)
        {
            double phi = start[1] + p * delta[1];
            for (r = 0; r < dimensions[0]; ++r)
            {
                double rho = start[0] + r * delta[0];

                double vr = values[0][i];
                double vp = values[1][i];
                ++i;

                *(ptr++) = cos(phi) * vr - rho * sin(phi) * vp;
                *(ptr++) = sin(phi) * vr + rho * cos(phi) * vp;
                *(ptr++) = 0;
            }
        }
    }
    else
    {
        int nCells = dimensions[0] * dimensions[1] * dimensions[2];
        for (i = 0; i < nCells; ++i)
        {
            *(ptr++) = values[0][i];
            *(ptr++) = values[1][i];
            *(ptr++) = 0;
        }
    }
}


// ****************************************************************************
//  Method:  avtCosmosFileFormat::ReadString
//
//  Purpose:
//      Read in a string from a file. Works like you would expect
//      ifile >> str to work, except this will compile on all platforms.
//
//  Programmer:  Akira Haddox
//  Creation:    July 22, 2003
//
// ****************************************************************************

void
avtCosmosFileFormat::ReadString(ifstream &ifile, std::string &str)
{
    char c;
    do
    {
        c = ifile.get();
        if (ifile.eof() || ifile.fail())
            return;
    }while (isspace(c));    

    str = c;

    while (!isspace(ifile.peek()))
    {
        if (ifile.eof() || ifile.fail())
            return;
        c = ifile.get();
        str += c;
    }
}

// ****************************************************************************
//  Method:  GetDirName
//
//  Purpose:
//    Returns the directory from a full path name
//
//  Arguments:
//    path       the full path name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************

string 
GetDirName(const char *path)
{
    string dir = "";

    int len = strlen(path);
    const char *last = path + (len-1);
    while (*last != '/' && last > path)
    {
        last--;
    }

    if (*last != '/')
    {
        return "";
    }

    char str[1024];
    strcpy(str, path);
    str[last-path+1] = '\0';

    return str;
}

