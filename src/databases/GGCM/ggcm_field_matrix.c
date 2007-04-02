#include "libggcm.h"

#include <string.h>
#include <stdlib.h>

/** Our fields are stored in an MHDdata record, in no particular order
 * (actually it depends on the input order, which we don't rely on).  This
 * scans through and copies the data for the desired field out of the
 * structure. */
float *ggcm_field(MHDdata * data, int nx, int ny, int nz,
                  char * const field_name)
{
    float *ret = (float*)malloc(sizeof(float) * (nx*ny*nz));
    MHDdata *dat = data;
#if 0
    int cnt=0;
    float *cur = ret;
#endif

    while(dat != NULL) {
        if(strcmp(dat->field_name, field_name) == 0) {
#if 0
            for(cnt = 0; cnt < (nx*ny*nz); ++cnt) {
                *cur++ = dat->data[cnt];
            }
#else
            memcpy(ret, dat->data, sizeof(float) * (nx*ny*nz));
#endif
            break;
        }
        dat = dat->next;
    }
    return ret;
}

float ***ggcm_field_matrix(const MHDdata * data, int nx, int ny, int nz,
                           char field_name[128])
{
    float ***fmat = m_alloc(nx, ny, nz);
    int i,j,k, count=0;
    MHDdata *dat = (MHDdata*) data;

    while (dat != NULL) {
        if (strcmp(dat->field_name, field_name) == 0) {
            for (k = 0; k < nz; ++k) {
                for (j = 0; j < ny; ++j) {
                    for (i = 0; i < nx; ++i) {
                        fmat[i][j][k] = dat->data[count++];
                    }
                }
            }
        }
        dat = dat->next;
    }
    return fmat;
}

/** alias for ggcm_field_matrix */
__deprecated float ***ggcmFieldMatrix(const MHDdata *data,
                                      int nx, int ny, int nz,
                                      char field_name[128]) {
    return ggcm_field_matrix(data, nx, ny, nz, field_name);
}
