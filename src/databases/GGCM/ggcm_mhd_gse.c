#include "libggcm.h"

#include <string.h>
#include <stdlib.h>

/** converts to GSE 'in place': from a flattened 3d array.  Returns a
 * dynamically allocated array of size nx*ny*nz */
float *ggcm_mhd_gse_inplace(float * const data, int nx, int ny, int nz)
{
    float *ret = (float*) malloc(sizeof(float)*(nx*ny*nz));
    float *cur_gse;
    float *cur_old;
    int x,y,z;

    cur_gse = ret;
    /* manual array index for: data[0][ny-1][nz-1] */
    cur_old = data + ( (ny-1)*nz + (nz-1) );
    for(x=0; x < nx; ++x) {
        for(y=(ny-1); y >= 0; --y) {
            /* manual array index: data[x][y][nz-1] */
            cur_old = data + ( (x*ny*nz) + (y * nz) + (nz-1) );
            for(z=0; z < nz; ++z) {
                *cur_gse++ = - (*cur_old--);
            }
        }
    }
    return ret;
}

/** APPEARS to swap the x and y dimensions, negating them as it does */
void ggcm_mhd_gse(float ***fg, float ***fg_gse, int nx, int ny, int nz,
        char field_name[128])
{
    int i,j,k;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            for (k = 0; k < nz; k++) {
                fg_gse[i][j][k] = fg[nx-1-i][ny-1-j][k];
                if (!strcmp(field_name, "bx") ||
                    !strcmp(field_name, "by") ||
                    !strcmp(field_name, "vx") ||
                    !strcmp(field_name, "vy") ||
                    !strcmp(field_name, "xjx") ||
                    !strcmp(field_name, "xjy")) {
                        fg_gse[i][j][k] = -fg_gse[i][j][k];
                }
            }
        }
    }
}

/** alias for ggcm_mhd_gse */
__deprecated void ggcmMHDGSE(float ***fg, float ***fg_gse,
                             int nx, int ny, int nz,
                             char field_name[128]) {
    ggcm_mhd_gse(fg, fg_gse, nx, ny, nz, field_name);
}
