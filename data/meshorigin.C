/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

DBoptlist *
MakeOptList(int origin)
{
    int tmp;
    DBoptlist *optlist = DBMakeOptlist(4);
    tmp = origin;
    DBAddOption(optlist, DBOPT_ORIGIN, (void *)&tmp);
    DBAddOption(optlist, DBOPT_XUNITS, (void *)"cm");
    DBAddOption(optlist, DBOPT_YUNITS, (void *)"cm");
    DBAddOption(optlist, DBOPT_ZUNITS, (void *)"cm");
    return optlist;
}

void
write_point3d(DBfile *dbfile, const char *meshname, int origin)
{
    /* Create some points to save. */
#define NPTS 100
    DBoptlist *opt = NULL;
    int i;
    float x[NPTS], y[NPTS], z[NPTS];
    int dims[] = {NPTS, NPTS, NPTS}, ndims = 3;
    float *coords[] = {(float*)x, (float*)y, (float*)z};
    for(i = 0; i < NPTS; ++i)
    {
        float t = ((float)i) / ((float)(NPTS-1));
        float angle = 3.14159 * 10. * t;
        x[i] = t * cos(angle);
        y[i] = t * sin(angle);
        z[i] = t;
    }
    /* Write a point mesh. */
    opt = MakeOptList(origin);
    DBPutPointmesh(dbfile, meshname, ndims, coords, NPTS,
                   DB_FLOAT, opt);
    DBFreeOptlist(opt);
}

void
write_rect3d(DBfile *dbfile, const char *meshname, int origin)
{
    /* Write a rectilinear mesh. */
    DBoptlist *opt = NULL;
    float x[] = {0., 1., 2.5, 5.};
    float y[] = {0., 2., 2.25, 2.55,  5.};
    float z[] = {0., 1., 3.};
    int dims[] = {4, 5, 3};
    int ndims = 3;
    float *coords[] = {x, y, z};
    opt = MakeOptList(origin);
    DBPutQuadmesh(dbfile, meshname, NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, opt);
    DBFreeOptlist(opt);
}

void
write_curv3d(DBfile *dbfile, const char *meshname, int origin)
{
    /* Write a curvilinear mesh. */
    DBoptlist *opt = NULL;
    float x[2][3][4] = {
       {{0.,1.,2.,3.},{0.,1.,2.,3.}, {0.,1.,2.,3.}},
       {{0.,1.,2.,3.},{0.,1.,2.,3.}, {0.,1.,2.,3.}}
    };
    float y[2][3][4] = {
       {{0.5,0.,0.,0.5},{1.,1.,1.,1.}, {1.5,2.,2.,1.5}},
       {{0.5,0.,0.,0.5},{1.,1.,1.,1.}, {1.5,2.,2.,1.5}}
    };
    float z[2][3][4] = {
       {{0.,0.,0.,0.},{0.,0.,0.,0.},{0.,0.,0.,0.}},
       {{1.,1.,1.,1.},{1.,1.,1.,1.},{1.,1.,1.,1.}}
    };
    int dims[] = {4, 3, 2};
    int ndims = 3;
    float *coords[] = {(float*)x, (float*)y, (float*)z};
    opt = MakeOptList(origin);
    DBPutQuadmesh(dbfile, meshname, NULL, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, opt);
    DBFreeOptlist(opt);
}

void
write_ucd3d(DBfile *dbfile, const char *meshname, int origin)
{
    /* Node coordinates */
    DBoptlist *opt = NULL;
    char zlname[1024];
    float x[] = {0.,2.,2.,0.,0.,2.,2.,0.,0.,2.,2.,0.,1.,2.,4.,4.};
    float y[] = {0.,0.,0.,0.,2.,2.,2.,2.,4.,4.,4.,4.,6.,0.,0.,0.};
    float z[] = {2.,2.,0.,0.,2.,2.,0.,0.,2.,2.,0.,0.,1.,4.,2.,0.};
    float *coords[] = {x, y, z};
    /* Connectivity */
    int nodelist[] = {
        1,2,3,4,5,6,7,8,    /* hex,     zone 1 */
        5,6,7,8,9,10,11,12, /* hex,     zone 2 */
        9,10,11,12,13,      /* pyramid, zone 3 */
        2,3,16,15,6,7,      /* prism,   zone 4 */
        2,15,14,6           /* tet,     zone 5 */
    };
    int lnodelist = sizeof(nodelist) / sizeof(int);
    /* shape type 1 has 8 nodes (hex) */
    /* shape type 2 has 5 nodes (pyramid) */
    /* shape type 3 has 6 nodes (prism) */
    /* shape type 4 has 4 nodes (tet) */
    int shapesize[] = {8,5,6,4};
    /* We have 2 hex, 1 pyramid, 1 prism, 1 tet */
    int shapecounts[] = {2,1,1,1};
    int nshapetypes = 4;
    int nnodes = 16;
    int nzones = 5;
    int ndims = 3;
    sprintf(zlname, "%s_zonelist", meshname);
    /* Write out connectivity information. */
    DBPutZonelist(dbfile, zlname, nzones, ndims, nodelist, lnodelist,
        1, shapesize, shapecounts, nshapetypes);
    /* Write an unstructured mesh. */
    opt = MakeOptList(origin);
    DBPutUcdmesh(dbfile, meshname, ndims, NULL, coords, nnodes, nzones,
        zlname, NULL, DB_FLOAT, opt);
    DBFreeOptlist(opt);
}

// ****************************************************************************
// Function: main
//
// Purpose: 
//   This program creates a Silo file with several different mesh types and 
//   we set the cell origin on each of them to 0 and 1 so we have data with
//   which to test cell origin features in VisIt.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 13 16:43:34 PST 2007
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int driver = DB_PDB;
    DBfile *dbfile = NULL;

    // Look through command line args.
    for(int j = 1; j < argc; ++j)
    {
        if (strcmp(argv[j], "-driver") == 0)
        {
            j++;

            if (strcmp(argv[j], "DB_HDF5") == 0)
                driver = DB_HDF5;
            else if (strcmp(argv[j], "DB_PDB") == 0)
                driver = DB_PDB;
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[j]);
            }
        }
    }

    /* Open the Silo file */
    dbfile = DBCreate("meshorigin.silo", DB_CLOBBER, DB_LOCAL,
        "Many mesh types with different cell/node offsets", driver);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_point3d(dbfile, "pointmesh0", 0);
    write_point3d(dbfile, "pointmesh1", 1);

    write_rect3d(dbfile, "rectmesh0", 0);
    write_rect3d(dbfile, "rectmesh1", 1);

    write_curv3d(dbfile, "curvemesh0", 0);
    write_curv3d(dbfile, "curvemesh1", 1);

    write_ucd3d(dbfile, "ucdmesh0", 0);
    write_ucd3d(dbfile, "ucdmesh1", 1);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
