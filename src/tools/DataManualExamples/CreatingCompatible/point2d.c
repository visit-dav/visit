#include <silo.h>
#include <stdio.h>
#include <math.h>

void
write_point2d(DBfile *dbfile)
{
    /* Create some points to save. */
#define NPTS 100
    int i;
    float x[NPTS], y[NPTS];
    int dims[] = {NPTS, NPTS}, ndims = 2;
    float *coords[] = {(float*)x, (float*)y};
    for(i = 0; i < NPTS; ++i)
    {
        float t = ((float)i) / ((float)(NPTS-1));
        float angle = 3.14159 * 10. * t;
        x[i] = t * cos(angle);
        y[i] = t * sin(angle);
    }
    /* Write a point mesh. */
    DBPutPointmesh(dbfile, "pointmesh", ndims, coords, NPTS,
                   DB_FLOAT, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("point2d.silo", DB_CLOBBER, DB_LOCAL,
        "2D point mesh", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_point2d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
