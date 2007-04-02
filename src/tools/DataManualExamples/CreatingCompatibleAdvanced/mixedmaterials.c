#include <silo.h>
#include <stdio.h>
#include <string.h>

#define NX 5
#define NY 4

/* Material arrays */

int matnos[] = {1,2,3};

char *matnames[] = {"Water", "Membrane", "Air"};

int matlist[] = {
    3, -1, -3, 1,
    3, -5, -7, 1,
    3, -9, -11, -14
};

float mix_vf[] = {
    0.75,0.25,     0.1875,0.8125,
    0.625,0.375,   0.4375,0.56250,
    0.3,0.7,       0.2,0.4,0.4,      0.45,0.55
};

int mix_zone[] = {
    1,1,  2,2,
    5,5,  6,6,
    9,9,  10,10,10,  11,11
};

int mix_mat[] = {
    2,3,  2,1,
    2,3,  2,1,
    2,3,  1,2,3,  2,1
};

int mix_next[] = {
    2,0,  4,0,
    6,0,  8,0,
    10,0, 12,13,0,  15,0
};

int mixlen = 15;

int
main(int argc, char *argv[])
{
    int i;
    int dims[2], mdims[2], ndims;
    float xc[] = {0., 0.5, 1.0, 1.5, 2.0};
    float yc[] = {0., 0.5, 1.0, 1.5};
    float *coords[2];
    DBfile *dbfile = NULL;
    DBoptlist *optlist = NULL;

    /* Open the Silo file */
    dbfile = DBCreate("mixedmaterials.silo", DB_CLOBBER, DB_LOCAL,
        "Shows how to write materials", DB_HDF5);

    /* Write the mesh to the file. */
    ndims = 2;
    dims[0] = NX;
    dims[1] = NY;
    coords[0] = xc;
    coords[1] = yc;
    DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, NULL);

    /* Write out the material */
    mdims[0] = NX-1;
    mdims[1] = NY-1;
    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_MATNAMES, matnames);
    DBPutMaterial(dbfile, "mat", "quadmesh", 3, matnos, matlist, mdims, 
        ndims, mix_next, mix_mat, mix_zone, mix_vf, mixlen, DB_FLOAT, 
        optlist);
    DBFreeOptlist(optlist);

    /* Close the Silo file. */
    DBClose(dbfile);

    return 0;
}
