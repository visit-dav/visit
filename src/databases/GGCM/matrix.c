/* defines three-dimensional matrix objects using pointers to arrays of 
   pointers */
#include "libggcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* these should be fixed to have more graceful error handling. */
__deprecated float ***matrixAllocate(int l, int m, int n)
{
    int i, j;
    float ***A;

    A = (float ***)malloc(l * sizeof(float **));
    if(A == NULL) {
        fprintf(stderr, "fMatrixAllocate3D:  memory unavailable\n");
        abort();
    }
    for(i = 0; i < l; i++) {
        A[i] = (float **)malloc(m * sizeof(float *));
        if(A[i] == NULL) {
            fprintf(stderr, "fMatrixAllocate3D:  memory unavailable\n");
            abort();
        }
        for(j = 0; j < m; j++) {
            A[i][j] = (float *)malloc(n * sizeof(float));
            if(A[i][j] == NULL) {
                fprintf(stderr, "fMatrixAllocate3D:  memory unavailable\n");
                abort();
            }
        }
    }

    return A;
}

__deprecated void matrixFree(float ***A, int l, int m, int n)
{
    int i, j;

    for(i = 0; i < l; i++) {
        for(j = 0; j < m; j++) {
            free(A[i][j]);
        };
        free(A[i]);
    };
    free(A);
}

/** new matrix allocation routines, that feel more like malloc() and free() */
float ***m_alloc(unsigned int x, unsigned int y, unsigned int z)
{
    float ***M;
    int *meta;

    int i, j;
    meta = malloc((sizeof(unsigned int)*3) + (x*sizeof(float**)));
    /* encode the array size into the beginning of the returned memory block */
    meta[0] = x;
    meta[1] = y;
    meta[2] = z;

    M = (float***)(meta + 3);

    for(i=0; i < x; ++i) {
        M[i] = (float**) malloc(y * sizeof(float*));
        for(j=0; j < y; ++j) {
            M[i][j] = (float*) malloc(z * sizeof(float));
        }
    }

    return M;
}

/** frees a 3D matrix; assumes it was allocated via m_alloc */
void m_free(float ***M)
{
    unsigned int x, y;
    int *meta;

    /* since 'M' was allocated by m_alloc, the 3 unsigned ints prior to M's
     * base address will be the dimensions of the array */

    meta = (int*)M;
    meta -= 3;

    x = meta[0];
    y = meta[1];

    int i,j;
    for(i=0; i < x; ++i) {
        for(j=0; j < y; ++j) {
            free(M[i][j]);
        }
        free(M[i]);
    }
    free(meta);
}
