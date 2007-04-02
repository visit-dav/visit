#include <visit_writer.h>

int
main(int argc, char *argv[])
{
#define NX 4
#define NY 5
#define NZ 3
    /* Rectilinear mesh coordinates. */
    float x[] = {0., 1., 2.5, 5.};
    float y[] = {0., 2., 2.25, 2.55,  5.};
    float z[] = {0., 1., 3.};
    int dims[] = {NX, NY, NZ};
    int ndims = 3;
    /* Zonal and Nodal variable data. */
    float zonal[NZ-1][NY-1][NX-1], nodal[NZ][NY][NX];
    /* Info about the variables to pass to visit_writer. */
    int nvars = 2;
    int vardims[] = {1, 1};
    int centering[] = {0, 1};
    const char *varnames[] = {"zonal", "nodal"};
    float *vars[] = {(float*)zonal, (float*)nodal};

    /* Create a zonal variable. */
    int i,j,k,index = 0;
    for(k = 0; k < NZ-1; ++k)
        for(j = 0; j < NY-1; ++j)
            for(i = 0; i < NX-1; ++i, ++index)
                zonal[k][j][i] = (float)index;

    /* Create a nodal variable. */
    index = 0;
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i, ++index)
                nodal[k][j][i] = (float)index;

    /* Pass the data to visit_writer to write a VTK file.*/
    write_rectilinear_mesh("vwrect3d.vtk", 0, dims, x, y, z, nvars,
        vardims, centering, varnames, vars);

    return 0;
}
