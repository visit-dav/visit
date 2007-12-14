#include "visit_writer.h"
#include <math.h>

int
main(int argc, char **argv)
{
    // Set up storage
    int nCellsX = 10;
    int nCellsY = 10;
    float cellCentered[nCellsX][nCellsY];
    float nodeCentered[nCellsX+1][nCellsY+1];

    // Put data in the arrays.
    int x, y;
    for(x=0;x<nCellsX;x++)
        for(y=0;y<nCellsY;y++)
            cellCentered[x][y] = sin(x/2.)*cos(y/2.);

    for(x=0;x<nCellsX+1;x++)
        for(y=0;y<nCellsY+1;y++)
            nodeCentered[x][y] = 1/(x*y+0.1);

    // Write out the mesh and the arrays.
    int dims[] = { nCellsX+1, nCellsY+1, 1 };    // The number of nodes in the mesh.
    int vardims[] = { 1, 1 };   // Two scalars
    int centering[] = { 1, 0 }; // node centered, cell centered
    const char * const varnames[] = { "nodeCentered", "cellCentered" };
    float *arrays[] = { (float*)nodeCentered, (float*)cellCentered };
    write_regular_mesh("example.vtk", 0, dims, 2, vardims, centering,
                       varnames, arrays);
}
