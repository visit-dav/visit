#include <visit_writer.h>
#include <math.h>

int
main(int argc, char *argv[])
{
#define NX 10
#define NY 20
#define NZ 30
    int i,j,k, index = 0;
    int dims[] = {NX, NY, NZ};
    int nvars = 2;
    int vardims[] = {1, 1};
    int centering[] = {0, 1};
    const char *varnames[] = {"zonal", "nodal"};
    float zonal[NZ-1][NY-1][NX-1], nodal[NZ][NY][NX];
    float *vars[] = {(float *)zonal, (float *)nodal};

    /* Create zonal variable */
    for(k = 0; k < NZ-1; ++k)
        for(j = 0; j < NY-1; ++j)
            for(i = 0; i < NX-1; ++i, ++index)
                zonal[k][j][i] = (float)index;

    /* Create nodal variable. */
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i)
                nodal[k][j][i] = sqrt(i*i + j*j + k*k);

    /* Use visit_writer to write a regular mesh with data. */
    write_regular_mesh("vwregmesh.vtk", 0, dims, nvars, vardims, 
        centering, varnames, vars);

    return 0;
}
