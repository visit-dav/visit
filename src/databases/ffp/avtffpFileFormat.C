// A reader for far field pattern also called antenna diagram or radiation pattern.
// Implements 3 ways to plot the mesh:
//  1) legacy, precondition is theta starts from 0 and ends at 180,
//     phi starts at phi0, ends near phi0+2*pi
//  2) user may have a ".unv" file that shall be used for connecting nodes to element
//     i.e. for making the mesh
//  3) in default, the code calls stripack of John Burkardt to make a triangle mesh
//     automagically
// Makes prints go to terminal or to vlog files as usual
#define INTERACTIVEREAD 0
// Validate the following if gzip is installed on your system.
// VisIt usually assumes it is installed.
#define GZSTUFF 1

#include <avtffpFileFormat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <math.h>
#include <string>

#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkCellType.h>

#include <avtCallback.h>
#include <avtIntervalTree.h>
#include <avtMaterial.h>

#include <avtDatabaseMetaData.h>

#include <Environment.h>
#include <Expression.h>

#include <DebugStream.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using     std::string;
using namespace std;

typedef void (*trmeshp_t)(int *np, double *xp, double *yp, double *zp,
    int *list, int *lptr, int *lend, int *lnew, int *near, int *next,
    double *dist, int *ier);

typedef void (*trlistp_t)(int *np, int *list, int *lptr, int *lend, int *nrow,
    int *nt, int *ltri, int *ier);

static trmeshp_t _trmeshp = 0;
static trlistp_t _trlistp = 0;

static int InitLibStripack()
{
    static char const * const envar = "VISIT_FFP_STRIPACK_PATH";
#ifdef _WIN32
    HINSTANCE libh = NULL;
    if (Environment::exists(envar))
        libh = LoadLibrary(Environment::get(envar).c_str());
    if (!libh)
        libh = LoadLibrary("libstripack.dll");
    if (!libh)
        return 1;

    _trmeshp = (trmeshp_t) GetProcAddress(libh, "trmesh");
   if (!_trmeshp)
        _trmeshp = (trmeshp_t) GetProcAddress(libh, "trmesh_");
   if (!_trmeshp)
        _trmeshp = (trmeshp_t) GetProcAddress(libh, "TRMESH");
   if (!_trmeshp)
        _trmeshp = (trmeshp_t) GetProcAddress(libh, "TRMESH_");

    _trlistp = (trlistp_t) GetProcAddress(libh, "trlist");
   if (!_trlistp)
        _trlistp = (trlistp_t) GetProcAddress(libh, "trlist_");
   if (!_trlistp)
        _trlistp = (trlistp_t) GetProcAddress(libh, "TRLIST");
   if (!_trlistp)
        _trlistp = (trlistp_t) GetProcAddress(libh, "TRLIST_");

    // We can't ever close it or we loose access to the functions
    //FreeLibrary(libh);
#else
    int const dlmode = RTLD_LAZY|RTLD_LOCAL;
    void *libh = 0;
    if (Environment::exists(envar))
        libh = dlopen(Environment::get(envar).c_str(), dlmode);
    if (!libh)
        libh = dlopen("libstripack.so", dlmode);
    if (!libh)
        libh = dlopen("libstripack.dylib", dlmode);
    if (!libh)
        return 1;


   // Account for various fortran name mangling schemes
   // https://en.wikipedia.org/wiki/Name_mangling#Fortran
    _trmeshp = (trmeshp_t) dlsym(libh, "trmesh");
   if (!_trmeshp)
        _trmeshp = (trmeshp_t) dlsym(libh, "trmesh_");
   if (!_trmeshp)
        _trmeshp = (trmeshp_t) dlsym(libh, "TRMESH");
   if (!_trmeshp)
        _trmeshp = (trmeshp_t) dlsym(libh, "TRMESH_");

    _trlistp = (trlistp_t) dlsym(libh, "trlist");
   if (!_trlistp)
        _trlistp = (trlistp_t) dlsym(libh, "trlist_");
   if (!_trlistp)
        _trlistp = (trlistp_t) dlsym(libh, "TRLIST");
   if (!_trlistp)
        _trlistp = (trlistp_t) dlsym(libh, "TRLIST_");

    // We can't ever close it or we loose access to the functions
    //dlclose(libh);
#endif

    return 0;
}

// Forces initialization at plugin load time
static const int dummy = InitLibStripack();

static void mytrmesh(int *np, double *xp, double *yp, double *zp,
    int *list, int *lptr, int *lend, int *lnew, int *lnear, int *next,
    double *dist, int *ier)
{
    if (_trmeshp)
    {
        (*_trmeshp)(np, xp, yp, zp, list, lptr, lend, lnew, lnear, next, dist, ier);
        return;
    }

    static bool first = true;
#ifdef PARALLEL
    if (first && PAR_Rank() == 0)
#else
    if (first)
#endif
    {
        char msg[1024];
        snprintf(msg, sizeof(msg), "\n\n"
            "trmesh method and/or libstripack not available.\n"
            "Please install libstripack to the VisIt installation lib dir or set\n"
            "VISIT_FFP_STRIPACK_PATH enviornment variable to specify its location.\n");
        if (!avtCallback::IssueWarning(msg))
            std::cerr << msg << std::endl;
        first = false;
    }

    *ier = -4;
    return;
}

static void mytrlist(int *np, int *list, int *lptr, int *lend,
    int *nrow, int *nt, int *ltri, int *ier)
{
    if (_trlistp)
    {
        (*_trlistp)(np, list, lptr, lend, nrow, nt, ltri, ier);
        return;
    }
}

// ****************************************************************************
//  Method: avtffpFileFormat constructor
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Sat Apr 28 11:57:59 PDT 2018
//
// ****************************************************************************

avtffpFileFormat::avtffpFileFormat(const char *fname)
    : avtSTSDFileFormat(fname)
{
    // INITIALIZE DATA MEMBERS
    fileRead = false;
    filename = fname ; // strdup(fname);
#if INTERACTIVEREAD
    if (debuglevel >= 1) fprintf(stdout, "Constructor file %s\n",filename.c_str());
#else
    debug1 << "Constructor file " << filename << endl;
#endif
    // File handles
    handle = NULL;
    gzhandle = Z_NULL;
    // Data contents
    nbtheta = 0;
    nbphi = 0;
    valeurs = NULL ;
    maleurs = NULL ;
    thetas = NULL ;
    phis = NULL ;
    fac = (double) 1.0 ; 
    phi0 = (double) 0.0 ;
    // Sets the mesh's range
    range[0] = 1.e30;
    range[1] = -1.e30;
    range[2] = 1.e30;
    range[3] = -1.e30;
    range[4] = 1.e30;
    range[5] = -1.e30;
    imod = 0 ; // Assume regular complete theta phi
    np = 0 ; // number of points for the free format
    nt = 0 ; // number of elements in the mesh
    // Time manager
    ffpTime = INVALID_TIME;
    ffpCycle = INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtffpFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Sat Apr 28 11:57:59 PDT 2018
//
// ****************************************************************************

void
avtffpFileFormat::FreeUpResources(void)
{
    if (valeurs != NULL)
    {
        free(valeurs) ;
        free(maleurs) ;
        valeurs = NULL ; 
        maleurs = NULL ; 
    }
    if (thetas != NULL)
    {
        free(thetas) ;
        free(phis) ;
        thetas = NULL ;
        phis = NULL ;
    }
    // Deallocate the node list for all elements:
    set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
    for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        delete [] itre->nodes;

    meshUnvElements.clear();

    fileRead = false;
}

// ****************************************************************************
//  Method: avtffpFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

avtffpFileFormat::~avtffpFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtffpFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Sat Apr 28 11:57:59 PDT 2018
//
// ****************************************************************************

void
avtffpFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
#ifdef PARALLEL
    md->SetFormatCanDoDomainDecomposition(true) ;
#endif
    // Make sure file is read
    if (!fileRead)
    {
#if INTERACTIVEREAD
        if (debuglevel >= 1) fprintf(stdout, "PopulateDatabaseMetaData ReadFile file %s\n",filename.c_str());
#else
        debug1 << "PopulateDatabaseMetaData ReadFile file " << filename << endl;
#endif
        ReadFile();
    }

    // CODE TO ADD A MESH
    string meshname = "mesh";
    //
    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH,
    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH
    // avtMeshType mt = AVT_RECTILINEAR_MESH;
    avtMeshType mt = AVT_UNSTRUCTURED_MESH;

    int nblocks = 1;
    int block_origin = 0;
    int spatial_dimension = 3;
    int topological_dimension = 2;
    //
    // Here's the call that tells the meta-data object that we have a mesh:
    //
#if INTERACTIVEREAD
    if (debuglevel >= 1) 
    {
        fprintf(stdout," X range = %lf,%lf\n",range[0],range[1]);
        fprintf(stdout," Y range = %lf,%lf\n",range[2],range[3]);
        fprintf(stdout," Z range = %lf,%lf\n",range[4],range[5]);
    }
#else
    debug1 << " X range = (" << range[0] << "," << range[1] << ")" << endl;
    debug1 << " Y range = (" << range[2] << "," << range[3] << ")" << endl;
    debug1 << " Z range = (" << range[4] << "," << range[5] << ")" << endl;
#endif
    // Basic deformed topology
    AddMeshToMetaData(md, meshname, mt, range, nblocks, block_origin,
                      spatial_dimension, topological_dimension);
    //
    // CODE TO ADD A SCALAR VARIABLE
    //
    string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    AddScalarVarToMetaData(md, "mffp", mesh_for_this_var, AVT_NODECENT);
    //

    //
    // CODE TO ADD A VECTOR VARIABLE
    //
    int vector_dim = 3;
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    AddVectorVarToMetaData(md, "mffpvector", mesh_for_this_var, AVT_NODECENT,vector_dim);
    //
    // Mesh of a sphere or radius 1
    double mrange[6] ;
    mrange[0] = -1. ;  mrange[1] = +1. ;  mrange[2] = -1. ;  mrange[3] = +1. ;  mrange[4] = -1. ;  mrange[5] = +1. ; 
    AddMeshToMetaData(md, "sphere", AVT_UNSTRUCTURED_MESH, mrange, nblocks, block_origin,
                      spatial_dimension, topological_dimension);
    AddScalarVarToMetaData(md, "sffp", "sphere", AVT_NODECENT);
    AddVectorVarToMetaData(md, "sffpvector", "sphere", AVT_NODECENT,vector_dim);

    // Mesh on a defoirmed sphere with a radius from 1 to 2
    mrange[0] = -2. ;  mrange[1] = +2. ;  mrange[2] = -2. ;  mrange[3] = +2. ;  mrange[4] = -2. ;  mrange[5] = +2. ; 
    AddMeshToMetaData(md, "cphere", AVT_UNSTRUCTURED_MESH, mrange, nblocks, block_origin,
                      spatial_dimension, topological_dimension);
    AddScalarVarToMetaData(md, "cffp", "cphere", AVT_NODECENT);
    AddVectorVarToMetaData(md, "cffpvector", "cphere", AVT_NODECENT,vector_dim);

    if (debuglevel >= 1) {
        // Add the tet mesh
        AddMeshToMetaData(md, "tesh", AVT_UNSTRUCTURED_MESH, range, nblocks, block_origin,
                          spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "tffp", "tesh", AVT_NODECENT);
        AddVectorVarToMetaData(md, "tffpvector", "tesh", AVT_NODECENT,vector_dim);
    }
    //
    // CODE TO ADD A TENSOR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    // int tensor_dim = 9;
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);
    //

    //
    // CODE TO ADD A MATERIAL
    //
    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes
    // string matname = ...
    // int nmats = ...;
    // vector<string> mnames;
    // for (int i = 0 ; i < nmats ; i++)
    // {
    //     char str[32];
    //     sprintf(str, "mat%d", i);
    //     -- or -- 
    //     strcpy(str, "Aluminum");
    //     mnames.push_back(str);
    // }
    // 
    // Here's the call that tells the meta-data object that we have a mat:
    //
    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);
    //
    //
    // Here's the way to add expressions:
    //Expression momentum_expr;
    //momentum_expr.SetName("momentum");
    //momentum_expr.SetDefinition("{u, v}");
    //momentum_expr.SetType(Expression::VectorMeshVar);
    //md->AddExpression(&momentum_expr);
    //Expression KineticEnergy_expr;
    //KineticEnergy_expr.SetName("KineticEnergy");
    //KineticEnergy_expr.SetDefinition("0.5*(momentum*momentum)/(rho*rho)");
    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);
    //md->AddExpression(&KineticEnergy_expr);
#ifdef PARALLEL
    if (PAR_Rank() != 0) 
        avtffpFileFormat::FreeUpResources() ;
#endif
}


// ****************************************************************************
//  Method: avtffpFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Sat Apr 28 11:57:59 PDT 2018
//
// ****************************************************************************

vtkDataSet *
avtffpFileFormat::GetMesh(const char *meshname)
{
#ifdef MDSERVER
    return 0;
#endif
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return 0 ;
#endif
#if INTERACTIVEREAD
    if (debuglevel >= 1) fprintf(stdout, "meshname=%s\n",meshname);
#else
    debug1 << "meshname= " << meshname << endl;
#endif
    if (!fileRead)
    {
#if INTERACTIVEREAD
        if (debuglevel >= 2) fprintf(stdout, "ReadFile meshname=%s for file %s\n",meshname,filename.c_str());
#else
        debug2 << "ReadFile meshname= " << meshname << " for file " << filename << endl;
#endif
        ReadFile();
    }
    if (strcmp(meshname, "mesh") == 0 || strcmp(meshname, "tesh") == 0 || strcmp(meshname, "sphere") == 0 || strcmp(meshname, "cphere") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        if (imod == 0)
        {
            if (phicomplete == 1 && thetacomplete == 1)
            {
#if INTERACTIVEREAD
                if (debuglevel >= 1) fprintf(stdout,"Building mesh #nodes=%d\n",nbtheta*nbphi+2);
#else
                debug1 << "Building mesh #nodes= " << nbtheta*nbphi+2 << endl;
#endif
                pts->SetNumberOfPoints(nbtheta*nbphi+2);
            }
            else
            {
#if INTERACTIVEREAD
                if (debuglevel >= 1) fprintf(stdout,"Building #nodes=%d\n",nbtheta*nbphi);
#else
                debug1 << "Building #nodes= " << nbtheta*nbphi << endl;
#endif
                pts->SetNumberOfPoints(nbtheta*nbphi);
            }
        } else {
#if INTERACTIVEREAD
            if (debuglevel >= 1) fprintf(stdout,"Building free mesh #nodes=%d\n",np);
#else
            debug1 << "Building free mesh #nodes= " << np << endl;
#endif
            pts->SetNumberOfPoints(np);
        }
        ugrid->SetPoints(pts);
        pts->Delete();
        double x,y,z ;
        int flag = 1;
        if (strcmp(meshname, "sphere") == 0) {
            flag = 2;
        }
        if (strcmp(meshname, "cphere") == 0) {
            flag = 3;
        }
        if (imod == 0)
        {
            if (phicomplete == 1 && thetacomplete == 1)
            {
                x =0. ; y = 0. ; z = 1.0 ;
                if (flag == 1) 
                {
                    x *= valeurs[0] ; y *= valeurs[0] ; z *= valeurs[0] ; 
                }
                if (flag == 3) 
                {
                    x *= 1.0+valeurs[0] ; y *= 1.0+valeurs[0] ; z *= 1.0+valeurs[0] ; 
                }
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"Node i=%d v=%lf m=%lf pos=(%lf,%lf,%lf)\n",0, valeurs[0], maleurs[0],x,y,z);
#else
                debug3 << "Node i=0 v=" <<  valeurs[0] << " m=" <<  maleurs[0] << " pos=(" << x << "," << y << "," << z << ")" << endl;
#endif
                pts->SetPoint(0, x, y, z);
                for (int i=0; i<nbtheta; i++)
                {
                    for (int j=0; j<nbphi; j++)
                    {
                        double theta = acos((double) -1.) * (((double) (i+1))/(nbtheta+1)) ;
                        double phi = 2 * acos(-1.) * (j)/((double) nbphi) + phi0 ;
                        x = sin(theta) * cos(phi); y = sin(theta) * sin(phi) ; z = cos(theta) ;
                        if (flag == 1) 
                        {
                            x *= valeurs[i*nbphi+j+1] ; y *= valeurs[i*nbphi+j+1] ; z *= valeurs[i*nbphi+j+1] ;
                        }
                        if (flag == 3) 
                        {
                            x *= 1.0+valeurs[i*nbphi+j+1] ; y *= 1.0+valeurs[i*nbphi+j+1] ; z *= 1.0+valeurs[i*nbphi+j+1] ;
                        }
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Node i=%d (theta=%lf,phi=%lf) v=%lf m=%lf pos=(%lf,%lf,%lf)\n",i*nbphi+j+1, theta*(180./acos(-1.)), phi*(180./acos(-1.)), valeurs[i*nbphi+j+1], maleurs[i*nbphi+j+1],x,y,z);
#else
                        debug3 << "Node i=" << i*nbphi+j+1 << " v=" <<  valeurs[i*nbphi+j+1] << " m=" <<  maleurs[i*nbphi+j+1] << " pos=(" << x << "," << y << "," << z << ")" << endl;
#endif
                        pts->SetPoint(i*nbphi+j+1, x, y, z);
                    }
                }
                x =0. ; y = 0. ; z = -1.0 ;
                if (flag == 1) 
                {
                    x *= valeurs[nbtheta*nbphi+1] ; y *= valeurs[nbtheta*nbphi+1] ; z *= valeurs[nbtheta*nbphi+1] ;
                }
                if (flag == 3) 
                {
                    x *= 1.0+valeurs[nbtheta*nbphi+1] ; y *= 1.0+valeurs[nbtheta*nbphi+1] ; z *= 1.0+valeurs[nbtheta*nbphi+1] ;
                }
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"Node i=%d v=%lf m=%lf pos=(%lf,%lf,%lf)\n",nbtheta*nbphi+1, valeurs[nbtheta*nbphi+1], maleurs[nbtheta*nbphi+1],x,y,z);
#else
                debug3 << "Node i=" << nbtheta*nbphi+1 << " v=" <<  valeurs[nbtheta*nbphi+1] << " m=" <<  maleurs[nbtheta*nbphi+1] << " pos=(" << x << "," << y << "," << z << ")" << endl;
#endif
                pts->SetPoint(nbtheta*nbphi+1, x, y, z);
            }
            else
            {
                double theta, phi ;
                for (int i=0; i<nbtheta; i++)
                {
                    for (int j=0; j<nbphi; j++)
                    {
                        theta = thetas[i] * (acos(-1.)/180.) ;
                        phi = phis[j] * (acos(-1.)/180.) ;
                        x = sin(theta) * cos(phi); y = sin(theta) * sin(phi) ; z = cos(theta) ;
                        if (flag == 1) 
                        {
                            x *= valeurs[i*nbphi+j] ; y *= valeurs[i*nbphi+j] ; z *= valeurs[i*nbphi+j] ;
                        }
                        if (flag == 3) 
                        {
                            x *= 1.0+valeurs[i*nbphi+j] ; y *= 1.0+valeurs[i*nbphi+j] ; z *= 1.0+valeurs[i*nbphi+j] ;
                        }
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Node i=%d (theta=%lf,phi=%lf) v=%lf m=%lf pos=(%lf,%lf,%lf)\n",i*nbphi+j, theta*(180./acos(-1.)), phi*(180./acos(-1.)), valeurs[i*nbphi+j], maleurs[i*nbphi+j],x,y,z);
#else
                        debug3 << "Node i=" << i*nbphi+j << " theta=" << theta*(180./acos(-1.)) << " phi=" << phi*(180./acos(-1.)) << " v=" <<  valeurs[i*nbphi+j] << " m=" <<  maleurs[i*nbphi+j] << " pos=(" << x << "," << y << "," << z << ")" << endl;
#endif
                        pts->SetPoint(i*nbphi+j, x, y, z);
                    }
                }
            }
        }
        else
        {
            double theta, phi ;
            for (int k=0; k<np; k++)
            {
                theta = thetas[k] * (acos(-1.)/180.) ;
                phi = phis[k] * (acos(-1.)/180.) ;
                x = sin(theta) * cos(phi); y = sin(theta) * sin(phi) ; z = cos(theta) ;
                if (flag == 1) 
                {
                    x *= valeurs[k] ; y *= valeurs[k] ; z *= valeurs[k] ;
                }
                if (flag == 3) 
                {
                    x *= 1.0+valeurs[k] ; y *= 1.0+valeurs[k] ; z *= 1.0+valeurs[k] ;
                }
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"Node i=%d (theta=%lf,phi=%lf) v=%lf m=%lf pos=(%lf,%lf,%lf)\n",k, theta*(180./acos(-1.)), phi*(180./acos(-1.)), valeurs[k], maleurs[k], x,y,z);
#else
                debug3 << "Node i=" << k << " theta=" << theta*(180./acos(-1.)) << " phi=" << phi*(180./acos(-1.)) << " v=" <<  valeurs[k] << " m=" <<  maleurs[k] << " pos=(" << x << "," << y << "," << z << ")" << endl;
#endif
                pts->SetPoint(k, x, y, z);
            }
        }

        // Build the mesh
        flag = 1;
        if (strcmp(meshname, "tesh") == 0) {
            flag = 2;
        }
        int nbelts = 0 ;
        if (flag == 1)
        {
            if (phicomplete == 1 && thetacomplete == 1)
            {
                nbelts = nbphi*(nbtheta+1) ;
            }
            else if (phicomplete == 1)
            {
                // All around
                nbelts = nbphi*(nbtheta-1) ;
            }
            else
            {
                nbelts=(nbphi-1)*(nbtheta-1) ;
            }
        }
        else
        {
            if (phicomplete == 1 && thetacomplete == 1)
            {
                nbelts = nbphi*(2*nbtheta);
            }
            else if (phicomplete == 1)
            {
                // All around
                nbelts = 2*nbphi*(nbtheta-1) ;
            }
            else
            {
                nbelts = 2*(nbphi-1)*(nbtheta-1);
            }
        }
        if (imod != 0) {
            nbelts = nt ;
        }
#if INTERACTIVEREAD
        if (debuglevel >= 1) fprintf(stdout,"Building #elements=%d\n",nbelts);
#else
        debug1 << "Building #elements= " << nbelts << endl;
#endif
        ugrid->Allocate(nbelts);
        vtkIdType verts[4];
        if (imod == 0)
        {
            if (phicomplete == 1 && thetacomplete == 1)
            {
                for (int j=0; j<nbphi; j++)
                {
                    verts[0] = j+1 ; verts[1] = (j+1) % nbphi + 1 ; verts[2] = 0 ;
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout,"\t + TRI=(%d,%d,%d)\n",verts[0],verts[1],verts[2]) ;
#else
                    debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
#endif
                }
                for (int i=1; i<nbtheta; i++)
                {
                    for (int j=0; j<nbphi; j++)
                    {
                        verts[0] = i*nbphi+(j+1) ;
                        verts[1] = i*nbphi + (j+1) % nbphi + 1 ;
                        verts[2] = (i-1)*nbphi + (j+1) % nbphi + 1 ;
                        verts[3] = (i-1)*nbphi + (j+1);
                        if (flag == 1)
                        {
                            ugrid->InsertNextCell(VTK_QUAD, 4, verts);
#if INTERACTIVEREAD
                            if (debuglevel >= 3) fprintf(stdout,"\t + QUAD=(%d,%d,%d,%d)\n",verts[0],verts[1],verts[2],verts[3]) ;
#else
                            debug3 << "\t + QUAD=(" << verts[0] << "," << verts[1] << "," <<verts[2] << "," <<verts[3] << ")" << endl;
#endif
                        } else {
                            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                            verts[0] = verts[2] ;
                            verts[1] = verts[3] ; 
                            verts[2] = i*nbphi+(j+1) ;
                            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                        }
                    }
                }
                for (int j=0; j<nbphi; j++)
                {
                    verts[0] = nbtheta*nbphi - j ; verts[1] = nbtheta*nbphi - ((j+1) % nbphi) ; verts[2] = nbtheta*nbphi + 1 ;
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout,"\t + TRI=(%d,%d,%d)\n",verts[0],verts[1],verts[2]) ;
#else
                    debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
#endif
                }
            }
            else if (phicomplete == 1)
            {
                for (int i=1; i<nbtheta; i++)
                {
                    for (int j=0; j<nbphi; j++)
                    {
                        verts[0] = i*nbphi+(j) ;
                        verts[1] = i*nbphi + (j+1) % nbphi ;
                        verts[2] = (i-1)*nbphi + (j+1) % nbphi ;
                        verts[3] = (i-1)*nbphi + (j);
                        if (flag == 1)
                        {
                            ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                            debug3 << "\t + QUAD=(" << verts[0] << "," << verts[1] << "," <<verts[2] << "," <<verts[3] << ")" << endl;
                        }
                        else
                        {
                            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                            debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
                            verts[0] = verts[2] ;
                            verts[1] = verts[3] ; 
                            verts[2] = i*nbphi+(j) ;
                            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                            debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
                        }
                    }
                }
            }
            else
            {
                for (int i=1; i<nbtheta; i++)
                {
                    for (int j=1; j<nbphi; j++)
                    {
                        verts[0] = i*nbphi+(j-1) ;
                        verts[1] = i*nbphi + j ;
                        verts[2] = (i-1)*nbphi + j ;
                        verts[3] = (i-1)*nbphi + (j-1);
                        if (flag == 1)
                        {
                            ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                            debug3 << "\t + QUAD=(" << verts[0] << "," << verts[1] << "," <<verts[2] << "," <<verts[3] << ")" << endl;
                        }
                        else
                        {
                            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                            debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
                            verts[0] = verts[2] ;
                            verts[1] = verts[3] ; 
                            verts[2] = i*nbphi+(j-1) ;
                            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                            debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
                        }
                    }
                }
            }
        } else {
            set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
            {
                // fprintf(stdout, "Print elt label=%d type=%d\n", itre->label, itre->typelt) ;
                switch (itre->typelt)
                {
                case 91:
                    for (int i=0; i < 3; i++)
                    {
                        //anUnvNode.label = itre->nodes[i];
                        //itrn = meshUnvNodes.find(anUnvNode);
                        //verts[i] = itrn->number;
                        verts[i] = itre->nodes[i] - 1 ; // Ideas stores at index 1, vtk starts at index 0
                    }
#if INTERACTIVEREAD
                    if (debuglevel >= 2) fprintf(stdout, "elt %d, node=(%d,%d,%d)\n",itre->number,itre->nodes[0],itre->nodes[1],itre->nodes[2]);
#else
                    debug3 << "\t + TRI=(" << verts[0] << "," << verts[1] << "," <<verts[2] << ")" << endl;
#endif
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                    break;
                case 94:
                    for (int i=0; i < 4; i++)
                    {
                        //anUnvNode.label = itre->nodes[i];
                        //itrn = meshUnvNodes.find(anUnvNode);
                        //verts[i] = itrn->number;
                        verts[i] = itre->nodes[i] - 1 ;
                    }
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout, "elt %d, node=(%d,%d,%d,%d)\n",itre->number,itre->nodes[0],itre->nodes[0],itre->nodes[1],itre->nodes[2],itre->nodes[3]);
#else
                    debug3 << "\t + QUAD=(" << verts[0] << "," << verts[1] << "," <<verts[2] << "," <<verts[3] << ")" << endl;
#endif
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                    break;
                default:
                    break;
                }
            }
        }
        return ugrid;
    }
    return 0;
}


// ****************************************************************************
//  Method: avtffpFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Sat Apr 28 11:57:59 PDT 2018
//
// ****************************************************************************

vtkDataArray *
avtffpFileFormat::GetVar(const char *varname)
{
#ifdef MDSERVER
    return 0;
#endif
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return 0 ;
#endif
#if INTERACTIVEREAD
    if (debuglevel >= 1) fprintf(stdout, "FFP: GetVar function is called, var=%s\n",varname);
#else
    debug1 << "FFP: GetVar function is called, var=" << varname << endl ;
#endif
    // Make sure file is read
    if (!fileRead)
    {
#if INTERACTIVEREAD
        if (debuglevel >= 2) fprintf(stdout, "ReadFile varname=%s for file %s\n",varname,filename.c_str());
#else
        debug2 << "ReadFile varname= " << varname << " for file " << filename << endl;
#endif
        ReadFile();
    }

    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);
    //

    //
    // If you do have a scalar variable, here is some code that may be helpful.
    //
    if (strcmp(varname, "mffp") == 0 || strcmp(varname, "sffp") == 0 || strcmp(varname, "cffp") == 0 || strcmp(varname, "tffp") == 0) {
        int ntuples ;
        if (imod == 0)
        {
            if (phicomplete == 1 && thetacomplete == 1)
                ntuples = nbtheta*nbphi+2; // this is the number of entries in the variable: nodes
            else
                ntuples = nbtheta*nbphi ;
        }
        else
            ntuples = np ;

#if INTERACTIVEREAD
        if (debuglevel >= 1) fprintf(stdout,"Scalar FFP values is %d\n",ntuples);
#else
        debug1 << "Scalar FFP values is" << ntuples << endl ;
#endif
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(ntuples);
        for (int i=0; i<ntuples; i++)
        {
#if INTERACTIVEREAD
            if (debuglevel >= 3) fprintf(stdout,"i=%d v=%lf\n",i, valeurs[i]) ;
#else
            debug3 << "i=" << i << " v=" << maleurs[i] << endl ;
#endif
            rv->SetTuple1(i, maleurs[i]);
        }
        return rv;
    }
    return 0;
}


// ****************************************************************************
//  Method: avtffpFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Sat Apr 28 11:57:59 PDT 2018
//
// ****************************************************************************

vtkDataArray *
avtffpFileFormat::GetVectorVar(const char *varname)
{
#ifdef MDSERVER
    return 0;
#endif
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return 0 ;
#endif
#if INTERACTIVEREAD
    if (debuglevel >= 1) fprintf(stdout, "FFP: GetVectorVar function is called, var=%s\n",varname);
#else
    debug1 << "FFP: GetVectorVar function is called, var=" << varname << endl ;
#endif
    // Make sure file is read
    if (!fileRead)
    {
#if INTERACTIVEREAD
        if (debuglevel >= 2) fprintf(stdout, "ReadFile vectorvarname=%s for file %s\n",varname,filename.c_str());
#else
        debug2 << "ReadFile vectorvarname= " << varname << " for file " << filename << endl;
#endif
        ReadFile();
    }

    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);
    //

    //
    // If you do have a vector variable, here is some code that may be helpful.
    //
    if (strcmp(varname, "mffpvector") == 0 || strcmp(varname, "sffpvector") == 0 || strcmp(varname, "cffpvector") == 0 || strcmp(varname, "tffpvector") == 0) {
        int ncomps = 3;  // This is the rank of the vector - typically 2 or 3.
        int ntuples ;
        if (imod == 0)
        {
            if (phicomplete == 1 && thetacomplete == 1)
                ntuples = nbtheta*nbphi+2; // this is the number of entries in the variable: nodes
            else
                ntuples = nbtheta*nbphi ;
        }
        else
            ntuples = np ;

        vtkFloatArray *rv = vtkFloatArray::New();
        int ucomps = (ncomps == 2 ? 3 : ncomps);
        rv->SetNumberOfComponents(ucomps);
        rv->SetNumberOfTuples(ntuples);
        float *one_entry = new float[ucomps];
        if (imod == 0)
        {
            if (phicomplete == 1 && thetacomplete == 1)
            {
                one_entry[0] = 0 ; one_entry[1] = 0 ; one_entry[2] = (float) maleurs[0] ;
                rv->SetTuple(0, one_entry);
                for (int i=0; i<nbtheta; i++)
                {
                    for (int j=0; j<nbphi; j++)
                    {
                        double theta = acos((double) -1.) * (((double) (i+1))/(nbtheta+1)) ;
                        double phi = 2 * acos(-1.) * (j)/((double) nbphi) + phi0 ;
                        one_entry[0] = (float) maleurs[i*nbphi+j+1] * sin(theta) * cos(phi);
                        one_entry[1] = (float) maleurs[i*nbphi+j+1] * sin(theta) * sin(phi);
                        one_entry[2] = (float) maleurs[i*nbphi+j+1] * cos(theta) ;
                        rv->SetTuple(i*nbphi+j+1, one_entry); 
                    }
                }
                one_entry[0] = 0 ; one_entry[1] = 0 ; one_entry[2] = - (float) maleurs[nbtheta*nbphi+1] ;
                rv->SetTuple(nbtheta*nbphi+1, one_entry);
            }
            else
            {
                double theta, phi ;
                for (int i=0; i<nbtheta; i++)
                {
                    for (int j=0; j<nbphi; j++)
                    {
                        theta = thetas[i] * (acos(-1.)/180.) ;
                        phi = phis[j] * (acos(-1.)/180.) ;
                        one_entry[0] = (float) maleurs[i*nbphi+j] * sin(theta) * cos(phi);
                        one_entry[1] = (float) maleurs[i*nbphi+j] * sin(theta) * sin(phi);
                        one_entry[2] = (float) maleurs[i*nbphi+j] * cos(theta) ;
                        rv->SetTuple(i*nbphi+j, one_entry); 
                    }
                }
            }
        }
        else
        {
            for (int i=0; i<np; i++)
            {
                double theta = thetas[i] * (acos(-1.)/180.) ;
                double phi = phis[i] * (acos(-1.)/180.) ;
                one_entry[0] = (float) maleurs[i] * sin(theta) * cos(phi);
                one_entry[1] = (float) maleurs[i] * sin(theta) * sin(phi);
                one_entry[2] = (float) maleurs[i] * cos(theta) ;
                rv->SetTuple(i, one_entry); 
            }
        }
        delete [] one_entry;
        return rv;
    }
    return 0;
}
void
avtffpFileFormat::ReadFile(void)
{
    if (fileRead)
    {
#if INTERACTIVEREAD
        if (debuglevel >= 1) fprintf(stdout, "ReadFile for file %s\n",filename.c_str());
#else
        debug1 << "ReadFile for file " << filename << endl;
#endif
        return;
    }

    // fileRead = true ;
    ifstream in(filename.c_str());
    if (!in)
    {
        fprintf(stdout, "Raise an exception for %s\n",filename.c_str());
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }


    debuglevel = 0;
    if (DebugStream::Level5())
        debuglevel = 5;
    else if (DebugStream::Level4())
        debuglevel = 4;
    else if (DebugStream::Level3())
        debuglevel = 3;
    else if (DebugStream::Level2())
        debuglevel = 2;
    else if  (DebugStream::Level1())
        debuglevel = 1;

#if defined(MDSERVER)
    debuglevel = 0;
#endif
#ifdef PARALLEL
    // Parallel mode other stuffs, read the data but keep silence please
    if (PAR_Rank() != 0) {
        if (debuglevel > 1) {
            debuglevel = 1;
        }
    }
#endif
    ffpTime = INVALID_TIME;
    ffpCycle = INVALID_CYCLE;
    TRY
    {
        nbtheta = 0;
        nbphi = 0;
        np = 0 ;
        //    maleurs = (double *)calloc((size_t ) (10000),sizeof(double)) ;
        //    valeurs = (double *)calloc((size_t ) (10000),sizeof(double)) ;
        double theta, phi, ffp, phase ;
        double ffpmax = -1.e30 ;

        set<RealStuff, RealStuff::compare_RealStuff> malistetheta;  // Sub list, sorted on the fly
        set<RealStuff, RealStuff::compare_RealStuff> malistephi;  // Sub list, sorted on the fly
        set<RealStuff, RealStuff::compare_RealStuff>::iterator itrl; // Sub-list node iterator
        vector<double> malisteffp; // the ffp values
        vector<double> thetasffp; // the theta values as a flat vector
        vector<double> phisffp; // the theta values as a flat vector

        RealStuff anotherThetaStuff ;
        RealStuff anotherPhiStuff ;
        RealStuff aRealStuff;
        anotherThetaStuff.number = 0 ;
        anotherPhiStuff.number = 0 ;
        int i = 0 ; int j = 0 ;

        if (strstr(filename.c_str(), ".gz") == NULL)
        {
            handle = fopen(filename.c_str(), "r");
            if (handle == NULL)
            {
                EXCEPTION1(InvalidDBTypeException, "This ffp file could not be openend.");
            }

#if INTERACTIVEREAD
            if (debuglevel >= 1) fprintf(stdout,"On the way to read ffp file %s\n",filename.c_str());
#else
            debug1 << "On the way to read ffp file " << filename << endl;
#endif
            const int len = 2048; // Longest line length
            char buf[len]; // A line length
            int nret = 0 ;
            while (fgets(buf, len, handle) != NULL)
            {
                // Look for factor rindex
                if (strstr((const char*)buf, "FAC=") != NULL)
                {
                    sscanf(strstr((const char*)buf, "FAC=")+4, "%lf\n", &fac);
#if INTERACTIVEREAD
                   if (debuglevel >= 1) fprintf(stdout,"fac=%lf\n",fac) ;
#else
                   debug1 << "fac=" << fac << endl;
#endif
                }
                // Look for frequency value
                if (strstr((const char*)buf, "FRE=") != NULL)
                {
                    sscanf(strstr((const char*)buf, "FRE=")+4, "%lf\n", &ffpTime);
#if INTERACTIVEREAD
                   if (debuglevel >= 1) fprintf(stdout,"frequency=%lf\n",ffpTime) ;
#else
                   debug1 << "ffpTime=" << ffpTime << endl;
#endif
                }

                // Avoid comment lines:
                if (buf[0] == '#' || buf[0] == '%') 
                {
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout,"Skipping comment line.\n") ;
#else
                    debug3 << "Skipping comment line" << endl;
#endif
                    continue ;
                }
                // Avoid empty lines:
                if (strlen(buf) < 10) 
                {
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout,"Skipping empty line.\n") ;
#else
                    debug3 << "Skipping empty line" << endl;
#endif
                    continue ;
                }
                
                // Make sure format is C readable:
                for (int i1=0; i1<strlen(buf); i1++)
                    if (buf[i1] == 'D')
                        buf[i1] = 'E';

                nret = sscanf(buf, "%lf %lf %lf %lf\n", &theta, &phi, &ffp, &phase);
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"Reading %d values: (%lf,%lf,%lf,%lf)\n", nret, theta, phi, ffp, phase) ;
#else
                debug3 << "Adding theta[" << anotherThetaStuff.number << "]=" << theta << " ffp=" << ffp << endl;
#endif
                if (nret == 4)
                {

                    // nret = sscanf(buf, "%lf %lf %lf %lf\n", &theta, &phi, &ffp, &phase);
                    // Insert another theta and phi value, looking for unique matrix theta and phi
                    aRealStuff.valeur = theta ;
                    itrl = malistetheta.find(aRealStuff);
                    if (itrl == malistetheta.end())
                    {
                        anotherThetaStuff.valeur = theta ;
                        malistetheta.insert(anotherThetaStuff);
                        anotherThetaStuff.number++;
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Adding theta[%d]=%lf ffp=%lf\n",anotherThetaStuff.number,theta,ffp) ;
#else
                        debug3 << "Adding theta[" << anotherThetaStuff.number << "]=" << theta << " ffp=" << ffp << endl;
#endif
#if CHECKTHETACOMPLETE
                        if (anotherThetaStuff.number == 1)
                        {
                            // Check value is 0
                            if (fabs(theta) > 1.e-6) 
                            {
                                fprintf(stdout, "First theta value %lf not zero in %s\n",theta,filename.c_str());
                                EXCEPTION1(InvalidFilesException, filename.c_str());
                            }
                        }
#endif
                    }
                    aRealStuff.valeur = phi ;
                    itrl = malistephi.find(aRealStuff);
                    if (itrl == malistephi.end())
                    {
                        anotherPhiStuff.valeur = phi ;
                        malistephi.insert(anotherPhiStuff);
                        anotherPhiStuff.number++;
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Adding phi[%d]=%lf ffp=%lf\n",anotherPhiStuff.number,phi,ffp) ;
#else
                        debug3 << "Adding phi[" << anotherPhiStuff.number << "]=" << phi << " ffp=" << ffp << endl;
#endif
                        if (anotherPhiStuff.number == 1)
                        {
                            // Check value is 0
                            if (phi != 0.) 
                            {
                                phi0 = phi ;
                            }
                        }
                    }
                    else
                    {
                        // Coherence check:
                        if (nbphi == 0)
                            nbphi = anotherPhiStuff.number ;
                    }
                    np++ ; // Increase the number of points being read
                    ffpmax = max(ffpmax, ffp) ;
                    malisteffp.push_back(ffp);
                    thetasffp.push_back(theta);
                    phisffp.push_back(phi);
                }
            }
            fclose(handle);
        }
#if GZSTUFF
        else
        {
#if INTERACTIVEREAD
            if (debuglevel >= 1) fprintf(stdout,"On the way to read ffp.gz file %s\n",filename.c_str());
#else
            debug1 << "On the way to read ffp.gz file " << filename << endl;
#endif
            gzhandle = gzopen(filename.c_str(), "r"); // Open the target file for reading
            if (gzhandle == Z_NULL)
            {
                EXCEPTION1(InvalidDBTypeException, "This ffp.gz file could not be openend.");
            }

            const int len = 2048; // Longest line length
            char buf[len]; // A line length
            int nret = 0 ;
            while (gzgets(gzhandle, buf, len) != Z_NULL)
            {
                // Look for factor rindex
                if (strstr((const char*)buf, "FAC=") != NULL)
                {
                    sscanf(strstr((const char*)buf, "FAC=")+4, "%lf\n", &fac);
#if INTERACTIVEREAD
                   if (debuglevel >= 1) fprintf(stdout,"fac=%lf\n",fac) ;
#else
                   debug1 << "fac=" << fac << endl;
#endif
                }
                // Look for frequency value
                if (strstr((const char*)buf, "FRE=") != NULL)
                {
                    sscanf(strstr((const char*)buf, "FRE=")+4, "%lf\n", &ffpTime);
#if INTERACTIVEREAD
                   if (debuglevel >= 1) fprintf(stdout,"frequency=%lf\n",ffpTime) ;
#else
                   debug1 << "ffpTime=" << ffpTime << endl;
#endif
                }

                // Avoid comment lines:
                if (buf[0] == '#' || buf[0] == '%') 
                {
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout,"Skipping comment line.\n") ;
#else
                    debug3 << "Skipping comment line" << endl;
#endif
                    continue ;
                }
                // Avoid empty lines:
                if (strlen(buf) < 10) 
                {
#if INTERACTIVEREAD
                    if (debuglevel >= 3) fprintf(stdout,"Skipping empty line.\n") ;
#else
                    debug3 << "Skipping empty line" << endl;
#endif
                    continue ;
                }
                
                // Make sure format is C readable:
                for (int i1=0; i1<strlen(buf); i1++)
                    if (buf[i1] == 'D')
                        buf[i1] = 'E';

                nret = sscanf(buf, "%lf %lf %lf %lf\n", &theta, &phi, &ffp, &phase);
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"Reading %d values: (%lf,%lf,%lf,%lf)\n", nret, theta, phi, ffp, phase) ;
#else
                debug3 << "Adding theta[" << anotherThetaStuff.number << "]=" << theta << " ffp=" << ffp << endl;
#endif
                if (nret == 4)
                {
                    // Insert another theta and phi value, looking for unique matrix theta and phi
                    aRealStuff.valeur = theta ;
                    itrl = malistetheta.find(aRealStuff);
                    if (itrl == malistetheta.end())
                    {
                        anotherThetaStuff.valeur = theta ;
                        malistetheta.insert(anotherThetaStuff);
                        anotherThetaStuff.number++;
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Adding theta[%d]=%lf ffp=%lf\n",anotherThetaStuff.number,theta,ffp) ;
#else
                        debug3 << "Adding theta[" << anotherThetaStuff.number << "]=" << theta << " ffp=" << ffp << endl;
#endif
#if CHECKTHETACOMPLETE
                        if (anotherThetaStuff.number == 1)
                        {
                            // Check value is 0
                            if (theta != 0.) 
                            {
                                    fprintf(stdout, "First theta value not zero in %s\n",filename.c_str());
                                    EXCEPTION1(InvalidFilesException, filename.c_str());
                            }
                        }
#endif
                    }
                    aRealStuff.valeur = phi ;
                    itrl = malistephi.find(aRealStuff);
                    if (itrl == malistephi.end())
                    {
                        anotherPhiStuff.valeur = phi ;
                        malistephi.insert(anotherPhiStuff);
                        anotherPhiStuff.number++;
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Adding phi[%d]=%lf ffp=%lf\n",anotherPhiStuff.number,phi,ffp) ;
#else
                        debug3 << "Adding phi[" << anotherPhiStuff.number << "]=" << phi << " ffp=" << ffp << endl;
#endif
                        if (anotherPhiStuff.number == 1)
                        {
                            // Check value is 0
                            if (phi != 0.) 
                            {
                                phi0 = (acos(-1.)/180.)*phi ;
                            }
                        }
                    }
                    else
                    {
                        // Coherence check:
                        if (nbphi == 0)
                            nbphi = anotherPhiStuff.number ;
                    }
                    ffpmax = max(ffpmax, ffp) ;
                    malisteffp.push_back(ffp);
                }
            }
            gzclose(gzhandle);
        }
#endif
        if (handle != NULL || gzhandle != Z_NULL)
        {
            // Debugging input values
            if (debuglevel >= 2) 
            {
#if INTERACTIVEREAD
                fprintf(stdout,"theta:") ;
#else
                debug2 << "theta:" ;
#endif
                i=0 ;
                for (itrl = malistetheta.begin(); itrl != malistetheta.end(); itrl++)
                {
                    i++ ;
#if INTERACTIVEREAD
                    fprintf(stdout," %lf",itrl->valeur) ;
#else
                    debug2 << " " << itrl->valeur;
#endif
                    if (i % 10 == 0)
                    {
#if INTERACTIVEREAD
                        fprintf(stdout,"\n") ;
#else
                        debug2 << endl;
#endif
                    }
                }
                if (i % 10 != 0)
                {
#if INTERACTIVEREAD
                    fprintf(stdout,"\n") ;
#else
                     debug2 << endl;
#endif
                }
#if INTERACTIVEREAD
                fprintf(stdout,"phi:") ;
#else
                debug2 << "phi:" ;
#endif
                j=0;
                for (itrl = malistephi.begin(); itrl != malistephi.end(); itrl++)
                {
                    j++ ;
#if INTERACTIVEREAD
                    fprintf(stdout," %lf",itrl->valeur) ;
#else
                    debug2 << " " << itrl->valeur;
#endif
                    if (j % 10 == 0)
                    {
#if INTERACTIVEREAD
                        fprintf(stdout,"\n") ;
#else
                        debug2 << endl;
#endif
                    }
                }
                if (j % 10 != 0)
                {
#if INTERACTIVEREAD
                    fprintf(stdout,"\n") ;
#else
                     debug2 << endl;
#endif
                }
            }
            // Time to look for nbtheta and nbphi:
            nbtheta = anotherThetaStuff.number - 2 ;
            nbphi = anotherPhiStuff.number ;

            // Detect the kind of balayage:
            // 1) Is phi end next to phi start ?
            j=0;
            double phistart, phiend, phipas ;
            for (itrl = malistephi.begin(); itrl != malistephi.end(); itrl++)
            {
                j++ ;
                if (j == 1)
                {
                    phistart = itrl->valeur ;
                }
                else
                {
                    if (j == nbphi)
                    {
                        phiend = itrl->valeur ;
                    }
                }
            }
            phicomplete = -1 ;
            if ( abs(360+(int) (phiend+(phiend-phistart)/(nbphi-1)) % 360 - 360 + (int)phistart % 360) < 1)
            {
                phicomplete = 1 ;
#if INTERACTIVEREAD
                if (debuglevel >= 1)
                    fprintf(stdout,"OK, phi loop\n") ;
#else
                debug1 << "OK, phi loop" << endl ;
#endif
            }
            else
            {
                phicomplete = 0 ;
 #if INTERACTIVEREAD
               if (debuglevel >= 1)
                   cout << "KO, phi loop incomplete phistart=" << phistart << " phiend=" << phiend << " phitarget=" << phiend+(phiend-phistart)/(nbphi-1) << endl ;
#else
               debug1 << "KO, phi loop incomplete phistart=" << phistart << " phiend=" << phiend << " phitarget=" << phiend+(phiend-phistart)/(nbphi-1) << endl ;
#endif
            }
            // 2) Is theta starting at 0 and over at 180 ?
            i=0;
            double thetastart, thetaend, thetapas ;
            for (itrl = malistetheta.begin(); itrl != malistetheta.end(); itrl++)
            {
                i++ ;
                if (i == 1)
                {
                    thetastart = itrl->valeur ;
                }
                else
                {
                    if (i == malistetheta.size())
                    {
                        thetaend = itrl->valeur ;
                    }
                }
            }
            thetacomplete = -1 ;
            if ( fabs(thetastart)+fabs(thetaend-180.) < 0.1)
            {
                thetacomplete = 1 ;
#if INTERACTIVEREAD
                if (debuglevel >= 1)
                    fprintf(stdout,"OK, thetacomplete loop\n") ;
#else
                debug1 << "OK, thetacomplete loop" << endl ;
#endif
            }
            else
            {
                thetacomplete = 0 ;
                nbtheta = nbtheta + 2 ;
 #if INTERACTIVEREAD
               if (debuglevel >= 1)
                   cout << "KO, theta loop incomplete thetastart=" << thetastart << " thetaend=" << thetaend << endl ;
#else
               debug1 << "KO, theta loop incomplete thetastart=" << thetastart << " thetaend=" << thetaend << endl ;
#endif
            }

            if (thetacomplete == 1)
            {
#if INTERACTIVEREAD
                if (debuglevel >= 1)
                {
                    fprintf(stdout,"nbtheta=%d\n",nbtheta) ;
                    fprintf(stdout,"nbphi=%d\n",nbphi) ;
                    fprintf(stdout,"ffpmax=%lf\n",ffpmax) ;
                    fprintf(stdout,"Allocate=%d\n",nbtheta*nbphi+2) ;
                }
#else
                debug1 << "nbtheta=" << nbtheta << endl;
                debug1 << "nbphi=" << nbphi << endl;
                debug1 << "ffpmax=" << ffpmax << endl;
                debug1 << "Allocate=" << nbtheta*nbphi+2 << endl;
#endif
                maleurs = (double *)calloc((size_t ) (nbtheta*nbphi+2),sizeof(double)) ;
                valeurs = (double *)calloc((size_t ) (nbtheta*nbphi+2),sizeof(double)) ;
                i=0; j=0;
                //for (itrl = malisteffp.begin(); itrl != malisteffp.end(); itrl++)
                for (int k = 0; k < malisteffp.size(); k++)
                {
                    j++ ; 
                    if (j == nbphi+1)
                    {
                        j = 1 ;
                        i++ ;
                    }
                    if (debuglevel > 3) fprintf(stdout,"malisteffp[k=%d](ithe=%d,iph=%d)=%lf\n",k,i,j,malisteffp[k]) ;
                    if (i == 0)
                    {
                        if (j == 1)
                        {
                            if (debuglevel > 3) fprintf(stdout,"fill north %d\n",0) ;
                            maleurs[0] = malisteffp[k] ;
                            valeurs[0] = fac * (pow((double)10.,(malisteffp[k]-ffpmax)/20.)) ;
                        }
                        else if (j > 1)
                        {
                            // Select another value for theta if you want:
                            //valeurs[0] = max(valeurs[0], fac * malisteffp[k]) ;
                        }
                    }
                    else if (i == nbtheta + 1)
                    {
                        if (j == 1)
                        {
                            if (debuglevel > 3) fprintf(stdout,"fill south %d\n",nbtheta*nbphi+1) ;
                            maleurs[nbtheta*nbphi+1] = malisteffp[k] ;
                            valeurs[nbtheta*nbphi+1] = fac * (pow((double)10.,(malisteffp[k]-ffpmax)/20.)) ;
                        }
                        else if (j > 1)
                        {
                            // Select another value for theta if you want:
                            //valeurs[nbtheta*nbphi+1] = max(valeurs[nbtheta*nbphi+1], fac * malisteffp[k]) ;
                        }
                    }
                    else if (i > 0)
                    {
                        if (debuglevel > 3) fprintf(stdout,"fill inbetween %d\n",(i-1)*nbphi+j) ;
                        maleurs[(i-1)*nbphi+j] = malisteffp[k] ;
                        valeurs[(i-1)*nbphi+j] = fac * (pow((double)10.,(malisteffp[k]-ffpmax)/20.)) ;
                    }
                }
                // Another coherence check:
                i=-1 ;
                for (itrl = malistetheta.begin(); itrl != malistetheta.end(); itrl++)
                {
                    i++ ;
                    if (i >=1 && i <= nbtheta)
                    {
                        double theta = acos((double) -1.) * ((double)i/(nbtheta+1)) ;
                        if (fabs(theta - (acos(-1.)/180.)*itrl->valeur) > 1.e-5*fabs(theta))
                        {
                            fprintf(stdout, "theta i=%d value is %lf not %lf in file %s\n",
                                    i,theta,(acos(-1.)/180.)*itrl->valeur,filename.c_str());
                            EXCEPTION1(InvalidFilesException, filename.c_str());                        
                        }
                    }
                }
                j=0 ;
                for (itrl = malistephi.begin(); itrl != malistephi.end(); itrl++)
                {
                    double phi = 2 * acos(-1.) * (j)/((double) nbphi) + phi0 ;
                    j++ ;
                    if (fabs(cos(phi) - cos((acos(-1.)/180.)*itrl->valeur)) > 1.e-5*fabs(cos(phi)))
                    {
                        fprintf(stdout, "phi j=%d value is %lf not %lf in file %s\n",
                                j,phi,(acos(-1.)/180.)*itrl->valeur,filename.c_str());
                        EXCEPTION1(InvalidFilesException, filename.c_str());                        
                    }
                }
                // End of pre-condition coherence check
#if INTERACTIVEREAD
                if (debuglevel >= 1) fprintf(stdout,"There are #read values=%d, #used values=%d\n",malisteffp.size(),nbtheta*nbphi+2) ;
#else
                debug1 << "There are #read values=" << malisteffp.size() << ", #used values=" << nbtheta*nbphi+2 << endl;
#endif
                malistetheta.clear();
                malistephi.clear();
                malisteffp.clear();
                // Compute the mesh range:
                double x,y,z ;
                x =0. ; y = 0. ; z = valeurs[0] ;
                range[0] = min(range[0],x);
                range[1] = max(range[1],x);
                range[2] = min(range[2],y);
                range[3] = max(range[3],y);
                range[4] = min(range[4],z);
                range[5] = max(range[5],z);
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"k=%d v=%lf m=%lf\n",0,valeurs[0],maleurs[0]) ;
#else
                debug3 << "k=" << 0 << " v=" << valeurs[0] << " m=" << maleurs[0] << endl;
#endif

                for (i=0; i<nbtheta; i++)
                {
                    for (j=0; j<nbphi; j++)
                    {
                        double theta = acos((double) -1.) * (((double) (i+1))/(nbtheta+1)) ;
                        double phi = 2 * acos(-1.) * (j)/((double) nbphi) + phi0 ;
                        x = sin(theta) * cos(phi); y = sin(theta) * sin(phi) ; z = cos(theta) ;
                        x *= valeurs[i*nbphi+j+1] ; y *= valeurs[i*nbphi+j+1] ; z *= valeurs[i*nbphi+j+1] ; 
                        range[0] = min(range[0],x);
                        range[1] = max(range[1],x);
                        range[2] = min(range[2],y);
                        range[3] = max(range[3],y);
                        range[4] = min(range[4],z);
                        range[5] = max(range[5],z);
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"k=%d v=%lf m=%lf\n",i*nbphi+j+1,valeurs[i*nbphi+j+1],maleurs[i*nbphi+j+1]) ;
#else
                        debug3 << "k=" << i*nbphi+j+1 << " v=" << valeurs[i*nbphi+j+1] << " m=" << maleurs[i*nbphi+j+1] << endl;
#endif
                    }
                }
                x =0. ; y = 0. ; z = - valeurs[nbtheta*nbphi+1] ;
#if INTERACTIVEREAD
                if (debuglevel >= 3) fprintf(stdout,"k=%d v=%lf m=%lf\n",nbtheta*nbphi+1,valeurs[nbtheta*nbphi+1],maleurs[nbtheta*nbphi+1]) ;
#else
                debug3 << "k=" << nbtheta*nbphi+1 << " v=" << valeurs[nbtheta*nbphi+1] << " m=" << maleurs[nbtheta*nbphi+1] << endl;
#endif
                range[0] = min(range[0],x);
                range[1] = max(range[1],x);
                range[2] = min(range[2],y);
                range[3] = max(range[3],y);
                range[4] = min(range[4],z);
                range[5] = max(range[5],z);
            }
            else
            {
                debug1 << "nbtheta=" << nbtheta << endl;
                debug1 << "nbphi=" << nbphi << endl;
                debug1 << "ffpmax=" << ffpmax << endl;

                // Partial domain is easier to perform:
                if (nbtheta*nbphi == np)
                {
                    maleurs = (double *)calloc((size_t ) (nbtheta*nbphi),sizeof(double)) ;
                    valeurs = (double *)calloc((size_t ) (nbtheta*nbphi),sizeof(double)) ;
                    thetas = (double *)calloc((size_t ) (nbtheta),sizeof(double)) ;
                    phis = (double *)calloc((size_t ) (nbphi),sizeof(double)) ;
                    i = 0;
                    for (itrl = malistetheta.begin(); itrl != malistetheta.end(); itrl++)
                        thetas[i++] = itrl->valeur ;
                    j = 0;
                    for (itrl = malistephi.begin(); itrl != malistephi.end(); itrl++)
                        phis[j++] = itrl->valeur ;
                    double x,y,z ;
                    double theta, phi ;
                    for (i=0; i< nbtheta; i++)
                    {
                        for (j=0; j< nbphi; j++)
                        {
                            theta = thetas[i] * (acos(-1.)/180.) ;
                            phi = phis[j] * (acos(-1.)/180.) ;
                            maleurs[i*nbphi+j] = malisteffp[i*nbphi+j] ;
                            valeurs[i*nbphi+j] = fac * (pow((double)10.,(malisteffp[i*nbphi+j]-ffpmax)/20.)) ;
                            x = sin(theta) * cos(phi); y = sin(theta) * sin(phi) ; z = cos(theta) ;
                            x *= valeurs[i*nbphi+j] ; y *= valeurs[i*nbphi+j] ; z *= valeurs[i*nbphi+j] ;
                            range[0] = min(range[0],x);
                            range[1] = max(range[1],x);
                            range[2] = min(range[2],y);
                            range[3] = max(range[3],y);
                            range[4] = min(range[4],z);
                            range[5] = max(range[5],z);
                        }
                    }
                }
                else
                {
                    imod = 1 ;
                    maleurs = (double *)calloc((size_t ) (np),sizeof(double)) ;
                    valeurs = (double *)calloc((size_t ) (np),sizeof(double)) ;
                    thetas = (double *)calloc((size_t ) (np),sizeof(double)) ;
                    phis = (double *)calloc((size_t ) (np),sizeof(double)) ;
                    double x,y,z ;
                    double theta, phi ;
                    for (int k=0; k< np; k++)
                    {
                        thetas[k] = thetasffp[k] ;
                        phis[k] = phisffp[k] ;
                        theta = thetas[k] * (acos(-1.)/180.) ;
                        phi = phis[k] * (acos(-1.)/180.) ;
                        maleurs[k] = malisteffp[k] ;
                        valeurs[k] = fac * (pow((double)10.,(malisteffp[k]-ffpmax)/20.)) ;
                        x = sin(theta) * cos(phi); y = sin(theta) * sin(phi) ; z = cos(theta) ;
                        x *= valeurs[k] ; y *= valeurs[k] ; z *= valeurs[k] ;
                        range[0] = min(range[0],x);
                        range[1] = max(range[1],x);
                        range[2] = min(range[2],y);
                        range[3] = max(range[3],y);
                        range[4] = min(range[4],z);
                        range[5] = max(range[5],z);
                    }
                    malisteffp.clear();
                    // Now we want to read the mesh file : 
                    // We read it as an unv file
#if 0
                    std::string str ("The sixth sick sheik's sixth sheep's sick.");
                    std::string key ("sixth");
                    std::size_t found = str.rfind(key);
                    if (found!=std::string::npos)
                        str.replace (found,key.length(),"seventh");
                    std::cout << str << '\n';
#endif
                    // We assume file is ffp.gz or ffp.gz or anything the same size as ffp
                    int i ;
                    if (strstr(filename.c_str(), ".gz") != NULL) {
                        i = filename.size() - 7 + 1 ;
                    } else {
                        i = filename.size() - 4 + 1;
                    }
                    std::string unvfile = filename ;
                    //fprintf(stdout,"UNV file is=%s\n", unvfile.c_str()) ;
                    unvfile.replace (i,3,"unv");
#if INTERACTIVEREAD
                    if (debuglevel >= 1) fprintf(stdout,"UNV file to read=%s\n", unvfile.c_str()) ;
#else
                    debug1 << "UNV file to read " << unvfile << endl;
#endif
                    int isunvread = 0 ;
                    FILE* unvhandle;
                    gzFile unvgandle ;
                    unvhandle = fopen(unvfile.c_str(), "r");
                    bool flipflop = false ;
                    const int len = 2048; // Longest line length
                    char buf[len]; // A line length
                    int label ; 
                    if (unvhandle == NULL)
                    {
#if INTERACTIVEREAD
                        if (debuglevel >= 1) fprintf(stdout, "Cannot read unv file %s\n",unvfile.c_str());
#else
                        debug1 << "Cannot read unv file " << unvfile << endl;
#endif
                        unvfile.append(".gz") ;
                        unvgandle = gzopen(unvfile.c_str(), "r");
                        if (unvgandle == Z_NULL) {
#if INTERACTIVEREAD
                            if (debuglevel >= 1) fprintf(stdout, "Cannot read unv.gz file %s\n",unvfile.c_str());
#else
                            debug1 << "Cannot read unv.gz file " << unvfile << endl;
#endif
                        } else {
#if INTERACTIVEREAD
                            if (debuglevel >= 1) fprintf(stdout, "Can read unv.gz file %s\n",unvfile.c_str());
#else
                            debug1 << "Can read unv.gz file " << unvfile << endl;
#endif
                            // Starts reading the compressed une file
                            while (gzgets(unvgandle, buf, len) != NULL) 
                            {
                                if (strstr(buf, "    -1") != NULL)
                                    flipflop = !flipflop ;
                                if (flipflop && strstr(buf, "  2412") != NULL) {
                                    // We can read the file
#if INTERACTIVEREAD
                                    if (debuglevel >= 1) fprintf(stdout, "Reading elements\n");
#else
                                    debug1 << "Reading elements " << endl;
#endif
                                    UnvElement anUnvElement;
                                    int nod[4];
                                    anUnvElement.number=0;
                                    while (gzgets(unvgandle, buf, len) != NULL)
                                    {
                                        int typelt, numprop,numat,colour,nbnel;
                                        sscanf(buf, "%d%d%d%d%d%d\n", &label, &typelt, &numprop, &numat, &colour, &nbnel);
                                        if (label < 0)
                                        {
#if INTERACTIVEREAD
                                            if (debuglevel >= 2) fprintf(stdout,"Found Element section end, nt=%d\n",nt);
#else
                                            debug2 << "Found Element section end, #cells=(" << nt << ")" << endl;
#endif
                                            isunvread = 1 ;
                                            break;
                                        }
                                        if (gzgets(unvgandle, buf, len) != NULL)
                                        {
                                            int iok = 0; 
                                            switch (typelt)
                                            {
                                            case 91:
                                                sscanf(buf, "%d %d %d\n", &nod[0], &nod[1], &nod[2]);
                                                nt++;
                                                iok = 4;
                                                break;
                                            case 94:
                                                sscanf(buf, "%d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3]);
                                                nt++;
                                                iok = 5;
                                                break;
                                            }
                                            if (iok != 0)
                                            {
#if !defined(MDSERVER)
                                                anUnvElement.label = label;
                                                anUnvElement.typelt = typelt;
                                                anUnvElement.matid = numat;
                                                anUnvElement.nodes = new int[nbnel];
                                                for (int i=0; i < nbnel; i++)
                                                    anUnvElement.nodes[i] = nod[i];

#if INTERACTIVEREAD
                                                if (debuglevel >= 4) fprintf(stdout, "\telt %d, #nodes=%d, first/last node=(%d,%d)\n",
                                                                             anUnvElement.label,nbnel,anUnvElement.nodes[0],anUnvElement.nodes[nbnel-1]);
#else
                                                debug4 << "\telt " << anUnvElement.label << ", #nodes=" << nbnel << ", first/last node=(" << anUnvElement.nodes[0] << "," << anUnvElement.nodes[nbnel-1] << ")" << endl;
#endif
                                                meshUnvElements.insert(anUnvElement);
                                                anUnvElement.number++;
#endif
                                            }
                                        }
                                    }
                                }
                            }
                            // Ends reading the unv file
                        }
                    } else {
#if INTERACTIVEREAD
                        if (debuglevel >= 1) fprintf(stdout, "Can read unv file %s\n",unvfile.c_str());
#else
                        debug1 << "Can read unv file " << unvfile << endl;
#endif
                        while (fgets(buf, len, unvhandle) != NULL) 
                        {
                            if (strstr(buf, "    -1") != NULL)
                                flipflop = !flipflop ;
                            if (flipflop && strstr(buf, "  2412") != NULL) {
                                // We can read the file
#if INTERACTIVEREAD
                                if (debuglevel >= 1) fprintf(stdout, "Reading elements\n");
#else
                                debug1 << "Reading elements " << endl;
#endif
                                UnvElement anUnvElement;
                                int nod[4];
                                anUnvElement.number=0;
                                while (fgets(buf, len, unvhandle) != NULL)
                                {
                                    int typelt, numprop,numat,colour,nbnel;
                                    sscanf(buf, "%d%d%d%d%d%d\n", &label, &typelt, &numprop, &numat, &colour, &nbnel);
                                    if (label < 0)
                                    {
#if INTERACTIVEREAD
                                        if (debuglevel >= 2) fprintf(stdout,"Found Element section end, nt=%d\n",nt);
#else
                                        debug2 << "Found Element section end, #cells=(" << nt << ")" << endl;
#endif
                                        isunvread = 1 ;
                                        break;
                                    }
                                    if (fgets(buf, len, unvhandle) != NULL)
                                    {
                                        int iok = 0; 
                                        switch (typelt)
                                        {
                                        case 91:
                                            sscanf(buf, "%d %d %d\n", &nod[0], &nod[1], &nod[2]);
                                            nt++;
                                            iok = 4;
                                            break;
                                        case 94:
                                            sscanf(buf, "%d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3]);
                                            nt++;
                                            iok = 5;
                                            break;
                                        }
                                        if (iok != 0)
                                        {
#if !defined(MDSERVER)
                                            anUnvElement.label = label;
                                            anUnvElement.typelt = typelt;
                                            anUnvElement.matid = numat;
                                            anUnvElement.nodes = new int[nbnel];
                                            for (int i=0; i < nbnel; i++)
                                                anUnvElement.nodes[i] = nod[i];

#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout, "\telt %d, #nodes=%d, first/last node=(%d,%d)\n",
                                                                         anUnvElement.label,nbnel,anUnvElement.nodes[0],anUnvElement.nodes[nbnel-1]);
#else
                                            debug4 << "\telt " << anUnvElement.label << ", #nodes=" << nbnel << ", first/last node=(" << anUnvElement.nodes[0] << "," << anUnvElement.nodes[nbnel-1] << ")" << endl;
#endif
                                            meshUnvElements.insert(anUnvElement);
                                            anUnvElement.number++;
#endif
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (isunvread == 0) {
                        // Final attempt: try to make the mesh on the file
                        double * xp = (double *)malloc(np*sizeof(double)) ;
                        double * yp = (double *)malloc(np*sizeof(double)) ;
                        double * zp = (double *)malloc(np*sizeof(double)) ;
                        // Builds the nodes positions on the unit sphere
                        for (int k=0; k < np; k++) {
                            double theta, phi ;
                            theta = thetas[k] * (acos(-1.)/180.) ; phi = phis[k] * (acos(-1.)/180.) ;
                            xp[k] = sin(theta) * cos(phi); yp[k] = sin(theta) * sin(phi) ; zp[k] = cos(theta) ;
                        }
                        int nrow = 6 ;
                        //int * ltri = (int *)malloc(nrow*2*(np-2)*sizeof(int)) ;
                        int * ltri = (int *)calloc((size_t ) (nrow*2*(np-2)),sizeof(int)) ;
#if 0
                        mystripack_(&np,&nt,xp,yp,zp,ltri) ;
#else
                        int ier = 0 ;
                        int lnew = 0 ;
                        // Now call the lib:
                        // double * dist = (double *)malloc(np*sizeof(double)) ;
                        double * dist = (double *)calloc((size_t ) (np),sizeof(double)) ;
                        //int * lend = (int *)malloc(np*sizeof(int)) ;
                        //int * list = (int *)malloc(np*sizeof(int)) ;
                        //int * lptr = (int *)malloc(6*(np-2)*sizeof(int)) ;
                        int * lend = (int *)calloc((size_t ) (np),sizeof(int)) ;
                        int * list = (int *)calloc((size_t ) (6*(np-2)),sizeof(int)) ;
                        int * lptr = (int *)calloc((size_t ) (6*(np-2)),sizeof(int)) ;
                        //int * near = (int *)malloc(np*sizeof(int)) ;
                        //int * next = (int *)malloc(np*sizeof(int)) ;
                        int * lnear = (int *)calloc((size_t ) (np),sizeof(int)) ;
                        int * next = (int *)calloc((size_t ) (np),sizeof(int)) ;
                        //  Compute the Delaunay triangulation.
                        fprintf(stdout,"Building the Delaunay mesh\n");
                        // Make some debugging
#if INTERACTIVEREAD
                        if (debuglevel >= 0) {
                            for (int k=0; k < np; k++) {
                                fprintf(stdout," Node %d : (%lf, %lf, %lf)\n", k, xp[k], yp[k], zp[k]) ;
                            }
                        }
#endif 
                        mytrmesh( &np, xp, yp, zp, list, lptr, lend, &lnew, lnear, next, dist, &ier ) ;
#if INTERACTIVEREAD
                        if (debuglevel >= 0)
                            fprintf(stdout,"Error code=%d, lnew=%d\n",ier,lnew);
#else
                        debug1 << "Returned error code from trmesh is " << ier << " lnew value is " << lnew << "." << endl;
#endif
                        // The following error cannot be raised by trmesh from Robert J. Renka and John Burkardt
                        if (ier == -4)
                        {
                            fprintf(stdout,"trmesh lib not implemented\n");
                            EXCEPTION1(InvalidDBTypeException,
                                       "This ffp file requires a valid trmesh third-party lib from ACM, toms 772.");
                            
                        }

                        // Create the triangle list.
                        mytrlist( &np, list, lptr, lend, &nrow, &nt, ltri, &ier ) ;
                        fprintf(stdout,"Built #triangles=%d\n",nt);
                        // Release intermediate memory between trmesh and trlist
                        free(dist); free(lend); free(list); free(lptr); free(lnear); free(next) ;
                        // Build the Ideas mesh
#endif
                        // Release memory build for stripack
                        free(xp);free(yp);free(zp);
                        // Builds the mesh unv data structure from ltri
                        UnvElement anUnvElement;
                        int nod[4];
                        anUnvElement.number=0;
                        for (int k=0; k< nt; k++) {
#if INTERACTIVEREAD
                            if (debuglevel >= 2) fprintf(stdout, "Build elt label=%d, nodes=(%d,%d,%d)\n", k+1, ltri[nrow*k], ltri[nrow*k+1], ltri[nrow*k+2]) ;
#else
                            debug2 << "\t + Building TRI=(" << ltri[nrow*k] << "," << ltri[nrow*k+1] << "," << ltri[nrow*k+2] << ")" << endl;
#endif
                            anUnvElement.label = k+1 ;
                            anUnvElement.typelt = 91;
                            anUnvElement.matid = 1;
                            anUnvElement.nodes = new int[3];
                            for (int i=0; i < 3; i++)
                                anUnvElement.nodes[i] = ltri[nrow*k+i];
                            meshUnvElements.insert(anUnvElement);
                            anUnvElement.number++;
                        }
#if INTERACTIVEREAD
                        if (debuglevel >= 1) fprintf(stdout, "On the fly built #triangles=%d\n", nt) ;
#else
                        debug1 << "On the fly built #triangles=" << nt << endl;
#endif
                        // Release final memory
                        free(ltri);
                    }
                }
            }
            // Release memory :
            thetasffp.clear();
            phisffp.clear();
#if INTERACTIVEREAD
            if (debuglevel >= 1) 
            {
                fprintf(stdout," X range = %lf,%lf\n",range[0],range[1]);
                fprintf(stdout," Y range = %lf,%lf\n",range[2],range[3]);
                fprintf(stdout," Z range = %lf,%lf\n",range[4],range[5]);
            }
#else
            debug1 << " X range = (" << range[0] << "," << range[1] << ")" << endl;
            debug1 << " Y range = (" << range[2] << "," << range[3] << ")" << endl;
            debug1 << " Z range = (" << range[4] << "," << range[5] << ")" << endl;
#endif
            //sleep(3) ;
        }
    }
    CATCH(InvalidFilesException)
    {
        debug1 << "Unable to read ffp()" << endl;
        RETHROW;
    }
    CATCH(InvalidDBTypeException)
    {
        debug1 << "Unable to read ffp() unless you compile stripack 772" << endl;
        RETHROW;
    }
    CATCHALL
    {
        debug1 << "Unable to read ffp()" << endl;
        fprintf(stdout,"Unable to read ffp()\n");
    }
    ENDTRY
    if (handle == NULL && gzhandle == Z_NULL)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    else
    {
#if INTERACTIVEREAD
        if (debuglevel >= 1) fprintf(stdout, "ReadFile flag is set for file %s\n",filename.c_str());
#else
        debug1 << "ReadFile flag is set for file " << filename << endl;
#endif
        fileRead = true;
    }
}

// ****************************************************************************
//  Method: avtffpFileFormat::GetTime
//
//  Purpose: Return the time associated with this ffp file
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 28, 2005 
//
// ****************************************************************************
double
avtffpFileFormat::GetTime(void)
{
    // ffpTime = (double) ffpCycle ;
    return ffpTime ;
}

// ****************************************************************************
//  Method: avtffpFileFormat::GetCycle
//
//  Purpose: Return the cycle associated with this ffp file
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 28, 2005 
//
// ****************************************************************************

int
avtffpFileFormat::GetCycle(void)
{
    return ffpCycle;
}

// ****************************************************************************
//  Method: avtffpFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name 
//
//  Notes: Although all this method does is simply call the format's base
//  class implementation of GuessCycle, doing this is a way for the Silo
//  format to "bless" the guesses that that method makes. Otherwise, VisIt
//  wouldn't know that Silo thinks those guesses are good. See notes in
//  avtSTXXFileFormatInterface::SetDatabaseMetaData for further explanation.
//
//  Programmer: Mark C. Miller 
//  Creation:   May 16, 2005 
//
// ****************************************************************************

int
avtffpFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}

////////////////////////////////////////////////////////////////
#if 0
bool avtffpFileFormat::ReturnsValidTime() { return false; }
bool avtffpFileFormat::ReturnsValidCycle() { return false; }
#endif
// STMD version of GetAuxiliaryData.
void * 
avtffpFileFormat::GetAuxiliaryData(const char *var, const char *type, void *,DestructorFunction &df)
{
    void *retval = 0;
#ifdef MDSERVER
    return retval ;
#endif
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return retval ;
#endif
#if INTERACTIVEREAD
    if (debuglevel >= 1) fprintf(stdout,"var='%s', type='%s'\n",var,type);
#else
    debug1 << "var='" << var << "', type='" << type << "%s'" << endl;
#endif
    if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        if (strstr(var, "mesh") != NULL)
        {
#if INTERACTIVEREAD
            if (debuglevel >= 3) 
            {
                fprintf(stdout," X range = %lf,%lf\n",range[0],range[1]);
                fprintf(stdout," Y range = %lf,%lf\n",range[2],range[3]);
                fprintf(stdout," Z range = %lf,%lf\n",range[4],range[5]);
            }
#else
            debug3 << " X range = (" << range[0] << "," << range[1] << ")" << endl;
            debug3 << " Y range = (" << range[2] << "," << range[3] << ")" << endl;
            debug3 << " Z range = (" << range[4] << "," << range[5] << ")" << endl;
#endif
            avtIntervalTree *itree = new avtIntervalTree(1, 3);
            itree->AddElement(0, range);
            itree->Calculate(true);
            df = avtIntervalTree::Destruct;
            return ((void *) itree);
        }
        else if (strstr(var, "sphere") != NULL)
        {
            double mrange[6] ;
            mrange[0] = -1. ;  mrange[1] = +1. ;  mrange[2] = -1. ;  mrange[3] = +1. ;  mrange[4] = -1. ;  mrange[5] = +1. ; 
            avtIntervalTree *itree = new avtIntervalTree(1, 3);
            itree->AddElement(0, mrange);
            itree->Calculate(true);
            df = avtIntervalTree::Destruct;
            return ((void *) itree);
        }
        else if (strstr(var, "cphere") != NULL)
        {
            double mrange[6] ;
            mrange[0] = -2. ;  mrange[1] = +2. ;  mrange[2] = -2. ;  mrange[3] = +2. ;  mrange[4] = -2. ;  mrange[5] = +2. ; 
            avtIntervalTree *itree = new avtIntervalTree(1, 3);
            itree->AddElement(0, mrange);
            itree->Calculate(true);
            df = avtIntervalTree::Destruct;
            return ((void *) itree);
        }
        else
            return retval;
    }
    return retval;
}
