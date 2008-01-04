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

#include <stdlib.h>
#include <string.h>
#include <silo.h>

// Programmer: Jeremy Meredith
// Date:       Fri Mar  4 13:19:48 PST 2005
//
// Purpose:    Make a dataset and put domain-boundary information
//             into the file.  This is intended to test our internal
//             ghost zone creation routines.
//
//
// Modifications:
//
//   Mark C. Miller, Mon Mar 21 18:47:40 PST 2005
//   Added driver option
//
int main(int argc, char **argv)
{
    int ndims0[] = {3,3};
    int zdims0[] = {2,2};
    float xc0[] = {0,1,2,  0,1,2,  0,1,2};
    float yc0[] = {0,0,0,  1,1,1,  2,2,2};
    float *coords0[] = {xc0, yc0};
    float zvar0[] = {0.4,0.5,0.6,0.7};

    int ndims1[] = {3,3};
    int zdims1[] = {2,2};
    float xc1[] = {0,1,2,  0,1,2,  0,1,2};
    float yc1[] = {2,2,2,  3,3,3,  4,4,4};
    float *coords1[] = {xc1, yc1};
    float zvar1[] = {1.4,1.5,1.6,1.7};

    int ndims2[] = {3,5};
    int zdims2[] = {2,4};
    float xc2[] = {2,3,4,  2,3,4,  2,3,4,  2,3,4,  2,3,4};
    float yc2[] = {0,0,0,  1,1,1,  2,2,2,  3,3,3,  4,4,4};
    float *coords2[] = {xc2, yc2};
    float zvar2[] = {2.1,2.2,2.3,2.4,2.5,2.6,2.7,2.8};

    int driver = DB_PDB;

    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-driver") == 0)
        {
            i++;

            if (strcmp(argv[i], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[i], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[i]);
            }
        }
        else
        {
            fprintf(stderr, "unrecognized argument \"%s\"\n", argv[i]);
            exit(-1);
        }

        i++;
    }


    //
    // Create the file
    //
    DBfile *db = DBCreate("ghost1.silo", DB_CLOBBER, DB_LOCAL,
                          "ghost zone test 1", driver);

    //
    // Write domain 0
    //
    DBMkDir(db, "domain0");
    DBSetDir(db, "domain0");

    DBPutQuadmesh(db, "mesh", NULL, coords0, ndims0, 2, DB_FLOAT, DB_NONCOLLINEAR, NULL);
    DBPutQuadvar1(db, "zvar", "mesh", zvar0, zdims0, 2, NULL,0, DB_FLOAT, DB_ZONECENT, NULL);

    DBSetDir(db, "..");

    //
    // Write domain 1
    //
    DBMkDir(db, "domain1");
    DBSetDir(db, "domain1");

    DBPutQuadmesh(db, "mesh", NULL, coords1, ndims1, 2, DB_FLOAT, DB_NONCOLLINEAR, NULL);
    DBPutQuadvar1(db, "zvar", "mesh", zvar1, zdims1, 2, NULL,0, DB_FLOAT, DB_ZONECENT, NULL);

    DBSetDir(db, "..");

    //
    // Write domain 2
    //
    DBMkDir(db, "domain2");
    DBSetDir(db, "domain2");

    DBPutQuadmesh(db, "mesh", NULL, coords2, ndims2, 2, DB_FLOAT, DB_NONCOLLINEAR, NULL);
    DBPutQuadvar1(db, "zvar", "mesh", zvar2, zdims2, 2, NULL,0, DB_FLOAT, DB_ZONECENT, NULL);

    DBSetDir(db, "..");

    //
    // Write multimesh, multivar
    //
    char *meshes[] = { "domain0/mesh",  "domain1/mesh",  "domain2/mesh" };
    int meshtypes[] = { DB_QUADMESH, DB_QUADMESH, DB_QUADMESH };
    DBPutMultimesh(db, "mesh", 3, meshes, meshtypes, NULL);

    char *zvars[] = { "domain0/zvar",  "domain1/zvar",  "domain2/zvar" };
    int zvartypes[] = { DB_QUADVAR, DB_QUADVAR, DB_QUADVAR };
    DBPutMultivar(db, "zvar", 3, zvars, zvartypes, NULL);

    //
    // Write domain connectivity
    //
    int Q = -1;
    int extents0[] = { 0,2, 0,2, Q,Q };
    int extents1[] = { 0,2, 2,4, Q,Q };
    int extents2[] = { 2,4, 0,4, Q,Q };

    int lneighbors0 = 2;
    int neighbors0[][11] = {{ 1, 0, 1,2,3,  0,2, 2,2, Q,Q }, // 0-1 boundary
                            { 2, 0, 1,2,3,  2,2, 0,2, Q,Q }};// 0-2 boundary

    int lneighbors1 = 2;
    int neighbors1[][11] = {{ 0, 0, 1,2,3,  0,2, 2,2, Q,Q }, // 1-0 boundary
                            { 2, 1, 1,2,3,  2,2, 2,4, Q,Q }};// 1-2 boundary

    int lneighbors2 = 2;
    int neighbors2[][11] = {{ 0, 1, 1,2,3,  2,2, 0,2, Q,Q }, // 2-0 boundary
                            { 1, 1, 1,2,3,  2,2, 2,4, Q,Q }};// 2-1 boundary

    DBMkDir(db, "Decomposition");
    DBSetDir(db, "Decomposition");

    int len;
    int ival;

    ival = 3;
    len = 1;
    DBWrite(db, "NumDomains", &ival, &len, 1, DB_INT);

    // domain0
    DBMkDir(db, "Domain_0");
    DBSetDir(db, "Domain_0");

    len = 1;
    DBWrite(db, "NumNeighbors", &lneighbors0, &len, 1, DB_INT);

    len = 6;
    DBWrite(db, "Extents", &extents0, &len, 1, DB_INT);

    len = 11;
    DBWrite(db, "Neighbor_0", &neighbors0[0][0], &len, 1, DB_INT);
    DBWrite(db, "Neighbor_1", &neighbors0[1][0], &len, 1, DB_INT);

    DBSetDir(db, "..");

    // domain1
    DBMkDir(db, "Domain_1");
    DBSetDir(db, "Domain_1");

    len = 1;
    DBWrite(db, "NumNeighbors", &lneighbors1, &len, 1, DB_INT);

    len = 6;
    DBWrite(db, "Extents", &extents1, &len, 1, DB_INT);

    len = 11;
    DBWrite(db, "Neighbor_0", &neighbors1[0][0], &len, 1, DB_INT);
    DBWrite(db, "Neighbor_1", &neighbors1[1][0], &len, 1, DB_INT);

    DBSetDir(db, "..");

    // domain2
    DBMkDir(db, "Domain_2");
    DBSetDir(db, "Domain_2");

    len = 1;
    DBWrite(db, "NumNeighbors", &lneighbors2, &len, 1, DB_INT);

    len = 6;
    DBWrite(db, "Extents", &extents2, &len, 1, DB_INT);

    len = 11;
    DBWrite(db, "Neighbor_0", &neighbors2[0][0], &len, 1, DB_INT);
    DBWrite(db, "Neighbor_1", &neighbors2[1][0], &len, 1, DB_INT);

    DBSetDir(db, "..");

    //
    // Done!  Close the file
    //

    DBClose(db);
}
