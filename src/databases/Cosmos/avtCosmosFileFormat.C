// ***************************************************************************
//                           avtCosmosFileFormat.C                           
// ***************************************************************************


#include <avtCosmosFileFormat.h>

#include <algorithm>
#include <fstream>
#include <math.h>
#include <string>

#include <vtkFloatArray.h>
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

using namespace std;

static string GetDirName(const char *path);

//
// A certain number of ghost zones are added in each dimension. But
// because these ghost zones are on the external faces as well as
// internal faces, we need to throw them out: We throw out 2 'shells'
// of cells on either side of the domain.
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
// ****************************************************************************

avtCosmosFileFormat::avtCosmosFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    dirname = GetDirName(fname);

    std::ifstream ifile(fname);

    if (ifile.fail())
    {
        EXCEPTION1(InvalidDBTypeException, "Unable to open cosmos file.");
    }
    
    ntimesteps = ndomains = dimensions[0] = -1;
    
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
        ifile >> key;

        if (ifile.eof() || ifile.fail())
                break;
        
        if (key == "#COORDINATES")
        {
            string value;
            ifile >> value;
            if (value == "spherical")
                sphericalCoordinates = true;
            else
                sphericalCoordinates = false;
        }
        else if (key == "#DOMAINS")
        {
            ifile >> ndomains;
            gridFileNames.resize(ndomains);
            scalarFileNames.resize(ndomains);
            vectorFileNames.resize(ndomains);
        }
        else if (key == "#TIMESTEPS")
        {
            ifile >> ntimesteps;
        }
        else if (key == "#TIMEFILE")
        {
            ifile >> timeFileName;
            timeFileName = dirname + timeFileName;
        }
        else if (key == "#DIMENSIONS")
        {
            ifile >> dimensions[0] >> dimensions[1] >> dimensions[2]; 
            dimensions[0] -= THROWOUT_DIM;
            dimensions[1] -= THROWOUT_DIM;

            if (dimensions[2] == 1)
            {
                rank = 2;
            }
            else
            {
                rank = 3;
                dimensions[2] -= THROWOUT_DIM;
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
                ifile >> value;
                gridFileNames[i] = dirname + value;
            }
        }
        else if (key == "#SCALAR")
        {
            if (ndomains < 0)
                EXCEPTION1(InvalidDBTypeException, "Domains not defined "
                                                   "before data.");    
            // Strip out leading white space.
            while(isspace(ifile.peek()))
                ifile.get();

            string value;
            getline(ifile, value);

            scalarVarNames.push_back(value);

            for (i = 0; i < ndomains; ++i)
            {
                ifile >> value;
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
            getline(ifile, value);
            int index = vectorVarNames.size();
            vectorVarNames.push_back(value);
            
            
            for (i = 0; i < ndomains; ++i)
            {
                vectorFileNames[i].push_back(TripleString());
                ifile >> value;
                vectorFileNames[i][index].x = dirname + value;
            }
            for (i = 0; i < ndomains; ++i)
            {
                ifile >> value;
                vectorFileNames[i][index].y = dirname + value;
            }

            if (rank == 3)
            { 
                for (i = 0; i < ndomains; ++i)
                {
                    ifile >> value;
                    vectorFileNames[i][index].z = dirname + value;
                }
            }
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

    if (rank == 3) 
        npoints = (dimensions[0] + 1) * (dimensions[1] + 1) *
                  (dimensions[2] + 1);
    else
        npoints = (dimensions[0] + 1) * (dimensions[1] + 1);
    nscalars = scalarVarNames.size();
    nvectors = vectorVarNames.size();

    meshes = new vtkDataSet *[ndomains];
    for (i = 0; i < ndomains; ++i)
        meshes[i] = NULL;
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
// ****************************************************************************

vtkDataSet *
avtCosmosFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
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
// ****************************************************************************

vtkDataArray *
avtCosmosFileFormat::GetVar(int ts, int dom, const char *name)
{
    int varfId;
    for (varfId = 0; varfId < scalarVarNames.size(); ++varfId)
        if(scalarVarNames[varfId] == name)
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
    // Allocate the array that we will be returning.
    //

    vtkFloatArray * fa = vtkFloatArray::New();
    fa->SetNumberOfTuples(dimensions[0] * dimensions[1] * dimensions[2]);
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
    if (rank == 3)
        realDims[2] = dimensions[2] + THROWOUT_DIM;
    else
        realDims[2] = 1;
    
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
    int startZ, endZ;
    if (rank == 3)
    {
        startZ = GHOST_PADDING;
        endZ = dimensions[2] + GHOST_PADDING;
    }
    else
    {
        startZ = 0;
        endZ = 1;
    }
    for (z = startZ; z < endZ; ++z)
    {
        for (y = GHOST_PADDING; y < dimensions[1] + GHOST_PADDING; ++y)
        {
            for (x = GHOST_PADDING; x < dimensions[0] + GHOST_PADDING; ++x)
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
    
    float *values[3];
    values[0] = new float[dimensions[0] * dimensions[1] * dimensions[2]];
    values[1] = new float[dimensions[0] * dimensions[1] * dimensions[2]];
    if (rank == 3)
        values[2] = new float[dimensions[0] * dimensions[1] * dimensions[2]];

    //
    // Read in the xyz components.
    //
    
    int startZ = 0;
    int endZ = 1;
    if (rank == 3)
    {
        startZ = GHOST_PADDING;
        endZ = dimensions[2] + GHOST_PADDING;
    }

    int i;
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
        if (rank == 3)
            realDims[2] = dimensions[2] + THROWOUT_DIM;
        else
            realDims[2] = 1;

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
        for (z = startZ; z < endZ; ++z)
        {
            for (y = GHOST_PADDING; y < dimensions[1] + GHOST_PADDING; ++y)
            {
                for (x = GHOST_PADDING; x < dimensions[0] + GHOST_PADDING; ++x)
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
 
    int nCells = dimensions[0] * dimensions[1] * dimensions[2];
    vtkFloatArray * fa = vtkFloatArray::New();
    fa->SetNumberOfComponents(3);
    fa->SetNumberOfTuples(nCells);
    float *ptr = fa->GetPointer(0);

    int j;
    for (i = 0; i < nCells; ++i)
    {
        for (j = 0; j < 2; ++j)
        {
            *(ptr++) = values[j][i];
        }
        if (rank == 3)
            *(ptr++) = values[2][i];
        else
            *(ptr++) = 0.;
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
// ****************************************************************************

void
avtCosmosFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i;

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    if (sphericalCoordinates)
        mesh->meshType = AVT_CURVILINEAR_MESH;
    else
        mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = rank;
    mesh->topologicalDimension = rank;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);
    
    for (i = 0; i < nscalars; i++)
        md->Add(new avtScalarMetaData(scalarVarNames[i], "mesh", AVT_ZONECENT));
    for (i = 0; i < nvectors; i++)
        md->Add(new avtVectorMetaData(vectorVarNames[i], "mesh", 
                                      AVT_ZONECENT, 3));

    //
    // Read in the time
    //

    md->SetNumStates(ntimesteps);

    std::ifstream tf;
    tf.open(timeFileName.c_str());
    if (tf.fail())
        EXCEPTION1(InvalidFilesException, timeFileName.c_str());

    int cycle, junk;
    double time;

    // 
    // The first 6 numbers we don't use.
    // 
    for (i = 0; i < 6; ++i)
        tf >> junk;

    for (i = 0; i < ntimesteps; ++i)
    {
        tf >> cycle >> time;
        md->SetCycle(i, cycle);
        md->SetTime(i, time);
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
// ****************************************************************************

void
avtCosmosFileFormat::ReadMesh(int domain)
{
    debug5 << "Reading in mesh for domain: " << domain << endl;
    std::ifstream file;
    file.open(gridFileNames[domain].c_str());
    if (file.fail())
        EXCEPTION1(InvalidFilesException, gridFileNames[domain].c_str());

    vtkDataSet *rv;

    int ptDim[3];
    ptDim[0] = dimensions[0] + 1;
    ptDim[1] = dimensions[1] + 1;
    if (rank == 3)
        ptDim[2] = dimensions[2] + 1;
    else
        ptDim[2] = 1;

    
    double start[3];
    double end[3];
    double delta[3];

    int i;
    for (i = 0; i < 3; ++i)
    {
        file >> start[i] >> end[i] >> delta[i];
        start[i] -= delta[i] / 2.0;
        start[i] += delta[i] * 2;
    }
    
    file.close();
    
    if (sphericalCoordinates)
    {
        vtkStructuredGrid *sg = vtkStructuredGrid::New();
        rv = sg;

        sg->SetDimensions(ptDim);
        vtkPoints *pts = vtkPoints::New();
        sg->SetPoints(pts);
        
        pts->SetNumberOfPoints(npoints);
       
        //
        // 3D Spherical coordinates
        //
        if (rank == 3)
        { 
            int r,t,p;
            i = -1;
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
            
                        //
                        // This is how the Cosmos file format considers
                        // the axes.
                        //
                        
                        x = rho * sin(theta) * cos(phi);
                        y = rho * sin(theta) * sin(phi);
                        z = rho * cos(theta);
                        
                        pts->SetPoint(++i, x, y, z);
                    }
                }
            }
        }
        //
        // 2D Spherical (polar) coordinates
        //
        else
        {
            int r,t;
            i = -1;
            double x,y;
            for (t = 0; t < ptDim[1]; ++t)
            {
                double theta = start[1] + t * delta[1];
                for (r = 0; r < ptDim[0]; ++r)
                {
                    double rho = start[0] + r * delta[0];

                    x = rho * cos(theta);
                    y = rho * sin(theta);
                
                    pts->SetPoint(++i, x, y, 0.);
                }
            }
        }
    }
    //
    // Cartesian Coordinates
    //
    else
    {
        vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
        rv = rg;

        rg->SetDimensions(ptDim);
        
        vtkFloatArray  *xcoord = vtkFloatArray::New();
        vtkFloatArray  *ycoord = vtkFloatArray::New();
        vtkFloatArray  *zcoord = vtkFloatArray::New();

        xcoord->SetNumberOfTuples(ptDim[0]);
        ycoord->SetNumberOfTuples(ptDim[1]);
        zcoord->SetNumberOfTuples(ptDim[2]);

        float *ptr = xcoord->GetPointer(0);
        for (i = 0; i < ptDim[0]; ++i)
            ptr[i] = (start[0] + i * delta[0]);
        
        ptr = ycoord->GetPointer(0);
        for (i = 0; i < ptDim[1]; ++i)
            ptr[i] = (start[1] + i * delta[1]);
        
        ptr = zcoord->GetPointer(0);
        if (rank == 3)
        {
            for (i = 0; i < ptDim[2]; ++i)
                ptr[i] = (start[2] + i * delta[2]);
        }
        else
            *ptr = 0;

        rg->SetXCoordinates(xcoord);
        rg->SetYCoordinates(ycoord);
        rg->SetZCoordinates(zcoord);
        
        xcoord->Delete();
        ycoord->Delete();
        zcoord->Delete();
    }

    meshes[domain] = rv;
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
