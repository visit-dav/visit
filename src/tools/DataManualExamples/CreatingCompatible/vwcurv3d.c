#include <visit_writer.h>

#define NX 4
#define NY 3
#define NZ 2

int
main(int argc, char *argv[])
{
    /* Curvilinear mesh points stored x0,y0,z0,x1,y1,z1,...*/
    float pts[] = {0, 0.5, 0, 1, 0, 0, 2, 0, 0,
        3, 0.5, 0, 0, 1, 0, 1, 1, 0,
        2, 1, 0, 3, 1, 0, 0, 1.5, 0,
        1, 2, 0, 2, 2, 0, 3, 1.5, 0, 
        0, 0.5, 1, 1, 0, 1, 2, 0, 1,
        3, 0.5, 1, 0, 1, 1, 1, 1, 1,
        2, 1, 1, 3, 1, 1, 0, 1.5, 1,
        1, 2, 1, 2, 2, 1, 3, 1.5, 1
    };
    int dims[] = {NX, NY, NZ};
    /* Zonal and nodal variable data. */
    float zonal[NZ-1][NY-1][NX-1], nodal[NZ][NY][NX];
    /* Info about the variables to pass to visit_writer. */
    int nvars = 2;
    int vardims[] = {1, 1};
    int centering[] = {0, 1};
    const char *varnames[] = {"zonal", "nodal"};
    float *vars[] = {(float *)zonal, (float *)nodal};
    int i,j,k, index = 0;

    /* Create zonal variable */
    for(k = 0; k < NZ-1; ++k)
        for(j = 0; j < NY-1; ++j)
            for(i = 0; i < NX-1; ++i, ++index)
                zonal[k][j][i] = (float)index;

    /* Create nodal variable. */
    index = 0;
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i, ++index)
                nodal[k][j][i] = index;

    /* Pass the data to visit_writer to write a binary VTK file. */
    write_curvilinear_mesh("vwcurv3d.vtk", 1, dims, pts, nvars,
        vardims, centering, varnames, vars);

    return 0;
}
