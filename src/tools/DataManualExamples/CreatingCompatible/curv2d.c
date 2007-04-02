#include <silo.h>
#include <stdio.h>

void
write_curv2d(DBfile *dbfile)
{
    /* Write a curvilinear mesh. */
    float x[3][4] = {{0., 1., 3., 3.5}, {0., 1., 2.5, 3.5}, {0.7, 1.3, 2.3, 3.5}};
    float y[3][4] = {{0., 0., 0., 0.}, {1.5, 1.5, 1.25, 1.5}, {3., 2.75, 2.75, 3.}};
    int dims[] = {4, 3};
    int ndims = 2;
    float *coords[] = {(float*)x, (float*)y};
    DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("curv2d.silo", DB_CLOBBER, DB_LOCAL,
        "2D curvilinear mesh", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_curv2d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
