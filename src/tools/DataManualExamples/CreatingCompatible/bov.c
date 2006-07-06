#include <stdio.h>
#include <visit-config.h>

#define NX 20
#define NY 30
#define NZ 40

int main(int argc, char *argv[])
{
    int i,j,k,index = 0;
    float values[NZ][NY][NX];
    FILE *f = NULL;

    /* Fill up the array */
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i)
                values[k][j][i] = index++;

    /* Write the BOV data file */
    f = fopen("bov.values", "wb");
    if(f == NULL)
    {
        fprintf(stderr, "Could not write data file\n");
        return -1;
    }
    fwrite((void *)values, sizeof(float), NX*NY*NZ, f);
    fclose(f);

    /* Write the BOV header file. */
    f = fopen("bov.bov", "wb");
    if(f == NULL)
    {
        fprintf(stderr, "Could not write header file\n");
        return -1;
    }
    fprintf(f, "TIME: 0.0\n");
    fprintf(f, "DATA_FILE: bov.values\n");
    fprintf(f, "DATA_SIZE: %d %d %d\n", NX, NY, NZ);
    fprintf(f, "DATA_FORMAT: FLOAT\n");
    fprintf(f, "VARIABLE: var\n");
#ifdef WORDS_BIGENDIAN
    fprintf(f, "DATA_ENDIAN: BIG\n");
#else
    fprintf(f, "DATA_ENDIAN: LITTLE\n");
#endif
    fprintf(f, "CENTERING: zonal\n");
    fprintf(f, "BRICK_ORIGIN: 0.0 0.0 0.0\n");
    fprintf(f, "BRICK_SIZE: 10.0 10.0 10.0\n");
    fclose(f);

    return 0;
}
