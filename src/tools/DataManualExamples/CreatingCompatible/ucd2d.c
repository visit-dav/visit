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

/*   
  *---*---*--*   nodes (5,6,7,8)
  |   |\5/ 3 |
  | 1 |4*----*   nodes (3,4)
  |   |/  2  |
  *---*------*   nodes(0,1,2)

  cell 1 quad(0,1,6,5)
  cell 2 quad(1,2,4,3)
  cell 3 quad(3,4,8,7)
  cell 4 tri(1,3,6)
  cell 5 tri(3,7,6)

*/
void
write_ucd2d(DBfile *dbfile)
{
    /* Node coordinates */
    float x[] = {0., 2., 5., 3., 5., 0., 2., 4., 5.};
    float y[] = {0., 0., 0., 3., 3., 5., 5., 5., 5.};
    float *coords[] = {x, y};
    /* Connectivity */
    int nodelist[] = {
        2,4,7,   /* tri zone 1. */
        4,8,7,   /* tri zone 2. */
        1,2,7,6, /* quad zone 3. */
        2,3,5,4, /* quad zone 4. */
        4,5,9,8  /* quad zone 5. */
    };
    int lnodelist = sizeof(nodelist) / sizeof(int);
    /* shape type 1 has 3 nodes (tri), shape type 2 is quad */
    int shapesize[] = {3, 4};
    /* We have 2 tris and 3 quads */
    int shapecounts[] = {2, 3};
    int nshapetypes = 2;
    int nnodes = 9;
    int nzones = 5;
    int ndims = 2;
    /* Write out connectivity information. */
    DBPutZonelist(dbfile, "zonelist", nzones, ndims, nodelist, lnodelist,
        1, shapesize, shapecounts, nshapetypes);
    /* Write an unstructured mesh. */
    DBPutUcdmesh(dbfile, "mesh", ndims, NULL, coords, nnodes, nzones,
        "zonelist", NULL, DB_FLOAT, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("ucd2d.silo", DB_CLOBBER, DB_LOCAL,
        "Unstructured 2D mesh", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_ucd2d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
