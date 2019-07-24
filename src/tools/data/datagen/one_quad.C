// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    int driver = DB_PDB;

    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[i], "DB_PDB") == 0)
            driver = DB_PDB;
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[i]);
        i++;
    }


    dbfile = DBCreate("one_quad.silo", DB_CLOBBER, DB_LOCAL,
                      "A single quad", driver);

    float x[4] = {0.0, 1.0, 0.0, 1.1};
    float y[4] = {0.0, 0.0, 1.0, 1.1};
    float *coords[2] = {x, y};
    float nodal[4] = {0.0, 1.0, 1.1, 2.0};

    char *coordnames[2];
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    int ndims = 2;
    int dims[2];
    dims[0] = 2; 
    dims[1] = 2;

    DBPutQuadmesh(dbfile, "mesh", coordnames, coords, dims, ndims,
        DB_FLOAT, DB_NONCOLLINEAR, NULL);

    DBPutQuadvar1(dbfile, "nodal", "mesh", nodal, dims, ndims,
                             NULL, 0, DB_FLOAT, DB_NODECENT, NULL);

    DBClose(dbfile);
}
