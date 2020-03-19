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

int
main(int argc, char **argv)
{
    DBfile *dbfile;
    int N = 21;
    int driver = DB_PDB;

    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[i], "DB_PDB") == 0)
            driver = DB_PDB;
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
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[i]);
        i++;
    }


    float x[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    float y[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float *coords[2] = {x, y};

    dbfile = DBCreate("lineout_test.silo", DB_CLOBBER, DB_LOCAL,
                      "2D grid with stair-step and linear fields", driver);

    char *coordnames[2];
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";

    int zonelist[] = {0,1,7,6,   1,2,8,7,   2,3,9,8,   3,4,10,9,   4,5,11,10};
    int zshapetype = DB_ZONETYPE_QUAD;
    int zshapesize = 4;
    int zshapecnt = 5;

    DBPutZonelist2(dbfile, "zl2d", 5, 2, zonelist, sizeof(zonelist)/sizeof(zonelist[0]), 0,
                   0, 0, &zshapetype, &zshapesize,
                   &zshapecnt, 1, NULL);

    DBPutUcdmesh(dbfile, "mesh", 2, coordnames, coords, 12, 5,
                     "zl2d", NULL, DB_FLOAT, NULL);

    float zc_var[] = {0.5, 1.5, 2.5, 3.5, 4.5};
    DBPutUcdvar1(dbfile, "zonal_var", "mesh", zc_var, 5, NULL, 0, DB_FLOAT,
        DB_ZONECENT, NULL);

    float nc_var[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    DBPutUcdvar1(dbfile, "nodal_var", "mesh", nc_var, 12, NULL, 0, DB_FLOAT,
        DB_NODECENT, NULL);

    float x1[] = {-1.5, -.5, 0.5, 1.5, -1.5, -.5, 0.5, 1.5, -1.5, -.5, 0.5, 1.5, -1.5, -.5, 0.5, 1.5};
    float y1[] = {-1.5, -1.5, -1.5, -1.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 1.5, 1.5, 1.5, 1.5};
    int zonelist2[] = {0,1,5,4,   1,2,6,5,   2,3,7,6,
                       4,5,9,8,   5,6,10,9,  6,7,11,10,
		       8,9,13,12, 9,10,14,13,10,11,15,14};

    zshapecnt = 9;
    DBPutZonelist2(dbfile, "zl2d2", 9, 2, zonelist2, sizeof(zonelist2)/sizeof(zonelist2[0]), 0,
                   0, 0, &zshapetype, &zshapesize,
                   &zshapecnt, 1, NULL);

    coords[0] = x1; coords[1] = y1;
    DBPutUcdmesh(dbfile, "mesh2", 2, coordnames, coords, 16, 9,
                     "zl2d2", NULL, DB_FLOAT, NULL);

    float zc_var2[] = {1.5, 0, -1.5, 0, 0, 0, -1.5, 0, 1.5};
    DBPutUcdvar1(dbfile, "zonal_var2", "mesh2", zc_var2, 9, NULL, 0, DB_FLOAT,
        DB_ZONECENT, NULL);

    // This is designed to yield a perfect quadratic along the diagonal
    // y=x and y=-x lineouts such that value of nc_var2 is equal to distance
    // from origin (0,0), squared. A resulting lineout *should* look like a
    // perfect quadratic curve.
    float a = (1.5*1.5 + 1.5*1.5);
    float b = (0.5*0.5 + 1.5*1.5);
    float c = (0.5*0.5 + 0.5*0.5);
    float nc_var2[] = {-a, -b, b, a, -b, -c, c, b, b, c, -c, -b, a, b, -b, -a};
    DBPutUcdvar1(dbfile, "nodal_var2", "mesh2", nc_var2, 16, NULL, 0, DB_FLOAT,
        DB_NODECENT, NULL);

    DBClose(dbfile);
}
