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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>

//
// Purpose: Build a simple, 2D, quadmesh with coordinate field chosen such
// that the mesh takes the shape of a circle. Also, add a node-centered
// elevation variable for a hemi-sphere above the disk.
//
// Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to output double precision versions of mesh and variables
//

void
build_mesh(int N, float **xCoords, float **yCoords, float **sphElev,
               double **xCoordsD, double **yCoordsD, double **sphElevD)
{
    int iMax = (N - 1) / 2;
    int jMax = (N - 1) / 2;

    float *xvals = new float[N * N];
    float *yvals = new float[N * N];
    float *evals = new float[N * N];
    double *xvalsD = new double[N * N];
    double *yvalsD = new double[N * N];
    double *evalsD = new double[N * N];

    for (int i = -iMax; i <= iMax; i++)
    {
        for (int j = jMax; j >= -jMax; j--)
        {
            int absi = i < 0 ? -i : i;
            int absj = j < 0 ? -j : j;
            int sq = absi < absj ? absj : absi;

            float radius = 1.4142135623731 * sq;
            double radiusD = 1.4142135623731 * sq;
            float x, y, angle;
            double xD, yD, angleD;
            if (absi != 0)
            {
                angle = atan((double) absj / (double) absi);
                angleD = atan((double) absj / (double) absi);
                x = radius * cos(angle);
                xD = radiusD * cos(angleD);
                y = radius * sin(angle);
                yD = radiusD * sin(angleD);
            }
            else
            {
                x = 0.0;
                xD = 0.0;
                y = radius;
                yD = radiusD;
            }

            if (absi != i)
            {
                x = -x;
                xD = -xD;
            }
            if (absj == j)
            {
                y = -y;
                yD = -yD;
            }

            int ii = i + iMax;
            int jj = j + jMax;
            int kk = jj * N + ii;
            xvals[kk] = x;
            xvalsD[kk] = xD;
            yvals[kk] = y;
            yvalsD[kk] = yD;
            evals[kk] = sqrt(2.0 * iMax * iMax - radius * radius);
            //evalsD[kk] = sqrt(2.0 * iMax * iMax - radiusD * radiusD);
            evalsD[kk] = evals[kk]; 
        }
    }

    *xCoords = xvals;
    *xCoordsD = xvalsD;
    *yCoords = yvals;
    *yCoordsD = yvalsD;
    *sphElev = evals;
    *sphElevD = evalsD;
}

int
main(int argc, char **argv)
{
    DBfile *dbfile;
    int N = 21;
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
               exit(-1);
            }
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            i++;
            N = atoi(argv[i]);
            if (N < 0 || N > 10000)
            {
               fprintf(stderr,"size, %d, too large\n", N);
               exit(-1);
            }
            if (N % 2 != 1)
            {
               fprintf(stderr,"size, %d, should be an odd number\n", N);
               exit(-1);
            }
        }

        i++;
    }


    float *coords[2] = {0, 0};
    double *coordsD[2] = {0, 0};
    float *sphElev = 0;
    double *sphElevD = 0;
    build_mesh(N, &coords[0], &coords[1], &sphElev,
                  &coordsD[0], &coordsD[1], &sphElevD);

    dbfile = DBCreate("quad_disk.silo", DB_CLOBBER, DB_LOCAL,
                      "2D logical grid deformed into a disk", driver);

    char *coordnames[2];
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    int ndims = 2;
    int dims[2];
    dims[0] = N; 
    dims[1] = N;

    DBPutQuadmesh(dbfile, "mesh", coordnames, coords, dims, ndims,
        DB_FLOAT, DB_NONCOLLINEAR, NULL);

    DBPutQuadmesh(dbfile, "meshD", coordnames, (float**) coordsD, dims, ndims,
        DB_DOUBLE, DB_NONCOLLINEAR, NULL);

    DBPutQuadvar1(dbfile, "sphElevD_on_meshD", "meshD", (float*) sphElevD, dims, ndims,
                             NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
    DBPutQuadvar1(dbfile, "sphElevD_on_mesh", "mesh", (float*) sphElevD, dims, ndims,
                             NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
    DBPutQuadvar1(dbfile, "sphElev_on_meshD", "meshD", sphElev, dims, ndims,
                             NULL, 0, DB_FLOAT, DB_NODECENT, NULL);
    DBPutQuadvar1(dbfile, "sphElev_on_mesh", "mesh", sphElev, dims, ndims,
                             NULL, 0, DB_FLOAT, DB_NODECENT, NULL);

    DBClose(dbfile);
}
