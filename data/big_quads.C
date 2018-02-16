/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// supress the following since silo uses char * in its API
#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wdeprecated-writable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#if 0
    varnames[0] = "u";
    vals[0] = dc;
    DBPutUcdvar(dbfile, "ud_1d", "ucd_1d", 1, varnames, (float**)vals,
        NX, NULL, 0, DB_DOUBLE, DB_NODECENT, 0);
    vals[0] = fc;
    DBPutUcdvar(dbfile, "uf_1d", "ucd_1d", 1, varnames, (float**)vals,
        NX, NULL, 0, DB_FLOAT, DB_NODECENT, 0);
#endif

int
main(int argc, char *argv[])
{
    int i;
    float ucdx[] = {0,1,2,0,1,2,0,1,2};
    float ucdy[] = {0,0,0,1,1,1,2,2,2};
    int shapesize = 4;
    int shapecnt = 4;
    int shapetype = DB_ZONETYPE_QUAD;
    int zonelist[] = {0,1,4,3,  1,2,5,4,  3,4,7,6,  4,5,8,7};
    int trishapesize = 3;
    int trishapecnt = 8;
    int trishapetype = DB_ZONETYPE_TRIANGLE;
    int trizonelist[] = {4,3,0, 4,0,1,  4,1,2, 4,2,5,  4,6,3, 4,7,6,  4,8,7, 4,5,8};
    char *coordnames[] = {"x","y"};
    float *ucdcoords[2] = {ucdx,ucdy};
    double nodal_field[] = {0, 0, 0, 0, 10, 0, 0, 0, 0};
    float qx[] = {0,.1,.2,.3,.4,.5,.6,.7,.8,.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0};
    float *qcoords[] = {qx, qx};
    int qdims[] = {21,21};
    double nodal_field_highres[21*21];

    // Check for the right driver.
    int driver = DB_PDB;
    for(int j = 1; j < argc; ++j)
    {
        if (strcmp(argv[j], "-driver") == 0)
        {
            j++;

            if (strcmp(argv[j], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[j], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[j]);
            }
        }
    }

    // Open the Silo file
    DBfile *dbfile = DBCreate("big_quads.silo", DB_CLOBBER, DB_LOCAL,
        "2D mesh with large quads to demonstrate shading artifacts", driver);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    // Output mesh as 4 quads
    DBPutUcdmesh(dbfile, "quads", 2, coordnames, ucdcoords,
                9, 4, "quads_zl", NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "quads_zl", 4, 2, zonelist, (int) sizeof(zonelist),
                0, 0, 0, &shapetype, &shapesize, &shapecnt, 1, NULL);
    DBPutUcdvar1(dbfile, "quads_field", "quads", nodal_field,
        9, NULL, 0, DB_DOUBLE, DB_NODECENT, 0);


    // Output mesh as 8 triangles with diagonals chosen "coorectly"
    DBPutUcdmesh(dbfile, "tris", 2, coordnames, ucdcoords,
                9, 8, "tris_zl", NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "tris_zl", 8, 2, trizonelist, (int) sizeof(trizonelist),
                0, 0, 0, &trishapetype, &trishapesize, &trishapecnt, 1, NULL);
    DBPutUcdvar1(dbfile, "tris_field", "tris", nodal_field,
        9, NULL, 0, DB_DOUBLE, DB_NODECENT, 0);


    // Output much finer resolution version of 4 quad case by doing
    // bi-linear sampling of each quad, the interpolation function
    // VisIt should be using internally to describe the field variation
    // over a quad.
    {
        for (int i = 0; i < 21; i++)
        {
            double h00, h10, h01, h11, xoff, yoff;
            for (int j = 0; j < 21; j++)
            {
                if (qx[i] <= 1.0)
                {
                    xoff = 0;
                    if (qx[j] <= 1.0)
                    {
                        yoff = 0;
                        h00=0; h10=0; h01=0; h11=10;
                    }
                    else
                    {
                        yoff = 1;
                        h00=0; h10=10; h01=0; h11=0;
                    }
                }
                else
                {
                    xoff = 1;
                    if (qx[j] <= 1.0)
                    {
                        yoff = 0;
                        h00=0; h10=00; h01=10; h11=0;
                    }
                    else
                    {
                        yoff = 1;
                        h00=10; h10=00; h01=0; h11=0;
                    }
                }
                double dx = qx[i] - xoff;
                double dy = qx[j] - yoff;
                double h0x = dx*h10 + (1-dx)*h00;
                double h1x = dx*h11 + (1-dx)*h01;
                double h = dy*h1x + (1-dy)*h0x;
                nodal_field_highres[j*21+i] = h; // [j*nx+i]
            }
        }
        DBPutQuadmesh(dbfile, "qmesh", coordnames, qcoords, qdims, 2,
            DB_FLOAT, DB_COLLINEAR, NULL);
        DBPutQuadvar1(dbfile, "highres_field", "qmesh", nodal_field_highres,
            qdims, 2, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
    }

    // Close the Silo file.
    DBClose(dbfile);
    return 0;
}
