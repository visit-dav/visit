#include <silo.h>
#include <stdio.h>

void
write_ucd3d(DBfile *dbfile)
{
    /* Node coordinates */
    float x[] = {0.,2.,2.,0.,0.,2.,2.,0.,0.,2.,2.,0.,1.,2.,4.,4.};
    float y[] = {0.,0.,0.,0.,2.,2.,2.,2.,4.,4.,4.,4.,6.,0.,0.,0.};
    float z[] = {2.,2.,0.,0.,2.,2.,0.,0.,2.,2.,0.,0.,1.,4.,2.,0.};
    float *coords[] = {x, y, z};
    /* Connectivity */
    int nodelist[] = {
        1,2,3,4,5,6,7,8,    /* hex,     zone 1 */
        5,6,7,8,9,10,11,12, /* hex,     zone 2 */
        9,10,11,12,13,      /* pyramid, zone 3 */
        2,3,16,15,6,7,      /* prism,   zone 4 */
        2,15,14,6           /* tet,     zone 5 */
    };
    int lnodelist = sizeof(nodelist) / sizeof(int);
    /* shape type 1 has 8 nodes (hex) */
    /* shape type 2 has 5 nodes (pyramid) */
    /* shape type 3 has 6 nodes (prism) */
    /* shape type 4 has 4 nodes (tet) */
    int shapesize[] = {8,5,6,4};
    /* We have 2 hex, 1 pyramid, 1 prism, 1 tet */
    int shapecounts[] = {2,1,1,1};
    int nshapetypes = 4;
    int nnodes = 16;
    int nzones = 5;
    int ndims = 3;
    /* Write out connectivity information. */
    DBPutZonelist(dbfile, "zonelist", nzones, ndims, nodelist, lnodelist,
        1, shapesize, shapecounts, nshapetypes);
    /* Write an unstructured mesh. */
    DBPutUcdmesh(dbfile, "mesh", ndims, NULL, coords, nnodes, nzones,
        "zonelist", NULL, DB_FLOAT, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("ucd3d.silo", DB_CLOBBER, DB_LOCAL,
        "Unstructured 3d mesh", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_ucd3d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
