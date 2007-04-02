#include <visit_writer.h>

#define NPTS 100

int
main(int argc, char *argv[])
{
    /* Create some points and data to save. */
    int i;
    float pts[NPTS][3], data[NPTS];
    int nvars = 2;
    int vardims[] = {1, 3};
    const char *varnames[] = {"data", "ptsvec"};
    float *vars[] = {(float *)pts, data};

    for(i = 0; i < NPTS; ++i)
    {
        /* Make a point. */
        float t = ((float)i) / ((float)(NPTS-1));
        float angle = 3.14159 * 10. * t;
        pts[i][0] = t * cos(angle);
        pts[i][1] = t * sin(angle);
        pts[i][2] = t;

        /* Make a scalar */
        data[i] = t * cos(angle);
    }

    /* Pass the mesh and data to visit_writer. */
    write_point_mesh("vwpoint3d.vtk", 1, NPTS, (float*)pts, nvars,
        vardims, varnames, vars);

    return 0;
}
