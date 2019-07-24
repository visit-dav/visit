// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <silo.h>
#include <stdio.h>
#include <math.h>

void
write_point3d(DBfile *dbfile)
{
    /* Create some points to save. */
#define NPTS 100
    int i;
    float x[NPTS], y[NPTS], z[NPTS];
    int ndims = 3;
    float *coords[] = {(float*)x, (float*)y, (float*)z};
    for(i = 0; i < NPTS; ++i)
    {
        float t = ((float)i) / ((float)(NPTS-1));
        float angle = 3.14159 * 10. * t;
        x[i] = t * cos(angle);
        y[i] = t * sin(angle);
        z[i] = t;
    }
    /* Write a point mesh. */
    DBPutPointmesh(dbfile, "pointmesh", ndims, coords, NPTS,
                   DB_FLOAT, NULL);
}

void
write_pointvar(DBfile *dbfile)
{
    int i;
    float var[NPTS];
    for(i = 0; i < NPTS; ++i)
        var[i] = (float)i;
    DBPutPointvar1(dbfile, "pointvar", "pointmesh", var, NPTS, DB_FLOAT, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("pointvar3d.silo", DB_CLOBBER, DB_LOCAL,
        "3D point variables", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_point3d(dbfile);
    write_pointvar(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
