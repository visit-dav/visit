#include <visit_writer.h>

/*   
  *---*---*--*   nodes (5,6,7,8)
  |   |\5/ 3 |
  | 1 |4*----*   nodes (3,4)
  |   |/  2  |
  *---*------*   nodes(0,1,2)

  cell 1 quad(0,1,6,5)
  cell 2 quad(1,2,4,3)
  cell 3 quad(3,4,8,7)
  cell 4 tri(1,3,6)
  cell 5 tri(3,7,6)

*/

int
main(int argc, char *argv[])
{
    /* Node coordinates */
    int nnodes = 9;
    int nzones = 5;
    float pts[] = {0., 0., 0., 2., 0., 0., 5., 0., 0.,
        3., 3., 0., 5., 3., 0., 0., 5., 0., 
        2., 5., 0., 4., 5., 0., 5., 5., 0.};

    /* Zone types */
    int zonetypes[] = {VISIT_TRIANGLE, VISIT_TRIANGLE,
        VISIT_QUAD, VISIT_QUAD, VISIT_QUAD};

    /* Connectivity */
    int connectivity[] = {
        1,3,6,    /* tri zone 1. */
        3,7,6,    /* tri zone 2. */
        0.,1,6,5, /* quad zone 3. */
        1,2,4,3,  /* quad zone 4. */
        3,4,8,7   /* quad zone 5. */
    };

    /* Data arrays */
    float nodal[] = {1,2,3,4,5,6,7,8,9};
    float zonal[] = {1,2,3,4,5};

    /* Info about the variables we're passing to visit_writer. */
    int nvars = 2;
    int vardims[] = {1, 1};
    int centering[] = {0, 1};
    const char *varnames[] = {"zonal", "nodal"};
    float *vars[] = {zonal, nodal};

    /* Pass the mesh and data to visit_writer. */
    write_unstructured_mesh("vwucd2d.vtk", 1, nnodes, pts, nzones,
        zonetypes, connectivity, nvars, vardims, centering,
        varnames, vars);

    return 0;
}
