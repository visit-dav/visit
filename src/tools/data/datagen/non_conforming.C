// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// suppress the following since silo uses char * in its API
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wwritable-strings"
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
    float x[] = {0,1,2,0,1,0,1,2};
    float y[] = {0,0,0,1,1,2,2,2};
    int shapesize = 4;
    int shapecnt = 3;
    int shapetype = DB_ZONETYPE_QUAD;
    int zonelist[] = {0,1,4,3,  3,4,6,5,  1,2,7,6};
    char *coordnames[] = {"x","y"};
    float *coords[2] = {x,y};
    float xcomp[8], ycomp[8], mag[8];
    char *varnames[] = {"xcomp","ycomp"};
    float *vals[2] = {xcomp,ycomp};

    // Check for the right driver.
    int driver = DB_PDB;
    for(int j = 1; j < argc; ++j)
    {
        if (strcmp(argv[j], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[j], "DB_PDB") == 0)
            driver = DB_PDB;
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[j]);
    }

    // Open the Silo file
    DBfile *dbfile = DBCreate("non_conforming.silo", DB_CLOBBER, DB_LOCAL,
        "2D, non-conforming mesh with continuous fields to test "
        "VisIt's expression system", driver);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    DBPutUcdmesh(dbfile, "mesh", 2, coordnames, coords,
                8, 3, "zonelist", NULL, DB_FLOAT, NULL);

    DBPutZonelist2(dbfile, "zonelist", 3, 2, zonelist, (int) sizeof(zonelist),
                0, 0, 0, &shapetype, &shapesize, &shapecnt, 1, NULL);

    for (i = 0; i < 8; i++)
    {
        xcomp[i] = 1/(x[i]+1);
        ycomp[i] = 1/(y[i]*y[i]+1);
        mag[i] = sqrt(xcomp[i]*xcomp[i]+ycomp[i]*ycomp[i]);
    }
    /* enforce C1 continuity at the hanging node */
    xcomp[4] = (xcomp[6]+xcomp[1]) / 2;
    ycomp[4] = (ycomp[6]+ycomp[1]) / 2;
    mag[4] = (mag[6]+mag[1]) / 2;

    DBPutUcdvar(dbfile, "vec", "mesh", 2, varnames, (float**)vals,
        8, NULL, 0, DB_FLOAT, DB_NODECENT, 0);

    DBPutUcdvar1(dbfile, "vec_mag", "mesh", mag,
        8, NULL, 0, DB_FLOAT, DB_NODECENT, 0);

    // Close the Silo file.
    DBClose(dbfile);
    return 0;
}
