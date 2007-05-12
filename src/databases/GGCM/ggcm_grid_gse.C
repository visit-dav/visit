#include "libggcm.h"

void ggcm_grid_gse(float *xg, float *yg, float *zg,
                   float *xg_gse, float *yg_gse, float *zg_gse,
                   const int nx, const int ny, const int nz)
{
    int i;
    for (i = 0; i < nx; i++) {
        xg_gse[i] = -xg[nx-i-1];
    }
    for (i = 0; i < ny; i++) {
        yg_gse[i] = -yg[ny-i-1];
    }
    for (i = 0; i < nz; i++) {
        zg_gse[i] = zg[i];
    }
}

/** alias for ggcm_grid_gse */
__deprecated void ggcmGridGSE(float *xg, float *yg, float *zg, 
                              float *x_gse, float *y_gse, float *z_gse,
                              const int nx, const int ny, const int nz) {
    ggcm_grid_gse(xg, yg, zg, x_gse, y_gse, z_gse, nx, ny, nz);
}
