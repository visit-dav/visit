// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    /* Zonal and Nodal variable data. */
    float zonal[NZ-1][NY-1][NX-1], nodal[NZ][NY][NX];
    float zonalvec[NZ-1][NY-1][NX-1][3], nodalvec[NZ][NY][NX][3];
    /* Info about the variables to pass to visit_writer. */
    int nvars = 4;
    int vardims[] = {1, 1, 3, 3};
    int centering[] = {0, 1, 0, 1};
    const char *varnames[] = {"zonal", "nodal", "zonalvec", "nodalvec"};
    float *vars[4];
    int i,j,k,index = 0;

    /* Create 2 zonal variables; 1 scalar, 1 vector. */
    vars[0] = (float*)zonal;
    vars[1] = (float*)nodal;
    vars[2] = (float*)zonalvec;
    vars[3] = (float*)nodalvec;
    for(k = 0; k < NZ-1; ++k)
        for(j = 0; j < NY-1; ++j)
            for(i = 0; i < NX-1; ++i, ++index)
            {
                zonal[k][j][i] = (float)index;

                zonalvec[k][j][i][0] = 1.f;
                zonalvec[k][j][i][1] = 0.f;
                zonalvec[k][j][i][2] = 0.f;
            }

    /* Create 2 nodal variables; 1 scalar, 1 vector. */
    index = 0;
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i, ++index)
            {
                nodal[k][j][i] = (float)index;

                nodalvec[k][j][i][0] = 0.f;
                nodalvec[k][j][i][1] = 1.f;
                nodalvec[k][j][i][2] = 0.f;
            }

    /* Pass the data to visit_writer to write a VTK file.*/
    write_rectilinear_mesh("vwrect3d.vtk", 0, dims, x, y, z, nvars,
        vardims, centering, varnames, vars);

    return 0;
}
