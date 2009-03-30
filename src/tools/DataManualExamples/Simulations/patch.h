#ifndef AMR_PATCH_H
#define AMR_PATCH_H
#include <stdio.h>
#include <stdlib.h>

#define ALLOC(N,T) (T *)calloc(N, sizeof(T))
#define REALLOC(P,N,T) (T *)realloc(P, (N) * sizeof(T))
#define FREE(P) if(P != NULL) free(P)

typedef struct
{
    unsigned char *data;
    int nx;
    int ny;
} image_t;

struct patch_t;

struct patch_t
{
    int            id;
    int            level;
    int            logical_extents[4];
    float          window[4];

    unsigned char *data;
    int            nx;
    int            ny;

    patch_t       *subpatches;
    int            nsubpatches;
};

void      patch_ctor(patch_t *patch);
void      patch_dtor(patch_t *patch);
void      patch_print(FILE *f, patch_t *patch);
void      patch_alloc_data(patch_t *patch, int nx, int ny);
patch_t  *patch_add_subpatch(patch_t *patch);
int       patch_num_patches(patch_t *patch);
patch_t  *patch_get_patch(patch_t *patch, int id);
int       patch_num_levels(patch_t *patch);
patch_t **patch_flat_array(patch_t *patch);
void      patch_refine(patch_t *patch, int refinement_ratio, 
                       void (*maskcb)(patch_t *, image_t*));

#endif
