/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <assert.h>
#include <cmath>
#include <cstring>
#include <string>
#include <visitstream.h>
#include <snprintf.h>

#include <silo.h>

#include "QuadMesh.h"

#ifdef WIN32
// What's the header for getcwd
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define MAX_CYCLES  10

#define XMAX 5
#define YMAX 5
#define ZMAX 5
#define MAX_DOMAINS 4

// supress the following since silo uses char * in its API
#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wdeprecated-writable-strings"
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
// ****************************************************************************

class ProgramOptions
{
public:
    ProgramOptions() : 
        isteps(40),
        jsteps(40),
        driver(DB_PDB),
        useAbsoluteFilePaths(false)
    {
    }

    int isteps;
    int jsteps;
    int driver;
    bool useAbsoluteFilePaths;
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
    char buf[1000];
    getcwd(buf, 1000);
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
    mkdir(dirname.c_str(), S_IRWXU);
    chdir(dirname.c_str());
#endif
}

// ****************************************************************************
// Function: DomainFileName
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
DomainFileName(int d)
{
    char filename[100];
    SNPRINTF(filename, 100, "TIME%04d.silo", d);
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
DirectoryName(int d)
{
    char dirname[100];
    SNPRINTF(dirname, 100, "p%05d", d);
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

    std::string filename = DomainFileName(cycle);
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
    SNPRINTF(root, MAX_STRING, "multidir%04d.root", cycle);
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
    

    std::string filename = DomainFileName(cycle);
    for(int dom = 0; dom < MAX_DOMAINS; ++dom)
    {
        std::string dirname = DirectoryName(dom);

        meshNames[dom] = new char[MAX_STRING];
        SNPRINTF(meshNames[dom], MAX_STRING, "%s%s/%s:/Mesh", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());
        meshTypes[dom] = DB_QUADMESH;

        radial[dom] = new char[MAX_STRING];
        SNPRINTF(radial[dom], MAX_STRING, "%s%s/%s:/radial", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_000[dom] = new char[MAX_STRING];
        SNPRINTF(Array_000[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_000", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_001[dom] = new char[MAX_STRING];
        SNPRINTF(Array_001[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_001", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_002[dom] = new char[MAX_STRING];
        SNPRINTF(Array_002[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_002", opt.useAbsoluteFilePaths?cwd.c_str():"",
            dirname.c_str(), filename.c_str());

        Array_003[dom] = new char[MAX_STRING];
        SNPRINTF(Array_003[dom], MAX_STRING, "%s%s/%s:/Array_comps/Array_003", opt.useAbsoluteFilePaths?cwd.c_str():"",
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
   
    for(int dom = 0; dom < MAX_DOMAINS; ++dom)
    {
        std::string dirname = DirectoryName(dom);
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
// ****************************************************************************

int
main(int argc, char *argv[])
{
    ProgramOptions opt;

    // Look through command line args.
    for(int j = 1; j < argc; ++j)
    {
        if (strcmp(argv[j], "-driver") == 0)
        {
            j++;

            if (strcmp(argv[j], "DB_HDF5") == 0)
            {
                opt.driver = DB_HDF5;
            }
            else if (strcmp(argv[j], "DB_PDB") == 0)
            {
                opt.driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Unrecognized driver name \"%s\"\n",
                   argv[j]);
            }
        }
        else if (strcmp(argv[j], "-abspaths") == 0)
        {
            opt.useAbsoluteFilePaths = true;
        }
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
