/*------------------------------------------------------------------------
 * mat3d_3across.c -- 3d curvilinear test case with up to 3 materials/zone
 *                    
 *
 * Programmer:  Jeremy Meredith,  March 28, 2000
 *
 *  This test case creates a 3x3x3 curvilinear mesh.  The 9 leftmost zones
 *  contain material 1.  The 9 rightmost zones contain material 3.  The 9
 *  central zones contain materials 1,2,3 in a 20/60/20 split.
 *
 *  This should (ideally) produce a strip of material 2 down the center 
 *  of the mesh.
 *
 *
 * Modifications:
 *   Mark C. Miller, Mon Jan 25 12:11:28 PST 2010
 *   Change default for driver to DB_PDB.
 *
 *-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "silo.h"

// supress the following since silo uses char * in its API
#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wdeprecated-writable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#define zx 3
#define zy 3

#define nx 4
#define ny 4

float x[nx*ny]={-1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2,  -1, 0, 1, 2};
float y[nx*ny]={-1,-1,-1,-1,   0, 0, 0, 0,   1, 1, 1, 1,   2, 2, 2, 2};

int nmat=4;
int matnos[] = {7,1,3,2};

int     matlist[nx*ny]    = { 1,-1,3,  1,-4,3,  1,-7,3 };
int     mix_mat [100]     = { 1,2,3,   1,2,3,  1,2,3  };
int     mix_next[100]     = { 2,3,0,   5,6,0,  8,9,0 };
#define M1 .2
#define M2 .6
#define M3 .2

float   mix_vf  [100]     = { M1,M2,M3, M1,M2,M3, M1,M2,M3 };

int     mix_zone[100]     = {  2, 2, 2,  5, 5, 5,  8, 8, 8 };
int     mixlen = 9;

int     dims[3];

#define ALLOC_N(t,n) (t*)(calloc(n, sizeof(n)));

/*--------------------*/
/*    Main Program    */
/*--------------------*/
int main(int argc, char **argv) {
    DBfile *db;
    int            i, driver = DB_PDB;
    char          *filename = "mat_3x3_3across.silo";
    char  *coordnames[3];
    float *coord[3];

    i=1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-driver") == 0)
        {
            i++;

            if (strcmp(argv[i], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[i], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[i]);
               exit(-1);
            }
        }
        i++;
    }

  coordnames[0]=strdup("x");
  coordnames[1]=strdup("y");
  coordnames[2]=strdup("z");

  coord[0] = x;
  coord[1] = y;
  coord[2] = NULL;

  dims[0]=nx;
  dims[1]=ny;
  dims[2]=1;

  db=DBCreate(filename, DB_CLOBBER, DB_LOCAL,
              "Mixed zone 2d test", driver);

  DBPutQuadmesh(db, "mesh", coordnames, coord, dims, 2, 
                DB_FLOAT, DB_NONCOLLINEAR, NULL);

  dims[0]=zx;
  dims[1]=zy;
  dims[2]=1;

  DBPutMaterial(db, "material", "mesh", nmat, matnos, matlist, dims, 2, 
                mix_next, mix_mat, mix_zone, mix_vf, mixlen, DB_FLOAT, NULL);



  char *expNames[3] = {"mat1", "mat2", "mat3"};
  char *expDefs[3] = {"matvf(<material>, 1)", "matvf(<material>, 2)", "matvf(<material>, 3)"};
  int   expTypes[3] = {DB_VARTYPE_SCALAR, DB_VARTYPE_SCALAR, DB_VARTYPE_SCALAR};
  DBoptlist *optlists[3] = {NULL, NULL, NULL};

  DBPutDefvars(db, "_visit_defvars", 3, expNames, expTypes, expDefs, optlists);

  DBClose(db);

  return 0;
}

