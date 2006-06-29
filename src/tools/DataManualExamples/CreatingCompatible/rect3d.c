#include <silo.h>
#include <stdio.h>

void
write_rect3d(DBfile *dbfile)
{
    /* Write a rectilinear mesh. */
    float x[] = {0., 1., 2.5, 5.};
    float y[] = {0., 2., 2.25, 2.55,  5.};
    float z[] = {0., 1., 3.};
    int dims[] = {4, 5, 3};
    int ndims = 3;
    float *coords[] = {x, y, z};
    DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("rect3d.silo", DB_CLOBBER, DB_LOCAL,
        "3D rectilinear mesh", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_rect3d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
