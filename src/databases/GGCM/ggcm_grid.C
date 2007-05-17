#include "libggcm.h"

#include <stdlib.h>
#include <string.h>

/** looks through an MHDdata structure for a particular
 * data set which is given by the third argument.
 * Returns the data set, and side effects the 2nd argument to give
 * the length of the set. */
float *ggcm_grid(MHDdata *data, int *ndim, char * const grid_name)
{
    MHDdata *dat = data;
    float *grid;

    /* find the size of the data referenced by grid_name, and
     * return it via side effect in ndim */
    while (dat != NULL) {
        if(strcmp(dat->field_name, grid_name) == 0) {
            *ndim = dat->dim_size[0];
            break;
        }
        dat = dat->next;
    }

    /* we know the sizeof the data now, gather them up */
    grid = (float *)malloc((*ndim)*sizeof(float));
    memcpy(grid, dat->data, sizeof(float) * (*ndim));
    return grid;
}

/** alias for ggcm_grid */
__deprecated float *ggcmGrid(MHDdata *data, int *ndim, char * const grid_name) {
    return ggcm_grid(data, ndim, grid_name);
}
