// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <silo.h>
#include <stdio.h>

void
write_curv3d(DBfile *dbfile)
{
    /* Write a curvilinear mesh. */
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
    float *coords[3];
    coords[0] = (float*)x; coords[1] = (float*)y; coords[2] = (float*)z;
    DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("curv3d.silo", DB_CLOBBER, DB_LOCAL,
        "3D curvilinear mesh", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_curv3d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
