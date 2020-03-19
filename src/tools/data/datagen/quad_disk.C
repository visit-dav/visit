// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>

// suppress the following since silo uses char * in its API
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wwritable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

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
        if (strcmp(argv[i], "-n") == 0)
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
        else if (strcmp(argv[i], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[i], "DB_PDB") == 0)
            driver = DB_PDB;
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[i]);
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
