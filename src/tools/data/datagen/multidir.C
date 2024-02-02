// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <assert.h>
#include <cmath>
#include <cstring>
#include <string>
#include <visitstream.h>

#include <silo.h>

#include "QuadMesh.h"

#ifdef WIN32
// What's the header for getcwd
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#define MAX_CYCLES  10

#define XMAX 5
#define YMAX 5
#define ZMAX 5
#define MAX_DOMAINS 4

// suppress the following since silo uses char * in its API
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wwritable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// global
float g_Center[2];

// ****************************************************************************
// Class: ProgramOptions
//
// Purpose:
//   Contains program options
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:44:33 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Jul 17 20:12:32 PDT 2012
//    Add option controlling use of absolute file paths in multi-block objects.
//
//    Mark C. Miller, Tue Sep 20 13:12:48 PDT 2022
//    Added dirPerTimestep option. Originally, this program created a directory
//    for each domain and then wrote timesteps out in each domain. Thats fine
//    but not typically what our application codes do. Instead, they write a 
//    directory per timestep with all domain files in that directory. If
//    dirPerTimestep is true, this program will behave that way.
// ****************************************************************************

class ProgramOptions
{
public:
    ProgramOptions() : 
        isteps(40),
        jsteps(40),
        driver(DB_PDB),
        useAbsoluteFilePaths(false),
        dirPerTimestep(false)
    {
    }

    int isteps;
    int jsteps;
    int driver;
    bool useAbsoluteFilePaths;
    bool dirPerTimestep;
};

// ****************************************************************************
// Function: GetCurrentDirectory
//
// Purpose: 
//   Get the current directory
//
// Returns:    Returns the current directory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

std::string
GetCurrentDirectory()
{
#ifdef WIN32
    cerr << "GetCurrentDirectory not implemented" << endl;
    // For now...
    return std::string();
#else
    char buf[1000] = {'\0'};
    if (!getcwd(buf, 1000))
    {
        cerr << "getcwd failed" << endl;
    }
    return std::string(buf);
#endif
}

// ****************************************************************************
// Function: GotoDirectory
//
// Purpose: 
//   Go into the specified directory, making it first if needed.
//
// Arguments:
//   dirname : The directory name,
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
GotoDirectory(const std::string &dirname)
{
#ifdef WIN32
    cerr << "GotoDirectory not implemented" << endl;
#else
    if ( (mkdir(dirname.c_str(), S_IRWXU) && (errno!=EEXIST))
      || chdir(dirname.c_str()) )
    {
        cerr << "GotoDirectory " << dirname << " failed!" << endl;
    }
#endif
}

// ****************************************************************************
// Function: InstanceFileName
//
// Purpose: 
//   Returns the domain filename at a given cycle.
//
// Arguments:
//   d : The cycle.
//
// Returns:    The domain filename.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

std::string
InstanceFileName(int c, int d, ProgramOptions const &opt)
{
    char filename[100];
    if (opt.dirPerTimestep)
        snprintf(filename, 100, "DOMAIN%03d.silo", d);
    else
        snprintf(filename, 100, "TIME%05d.silo", c);
    return filename;
}

// ****************************************************************************
// Function: DirectoryName
//
// Purpose: 
//   Returns the directory name for a domain.
//
// Arguments:
//   d : The domain.
//
// Returns:    The domain directory name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

std::string
DirectoryName(int c, int d, ProgramOptions const &opt)
{
    char dirname[100];

    if (opt.dirPerTimestep)
        snprintf(dirname, 100, "c%05d", c);
    else
        snprintf(dirname, 100, "p%03d", d);
    return dirname;
}

// ****************************************************************************
// Function: 
//
// Purpose: 
//   Mesh data callback functions.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

float
Radial(float *pt, QuadMesh *)
{
    float dx = pt[0] - g_Center[0];
    float dy = pt[1] - g_Center[1];
    return (float)sqrt(dx*dx + dy*dy);
}

float
RadialSin(float *pt, QuadMesh *m)
{
    return sin(Radial(pt,m));
}

float
RadialCos(float *pt, QuadMesh *m)
{
    return cos(Radial(pt,m));
}

float
RadialCos2(float *pt, QuadMesh *m)
{
    float c = RadialCos(pt, m);
    return c * c;
}

float
Wavy(float *pt, QuadMesh *m)
{
    float dx = pt[0] - g_Center[0];
    float dy = pt[1] - g_Center[1];
    float angle;
    if(dx == 0. && dy > 0.)
        angle = M_PI/2.;
    else if(dx == 0.)
        angle = 3*M_PI/2.;
    else
        angle = atan(dy/dx);
    float r = (float)sqrt(dx*dx + dy*dy);
    r = r + 0.5 * sin(10 * angle);
    return r;
}

// ****************************************************************************
// Function: WriteDomainFile
//
// Purpose: 
//   Writes the current cycle's data file for the specified domain.
//
// Arguments:
//   cycle : The current cycle.
//   dom   : The domain that we want to write.
//   opt   : Program options.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
WriteDomainFile(int cycle, int dom, const ProgramOptions &opt)
{
    static const float extent_origins[MAX_DOMAINS][3] = {
        {0.   ,0.   ,0.},
        {-XMAX,0    ,0.},
        {-XMAX,-YMAX,0.},
        {0.   ,-YMAX,0.}
    };

    RectilinearMesh M(opt.isteps, opt.jsteps, 1);
    M.SetXValues(extent_origins[dom][0], extent_origins[dom][0] + XMAX);
    M.SetYValues(extent_origins[dom][1] + YMAX, extent_origins[dom][1] + 2*YMAX);
    M.SetZValues(extent_origins[dom][2], extent_origins[dom][2] + ZMAX);
    M.SetMeshLabels("Width", "Height", "Depth");
    M.SetMeshUnits("cm", "cm", "cm");

    M.CreateNodalData("radial", Radial, "cm");
    M.CreateNodalData("Array_comps/Array_000", RadialSin, "cm");
    M.CreateNodalData("Array_comps/Array_001", RadialCos, "cm");
    M.CreateNodalData("Array_comps/Array_002", RadialCos2, "cm");
    M.CreateNodalData("Array_comps/Array_003", Wavy, "cm");

    std::string filename = InstanceFileName(cycle, dom, opt);
    DBfile *db;
    db = DBCreate(filename.c_str(), DB_CLOBBER, DB_LOCAL, 
            "This is one domain of a dataset that tests multimeshes and "
            "multivars with various directory heirarchy levels", opt.driver);
    M.WriteFile(db);
    DBClose(db);
}

// ****************************************************************************
// Function: WriteMasterFile
//
// Purpose: 
//   Writes the master file for the current cycle.
//
// Arguments:
//   cycle : The current cycle.
//   opt   : Program options.
//  
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Jul 17 20:10:52 PDT 2012
//    Add ability to test absolute file paths stored in multi-block objects.
// ****************************************************************************

void
WriteMasterFile(int cycle, const ProgramOptions &opt)
{
#define MAX_STRING 500

    char root[MAX_STRING];
    if (opt.dirPerTimestep)
        snprintf(root, MAX_STRING, "%s.silo", DirectoryName(cycle, 0, opt).c_str());
    else
        snprintf(root, MAX_STRING, "multidir%05d.root", cycle);
    DBfile *db = DBCreate(root, DB_CLOBBER, DB_LOCAL, 
            "Master file of a dataset that tests multimeshes and "
            "multivars with various directory heirarchy levels", opt.driver);

    char *meshNames[MAX_DOMAINS];
    int  meshTypes[MAX_DOMAINS];
    char *radial[MAX_DOMAINS];
    char *Array_000[MAX_DOMAINS];
    char *Array_001[MAX_DOMAINS];
    char *Array_002[MAX_DOMAINS];
    char *Array_003[MAX_DOMAINS];
    int   varTypes[MAX_DOMAINS];
    std::string cwd = GetCurrentDirectory() + "/";
    

    for(int dom = 0; dom < MAX_DOMAINS; ++dom)
    {
        std::string dirname = DirectoryName(cycle, dom, opt);
        std::string filename = InstanceFileName(cycle, dom, opt);

        meshNames[dom] = new char[MAX_STRING];
        snprintf(meshNames[dom], MAX_STRING, "%s%s/%s:/Mesh", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());
        meshTypes[dom] = DB_QUADMESH;

        radial[dom] = new char[MAX_STRING];
        snprintf(radial[dom], MAX_STRING, "%s%s/%s:/radial", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_000[dom] = new char[MAX_STRING];
        snprintf(Array_000[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_000", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_001[dom] = new char[MAX_STRING];
        snprintf(Array_001[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_001", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_002[dom] = new char[MAX_STRING];
        snprintf(Array_002[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_002", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_003[dom] = new char[MAX_STRING];
        snprintf(Array_003[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_003", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        varTypes[dom] = DB_QUADVAR;
    }

    DBPutMultimesh(db, "Mesh", MAX_DOMAINS, meshNames, meshTypes, NULL);
    DBPutMultivar(db, "radial", MAX_DOMAINS, radial, varTypes, NULL);

    DBMkDir(db, "Array_comps");
    DBSetDir(db, "Array_comps");
    DBPutMultivar(db, "Array_000", MAX_DOMAINS, Array_000, varTypes, NULL);
    DBPutMultivar(db, "Array_001", MAX_DOMAINS, Array_001, varTypes, NULL);
    DBPutMultivar(db, "Array_002", MAX_DOMAINS, Array_002, varTypes, NULL);
    DBPutMultivar(db, "Array_003", MAX_DOMAINS, Array_003, varTypes, NULL);
    // Write an expression to combine them
    char *exprNames[] = {"Array"};
    int   exprTypes[] = {DB_VARTYPE_ARRAY};
    char *exprDefs[]  = {"array_compose(<Array_comps/Array_000>, <Array_comps/Array_001>, <Array_comps/Array_002>, <Array_comps/Array_003>)"};
    DBPutDefvars(db, "expressions", 1, exprNames, exprTypes, exprDefs, NULL);

    DBSetDir(db, "..");

    DBClose(db);

    for(int dom = 0; dom < MAX_DOMAINS; ++dom)
    {
        delete [] meshNames[dom];
        delete [] radial[dom];
        delete [] Array_000[dom];
        delete [] Array_001[dom];
        delete [] Array_002[dom];
        delete [] Array_003[dom];
    }
}

// ****************************************************************************
// Function: WriteCycle
//
// Purpose: 
//   Writes the current cycle's data.
//
// Arguments:
//   cycle : The current cycle.
//   opt   : Program options.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
WriteCycle(int cycle, const ProgramOptions &opt)
{
    std::string topdir = GetCurrentDirectory();

    for (int dom = 0; dom < MAX_DOMAINS; ++dom)
    {
        std::string dirname = DirectoryName(cycle, dom, opt);
        GotoDirectory(dirname);

        // Create the file
        WriteDomainFile(cycle, dom, opt);

        GotoDirectory(topdir);
    }

    WriteMasterFile(cycle, opt);
}

// ****************************************************************************
// Function: main
//
// Purpose: 
//   The main program
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 10:38:38 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Jul 17 20:12:15 PDT 2012
//    Add option to use absolute file paths for multi-block objects.
//
//    Mark C. Miller, Tue Sep 20 13:15:06 PDT 2022
//    Add dirpts option
// ****************************************************************************

int
main(int argc, char *argv[])
{
    ProgramOptions opt;

    // Look through command line args.
    for(int j = 1; j < argc; ++j)
    {
        if (!strncmp(argv[j], "DB_HDF5", 7))
            opt.driver = DB_HDF5;
        else if (!strncmp(argv[j], "DB_PDB", 6))
            opt.driver = DB_PDB;
        else if (!strncmp(argv[j], "-abspaths", 9))
            opt.useAbsoluteFilePaths = true;
        else if (!strncmp(argv[j], "-dirpts", 7))
            opt.dirPerTimestep = true;
        else
           fprintf(stderr,"Unrecognized driver name \"%s\"\n", argv[j]);
    }

    // Make all of the data go into a subdirectory so it's easier to delete later.
    std::string cwd = GetCurrentDirectory();
    GotoDirectory("multidir_test_data");

    for(int cycle = 0; cycle < MAX_CYCLES; ++cycle)
    {
        // Compute a new value for g_Center
        float t = float(cycle) / float(MAX_CYCLES);
        float angle = t * 2. * M_PI;
        g_Center[0] = 0.5 * XMAX * cos(angle);
        g_Center[1] = YMAX + 0.5 * YMAX * sin(angle);

        // Write the data.
        WriteCycle(cycle, opt);
    }

    GotoDirectory(cwd);

    return 0;
}
