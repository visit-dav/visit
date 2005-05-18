// ***************************************************************************
//                              avtRectFileFormat.C                           
//
//  Purpose:  Sample database which reads multi-domain multi-timestep files
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
// ****************************************************************************


#include <avtRectFileFormat.h>

#include <vector>
#include <string>

#include <vtkStructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


using std::vector;
using std::string;

static string GetDirName(const char *path);

#if defined(_WIN32)
// Define >> for strings since Windows does not seem to have it.
ifstream &operator >> (ifstream &s, string &str)
{
    char tmp[1000];
    s >> tmp;
    str = string(tmp);
    return s;
}
#endif

// ****************************************************************************
//  Constructor:  avtRectFileFormat::avtRectFileFormat
//
//  Arguments:
//    fname      the file name of the root .viz file
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
// ****************************************************************************
avtRectFileFormat::avtRectFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    dirname = GetDirName(fname);
    filename = fname;

    ifstream ifile(filename.c_str());

    if (ifile.fail())
    {
        debug1 << "Unable to open viz file " << fname << endl;
    }
    else
    {
        ReadVizFile(ifile);
    }
    ifile.close();

    cachedMeshes = new vtkDataSet*[ndomains];
    for (int d=0; d<ndomains; d++)
        cachedMeshes[d] = NULL;
}

// ****************************************************************************
//  Destructor:  avtRectFileFormat::~avtRectFileFormat
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
// ****************************************************************************
avtRectFileFormat::~avtRectFileFormat()
{
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given time step and
//    domain.  This function will return a cached reference to the mesh
//    for this domain, since the mesh does not change over time.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
vtkDataSet *
avtRectFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    if (cachedMeshes[dom] != NULL)
    {
        // The reference count will be decremented by the generic database,
        // because it will assume it owns it.
        cachedMeshes[dom]->Register(NULL);
        return cachedMeshes[dom];
    }

    vtkDataSet *ds = ReadMesh(ts, dom, mesh);
    cachedMeshes[dom] = ds;
    ds->Register(NULL);

    return ds;
}

// ****************************************************************************
//  Method:  avtRectFileFormat::ReadMesh
//
//  Purpose:
//    Reads the mesh with the given name for the given time step and
//    domain from the file.  NOTE: time step is ignored in this case
//    because the mesh does not change over time.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
vtkDataSet *
avtRectFileFormat::ReadMesh(int ts, int dom, const char *name)
{
    // timestep (ts) is unused because the mesh is constant over time
    // should really interact with variable cache (see Exodus format)
    // so the mesh is only read once for all time steps

    if (strcmp(name, "Mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    //
    // Open the file
    //
    char fname[256];
    sprintf(fname, "%sgrid/domain%04d", dirname.c_str(), dom);
    ifstream in(fname, ios::in);
    if (!in)
    {
        cerr << "Unable to open grid file " << fname << endl;
        return NULL;
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
    vtkPoints            *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    int dims[3];
    dims[0] = dxsize[dom];
    dims[1] = dysize[dom];
    dims[2] = dzsize[dom];
    sgrid->SetDimensions(dims);

    //
    // Populate the coordinates.
    //
    points->SetNumberOfPoints(numpts[dom]);
    float *pts = (float *) points->GetVoidPointer(0);

    for (int d=0; d<3; d++)
    {
        for (int i=0; i<numpts[dom]; i++)
        {
            in >> pts[i*3 + d];
        }
    }

    in.close();

    return sgrid;
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain.  NOTE: a binary implementation should use fseek() or similar
//    to skip to the correct part of the data file.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
vtkDataArray *
avtRectFileFormat::GetVar(int ts, int dom, const char *name)
{
    for (int i = 0; i < nvars; i++)
    {
        if (varnames[i] == name)
        {
            //
            // Open the file
            //
            char fname[256];
            sprintf(fname, "%s%s%04d/domain%04d", dirname.c_str(),
                    basename.c_str(), ts, dom);

            ifstream in(fname, ios::in);
            if (!in)
            {
                cerr << "Unable to open data file " << fname << endl;
                return NULL;
            }

            //
            // Skip to the right part of the file
            // (this is horribly inefficient as it
            //  is a text file)
            //
            char buff[4096];
            for (int j=0; j<i*numpts[dom]; j++)
                in >> buff;

            //
            // Create the array and read from the file
            //
            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(numpts[dom]);
            float *ptr = (float*)scalars->GetVoidPointer(0);
            for (int k=0; k<numpts[dom]; k++)
            {
                in >> ptr[k];
            }

            in.close();
            return scalars;
        }
    }

    EXCEPTION1(InvalidVariableException, name);
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetCycles
//
//  Purpose:
//    Returns the actual cycle numbers for each time step.
//
//  Arguments:
//   cycles      the output vector of cycle numbers 
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
void
avtRectFileFormat::GetCycles(vector<int> &cycles)
{
    int nTimesteps = GetNTimesteps();

    cycles.resize(nTimesteps);
    for (int i = 0 ; i < nTimesteps ; i++)
    {
        cycles[i] = i;
    }
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetNTimesteps
//
//  Purpose:
//    Returns the number of timesteps
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
int
avtRectFileFormat::GetNTimesteps()
{
    return ntimesteps;
}

// ****************************************************************************
//  Method:  avtRectFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//    timeState  The time index to use (if metadata varies with time)
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg
// ****************************************************************************
void
avtRectFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);
    for (int i=0; i < nvars; i++)
    {
        md->Add(new avtScalarMetaData(varnames[i], "Mesh", AVT_NODECENT));
    }

    SetUpDomainConnectivity();
}

// ****************************************************************************
//  Method:  avtRectFileFormat::SetUpDomainConnectivity
//
//  Purpose:
//    Sets up the connectivity between domains so that ghost zones
//    are automatically created.  NOTE: assumes the domains are slabs
//    in the Z direction listed in increasing order.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Nov 12 16:47:46 PST 2003
//    Use curvilinear domain boundaries, since structured domain boundaries is
//    now an abstract type.
//
// ****************************************************************************
void
avtRectFileFormat::SetUpDomainConnectivity()
{
    if (ndomains < 3)
        return;

    avtCurvilinearDomainBoundaries *dbi=new avtCurvilinearDomainBoundaries();
    dbi->SetNumDomains(ndomains);

    int zstart = 1;
    int extents[6];
    int orientation[3] = {1,2,3};
    int boundary[6];

    for (int i=0; i<ndomains; i++)
    {
        extents[0] = 1;
        extents[1] = dxsize[i];
        extents[2] = 1;
        extents[3] = dysize[i];
        extents[4] = zstart;
        extents[5] = zstart + dzsize[i]-1;

        boundary[0] = 1;
        boundary[1] = dxsize[i];
        boundary[2] = 1;
        boundary[3] = dysize[i];

        dbi->SetExtents(i, extents);

        if (i > 0)
        {
            boundary[4] = zstart;
            boundary[5] = zstart;
            dbi->AddNeighbor(i, i-1, (i>1)?1:0, orientation, boundary);
        }

        if (i < ndomains-1)
        {
            boundary[4] = zstart + dzsize[i]-1;
            boundary[5] = zstart + dzsize[i]-1;
            dbi->AddNeighbor(i, i+1, 0, orientation, boundary);
        }

        dbi->Finish(i);

        zstart += dzsize[i]-1;
    }

    void_ref_ptr vr = void_ref_ptr(dbi,
                                   avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
}

// ****************************************************************************
//  Method:  avtRectFileFormat::ReadVizFile
//
//  Purpose:
//    Read the .viz (database meta-data) file
//
//  Arguments:
//    in         the open ifstream to read from
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
// ****************************************************************************
void
avtRectFileFormat::ReadVizFile(ifstream &in)
{
    int i;
    string buff;

    in >> buff;
    if (buff != "name")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> basename;

    in >> buff;
    if (buff != "domains")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> ndomains;

    in >> buff;
    if (buff != "timesteps")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> ntimesteps;

    in >> buff;
    if (buff != "variables")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> nvars;

    for (i=0; i<nvars;i++)
    {
        in >> buff;
        varnames.push_back(buff);
    }

    in >> buff;
    if (buff != "size")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> xsize >> ysize >> zsize;

    for (i=0; i<ndomains; i++)
    {
        in >> buff;
        int x,y,z;
        in >> x >> y >> z;
        dxsize.push_back(x);
        dysize.push_back(y);
        dzsize.push_back(z);
        numpts.push_back(x*y*z);
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
