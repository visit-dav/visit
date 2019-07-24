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
// Purpose: Build a one zone, arbitrary polyhedral mesh consisting of
// a single hex that has had one corner cut off.
//
// Programmer: Mark C. Miller, Sat Oct 10 01:32:23 PDT 2009
//

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

    dbfile = DBCreate("arbpoly.silo", DB_CLOBBER, DB_LOCAL,
                      "Simple arbitrary polyhedral clipped hex", driver);

    const float t = 0.75;
    // node ids  0   1   2   3   4   5   6   7   8   9
    float x[] = {0,  0,  1,  1,  0,  0,  t,  1,  1,  1};
    float y[] = {0,  0,  0,  0,  1,  1,  1,  1,  1,  t};
    float z[] = {0,  1,  1,  0,  0,  1,  1,  t,  0,  1};
    float *coords[3]={x, y, z};
    // face ids       0         1         2         3           4           5           6
    int nodecnts[] = {4,        4,        4,        5,          5,          5,          3};
    int nodelist[] = {0,3,2,1,  5,4,0,1,  4,8,3,0,  1,2,9,6,5,  2,3,8,7,9,  4,5,6,7,8,  6,7,9};
    int facecnts[] = {7};
    int facelist[] = {0,1,2,3,4,5,6};

    DBoptlist *optlist = DBMakeOptlist(3);
    char *phzlname = "phzl";
    char *coordnames[] = {"x","y","z"};
    DBAddOption(optlist, DBOPT_PHZONELIST, phzlname);
    DBPutUcdmesh(dbfile, "clipped_hex", 3, coordnames, (float **)coords, 10,
                 1, 0, 0, DB_FLOAT, optlist);
    DBPutPHZonelist(dbfile, "phzl", 7, nodecnts, 30, nodelist,
                    0, 1, facecnts, 7, facelist, 0, 0, 0, 0);
    DBClose(dbfile);
}
