#include <silo.h>
#include <stdio.h>

/*   
  *---*---*--*   nodes (5,6,7,8)
  |   |\5/ 3 |
  | 1 |4*----*   nodes (3,4)
  |   |/  2  |
  *---*------*   nodes(0,1,2)

  cell 1 quad(0,1,6,5)
  cell 2 quad(1,2,4,3)
  cell 3 quad(3,4,8,7)
  cell 4 tri(1,3,6)
  cell 5 tri(3,7,6)

*/
void
write_ucd2d(DBfile *dbfile)
{
    /* Node coordinates */
    float x[] = {0., 2., 5., 3., 5., 0., 2., 4., 5.};
    float y[] = {0., 0., 0., 3., 3., 5., 5., 5., 5.};
    float *coords[] = {x, y};
    /* Connectivity */
    int nodelist[] = {
        2,4,7,   /* tri zone 1. */
        4,8,7,   /* tri zone 2. */
        1,2,7,6, /* quad zone 3. */
        2,3,5,4, /* quad zone 4. */
        4,5,9,8  /* quad zone 5. */
    };
    int lnodelist = sizeof(nodelist) / sizeof(int);
    /* shape type 1 has 3 nodes (tri), shape type 2 is quad */
    int shapesize[] = {3, 4};
    /* We have 2 tris and 3 quads */
    int shapecounts[] = {2, 3};
    int nshapetypes = 2;
    int nnodes = 9;
    int nzones = 5;
    int ndims = 2;

    /* Write out connectivity information. */
    DBPutZonelist(dbfile, "zonelist", nzones, ndims, nodelist, lnodelist,
        1, shapesize, shapecounts, nshapetypes);
    /* Write an unstructured mesh. */
    DBPutUcdmesh(dbfile, "mesh", ndims, NULL, coords, nnodes, nzones,
        "zonelist", NULL, DB_FLOAT, NULL);
}

void
write_ucdvars(DBfile *dbfile)
{
    float nodal[] = {1.,2.,3.,4.,5.,6.,7.,8.,9.};
    float zonal[] = {1.,2.,3.,4.,5.};
    int nnodes = 9;
    int nzones = 5;
    DBoptlist *optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void*)"g/cc");

    /* Write a zone-centered variable. */
    DBPutUcdvar1(dbfile, "zonal", "mesh", zonal, nzones, NULL, 0,
       DB_FLOAT, DB_ZONECENT, optlist);
    /* Write a node-centered variable. */
    DBPutUcdvar1(dbfile, "nodal", "mesh", nodal, nnodes, NULL, 0,
       DB_FLOAT, DB_NODECENT, NULL);

    DBFreeOptlist(optlist);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("ucdvar2d.silo", DB_CLOBBER, DB_LOCAL,
        "Unstructured 2D mesh with variables", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_ucd2d(dbfile);
    write_ucdvars(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
