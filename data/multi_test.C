/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

//
// multi_test.C -- Multi-Block Test File Generator.
//
// This test file creates multi-block objects, that are based on the same
// data sets as the objects created by testall.c.
//
//      multi_rect2d.*  - 12 blocks     (3 x 4)
//      multi_curv2d.*  -  5 blocks     (5 x 1)
//      multi_point2d.* -  5 blocks     (5 x 1)
//      multi_rect3d.*  - 36 blocks     (3 x 4 x 3)
//      multi_curv3d.*  - 36 blocks     (3 x 4 x 3)
//      multi_ucd3d.*   - 20 blocks     (1 x 1 x 20)
//

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>

#define MAXBLOCKS       100         // maximum number of blocks in an object
#define MAXNUMVARS      10          // maximum number of vars to output
#define STRLEN          256 
#define MIXMAX          20000       // Maximum length of the mixed arrays
#define MAXMATNO        3

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)

#define ALLOC_N(T,N)    ((T*)calloc((N),sizeof(T)))
#define FREE(M)         if(M){free(M);(M)=NULL;}
#define SET_OPTIONS(ES,EX,ZCNTS,MLEN,MCNTS,MLISTS,HASEXT)   \
    if (optlist) DBFreeOptlist(optlist);                    \
    optlist = DBMakeOptlist(20);                            \
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);              \
    DBAddOption(optlist, DBOPT_TIME, &time);                \
    DBAddOption(optlist, DBOPT_DTIME, &dtime);              \
    DBAddOption(optlist, DBOPT_NMATNOS, &nmatnos);          \
    DBAddOption(optlist, DBOPT_MATNOS, matnos);             \
    DBAddOption(optlist, DBOPT_EXTENTS_SIZE, &ES);          \
    if (EX != 0)                                            \
       DBAddOption(optlist, DBOPT_EXTENTS, EX);             \
    if (ZCNTS != 0)                                         \
       DBAddOption(optlist, DBOPT_ZONECOUNTS, ZCNTS);       \
    if (MLEN != 0)                                          \
       DBAddOption(optlist, DBOPT_MIXLENS, MLEN);           \
    if (MCNTS != 0)                                         \
       DBAddOption(optlist, DBOPT_MATCOUNTS, MCNTS);        \
    if (MLISTS != 0)                                        \
       DBAddOption(optlist, DBOPT_MATLISTS, MLISTS);        \
    if (HASEXT != 0)                                        \
       DBAddOption(optlist, DBOPT_HAS_EXTERNAL_ZONES, HASEXT)


// variables set by argc/argv 
static int noDups = 0;
static int noCycles = 0;
static int dupMultiMatToo = 0;
static int userDefZonelist = 0;
static int noTimeInvariantMultimesh = 0;
static int noHalfMesh = 0;

double varextents[MAXNUMVARS][2*MAXBLOCKS];
int mixlens[MAXBLOCKS];
int zonecounts[MAXBLOCKS];
int has_external_zones[MAXBLOCKS];
int matcounts[MAXBLOCKS];
int matlists[MAXBLOCKS][MAXMATNO+1];

void          build_block_rect2d(DBfile *, char[MAXBLOCKS][STRLEN], int, int);
void          build_block_curv2d(DBfile *, char[MAXBLOCKS][STRLEN], int, int);
void          build_block_point2d(DBfile *, char[MAXBLOCKS][STRLEN], int, int);
void          build_block_rect3d(DBfile *, char[MAXBLOCKS][STRLEN], int, int,
                                 int);
void          build_block_curv3d(DBfile *, char[MAXBLOCKS][STRLEN], int, int,
                                 int);
void          build_block_ucd3d(DBfile *, char[MAXBLOCKS][STRLEN], int, int,
                                int, int);

int           build_multi(DBfile *, int, int, int, int, int, int, int, int);
static void   put_extents(float *arr, int len, double *ext_arr, int block);
static void   fill_rect3d_bkgr(int matlist[], int nx, int ny, int nz,
                               int matno);
static void   fill_rect3d_mat(float x[], float y[], float z[], int matlist[],
                              int nx, int ny, int nz, int mix_next[],
                              int mix_mat[], int mix_zone[], float mix_vf[],
                              int *mixlen, int matno, double xcenter,
                              double ycenter, double zcenter, double radius);

static void
put_extents(float *arr, int len, double *ext_arr, int block)
{
   int i;
   double min = arr[0], max = min;
   for (i = 0; i < len; i++)
   {
      if (arr[i] < min)
         min = arr[i];
      if (arr[i] > max)
         max = arr[i];
   }
   ext_arr[2*block] = min;
   ext_arr[2*block+1] = max;
}

static int 
count_mats(int nzones, int *matlist, int *unique_mats)
{
   int i, num_mats = 0;
   int mat_map[MAXMATNO+1] = {0,0,0,0};
   for (i = 0; i < nzones; i++)
   {
      if (matlist[i]>=0)
         mat_map[matlist[i]] = 1;
   }
   for (i = 0; i < MAXMATNO+1; i++)
      if (mat_map[i] == 1)
         unique_mats[num_mats++] = i;

   return num_mats;
}

static void
fill_rect3d_bkgr(int matlist[], int nx, int ny, int nz, int matno)
{
    int             i, j, k;

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz; k++)
            {
                matlist[k * nx * ny + j * nx + i] = matno;
            }
        }
    }
}

static void
fill_rect3d_mat(float x[], float y[], float z[], int matlist[], int nx,
                int ny, int nz, int mix_next[], int mix_mat[], int mix_zone[],
                float mix_vf[], int *mixlen, int matno, double xcenter,
                double ycenter, double zcenter, double radius)
{
    int             i, j, k, l, m, n;
    double          dist;
    int             cnt;
    int             mixlen2;
    int            *itemp;
    float           dx, dy, dz;
    float           xx[10], yy[10], zz[10];

    mixlen2 = *mixlen;

    itemp = ALLOC_N(int, (nx + 1) * (ny + 1) * (nz + 1));

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter) +
                            (z[k] - zcenter) * (z[k] - zcenter));
                itemp[k * (nx + 1) * (ny + 1) + j * (nx + 1) + i] =
                    (dist < radius) ? 1 : 0;
            }
        }
    }
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz; k++)
            {
                cnt = itemp[(i) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k] +
                      itemp[(i) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k] +
                      itemp[(i) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k + 1] +
                      itemp[(i) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k + 1] +
                      itemp[(i + 1) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k] +
                      itemp[(i + 1) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k] +
                      itemp[(i + 1) * (nx + 1) * (ny + 1) + (j + 1) * (nx + 1) + k + 1] +
                      itemp[(i + 1) * (nx + 1) * (ny + 1) + (j) * (nx + 1) + k + 1];
                if (cnt == 0)
                {
                    /* EMPTY */
                }
                else if (cnt == 8)
                {
                    matlist[i * nx * ny + j * nx + k] = matno;
                }
                else
                {
                    dx = (x[i + 1] - x[i]) / 11.;
                    dy = (y[j + 1] - y[j]) / 11.;
                    dz = (z[k + 1] - z[k]) / 11.;
                    for (l = 0; l < 10; l++)
                    {
                        xx[l] = x[i] + (dx / 2.) + (l * dx);
                        yy[l] = y[j] + (dy / 2.) + (l * dy);
                        zz[l] = z[k] + (dz / 2.) + (l * dz);
                    }
                    cnt = 0;
                    for (l = 0; l < 10; l++)
                    {
                        for (m = 0; m < 10; m++)
                        {
                            for (n = 0; n < 10; n++)
                            {
                                dist = sqrt((xx[l] - xcenter) *
                                            (xx[l] - xcenter) +
                                            (yy[m] - ycenter) *
                                            (yy[m] - ycenter) +
                                            (zz[n] - zcenter) *
                                            (zz[n] - zcenter));
                                cnt += (dist < radius) ? 1 : 0;
                            }
                        }
                    }
                    matlist[i * nx * ny + j * nx + k] = -(mixlen2 + 1);
                    mix_mat[mixlen2] = matno - 1;
                    mix_mat[mixlen2 + 1] = matno;
                    mix_next[mixlen2] = mixlen2 + 2;
                    mix_next[mixlen2 + 1] = 0;
                    mix_zone[mixlen2] = i * nx * ny + j * nx + k;
                    mix_zone[mixlen2 + 1] = i * nx * ny + j * nx + k;
                    mix_vf[mixlen2] = 1. - (((float)cnt) / 1000.);
                    mix_vf[mixlen2 + 1] = ((float)cnt) / 1000.;
                    mixlen2 += 2;
                }
            }
        }
    }

    FREE(itemp);

    *mixlen = mixlen2;
}

void
build_curves(DBfile *dbfile,  char dirnames[MAXBLOCKS][STRLEN])
{
#define NVALS 100
#define NCURVES 3
#define LINE 0
#define WAVE 1
#define LOG 2

    int i;
    float x[NCURVES][NVALS];
    float y[NCURVES][NVALS];
    for (i = 0; i < NVALS; i++)
    {
        x[LINE][i] = (float) i;
        y[LINE][i] = 0.5 * x[LINE][i] + -5.0; 

        x[WAVE][i] = (float) i;
        y[WAVE][i] = sin(9.0*i*3.1415926 / 180.);

        x[LOG][i] = log(i+1.0);
        y[LOG][i] = (float) i;
    }

    DBoptlist *optlist = DBMakeOptlist(10);
    char *xaxis="X Axis";
    char *yaxis="Y Axis";
    DBAddOption(optlist, DBOPT_XLABEL, xaxis);
    DBAddOption(optlist, DBOPT_YLABEL, yaxis);
    char *units="cm";
    DBAddOption(optlist, DBOPT_XUNITS, units);
    DBAddOption(optlist, DBOPT_YUNITS, units);


    char *line = "line";
    char *wave = "wave";
    char *log = "log";
    DBAddOption(optlist, DBOPT_LABEL, line);
    DBPutCurve(dbfile, "line", x[LINE], y[LINE], DB_FLOAT, NVALS, optlist);
    DBClearOption(optlist, DBOPT_LABEL);
    DBAddOption(optlist, DBOPT_LABEL, wave);
    DBPutCurve(dbfile, "wave", x[WAVE], y[WAVE], DB_FLOAT, NVALS, optlist);
    DBClearOption(optlist, DBOPT_LABEL);
    DBAddOption(optlist, DBOPT_LABEL, log);
    DBPutCurve(dbfile, "log", x[LOG], y[LOG], DB_FLOAT, NVALS, optlist);

    DBFreeOptlist(optlist);

}

//
// Modifications:
//   Brad Whitlock, Tue May 16 18:47:42 PST 2006
//   Added missing code to set the extents for each block. The missing
//   extents coding caused zeroes to be written for the optlist's data
//   extents for all of the blocks, resulting in images that were the
//   wrong color.
//
void
build_block_rect2d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                   int nblocks_x, int nblocks_y)
{
#undef NX
#define NX 30
#undef NY
#define NY 40
#undef NZ
#define NZ 30
    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    int             ndims;
    int             dims[3], zdims[3];
    float          *coords[3];
    float           x[NX + 1], y[NY + 1];

    char           *meshname, *var1name, *var2name, *var3name, *var4name, *matname;
    char           *meshnamedup, *var1namedup, *var3namedup, *var4namedup, *matnamedup;
    float           d[NX * NY], p[NX * NY], u[(NX + 1) * (NY + 1)], v[(NX + 1) * (NY + 1)];

    int             nmats;
    int             matnos[3];
    int             matlist[NX * NY];
    int             dims2[3];
    int             mixlen;
    int             mix_mat[NX * NY];
    float           mix_vf[NX * NY];

    DBoptlist      *optlist;

    int             i, j, k, l;
    float           xave, yave;
    float           xcenter, ycenter;
    float           dist;
    float           dx, dy;
    float           xx[20], yy[20];
    int             cnt;
    int             itemp[(NX + 1) * (NY + 1)];

    int             block;
    int             delta_x, delta_y;
    int             base_x, base_y;
    int             n_x, n_y;

    float           x2[NX + 1], y2[NY + 1];
    float           d2[NX * NY], p2[NX * NY], u2[(NX + 1) * (NY + 1)], v2[(NX + 1) * (NY + 1)];
    int             matlist2[NX * NY];
    int             mixlen2;
    int             mix_next2[NX * NY], mix_mat2[NX * NY], mix_zone2[NX * NY];
    float           mix_vf2[NX * NY];

    //
    // Create the mesh.
    //
    meshname = "mesh1";
    meshnamedup = "mesh1_dup";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coords[0] = x;
    coords[1] = y;
    ndims = 2;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    for (i = 0; i < NX + 1; i++)
        x[i] = i * (1. / NX);
    for (i = 0; i < NY + 1; i++)
        y[i] = i * (1. / NX);

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var1namedup = "d_dup";
    var2name = "p";
    xcenter = .5;
    ycenter = .5;
    zdims[0] = NX;
    zdims[1] = NY;
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            xave = (x[i] + x[i + 1]) / 2.;
            yave = (y[j] + y[j + 1]) / 2.;
            dist = sqrt((xave - xcenter) * (xave - xcenter) +
                        (yave - ycenter) * (yave - ycenter));
            d[j * NX + i] = dist;
            p[j * NX + i] = 1. / (dist + .0001);
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var3namedup = "u_dup";
    var4name = "v";
    var4namedup = "v_dup";
    xcenter = .5001;
    ycenter = .5001;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            u[j * (NX + 1) + i] = (x[i] - xcenter) / dist;
            v[j * (NX + 1) + i] = (y[j] - ycenter) / dist;
        }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    matnamedup = "mat1_dup";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = NX;
    dims2[1] = NY;
    mixlen = 0;

    //
    // Put in material 1.
    //
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            matlist[j * NX + i] = 1;
        }
    }

    //
    // Overlay material 2.
    //
    xcenter = .5;
    ycenter = .5;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            itemp[j * (NX + 1) + i] = (dist < .4) ? 1 : 0;
        }
    }
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            cnt = itemp[(j) * (NX + 1) + (i)] + itemp[(j + 1) * (NX + 1) + (i)] +
                itemp[(j + 1) * (NX + 1) + (i + 1)] + itemp[(j) * (NX + 1) + (i + 1)];
            if (cnt == 0)
            {
                /* do nothing */
            }
            else if (cnt == 4)
            {
                matlist[j * NX + i] = 2;
            }
            else
            {
                dx = (x[i + 1] - x[i]) / 21.;
                dy = (y[j + 1] - y[j]) / 21.;
                for (k = 0; k < 20; k++)
                {
                    xx[k] = x[i] + (dx / 2.) + (k * dx);
                    yy[k] = y[j] + (dy / 2.) + (k * dy);
                }
                cnt = 0;
                for (k = 0; k < 20; k++)
                {
                    for (l = 0; l < 20; l++)
                    {
                        dist = sqrt((xx[k] - xcenter) *
                                    (xx[k] - xcenter) +
                                    (yy[l] - ycenter) *
                                    (yy[l] - ycenter));
                        cnt += (dist < .4) ? 1 : 0;
                    }
                }
                matlist[j * NX + i] = -(mixlen + 1);
                mix_mat[mixlen] = 1;
                mix_mat[mixlen + 1] = 2;
                mix_vf[mixlen] = 1. - (((float)cnt) / 400.);
                mix_vf[mixlen + 1] = ((float)cnt) / 400.;
                mixlen += 2;
            }
        }
    }

    //
    // Overlay material 3.
    //
    xcenter = .5;
    ycenter = .5;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            itemp[j * (NX + 1) + i] = (dist < .2) ? 1 : 0;
        }
    }
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NX; j++)
        {
            cnt = itemp[(j) * (NX + 1) + (i)] + itemp[(j + 1) * (NX + 1) + (i)] +
                itemp[(j + 1) * (NX + 1) + (i + 1)] + itemp[(j) * (NX + 1) + (i + 1)];
            if (cnt == 0)
            {
                /* do nothing */
            }
            else if (cnt == 4)
            {
                matlist[j * NX + i] = 3;
            }
            else
            {
                dx = (x[i + 1] - x[i]) / 21.;
                dy = (y[j + 1] - y[j]) / 21.;
                for (k = 0; k < 20; k++)
                {
                    xx[k] = x[i] + (dx / 2.) + (k * dx);
                    yy[k] = y[j] + (dy / 2.) + (k * dy);
                }
                cnt = 0;
                for (k = 0; k < 20; k++)
                {
                    for (l = 0; l < 20; l++)
                    {
                        dist = sqrt((xx[k] - xcenter) *
                                    (xx[k] - xcenter) +
                                    (yy[l] - ycenter) *
                                    (yy[l] - ycenter));
                        cnt += (dist < .2) ? 1 : 0;
                    }
                }
                matlist[j * NX + i] = -(mixlen + 1);
                mix_mat[mixlen] = 2;
                mix_mat[mixlen + 1] = 3;
                mix_vf[mixlen] = 1. - (((float)cnt) / 400.);
                mix_vf[mixlen + 1] = ((float)cnt) / 400.;
                mixlen += 2;
            }
        }
    }

    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;

    coords[0] = x2;
    coords[1] = y2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    dims2[0] = delta_x;
    dims2[1] = delta_y;

    //
    // Create the blocks for the multi-block object.
    //
    for (block = 0; block < nblocks_x * nblocks_y; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        //
        // Now extract the data for this block.
        //
        base_x = (block % nblocks_x) * delta_x;
        base_y = (block / nblocks_x) * delta_y;

        for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
            x2[i] = x[n_x];
        for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
            y2[j] = y[n_y];

        for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
            for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
            {
                u2[j * (delta_x + 1) + i] = u[n_y * (NX + 1) + n_x];
                v2[j * (delta_x + 1) + i] = v[n_y * (NX + 1) + n_x];
            }

        mixlen2 = 0;
        for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
            for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
            {
                d2[j * delta_x + i] = d[n_y * NX + n_x];
                p2[j * delta_x + i] = p[n_y * NX + n_x];

                if (matlist[n_y * NX + n_x] < 0)
                {
                    mixlen = -matlist[n_y * NX + n_x] - 1;

                    matlist2[j * delta_x + i] = -(mixlen2 + 1);
                    mix_mat2[mixlen2] = mix_mat[mixlen];
                    mix_mat2[mixlen2 + 1] = mix_mat[mixlen + 1];
                    mix_next2[mixlen2] = mixlen2 + 2;
                    mix_next2[mixlen2 + 1] = 0;
                    mix_zone2[mixlen2] = j * delta_x + i;
                    mix_zone2[mixlen2 + 1] = j * delta_x + i;
                    mix_vf2[mixlen2] = mix_vf[mixlen];
                    mix_vf2[mixlen2 + 1] = mix_vf[mixlen + 1];
                    mixlen2 += 2;
                }
                else
                    matlist2[j * delta_x + i] = matlist[n_y * NX + n_x];
            }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }

        //
        // Write out the variables.
        //
        optlist = DBMakeOptlist(10);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        char *xaxis="X Axis";
        char *yaxis="Y Axis";
        DBAddOption(optlist, DBOPT_XLABEL, xaxis);
        DBAddOption(optlist, DBOPT_YLABEL, yaxis);
        char *units="cm";
        DBAddOption(optlist, DBOPT_XUNITS, units);
        DBAddOption(optlist, DBOPT_YUNITS, units);

        // populate optional data arrays
        put_extents(x2,dims[0],varextents[0],block);
        put_extents(y2,dims[1],varextents[1],block);
        has_external_zones[block] = 0;
        if ((varextents[0][2*block] <= 0.0) ||
            (varextents[1][2*block] <= 0.0) ||
            (varextents[0][2*block+1] >= 1.0) ||
            (varextents[1][2*block+1] >= 1.0))
            has_external_zones[block] = 1;
        zonecounts[block] = (dims[0]-1)*(dims[1]-1);
        DBPutQuadmesh(dbfile, meshname, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, optlist);
        if (!noDups && block < 2)
            DBPutQuadmesh(dbfile, meshnamedup, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, optlist);

        put_extents(d2,(dims[0]-1)*(dims[1]-1),varextents[3],block);
        DBPutQuadvar1(dbfile, var1name, meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var1namedup, meshnamedup, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(p2,(dims[0]-1)*(dims[1]-1),varextents[4],block);
        DBPutQuadvar1(dbfile, var2name, meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(u2,dims[0]*dims[1],varextents[5],block);
        DBPutQuadvar1(dbfile, var3name, meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var3namedup, meshnamedup, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(v2,dims[0]*dims[1],varextents[6],block);
        DBPutQuadvar1(dbfile, var4name, meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var4namedup, meshnamedup, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        matcounts[block] = count_mats(dims2[0]*dims2[1],matlist2,matlists[block]);
        mixlens[block] = mixlen2;
            DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, dims2, ndims, mix_next2, mix_mat2,
                      mix_zone2, mix_vf2, mixlen2, DB_FLOAT, optlist);
        if (!noDups && block < 2)
            DBPutMaterial(dbfile, matnamedup, meshnamedup, nmats, matnos,
                      matlist2, dims2, ndims, mix_next2, mix_mat2,
                      mix_zone2, mix_vf2, mixlen2, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }
    }
}

void
build_block_curv2d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                   int nblocks_x, int nblocks_y)
{
    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    int             ndims;
    int             dims[3], zdims[3];
    float          *coords[3];
    float           x[(NX + 1) * (NY + 1)], y[(NX + 1) * (NY + 1)];

    char           *meshname, *var1name, *var2name, *var3name, *var4name, *matname;
    char           *meshnamedup, *var1namedup, *var3namedup, *var4namedup, *matnamedup;
    float           d[NX * NY], p[NX * NY], u[(NX + 1) * (NY + 1)], v[(NX + 1) * (NY + 1)];

    int             nmats;
    int             matnos[3];
    int             matlist[NX * NY];
    int             dims2[3];
    int             mixlen;
    int             mix_next[NX * NY], mix_mat[NX * NY], mix_zone[NX * NY];
    float           mix_vf[NX * NY];

    DBoptlist      *optlist = NULL;

    int             i, j;
    float           xave, yave;
    float           xcenter, ycenter;
    float           theta, dtheta;
    float           r, dr;
    float           dist;

    int             block;
    int             delta_x, delta_y;
    int             base_x, base_y;
    int             n_x, n_y;

    float           x2[(NX + 1) * (NY + 1)], y2[(NX + 1) * (NY + 1)];
    float           d2[NX * NY], p2[NX * NY], u2[(NX + 1) * (NY + 1)], v2[(NX + 1) * (NY + 1)];
    int             matlist2[NX * NY];

    // 
    // Create the mesh.
    //
    meshname = "mesh1";
    meshnamedup = "mesh1_dup";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    ndims = 2;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dtheta = (180. / NX) * (3.1415926 / 180.);
    dr = 3. / NY;
    theta = 0;
    for (i = 0; i < NX + 1; i++)
    {
        r = 2.;
        for (j = 0; j < NY + 1; j++)
        {
            x[j * (NX + 1) + i] = r * cos(theta);
            y[j * (NX + 1) + i] = r * sin(theta);
            r += dr;
        }
        theta += dtheta;
    }

    // 
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var1namedup = "d_dup";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    zdims[0] = NX;
    zdims[1] = NY;
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            xave = (x[(j) * (NX + 1) + i] + x[(j) * (NX + 1) + i + 1] +
                    x[(j + 1) * (NX + 1) + i + 1] + x[(j + 1) * (NX + 1) + i]) / 4.;
            yave = (y[(j) * (NX + 1) + i] + y[(j) * (NX + 1) + i + 1] +
                    y[(j + 1) * (NX + 1) + i + 1] + y[(j + 1) * (NX + 1) + i]) / 4.;
            dist = sqrt((xave - xcenter) * (xave - xcenter) +
                        (yave - ycenter) * (yave - ycenter));
            d[j * NX + i] = dist;
            p[j * NX + i] = 1. / (dist + .0001);
        }
    }

    // 
    // Create the velocity component arrays. Note that the indexing
    // on the x and y coordinates is for rectilinear meshes. It
    // generates a nice vector field.
    //
    var3name = "u";
    var3namedup = "u_dup";
    var4name = "v";
    var4namedup = "v_dup";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            u[j * (NX + 1) + i] = (x[i] - xcenter) / dist;
            v[j * (NX + 1) + i] = (y[j] - ycenter) / dist;
        }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    matnamedup = "mat1_dup";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = NX;
    dims2[1] = NY;
    mixlen = 0;

    //
    // Put in the material in 3 shells.
    //
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < 10; j++)
        {
            matlist[j * NX + i] = 1;
        }
        for (j = 10; j < 20; j++)
        {
            matlist[j * NX + i] = 2;
        }
        for (j = 20; j < NY; j++)
        {
            matlist[j * NX + i] = 3;
        }
    }

    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;

    coords[0] = x2;
    coords[1] = y2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    dims2[0] = delta_x;
    dims2[1] = delta_y;

    // 
    // Create the blocks for the multi-block object.
    //
    for (block = 0; block < nblocks_x * nblocks_y; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        // 
        // Now extract the data for this block.
        //
        base_x = (block % nblocks_x) * delta_x;
        base_y = (block / nblocks_x) * delta_y;

        for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
            for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
            {
                x2[j * (delta_x + 1) + i] = x[n_y * (NX + 1) + n_x];
                y2[j * (delta_x + 1) + i] = y[n_y * (NX + 1) + n_x];
                u2[j * (delta_x + 1) + i] = u[n_y * (NX + 1) + n_x];
                v2[j * (delta_x + 1) + i] = v[n_y * (NX + 1) + n_x];
            }

        for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
            for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
            {
                d2[j * delta_x + i] = d[n_y * NX + n_x];
                p2[j * delta_x + i] = p[n_y * NX + n_x];
                matlist2[j * delta_x + i] = matlist[n_y * NX + n_x];
            }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }

        //
        // Write out the variables.
        //
        cycle = 48;
        time = 4.8;
        dtime = 4.8;

        optlist = DBMakeOptlist(10);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        char *xaxis="X Axis";
        char *yaxis="Y Axis";
        DBAddOption(optlist, DBOPT_XLABEL, xaxis);
        DBAddOption(optlist, DBOPT_YLABEL, yaxis);
        char *units="cm";
        DBAddOption(optlist, DBOPT_XUNITS, units);
        DBAddOption(optlist, DBOPT_YUNITS, units);

        put_extents(x2,dims[0]*dims[1],varextents[0],block);
        put_extents(y2,dims[0]*dims[1],varextents[1],block);
        has_external_zones[block] = 1;
        zonecounts[block] = (dims[0]-1)*(dims[1]-1);
        DBPutQuadmesh(dbfile, meshname, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_NONCOLLINEAR, optlist);
        if (!noDups && block < 2)
            DBPutQuadmesh(dbfile, meshnamedup, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_NONCOLLINEAR, optlist);

        put_extents(d2,(dims[0]-1)*(dims[1]-1),varextents[3],block);
        DBPutQuadvar1(dbfile, var1name, meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var1namedup, meshnamedup, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(p2,(dims[0]-1)*(dims[1]-1),varextents[4],block);
        DBPutQuadvar1(dbfile, var2name, meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(u2,dims[0]*dims[1],varextents[5],block);
        DBPutQuadvar1(dbfile, var3name, meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var3namedup, meshnamedup, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(v2,dims[0]*dims[1],varextents[6],block);
        DBPutQuadvar1(dbfile, var4name, meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var4namedup, meshnamedup, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        matcounts[block] = count_mats(dims2[0]*dims2[1],matlist2,matlists[block]);
        mixlens[block] = mixlen;
        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, dims2, ndims, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);
        if (!noDups && block < 2)
            DBPutMaterial(dbfile, matnamedup, meshnamedup, nmats, matnos,
                      matlist2, dims2, ndims, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }
    }
}

void
build_block_point2d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                    int nblocks_x, int nblocks_y)
{
    int             cycle;
    float           time;
    double          dtime;
    float          *coords[3];
    float           x[(NX + 1) * (NY + 1)], y[(NX + 1) * (NY + 1)];

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *meshnamedup, *var1namedup, *var3namedup, *var4namedup;
    float           d[(NX + 1) * (NY + 1)], p[(NX + 1) * (NY + 1)];
    float           u[(NX + 1) * (NY + 1)], v[(NX + 1) * (NY + 1)];

    DBoptlist      *optlist = NULL;

    int             i, j;
    float           xcenter, ycenter;
    float           theta, dtheta;
    float           r, dr;
    float           dist;

    int             block;
    int             delta_x, delta_y;
    int             base_x, base_y;
    int             n_x, n_y;
    int             npts;
    float          *vars[1];

    float           x2[(NX + 1) * (NY + 1)], y2[(NX + 1) * (NY + 1)];
    float           d2[(NX + 1) * (NY + 1)], p2[(NX + 1) * (NY + 1)];
    float           u2[(NX + 1) * (NY + 1)], v2[(NX + 1) * (NY + 1)];

    // 
    // Create the mesh.
    //
    meshname = "mesh1";
    meshnamedup = "mesh1_dup";
    coords[0] = x;
    coords[1] = y;
    dtheta = (180. / NX) * (3.1415926 / 180.);
    dr = 3. / NY;
    theta = 0;
    for (i = 0; i < NX + 1; i++)
    {
        r = 2.;
        for (j = 0; j < NY + 1; j++)
        {
            x[j * (NX + 1) + i] = r * cos(theta);
            y[j * (NX + 1) + i] = r * sin(theta);
            r += dr;
        }
        theta += dtheta;
    }

    // 
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var1namedup = "d_dup";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[j * (NX + 1) + i] - xcenter) * (x[j * (NX + 1) + i] - xcenter) +
                        (y[j * (NX + 1) + i] - ycenter) * (y[j * (NX + 1) + i] - ycenter));
            d[j * (NX + 1) + i] = dist;
            p[j * (NX + 1) + i] = 1. / (dist + .0001);
        }
    }

    // 
    // Create the velocity component arrays.
    //
    var3name = "u";
    var3namedup = "u_dup";
    var4name = "v";
    var4namedup = "v_dup";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            u[j * (NX + 1) + i] = (x[i] - xcenter) / dist;
            v[j * (NX + 1) + i] = (y[j] - ycenter) / dist;
        }
    }

    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;

    coords[0] = x2;
    coords[1] = y2;

    // 
    // Create the blocks for the multi-block object.
    //
    int totalblocks = nblocks_x * nblocks_y;
    for (block = 0; block < totalblocks; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        // 
        // Now extract the data for this block.
        //
        base_x = (block % nblocks_x) * delta_x;
        base_y = (block / nblocks_x) * delta_y;

        // 
        // Duplicated nodes across domain boundaries is necessary 
        // for correct zone-connectivity of meshes, but not for point
        // meshes (whose zones are points), so ensure that nodes are
        // not duplicated. 
        //
        int dx, dy;

        if (block < (totalblocks -1))
        {
            dx = delta_x;
            dy = delta_y + 1;
        }
        else
        {
            dx = delta_x + 1;
            dy = delta_y + 1;
        }
        for (j = 0, n_y = base_y; j < dy ; j++, n_y++)
        {
            for (i = 0, n_x = base_x; i < dx ; i++, n_x++)
            {
                x2[j * (dx) + i] = x[n_y * (NX + 1) + n_x];
                y2[j * (dx) + i] = y[n_y * (NX + 1) + n_x];
                d2[j * (dx) + i] = d[n_y * (NX + 1) + n_x];
                p2[j * (dx) + i] = p[n_y * (NX + 1) + n_x];
                u2[j * (dx) + i] = u[n_y * (NX + 1) + n_x];
                v2[j * (dx) + i] = v[n_y * (NX + 1) + n_x];
            }
        }
        npts = dx * dy;

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }

        //
        // Write out the variables.
        //
        cycle = 48;
        time = 4.8;
        dtime = 4.8;


        optlist = DBMakeOptlist(10);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        char *xaxis="X Axis";
        char *yaxis="Y Axis";
        DBAddOption(optlist, DBOPT_XLABEL, xaxis);
        DBAddOption(optlist, DBOPT_YLABEL, yaxis);
        char *units="cm";
        DBAddOption(optlist, DBOPT_XUNITS, units);
        DBAddOption(optlist, DBOPT_YUNITS, units);

        put_extents(x2,npts,varextents[0],block);
        put_extents(y2,npts,varextents[1],block);
        zonecounts[block] = 0;
        DBPutPointmesh(dbfile, meshname, 2, coords, npts, DB_FLOAT, optlist);
        if (!noDups && block < 2)
            DBPutPointmesh(dbfile, meshnamedup, 2, coords, npts, DB_FLOAT, optlist);

        put_extents(d2,npts,varextents[3],block);
        vars[0] = d2;
        DBPutPointvar(dbfile, var1name, meshname, 1, vars, npts, DB_FLOAT,
                      optlist);
        if (!noDups && block < 2)
            DBPutPointvar(dbfile, var1namedup, meshnamedup, 1, vars, npts, DB_FLOAT,
                      optlist);

        put_extents(p2,npts,varextents[4],block);
        vars[0] = p2;
        DBPutPointvar(dbfile, var2name, meshname, 1, vars, npts, DB_FLOAT,
                      optlist);

        put_extents(u2,npts,varextents[5],block);
        vars[0] = u2;
        DBPutPointvar(dbfile, var3name, meshname, 1, vars, npts, DB_FLOAT,
                      optlist);
        if (!noDups && block < 2)
            DBPutPointvar(dbfile, var3namedup, meshnamedup, 1, vars, npts, DB_FLOAT,
                      optlist);

        put_extents(v2,npts,varextents[6],block);
        vars[0] = v2;
        DBPutPointvar(dbfile, var4name, meshname, 1, vars, npts, DB_FLOAT,
                      optlist);
        if (!noDups && block < 2)
            DBPutPointvar(dbfile, var4namedup, meshnamedup, 1, vars, npts, DB_FLOAT,
                      optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }
    }
}

void
build_block_rect3d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                   int nblocks_x, int nblocks_y, int nblocks_z)
{
    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    int             ndims;
    int             dims[3], zdims[3];
    float          *coords[3];
    float           x[NX + 1], y[NY + 1], z[NZ + 1];

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *meshnamedup, *var1namedup, *var3namedup, *var4namedup;
    char           *var5name, *matname;
    char           *var5namedup, *matnamedup;
    float           d[NX * NY * NZ], p[NX * NY * NZ];
    float           u[(NX + 1) * (NY + 1) * (NZ + 1)], v[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           w[(NX + 1) * (NY + 1) * (NZ + 1)];

    int             nmats;
    int             matnos[3];
    int             matlist[NX * NY * NZ];
    int             dims2[3];
    int             mixlen;
    int             mix_next[MIXMAX], mix_mat[MIXMAX], mix_zone[MIXMAX];
    float           mix_vf[MIXMAX];

    DBoptlist      *optlist = NULL;

    int             i, j, k;
    float           xave, yave, zave;
    float           xcenter, ycenter, zcenter;
    float           dist;

    int             block;
    int             delta_x, delta_y, delta_z;
    int             base_x, base_y, base_z;
    int             n_x, n_y, n_z;

    float           x2[NX + 1], y2[NY + 1], z2[NZ + 1];
    float           d2[NX * NY * NZ], p2[NX * NY * NZ];
    float           u2[(NX + 1) * (NY + 1) * (NZ + 1)], v2[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           w2[(NX + 1) * (NY + 1) * (NZ + 1)];
    int             matlist2[NX * NY * NZ];
    int             mixlen2;
    int             mix_next2[MIXMAX], mix_mat2[MIXMAX], mix_zone2[MIXMAX];
    float           mix_vf2[MIXMAX];

    // 
    // Create the mesh.
    //
    meshname = "mesh1";
    meshnamedup = "mesh1_dup";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;
    for (i = 0; i < NX + 1; i++)
        x[i] = i * (1. / NX);
    for (i = 0; i < NY + 1; i++)
        y[i] = i * (1. / NY);
    for (i = 0; i < NZ + 1; i++)
        z[i] = i * (1. / NZ);

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var1namedup = "d_dup";
    var2name = "p";
    xcenter = .5;
    ycenter = .5;
    zcenter = .5;
    zdims[0] = NX;
    zdims[1] = NY;
    zdims[2] = NZ;
    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            for (k = 0; k < NZ; k++)
            {
                xave = (x[i] + x[i + 1]) / 2.;
                yave = (y[j] + y[j + 1]) / 2.;
                zave = (z[k] + z[k + 1]) / 2.;
                dist = sqrt((xave - xcenter) * (xave - xcenter) +
                            (yave - ycenter) * (yave - ycenter) +
                            (zave - zcenter) * (zave - zcenter));
                d[k * NX * NY + j * NX + i] = dist;
                p[k * NX * NY + j * NX + i] = 1. / (dist + .0001);
            }
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var3namedup = "u_dup";
    var4name = "v";
    var4namedup = "v_dup";
    var5name = "w";
    var5namedup = "w_dup";
    xcenter = .5001;
    ycenter = .5001;
    zcenter = .5001;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter) +
                            (z[k] - zcenter) * (z[k] - zcenter));
                u[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (x[i] - xcenter) / dist;
                v[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (y[j] - ycenter) / dist;
                w[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (z[k] - zcenter) / dist;
            }
        }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    matnamedup = "mat1_dup";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = NX;
    dims2[1] = NY;
    dims2[2] = NZ;
    mixlen = 0;

    //
    // Put in the material for the entire mesh.
    //
    fill_rect3d_bkgr(matlist, NX, NY, NZ, 1);

    mixlen = 0;
    fill_rect3d_mat(x, y, z, matlist, NX, NY, NZ, mix_next,
                  mix_mat, mix_zone, mix_vf, &mixlen, 2, .5, .5, .5, .6);
    fill_rect3d_mat(x, y, z, matlist, NX, NY, NZ, mix_next,
                  mix_mat, mix_zone, mix_vf, &mixlen, 3, .5, .5, .5, .4);
    if (mixlen > MIXMAX)
    {
        printf("memory overwrite: mixlen = %d > %d\n", mixlen, MIXMAX);
        exit(-1);
    }

    //
    // Now extract the data for this block.
    //
    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;
    delta_z = NZ / nblocks_z;

    coords[0] = x2;
    coords[1] = y2;
    coords[2] = z2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    dims[2] = delta_z + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    zdims[2] = delta_z;
    dims2[0] = delta_x;
    dims2[1] = delta_y;
    dims2[2] = delta_z;

    //
    // Create the blocks for the multi-block object.
    //
    for (block = 0; block < nblocks_x * nblocks_y * nblocks_z; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        //
        // Now extract the data for this block.
        //
        base_x = (block % nblocks_x) * delta_x;
        base_y = ((block % (nblocks_x * nblocks_y)) / nblocks_x) * delta_y;
        base_z = (block / (nblocks_x * nblocks_y)) * delta_z;

        for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
            x2[i] = x[n_x];
        for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
            y2[j] = y[n_y];
        for (k = 0, n_z = base_z; k < delta_z + 1; k++, n_z++)
            z2[k] = z[n_z];

        for (k = 0, n_z = base_z; k < delta_z + 1; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
                {
                    u2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        u[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    v2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        v[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    w2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        w[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                }

        mixlen2 = 0;
        for (k = 0, n_z = base_z; k < delta_z; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
                {
                    d2[k * delta_x * delta_y + j * delta_x + i] =
                        d[n_z * NX * NY + n_y * NX + n_x];
                    p2[k * delta_x * delta_y + j * delta_x + i] =
                        p[n_z * NX * NY + n_y * NX + n_x];

                    if (matlist[n_z * NX * NY + n_y * NX + n_x] < 0)
                    {
                        mixlen = -matlist[n_z * NX * NY + n_y * NX + n_x] - 1;

                        matlist2[k * delta_x * delta_y + j * delta_x + i]
                            = -(mixlen2 + 1);
                        mix_mat2[mixlen2] = mix_mat[mixlen];
                        mix_mat2[mixlen2 + 1] = mix_mat[mixlen + 1];
                        mix_next2[mixlen2] = mixlen2 + 2;
                        mix_next2[mixlen2 + 1] = 0;
                        mix_zone2[mixlen2]
                            = k * delta_x * delta_y + j * delta_x + i;
                        mix_zone2[mixlen2 + 1]
                            = k * delta_x * delta_y + j * delta_x + i;
                        mix_vf2[mixlen2] = mix_vf[mixlen];
                        mix_vf2[mixlen2 + 1] = mix_vf[mixlen + 1];
                        mixlen2 += 2;
                    } else
                        matlist2[k * delta_x * delta_y + j * delta_x + i]
                            = matlist[n_z * NX * NY + n_y * NX + n_x];
                }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }

        //
        // Write out the variables.
        //
        cycle = 48;
        time = 4.8;
        dtime = 4.8;

        optlist = DBMakeOptlist(10);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        char *xaxis="X Axis";
        char *yaxis="Y Axis";
        char *zaxis="Z Axis";
        DBAddOption(optlist, DBOPT_XLABEL, xaxis);
        DBAddOption(optlist, DBOPT_YLABEL, yaxis);
        DBAddOption(optlist, DBOPT_ZLABEL, zaxis);
        char *units="cm";
        DBAddOption(optlist, DBOPT_XUNITS, units);
        DBAddOption(optlist, DBOPT_YUNITS, units);
        DBAddOption(optlist, DBOPT_ZUNITS, units);

        // populate varextetnts optional data array
        put_extents(x2,dims[0],varextents[0],block);
        put_extents(y2,dims[1],varextents[1],block);
        put_extents(z2,dims[2],varextents[2],block);

        // populate 'has_external_zones' optional data array
        has_external_zones[block] = 0;
        if ((varextents[0][2*block] <= 0.0) ||
            (varextents[1][2*block] <= 0.0) ||
            (varextents[2][2*block] <= 0.0) ||
            (varextents[0][2*block+1] >= 1.0) ||
            (varextents[1][2*block+1] >= 1.0) ||
            (varextents[2][2*block+1] >= 1.0))
            has_external_zones[block] = 1;

        zonecounts[block] = (dims[0]-1)*(dims[1]-1)*(dims[2]-1);
        DBPutQuadmesh(dbfile, meshname, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, optlist);
        if (!noDups && block < 2)
            DBPutQuadmesh(dbfile, meshnamedup, coordnames, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, optlist);

        put_extents(d2,(dims[0]-1)*(dims[1]-1)*(dims[2]-1),varextents[3],block);
        DBPutQuadvar1(dbfile, var1name, meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var1namedup, meshnamedup, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(p2,(dims[0]-1)*(dims[1]-1)*(dims[2]-1),varextents[4],block);
        DBPutQuadvar1(dbfile, var2name, meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(u2,dims[0]*dims[1]*dims[2],varextents[5],block);
        DBPutQuadvar1(dbfile, var3name, meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var3namedup, meshnamedup, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(v2,dims[0]*dims[1]*dims[2],varextents[6],block);
        DBPutQuadvar1(dbfile, var4name, meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var4namedup, meshnamedup, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(w2,dims[0]*dims[1]*dims[2],varextents[7],block);
        DBPutQuadvar1(dbfile, var5name, meshname, w2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var5namedup, meshnamedup, w2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        matcounts[block] = count_mats(dims2[0]*dims2[1]*dims2[2],matlist2,matlists[block]);
        mixlens[block] = mixlen2;
        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                  matlist2, dims2, ndims, mix_next2, mix_mat2, mix_zone2,
                      mix_vf2, mixlen2, DB_FLOAT, optlist);
        if (!noDups && block < 2)
            DBPutMaterial(dbfile, matnamedup, meshnamedup, nmats, matnos,
                  matlist2, dims2, ndims, mix_next2, mix_mat2, mix_zone2,
                      mix_vf2, mixlen2, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }
    }
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Jan  9 08:58:03 PST 2004
//    Added a bogus history variable.  It has values only for the real zones
//    and not for the ghost zones ('4315).
//
//    Mark C. Miller, Mon Aug  9 20:35:49 PDT 2004
//    Added output of optional global node numbers
//
// ****************************************************************************

void
build_block_ucd3d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                  int nblocks_x, int nblocks_y, int nblocks_z, int iteration)
{
#undef  NX
#define NX 30
#undef  NY
#define NY 40
#undef  NZ
#define NZ 30

    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    float          *coords[3];
    float           x[(NX + 1) * (NY + 1) * (NZ + 1)], y[(NX + 1) * (NY + 1) * (NZ + 1)],
                    z[(NX + 1) * (NY + 1) * (NZ + 1)];
    int             nfaces, nzones, nnodes;
    int             lfacelist, lzonelist;
    int             fshapesize, fshapecnt, zshapetype, zshapesize, zshapecnt;
    int             zonelist[16000];
    int             facelist[10000];
    int             zoneno[2000];
    int             gnodeno[2646];

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *meshnamedup, *var1namedup, *var3namedup, *var4namedup;
    char           *var5name, *var6name, *matname;
    char           *var5namedup, *var6namedup, *matnamedup;
    float          *vars[1];
    char           *varnames[1];
    float           d[(NX + 1) * (NY + 1) * (NZ + 1)], p[(NX + 1) * (NY + 1) * (NZ + 1)],
                    u[(NX + 1) * (NY + 1) * (NZ + 1)], v[(NX + 1) * (NY + 1) * (NZ + 1)],
                    w[(NX + 1) * (NY + 1) * (NZ + 1)];

    int             nmats;
    int             matnos[3];
    int             matlist[NX * NY * NZ];
    int             mixlen;
    int             mix_next[4500], mix_mat[4500], mix_zone[4500];
    float           mix_vf[4500];
    float           xstrip[NX + NY + NZ], ystrip[NX + NY + NZ], zstrip[NX + NY + NZ];

    DBoptlist      *optlist;

    DBfacelist     *fl;

    int             i, j, k;
    int             iz;
    float           xcenter, ycenter;
    float           theta, dtheta;
    float           r, dr;
    float           h, dh;
    float           dist;

    int             block;
    int             delta_x, delta_y, delta_z;
    int             n_x, n_y, n_z;

    int             imin, imax, jmin, jmax, kmin, kmax;
    int             nx, ny, nz;

    float           x2[2646], y2[2646], z2[2646];
    float           d2[2646], p2[2646], u2[2646], v2[2646], w2[2646];
    float           hist2[(NX-1)*(NY-1)*(NZ-1)];
    int             matlist2[2000], ghost[2000];

    int             nreal;
    int             ighost;
    int             itemp;
    int             hi_off;

    int             one = 1;
    DBobject       *obj;

    //
    // Create the coordinate arrays for the entire mesh.
    //
    dh = 20. / (float)NX;
    dtheta = (180. / (float)NY) * (3.1415926 / 180.);
    dr = 3. / (float)NZ;
    h = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        theta = 0.;
        for (j = 0; j < NY + 1; j++)
        {
            r = 2.;
            for (k = 0; k < NZ + 1; k++)
            {
                x[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = r * cos(theta);
                y[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = r * sin(theta);
                z[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = h;
                r += dr;
            }
            theta += dtheta;
        }
        h += dh;
    }

    //
    // Create the density and pressure arrays for the entire mesh.
    //
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] - xcenter) *
                            (x[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] - xcenter) +
                            (y[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] - ycenter) *
                            (y[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] - ycenter));
                d[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = iteration > 100 ? 10 * iteration * dist : dist;
                p[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = iteration > 100 ? 10 * iteration * 1. / (dist + .0001) :
                                                                                                   1. / (dist + .0001);
            }
        }
    }

    //
    // Create the velocity component arrays for the entire mesh.
    //
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter));
                u[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = iteration > 100 ? 10 * iteration * (x[i] - xcenter) / dist :
                                                                                                   (x[i] - xcenter) / dist;
                v[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = iteration > 100 ? 10 * iteration * (y[j] - ycenter) / dist :
                                                                                                   (y[j] - ycenter) / dist;
                w[i * (NX + 1) * (NY + 1) + j * (NX + 1) + k] = 0.;
            }
        }
    }

    //
    // Put in the material for the entire mesh.
    //
    fill_rect3d_bkgr(matlist, NX, NY, NZ, 1);

    for (i = 0; i < NY; i++)
    {
        xstrip[i] = (float)i;
        ystrip[i] = (float)i;
        zstrip[i] = (float)i;
    }

    mixlen = 0;
    fill_rect3d_mat(xstrip, ystrip, zstrip, matlist, NX, NY, NZ, mix_next,
              mix_mat, mix_zone, mix_vf, &mixlen, 2, 15., 20., 15., 10.);
    fill_rect3d_mat(xstrip, ystrip, zstrip, matlist, NX, NY, NZ, mix_next,
               mix_mat, mix_zone, mix_vf, &mixlen, 3, 15., 20., 15., 5.);
    if (mixlen > 4500)
    {
        printf("memory overwrite: mixlen = %d > 4500\n", mixlen);
        exit(-1);
    }

    //
    // Set up variables that are independent of the block number.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    meshname = "mesh1";
    meshnamedup = "mesh1_dup";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    var1name = "d";
    var1namedup = "d_dup";
    var2name = "p";
    var3name = "u";
    var3namedup = "u_dup";
    var4name = "v";
    var4namedup = "v_dup";
    var5name = "w";
    var5namedup = "w_dup";
    var6name = "hist";
    var6namedup = "hist_dup";

    matname = "mat1";
    matnamedup = "mat1_dup";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;

    //
    // Now extract the data for this block.
    //
    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;
    delta_z = NZ / nblocks_z;

    coords[0] = x2;
    coords[1] = y2;
    coords[2] = z2;

    //
    // Create the blocks for the multi-block object.
    //
    for (block = 0; block < nblocks_x * nblocks_y * nblocks_z; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        //
        // Now extract the data for this block.
        //
        imin = (block % nblocks_x) * delta_x - 1;
        imax = MIN(imin + delta_x + 3, NX + 1);
        imin = MAX(imin, 0);
        nx = imax - imin;
        jmin = ((block % (nblocks_x * nblocks_y)) / nblocks_x) * delta_y - 1;
        jmax = MIN(jmin + delta_y + 3, NY + 1);
        jmin = MAX(jmin, 0);
        ny = jmax - jmin;
        kmin = (block / (nblocks_x * nblocks_y)) * delta_z - 1;
        kmax = MIN(kmin + delta_z + 3, NZ + 1);
        kmin = MAX(kmin, 0);
        nz = kmax - kmin;

        for (k = 0, n_z = kmin; n_z < kmax; k++, n_z++)
            for (j = 0, n_y = jmin; n_y < jmax; j++, n_y++)
                for (i = 0, n_x = imin; n_x < imax; i++, n_x++)
                {
                    int idx = n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x;
                    x2[k * nx * ny + j * nx + i] = x[idx];
                    y2[k * nx * ny + j * nx + i] = y[idx];
                    z2[k * nx * ny + j * nx + i] = z[idx];
                    d2[k * nx * ny + j * nx + i] = d[idx];
                    p2[k * nx * ny + j * nx + i] = p[idx];
                    u2[k * nx * ny + j * nx + i] = u[idx];
                    v2[k * nx * ny + j * nx + i] = v[idx];
                    w2[k * nx * ny + j * nx + i] = w[idx];
                    gnodeno[k * nx * ny + j * nx + i] = idx;
                }

        iz = 0;
        for (k = 0, n_z = kmin; n_z < kmax - 1; k++, n_z++)
            for (j = 0, n_y = jmin; n_y < jmax - 1; j++, n_y++)
                for (i = 0, n_x = imin; n_x < imax - 1; i++, n_x++)
                {
                    zonelist[iz] = (k + 0) * nx * ny + (j + 1) * nx + i + 1;
                    zonelist[iz + 1] = (k + 0) * nx * ny + (j + 0) * nx + i + 1;
                    zonelist[iz + 2] = (k + 1) * nx * ny + (j + 0) * nx + i + 1;
                    zonelist[iz + 3] = (k + 1) * nx * ny + (j + 1) * nx + i + 1;
                    zonelist[iz + 4] = (k + 0) * nx * ny + (j + 1) * nx + i + 0;
                    zonelist[iz + 5] = (k + 0) * nx * ny + (j + 0) * nx + i + 0;
                    zonelist[iz + 6] = (k + 1) * nx * ny + (j + 0) * nx + i + 0;
                    zonelist[iz + 7] = (k + 1) * nx * ny + (j + 1) * nx + i + 0;
                    iz += 8;

                    matlist2[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] =
                        matlist[n_z * NX * NY + n_y * NX + n_x];

                    if (((k == 0 || n_z == kmax - 2) &&
                         (n_z != 0 && n_z != NZ - 1)) ||
                        ((j == 0 || n_y == jmax - 2) &&
                         (n_y != 0 && n_y != NY - 1)) ||
                        ((i == 0 || n_x == imax - 2) &&
                         (n_x != 0 && n_x != NX - 1)))
                        ghost[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] = 1;
                    else
                        ghost[k * (nx - 1) * (ny - 1) + j * (nx - 1) + i] = 0;
                }

        //
        // Resort the zonelist, matlist so that the ghost zones are at the
        // end.
        //
        nzones = (nx - 1) * (ny - 1) * (nz - 1);
        for (i = 0 ; i < nzones ; i++)
            hist2[i] = 10.;

        nreal = nzones;
        for (i = 0; i < nzones; i++)
            nreal -= ghost[i];
        ighost = nzones - 1;
        for (i = 0; i < nreal; i++)
        {
            if (ghost[i] == 1)
            {
                //
                // Find the first non ghost zone.
                //
                while (ghost[ighost] == 1)
                    ighost--;
                j = ighost;

                itemp = zonelist[i * 8];
                zonelist[i * 8] = zonelist[j * 8];
                zonelist[j * 8] = itemp;
                itemp = zonelist[i * 8 + 1];
                zonelist[i * 8 + 1] = zonelist[j * 8 + 1];
                zonelist[j * 8 + 1] = itemp;
                itemp = zonelist[i * 8 + 2];
                zonelist[i * 8 + 2] = zonelist[j * 8 + 2];
                zonelist[j * 8 + 2] = itemp;
                itemp = zonelist[i * 8 + 3];
                zonelist[i * 8 + 3] = zonelist[j * 8 + 3];
                zonelist[j * 8 + 3] = itemp;
                itemp = zonelist[i * 8 + 4];
                zonelist[i * 8 + 4] = zonelist[j * 8 + 4];
                zonelist[j * 8 + 4] = itemp;
                itemp = zonelist[i * 8 + 5];
                zonelist[i * 8 + 5] = zonelist[j * 8 + 5];
                zonelist[j * 8 + 5] = itemp;
                itemp = zonelist[i * 8 + 6];
                zonelist[i * 8 + 6] = zonelist[j * 8 + 6];
                zonelist[j * 8 + 6] = itemp;
                itemp = zonelist[i * 8 + 7];
                zonelist[i * 8 + 7] = zonelist[j * 8 + 7];
                zonelist[j * 8 + 7] = itemp;

                itemp = matlist2[i];
                matlist2[i] = matlist2[j];
                matlist2[j] = itemp;

                itemp = ghost[i];
                ghost[i] = ghost[j];
                ghost[j] = itemp;
            }
        }

        //
        // Calculate the external face list.
        //
        nnodes = nx * ny * nz;
        hi_off = nzones - nreal;

        zshapesize = 8;
        zshapecnt = nzones;
        zshapetype = DB_ZONETYPE_HEX;
        lzonelist = nzones * 8;

        fl = DBCalcExternalFacelist2(zonelist, nnodes, 0, hi_off, 0,
                                     &zshapetype, &zshapesize, &zshapecnt, 1,
                                     matlist2, 0);

        nfaces = fl->nfaces;
        fshapecnt = fl->nfaces;
        fshapesize = 4;
        lfacelist = fl->lnodelist;
        for (i = 0; i < lfacelist; i++)
            facelist[i] = fl->nodelist[i];
        for (i = 0; i < nfaces; i++)
            zoneno[i] = fl->zoneno[i];

        DBFreeFacelist(fl);

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }

        //
        // Write out the mesh and variables.
        //
        optlist = DBMakeOptlist(11);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        char *xaxis="X Axis";
        char *yaxis="Y Axis";
        char *zaxis="Z Axis";
        DBAddOption(optlist, DBOPT_XLABEL, xaxis);
        DBAddOption(optlist, DBOPT_YLABEL, yaxis);
        DBAddOption(optlist, DBOPT_ZLABEL, zaxis);
        char *units="cm";
        DBAddOption(optlist, DBOPT_XUNITS, units);
        DBAddOption(optlist, DBOPT_YUNITS, units);
        DBAddOption(optlist, DBOPT_ZUNITS, units);
        DBAddOption(optlist, DBOPT_HI_OFFSET, &hi_off);
        DBAddOption(optlist, DBOPT_NODENUM, gnodeno); 

        if (nfaces > 0)
            DBPutFacelist(dbfile, "fl1", nfaces, 3, facelist, lfacelist, 0,
                      zoneno, &fshapesize, &fshapecnt, 1, NULL, NULL, 0);

        //
        // Output the zonelist.  This is being done at the object
        // level to add the hi_offset option which can't be output
        // with the DBPutZonelist routine.
        //
        obj = DBMakeObject("zl1", DB_ZONELIST, 10);

        DBAddIntComponent(obj, "ndims", 3);
        DBAddIntComponent(obj, "nzones", nzones);
        DBAddIntComponent(obj, "nshapes", 1);
        DBAddIntComponent(obj, "lnodelist", lzonelist);
        DBAddIntComponent(obj, "origin", 0);
        DBAddIntComponent(obj, "hi_offset", hi_off);
        DBAddVarComponent(obj, "nodelist", "zl1_nodelist");
        DBAddVarComponent(obj, "shapecnt", "zl1_shapecnt");
        DBAddVarComponent(obj, "shapesize", "zl1_shapesize");

        if (userDefZonelist)
        {
            DBWriteObject(dbfile, obj, 0);

            DBWrite(dbfile, "zl1_nodelist", zonelist, &lzonelist, 1, DB_INT);
            DBWrite(dbfile, "zl1_shapecnt", &zshapecnt, &one, 1, DB_INT);
            DBWrite(dbfile, "zl1_shapesize", &zshapesize, &one, 1, DB_INT);
        }
        else
        {
            DBPutZonelist2(dbfile, "zl1", nzones, 3, zonelist, lzonelist,
                0, 0, hi_off, &zshapetype, &zshapesize, &zshapecnt, 1, NULL); 
        }
        DBFreeObject(obj);

        if (!noDups && block < iteration)
        {
            obj = DBMakeObject("zl1_dup", DB_ZONELIST, 10);

            DBAddIntComponent(obj, "ndims", 3);
            DBAddIntComponent(obj, "nzones", nzones);
            DBAddIntComponent(obj, "nshapes", 1);
            DBAddIntComponent(obj, "lnodelist", lzonelist);
            DBAddIntComponent(obj, "origin", 0);
            DBAddIntComponent(obj, "hi_offset", 0);
            DBAddVarComponent(obj, "nodelist", "zl1_nodelist");
            DBAddVarComponent(obj, "shapecnt", "zl1_shapecnt");
            DBAddVarComponent(obj, "shapesize", "zl1_shapesize");

            if (userDefZonelist)
            {
                DBWriteObject(dbfile, obj, 0);
            }
            else
            {
                DBPutZonelist2(dbfile, "zl1_dup", nzones, 3, zonelist, lzonelist,
                    0, 0, 0, &zshapetype, &zshapesize, &zshapecnt, 1, NULL); 
            }
            DBFreeObject(obj);
        }

        //
        // Output the rest of the mesh and variables.
        //
        put_extents(x2,nnodes,varextents[0],block);
        put_extents(y2,nnodes,varextents[1],block);
        put_extents(z2,nnodes,varextents[2],block);
        has_external_zones[block] = nfaces ? 1 : 0;
        zonecounts[block] = nzones;
        if (iteration <= 100)
        {
            if (nfaces > 0)
                DBPutUcdmesh(dbfile, meshname, 3, coordnames, coords,
                             nnodes, nzones, "zl1", "fl1", DB_FLOAT, optlist);
            else
                DBPutUcdmesh(dbfile, meshname, 3, coordnames, coords,
                             nnodes, nzones, "zl1", NULL, DB_FLOAT, optlist);

            if (!noHalfMesh)
            {
                char tmpName[256];
                if (block < nblocks_x * nblocks_y * nblocks_z / 2)
                    sprintf(tmpName, "%s_back", meshname);
                else
                    sprintf(tmpName, "%s_front", meshname);

                if (nfaces > 0)
                    DBPutUcdmesh(dbfile, tmpName, 3, coordnames, coords,
                                 nnodes, nzones, "zl1", "fl1", DB_FLOAT, optlist);
                else
                    DBPutUcdmesh(dbfile, tmpName, 3, coordnames, coords,
                                 nnodes, nzones, "zl1", NULL, DB_FLOAT, optlist);
            }
        }

        //
        // Put only the mesh in the first file for the time-invariant
        // multimesh test which starts with an iteration value of 100
        //
        if (iteration == 100)
        {
            DBSetDir(dbfile, "..");
            continue;
        }

        if (!noDups && block < iteration)
            DBPutUcdmesh(dbfile, meshnamedup, 3, coordnames, coords,
                nnodes, nzones, "zl1_dup", NULL, DB_FLOAT, NULL);

        put_extents(d2,nzones,varextents[3],block);
        vars[0] = d2;
        varnames[0] = var1name;
        DBPutUcdvar(dbfile, var1name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        varnames[0] = var1namedup;
        if (!noDups && block < iteration)
            DBPutUcdvar(dbfile, var1namedup, meshnamedup, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noHalfMesh)
        {
            char tmpVarName[256];
            char tmpMeshName[256];
            if (block < nblocks_x * nblocks_y * nblocks_z / 2)
            {
                sprintf(tmpMeshName, "%s_back", meshname);
                sprintf(tmpVarName, "%s_back", var1name);
            }
            else
            {
                sprintf(tmpMeshName, "%s_front", meshname);
                sprintf(tmpVarName, "%s_front", var1name);
            }
            DBPutUcdvar(dbfile, tmpVarName, tmpMeshName, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        }

        put_extents(p2,nzones,varextents[4],block);
        vars[0] = p2;
        varnames[0] = var2name;
        DBPutUcdvar(dbfile, var2name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(u2,nnodes,varextents[5],block);
        vars[0] = u2;
        varnames[0] = var3name;
        DBPutUcdvar(dbfile, var3name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        varnames[0] = var3namedup;
        if (!noDups && block < iteration)
            DBPutUcdvar(dbfile, var3namedup, meshnamedup, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(v2,nnodes,varextents[6],block);
        vars[0] = v2;
        varnames[0] = var4name;
        DBPutUcdvar(dbfile, var4name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        varnames[0] = var4namedup;
        if (!noDups && block < iteration)
            DBPutUcdvar(dbfile, var4namedup, meshnamedup, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(w2,nnodes,varextents[7],block);
        vars[0] = w2;
        varnames[0] = var5name;
        DBPutUcdvar(dbfile, var5name, meshname, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        varnames[0] = var5namedup;
        if (!noDups && block < iteration)
            DBPutUcdvar(dbfile, var5namedup, meshnamedup, 1, varnames, vars,
                    nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(hist2,nzones-hi_off,varextents[8],block);
        vars[0] = hist2;
        varnames[0] = var6name;
        DBPutUcdvar(dbfile, var6name, meshname, 1, varnames, vars,
                    nzones-hi_off, NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        varnames[0] = var6namedup;
        if (!noDups && block < iteration) 
            DBPutUcdvar(dbfile, var6namedup, meshnamedup, 1, varnames, vars,
                    nzones-hi_off, NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        if (!noHalfMesh)
        {
            char tmpVarName[256];
            char tmpMeshName[256];
            if (block < nblocks_x * nblocks_y * nblocks_z / 2)
            {
                sprintf(tmpMeshName, "%s_back", meshname);
                sprintf(tmpVarName, "%s_back", var6name);
            }
            else
            {
                sprintf(tmpMeshName, "%s_front", meshname);
                sprintf(tmpVarName, "%s_front", var6name);
            }
            DBPutUcdvar(dbfile, tmpVarName, tmpMeshName, 1, varnames, vars,
                    nzones-hi_off, NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        }

        matcounts[block] = count_mats(nzones,matlist2,matlists[block]);
        mixlens[block] = mixlen;
        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, &nzones, 1, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);
        if (!noDups && block < iteration)
            DBPutMaterial(dbfile, matnamedup, meshnamedup, nmats, matnos,
                      matlist2, &nzones, 1, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }
    }
}

void
build_block_curv3d(DBfile *dbfile, char dirnames[MAXBLOCKS][STRLEN],
                   int nblocks_x, int nblocks_y, int nblocks_z)
{
#undef  NX
#define NX 30
#undef  NY
#define NY 40
#undef  NZ
#define NZ 30

    int             cycle;
    float           time;
    double          dtime;
    char           *coordnames[3];
    float          *coords[3];

    float           x[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           y[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           z[(NX + 1) * (NY + 1) * (NZ + 1)];

    int             ndims, zdims[3];
    int             dims[3], dims2[3];

    char           *meshname, *var1name, *var2name, *var3name, *var4name;
    char           *meshnamedup, *var1namedup, *var3namedup, *var4namedup;
    char           *var5name, *matname;
    char           *var5namedup, *matnamedup;

    float           d[NX * NY * NZ], p[NX * NY * NZ];
    float           u[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           v[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           w[(NX + 1) * (NY + 1) * (NZ + 1)];

    int             nmats;
    int             matnos[3];
    int             matlist[NX * NY * NZ];
    int             mixlen;
    int             mix_next[NX * NY * NZ], mix_mat[NX * NY * NZ];
    int             mix_zone[NX * NY * NZ];
    float           mix_vf[NX * NY * NZ];

    DBoptlist      *optlist = NULL;

    int             i, j, k;

    float           xave, yave;
    float           xcenter, ycenter;

    float           theta, dtheta;
    float           r, dr;
    float           h, dh;
    float           dist;

    int             block;
    int             delta_x, delta_y, delta_z;
    int             base_x, base_y, base_z;
    int             n_x, n_y, n_z;

    float           x2[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           y2[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           z2[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           d2[NX * NY * NZ], p2[NX * NY * NZ];
    float           u2[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           v2[(NX + 1) * (NY + 1) * (NZ + 1)];
    float           w2[(NX + 1) * (NY + 1) * (NZ + 1)];
    int             matlist2[NX * NY * NZ];

    //
    // Create the mesh.
    //
    meshname = "mesh1";
    meshnamedup = "mesh1_dup";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;

    dtheta = -(180. / NX) * (3.1415926 / 180.);
    dh = 1;
    dr = 3. / NY;
    theta = 3.1415926536;

    for (i = 0; i < NX + 1; i++)
    {
        r = 2.;
        for (j = 0; j < NY + 1; j++)
        {
            h = 0.;
            for (k = 0; k < NZ + 1; k++)
            {
                x[k * (NX + 1) * (NY + 1) + ((j * (NX + 1)) + i)] = r * cos(theta);
                y[k * (NX + 1) * (NY + 1) + ((j * (NX + 1)) + i)] = r * sin(theta);
                z[k * (NX + 1) * (NY + 1) + ((j * (NX + 1)) + i)] = h;
                h += dh;
            }
            r += dr;
        }
        theta += dtheta;
    }

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var1namedup = "d_dup";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    zdims[0] = NX;
    zdims[1] = NY;
    zdims[2] = NZ;

    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            for (k = 0; k < NZ; k++)
            {

                xave = (x[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] +
                      x[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i + 1] +
                x[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i + 1] +
                x[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i]) / 4.;

                yave = (y[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] +
                      y[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i + 1] +
                y[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i + 1] +
                y[k * (NX + 1) * (NY + 1) + (j + 1) * (NX + 1) + i]) / 4.;

                dist = sqrt((xave - xcenter) * (xave - xcenter) +
                            (yave - ycenter) * (yave - ycenter));
                d[k * (NX) * (NY) + j * (NX) + i] = dist;
                p[k * (NX) * (NY) + j * (NX) + i] = 1. / (dist + .0001);
            }
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var3namedup = "u_dup";
    var4name = "v";
    var4namedup = "v_dup";
    var5name = "w";
    var5namedup = "w_dup";
    xcenter = 0.;
    ycenter = 0.;

    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter));
                u[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (x[i] - xcenter) / dist;
                v[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = (y[j] - ycenter) / dist;
                w[k * (NX + 1) * (NY + 1) + j * (NX + 1) + i] = 0.;
            }

        }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    matnamedup = "mat1_dup";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = NX;
    dims2[1] = NY;
    dims2[2] = NZ;

    mixlen = 0;

    //
    // Put in the material in 3 shells.
    //
    for (i = 0; i < NX; i++)
    {
        for (k = 0; k < NZ; k++)
        {
            for (j = 0; j < 10; j++)
            {
                matlist[k * NX * NY + j * NX + i] = 1;
            }
            for (j = 10; j < 20; j++)
            {
                matlist[k * NX * NY + j * NX + i] = 2;
            }
            for (j = 20; j < NY; j++)
            {
                matlist[k * NX * NY + j * NX + i] = 3;
            }
        }

    }

    //
    // Now extract the data for this block.
    //
    delta_x = NX / nblocks_x;
    delta_y = NY / nblocks_y;
    delta_z = NZ / nblocks_z;

    coords[0] = x2;
    coords[1] = y2;
    coords[2] = z2;
    dims[0] = delta_x + 1;
    dims[1] = delta_y + 1;
    dims[2] = delta_z + 1;
    zdims[0] = delta_x;
    zdims[1] = delta_y;
    zdims[2] = delta_z;
    dims2[0] = delta_x;
    dims2[1] = delta_y;
    dims2[2] = delta_z;

    //
    // Create the blocks for the multi-block object.
    //
    for (block = 0; block < nblocks_x * nblocks_y * nblocks_z; block++)
    {
        fprintf(stderr, "\t%s\n", dirnames[block]);

        //
        // Now extract the data for this block.
        //
        base_x = (block % nblocks_x) * delta_x;
        base_y = ((block % (nblocks_x * nblocks_y)) / nblocks_x) * delta_y;
        base_z = (block / (nblocks_x * nblocks_y)) * delta_z;

        for (k = 0, n_z = base_z; k < delta_z + 1; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y + 1; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x + 1; i++, n_x++)
                {
                    x2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        x[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    y2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        y[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    z2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        z[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    u2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        u[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    v2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        v[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                    w2[k * (delta_x + 1) * (delta_y + 1) + j * (delta_x + 1) + i] =
                        w[n_z * (NX + 1) * (NY + 1) + n_y * (NX + 1) + n_x];
                }

        for (k = 0, n_z = base_z; k < delta_z; k++, n_z++)
            for (j = 0, n_y = base_y; j < delta_y; j++, n_y++)
                for (i = 0, n_x = base_x; i < delta_x; i++, n_x++)
                {
                    d2[k * delta_x * delta_y + j * delta_x + i] =
                        d[n_z * NX * NY + n_y * NX + n_x];
                    p2[k * delta_x * delta_y + j * delta_x + i] =
                        p[n_z * NX * NY + n_y * NX + n_x];
                    matlist2[k * delta_x * delta_y + j * delta_x + i] =
                        matlist[n_z * NX * NY + n_y * NX + n_x];
                }

        if (DBSetDir(dbfile, dirnames[block]) == -1)
        {
            fprintf(stderr, "Could not set directory \"%s\"\n",
                    dirnames[block]);
            return;
        }

        //
        // Write out the variables.
        //
        cycle = 48;
        time = 4.8;
        dtime = 4.8;

        optlist = DBMakeOptlist(10);
        DBAddOption(optlist, DBOPT_CYCLE, &cycle);
        DBAddOption(optlist, DBOPT_TIME, &time);
        DBAddOption(optlist, DBOPT_DTIME, &dtime);
        char *xaxis="X Axis";
        char *yaxis="Y Axis";
        char *zaxis="Z Axis";
        DBAddOption(optlist, DBOPT_XLABEL, xaxis);
        DBAddOption(optlist, DBOPT_YLABEL, yaxis);
        DBAddOption(optlist, DBOPT_ZLABEL, zaxis);
        char *units="cm";
        DBAddOption(optlist, DBOPT_XUNITS, units);
        DBAddOption(optlist, DBOPT_YUNITS, units);
        DBAddOption(optlist, DBOPT_ZUNITS, units);

        put_extents(x2,dims[0]*dims[1]*dims[2],varextents[0],block);
        put_extents(y2,dims[0]*dims[1]*dims[2],varextents[1],block);
        put_extents(z2,dims[0]*dims[1]*dims[2],varextents[2],block);
        has_external_zones[block] = 1;
        zonecounts[block] = (dims[0]-1)*(dims[1]-1)*(dims[2]-1);
        DBPutQuadmesh(dbfile, meshname, coordnames, coords,
                      dims, ndims, DB_FLOAT, DB_NONCOLLINEAR,
                      optlist);
        if (!noDups && block < 2)
            DBPutQuadmesh(dbfile, meshnamedup, coordnames, coords,
                      dims, ndims, DB_FLOAT, DB_NONCOLLINEAR,
                      optlist);

        put_extents(d2,(dims[0]-1)*(dims[1]-1)*(dims[2]-1),varextents[3],block);
        DBPutQuadvar1(dbfile, var1name, meshname, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var1namedup, meshnamedup, d2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(p2,(dims[0]-1)*(dims[1]-1)*(dims[2]-1),varextents[4],block);
        DBPutQuadvar1(dbfile, var2name, meshname, p2, zdims, ndims,
                      NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

        put_extents(u2,dims[0]*dims[1]*dims[2],varextents[5],block);
        DBPutQuadvar1(dbfile, var3name, meshname, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var3namedup, meshnamedup, u2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(v2,dims[0]*dims[1]*dims[2],varextents[6],block);
        DBPutQuadvar1(dbfile, var4name, meshname, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var4namedup, meshnamedup, v2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        put_extents(w2,dims[0]*dims[1]*dims[2],varextents[7],block);
        DBPutQuadvar1(dbfile, var5name, meshname, w2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);
        if (!noDups && block < 2)
            DBPutQuadvar1(dbfile, var5namedup, meshnamedup, w2, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

        matcounts[block] = count_mats((dims[0]-1)*(dims[1]-1)*(dims[2]-1),matlist2,matlists[block]);
        mixlens[block] = mixlen;
        DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                      matlist2, dims2, ndims, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);
        if (!noDups && block < 2)
            DBPutMaterial(dbfile, matnamedup, meshnamedup, nmats, matnos,
                      matlist2, dims2, ndims, mix_next, mix_mat, mix_zone,
                      mix_vf, mixlen, DB_FLOAT, optlist);

        DBFreeOptlist(optlist);

        if (DBSetDir(dbfile, "..") == -1)
        {
            fprintf(stderr, "Could not return to base directory\n");
            return;
        }
    }
}

int
build_multi(DBfile *dbfile, int meshtype, int vartype, int dim, int nblocks_x,
            int nblocks_y, int nblocks_z, int coord_type, int iteration)
{
    int             i,j,k;
    int             cycle;
    float           time;
    double          dtime;
    int             nmatnos;
    int             matnos[3];
    char            names[MAXBLOCKS][STRLEN];
    char            namesdup[MAXBLOCKS][STRLEN];
    char           *meshnames[MAXBLOCKS];
    char           *meshnamesdup[MAXBLOCKS];
    int             meshtypes[MAXBLOCKS];
    char            names1[MAXBLOCKS][STRLEN];
    char            names2[MAXBLOCKS][STRLEN];
    char            names3[MAXBLOCKS][STRLEN];
    char            names4[MAXBLOCKS][STRLEN];
    char            names5[MAXBLOCKS][STRLEN];
    char            names6[MAXBLOCKS][STRLEN];
    char            names1dup[MAXBLOCKS][STRLEN];
    char            names3dup[MAXBLOCKS][STRLEN];
    char            names4dup[MAXBLOCKS][STRLEN];
    char            names5dup[MAXBLOCKS][STRLEN];
    char            names6dup[MAXBLOCKS][STRLEN];
    char           *var1names[MAXBLOCKS];
    char           *var2names[MAXBLOCKS];
    char           *var3names[MAXBLOCKS];
    char           *var4names[MAXBLOCKS];
    char           *var5names[MAXBLOCKS];
    char           *var6names[MAXBLOCKS];
    char           *var1namesdup[MAXBLOCKS];
    char           *var3namesdup[MAXBLOCKS];
    char           *var4namesdup[MAXBLOCKS];
    char           *var5namesdup[MAXBLOCKS];
    char           *var6namesdup[MAXBLOCKS];
    int             vartypes[MAXBLOCKS];
    char            names0[MAXBLOCKS][STRLEN];
    char            names0dup[MAXBLOCKS][STRLEN];
    char           *matnames[MAXBLOCKS];
    char           *matnamesdup[MAXBLOCKS];
    char            dirnames[MAXBLOCKS][STRLEN];

    DBoptlist      *optlist = NULL;
    int             one = 1;

    int             nblocks = nblocks_x * nblocks_y * nblocks_z;
    int             extentssize;
    int            *tmpList;
    double         *tmpExtents;

    //
    // Initialize the names and create the directories for the blocks.
    //
    for (i = 0; i < nblocks; i++)
    {
        if (iteration > 100)
            sprintf(names[i], "multi_ucd3d_ti_base:/block%d/mesh1", i);
        else
            sprintf(names[i], "/block%d/mesh1", i);
        meshnames[i] = names[i];
        meshtypes[i] = meshtype;

        sprintf(names1[i], "/block%d/d", i);
        sprintf(names2[i], "/block%d/p", i);
        sprintf(names3[i], "/block%d/u", i);
        sprintf(names4[i], "/block%d/v", i);
        sprintf(names5[i], "/block%d/w", i);
        sprintf(names6[i], "/block%d/hist", i);
        var1names[i] = names1[i];
        var2names[i] = names2[i];
        var3names[i] = names3[i];
        var4names[i] = names4[i];
        var5names[i] = names5[i];
        var6names[i] = names6[i];
        vartypes[i] = vartype;

        sprintf(names0[i], "/block%d/mat1", i);
        matnames[i] = names0[i];

        if (!noDups)
        {
            if (i < iteration)
            {
                sprintf(namesdup[i], "/block%d/mesh1_dup", i);
                meshnamesdup[i] = namesdup[i];

                sprintf(names1dup[i], "/block%d/d_dup", i);
                sprintf(names3dup[i], "/block%d/u_dup", i);
                sprintf(names4dup[i], "/block%d/v_dup", i);
                sprintf(names5dup[i], "/block%d/w_dup", i);
                sprintf(names6dup[i], "/block%d/hist_dup", i);
                var1namesdup[i] = names1dup[i];
                var3namesdup[i] = names3dup[i];
                var4namesdup[i] = names4dup[i];
                var5namesdup[i] = names5dup[i];
                var6namesdup[i] = names6dup[i];

                sprintf(names0dup[i], "/block%d/mat1_dup", i);
                matnamesdup[i] = names0dup[i];
            }
            else
            {
                sprintf(namesdup[i], "EMPTY");
                meshnamesdup[i] = namesdup[i];

                sprintf(names1dup[i], "EMPTY");
                sprintf(names3dup[i], "EMPTY");
                sprintf(names4dup[i], "EMPTY");
                sprintf(names5dup[i], "EMPTY");
                sprintf(names6dup[i], "EMPTY");
                var1namesdup[i] = names1dup[i];
                var3namesdup[i] = names3dup[i];
                var4namesdup[i] = names4dup[i];
                var5namesdup[i] = names5dup[i];
                var6namesdup[i] = names6dup[i];

                sprintf(names0dup[i], "EMPTY");
                matnamesdup[i] = names0dup[i];
            }
        }

        //
        // make the directory for the block mesh
        //
        sprintf(dirnames[i], "/block%d", i);

        if (DBMkDir(dbfile, dirnames[i]) == -1)
        {
            fprintf(stderr, "Could not make directory \"%s\"\n", dirnames[i]);
            return (-1);
        }
    }

    //
    // Write some misc. curve objects to a couple of the dirs
    //
    build_curves(dbfile, dirnames);

    //
    // create the blocks
    //
    switch (meshtype)
    {
    case DB_QUADMESH:
        if (coord_type == DB_COLLINEAR)
        {
            if (dim == 2)
                build_block_rect2d(dbfile, dirnames, nblocks_x, nblocks_y);
            else if (dim == 3)
                build_block_rect3d(dbfile, dirnames, nblocks_x, nblocks_y,
                                   nblocks_z);
        }
        else if (coord_type == DB_NONCOLLINEAR)
        {
            if (dim == 2)
                build_block_curv2d(dbfile, dirnames, nblocks_x, nblocks_y);
            else if (dim == 3)
                build_block_curv3d(dbfile, dirnames, nblocks_x, nblocks_y,
                                   nblocks_z);
        }
        break;

    case DB_UCDMESH:
        if (dim == 3)
            build_block_ucd3d(dbfile, dirnames, nblocks_x, nblocks_y,
                              nblocks_z, iteration);

        break;

    case DB_POINTMESH:
        if (dim == 2)
            build_block_point2d(dbfile, dirnames, nblocks_x, nblocks_y);

        break;

    default:
        fprintf(stderr, "Bad mesh type.\n");
        return (-1);
    }

    //
    // create the option lists for the multi-block calls.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;
    nmatnos = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;

    optlist = DBMakeOptlist(6);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    DBAddOption(optlist, DBOPT_NMATNOS, &nmatnos);
    DBAddOption(optlist, DBOPT_MATNOS, matnos);

    //
    // create the multi-block mesh
    //
    extentssize = 2 * dim;
    tmpExtents = (double *) malloc(nblocks * extentssize * sizeof(double));
    for (i = 0; i < nblocks; i++)
    {
       for (j = 0; j < dim; j++)
       {
          tmpExtents[i*extentssize+j] = varextents[j][2*i];
          tmpExtents[i*extentssize+j+dim] = varextents[j][2*i+1];
       }
    }
    SET_OPTIONS(extentssize,tmpExtents,zonecounts,0,0,0,has_external_zones);
    if (DBPutMultimesh(dbfile, "mesh1", nblocks,
                       meshnames, meshtypes, optlist) == -1)
    {
        DBFreeOptlist(optlist);
        free(tmpExtents);
        fprintf(stderr, "Error creating multi mesh\n");
        return (-1);
    }
    if (!noHalfMesh)
    {
        char tmpNames[MAXBLOCKS][STRLEN];
        char *tmpMeshnames[MAXBLOCKS];
        for (i = 0; i < nblocks; i++)
        {
            if (i < nblocks / 2)
                sprintf(tmpNames[i], "/block%d/mesh1_back", i);
            else
                sprintf(tmpNames[i], "/block%d/mesh1_front", i);
            tmpMeshnames[i] = tmpNames[i];
        }
        DBPutMultimesh(dbfile, "mesh1_back", nblocks / 2,
            tmpMeshnames, meshtypes, NULL);
        DBPutMultimesh(dbfile, "mesh1_front", nblocks / 2,
            &tmpMeshnames[nblocks/2], meshtypes, NULL);
    }

    //
    // put only the multimesh in for the first file of the time-invariant
    // mesh test dataset which starts at with an 'iteration' value of 100
    //
    if (iteration == 100)
    {
        DBFreeOptlist(optlist);
        free(tmpExtents);
        return 0;
    }

    // make a hidden mesh (one that shouldn't be displayed in the GUI
    DBAddOption(optlist, DBOPT_HIDE_FROM_GUI, &one) ;
    if (DBPutMultimesh(dbfile, "mesh1_hidden", nblocks,
                       meshnames, meshtypes, optlist) == -1)
    {
        fprintf(stderr, "Error creating hidden multi mesh\n");
        DBFreeOptlist(optlist);
        free(tmpExtents);
        return (-1);
    }

    free(tmpExtents);

    if (!noDups && DBPutMultimesh(dbfile, "mesh1_dup", nblocks,
                       meshnamesdup, meshtypes, NULL) == -1)
    {
        fprintf(stderr, "Error creating multi mesh\n");
        return (-1);
    }

    // create the multi-block variables
    extentssize = 2;
    SET_OPTIONS(extentssize,varextents[3],0,0,0,0,0);
    if (DBPutMultivar(dbfile, "d", nblocks, var1names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var d\n");
        return (-1);
    }
    if (!noDups && DBPutMultivar(dbfile, "d_dup", nblocks, var1namesdup, vartypes, NULL)
        == -1)
    {
        fprintf(stderr, "Error creating multi var d\n");
        return (-1);
    }
    if (!noHalfMesh)
    {
        srand(0xDeadBeef);
        char tmpNames[MAXBLOCKS][STRLEN];
        char *tmpVarnames[MAXBLOCKS];
        for (i = 0; i < nblocks; i++)
        {
            if (i < nblocks / 2)
                sprintf(tmpNames[i], "/block%d/d_back", i);
            else
                sprintf(tmpNames[i], "/block%d/d_front", i);
            if (i == 0 || (rand() % 7) == 0)
                sprintf(tmpNames[i], "EMPTY", i);
            tmpVarnames[i] = tmpNames[i];
        }
        DBPutMultivar(dbfile, "d_split", nblocks, tmpVarnames, vartypes, NULL);
    }

    SET_OPTIONS(extentssize,varextents[4],0,0,0,0,0);
    if (DBPutMultivar(dbfile, "p", nblocks, var2names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var p\n");
        return (-1);
    }
    SET_OPTIONS(extentssize,varextents[5],0,0,0,0,0);
    if (DBPutMultivar(dbfile, "u", nblocks, var3names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var u\n");
        return (-1);
    }
    if (!noDups && DBPutMultivar(dbfile, "u_dup", nblocks, var3namesdup, vartypes, NULL)
        == -1)
    {
        fprintf(stderr, "Error creating multi var u\n");
        return (-1);
    }
    SET_OPTIONS(extentssize,varextents[6],0,0,0,0,0);
    if (DBPutMultivar(dbfile, "v", nblocks, var4names, vartypes, optlist)
        == -1)
    {
        DBFreeOptlist(optlist);
        fprintf(stderr, "Error creating multi var v\n");
        return (-1);
    }
    if (!noDups && DBPutMultivar(dbfile, "v_dup", nblocks, var4namesdup, vartypes, NULL)
        == -1)
    {
        fprintf(stderr, "Error creating multi var v\n");
        return (-1);
    }

    // create a hidden variable
    DBAddOption(optlist, DBOPT_HIDE_FROM_GUI, &one) ;
    if (!noDups && DBPutMultivar(dbfile, "v_dup_hidden", nblocks, var4namesdup,
        vartypes, optlist) == -1)
    {
        fprintf(stderr, "Error creating multi var v\n");
        return (-1);
    }

    if (dim == 3)
    {
        SET_OPTIONS(extentssize,varextents[7],0,0,0,0,0);
        if (DBPutMultivar(dbfile, "w", nblocks, var5names, vartypes, optlist)
            == -1)
        {
            DBFreeOptlist(optlist);
            fprintf(stderr, "Error creating multi var w\n");
            return (-1);
        }
        if (!noDups && DBPutMultivar(dbfile, "w_dup", nblocks, var5namesdup, vartypes, NULL)
            == -1)
        {
            fprintf(stderr, "Error creating multi var w\n");
            return (-1);
        }
    }
    if ((meshtype == DB_UCDMESH) && (dim == 3))
    {
        SET_OPTIONS(extentssize,varextents[8],0,0,0,0,0);
        if (DBPutMultivar(dbfile, "hist", nblocks, var6names, vartypes, optlist)
            == -1)
        {
            DBFreeOptlist(optlist);
            fprintf(stderr, "Error creating multi var w\n");
            return (-1);
        }
        if (!noDups && DBPutMultivar(dbfile, "hist_dup", nblocks, var6namesdup, vartypes, NULL)
            == -1)
        {
            fprintf(stderr, "Error creating multi var w\n");
            return (-1);
        }
    }
    // create the multi-block material
    k = 0;
    for (i = 0; i < nblocks; i++)
       k += matcounts[i];
    tmpList = (int *) malloc(k * sizeof(int));
    k = 0;
    for (i = 0; i < nblocks; i++)
       for (j = 0; j < matcounts[i]; j++)
          tmpList[k++] = matlists[i][j];
    extentssize = 0;
    SET_OPTIONS(extentssize,0,0,mixlens,matcounts,tmpList,0);
    if (meshtype != DB_POINTMESH)
    {
        if (DBPutMultimat(dbfile, "mat1", nblocks, matnames, optlist) == -1)
        {
            DBFreeOptlist(optlist);
            fprintf(stderr, "Error creating multi material\n");
            return (-1);
        }
        if (dupMultiMatToo && !noDups &&
            DBPutMultimat(dbfile, "mat1_dup", nblocks, matnamesdup, NULL) == -1)
        {
            fprintf(stderr, "Error creating multi material\n");
            return (-1);
        }
    }
    free(tmpList);
    DBFreeOptlist(optlist);

#ifdef DB_VARTYPE_SCALAR
    char   vnames[4][STRLEN];
    char   defns[4][STRLEN];
    const char *pvnames[4] = {vnames[0], vnames[1], vnames[2], vnames[3]};
    const char *pdefns[4] = {defns[0], defns[1], defns[2], defns[3]};
    int    types[4];

    types[0] = DB_VARTYPE_SCALAR;
    sprintf(vnames[0], "sum");
    if (dim == 2)
        sprintf(defns[0], "u+v");
    else
        sprintf(defns[0], "u+v+w");

    types[1] = DB_VARTYPE_VECTOR;
    sprintf(vnames[1], "vec");
    if (dim == 2)
        sprintf(defns[1], "{u,v}");
    else
        sprintf(defns[1], "{u,v,w}");

    types[2] = DB_VARTYPE_SCALAR;
    sprintf(vnames[2], "nmats");
    sprintf(defns[2], "nmats(mat1)");

    types[3] = DB_VARTYPE_SCALAR;
    sprintf(vnames[3], "mag");
    sprintf(defns[3], "magnitude(vec)");

    // Create a list of optlist objects
    DBoptlist *optlists[4];
    optlists[0] = (DBoptlist*) 0;
    optlists[1] = (DBoptlist*) 0;
    optlists[2] = (DBoptlist*) 0;
    optlists[3] = DBMakeOptlist(5);

    // declare the "magnitude(vec)" expression hidden
    DBAddOption(optlists[3], DBOPT_HIDE_FROM_GUI, &one) ;

    DBPutDefvars(dbfile, "defvars", 4, pvnames, types, pdefns, optlists);
    DBFreeOptlist(optlists[3]);
#endif

    return (0);
}


int
main(int argc, char **argv)
{
    DBfile         *dbfile;
    int             driver = DB_PDB;
    int             iter;

    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-nodups") == 0)
        {
            noDups = 1;
        }
        else if (strcmp(argv[i], "-driver") == 0)
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
        else if (strcmp(argv[i], "-dupMultiMatToo") == 0)
        {
            dupMultiMatToo = 1;
        }
        else if (strcmp(argv[i], "-userDefZonelist") == 0)
        {
            userDefZonelist = 1;
        }
        else if (strcmp(argv[i], "-noCycles") == 0)
        {
            noCycles = 1;
        }
        else if (strcmp(argv[i], "-noTimeInvariantMultimesh") == 0)
        {
            noTimeInvariantMultimesh = 1;
        }
        else if (strcmp(argv[i], "-noHalfMesh") == 0)
        {
            noHalfMesh = 1;
        }
        else
        {
            fprintf(stderr, "unrecognized argument \"%s\"\n", argv[i]);
            exit(-1);
        }

        i++;
    }

    //
    // Create the multi-block rectilinear 2d mesh.
    //
    fprintf(stderr, "creating multi_rect2d.silo\n");
    if ((dbfile = DBCreate("multi_rect2d.silo", DB_CLOBBER, DB_LOCAL,
                           "multi-block rectilinear 2d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create 'multi_rect2d.silo'.\n");
    }
    else if (build_multi(dbfile, DB_QUADMESH, DB_QUADVAR, 2, 3, 4, 1,
                         DB_COLLINEAR, 2) == -1)
    {
        fprintf(stderr, "Error in creating 'multi_rect2d.silo'.\n");
        DBClose(dbfile);
    }
    else
        DBClose(dbfile);

    //
    // Create the multi-block curvilinear 2d mesh.
    //
    fprintf(stderr, "creating multi_curv2d.silo\n");
    if ((dbfile = DBCreate("multi_curv2d.silo", DB_CLOBBER, DB_LOCAL,
                           "multi-block curvilinear 2d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create 'multi_curv2d.silo'.\n");
    }
    else if (build_multi(dbfile, DB_QUADMESH, DB_QUADVAR, 2, 5, 1, 1,
                         DB_NONCOLLINEAR, 2) == -1)
    {
        fprintf(stderr, "Error in creating 'multi_curv2d.silo'.\n");
        DBClose(dbfile);
    }
    else
        DBClose(dbfile);

    //
    // Create the multi-block point 2d mesh.
    //
    fprintf(stderr, "creating multi_point2d.silo\n");
    if ((dbfile = DBCreate("multi_point2d.silo", DB_CLOBBER, DB_LOCAL,
                           "multi-block point 2d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create 'multi_point2d.silo'.\n");
    }
    else if (build_multi(dbfile, DB_POINTMESH, DB_POINTVAR, 2, 5, 1, 1,
                         0, 2) == -1)
    {
        fprintf(stderr, "Error in creating 'multi_point2d.silo'.\n");
        DBClose(dbfile);
    }
    else
        DBClose(dbfile);

    //
    // Create the multi-block rectilinear 3d mesh.
    //
    fprintf(stderr, "creating multi_rect3d.silo\n");
    if ((dbfile = DBCreate("multi_rect3d.silo", DB_CLOBBER, DB_LOCAL,
                           "multi-block rectilinear 3d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create 'multi_rect3d.silo'.\n");
    }
    else if (build_multi(dbfile, DB_QUADMESH, DB_QUADVAR, 3, 3, 4, 3,
                         DB_COLLINEAR, 2) == -1)
    {
        fprintf(stderr, "Error in creating 'multi_rect3d.silo'.\n");
        DBClose(dbfile);
    }
    else
        DBClose(dbfile);

    //
    // Create the multi-block curvilinear 3d mesh.
    //
    fprintf(stderr, "creating multi_curv3d.silo\n");
    if ((dbfile = DBCreate("multi_curv3d.silo", DB_CLOBBER, DB_LOCAL,
                           "multi-block curvilinear 3d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create 'multi_curv3d.silo'.\n");
    }
    else if (build_multi(dbfile, DB_QUADMESH, DB_QUADVAR, 3, 3, 4, 3,
                         DB_NONCOLLINEAR, 2) == -1)
    {
        fprintf(stderr, "Error in creating 'multi_curv3d.silo'.\n");
        DBClose(dbfile);
    }
    else
        DBClose(dbfile);

    //
    // Create the multi-block ucd 3d mesh.
    //
    fprintf(stderr, "creating multi_ucd3d.silo\n");
    if ((dbfile = DBCreate("multi_ucd3d.silo", DB_CLOBBER, DB_LOCAL,
                           "multi-block ucd 3d test file", driver))
        == NULL)
    {
        fprintf(stderr, "Could not create 'multi_ucd3d.silo'.\n");
    }
    else if (build_multi(dbfile, DB_UCDMESH, DB_UCDVAR, 3, 3, 4, 3, 0, 2) == -1)
    {
        fprintf(stderr, "Error in creating 'multi_ucd3d.silo'.\n");
        DBClose(dbfile);
    }
    else
        DBClose(dbfile);


    //
    // Create time series of multi-block ucd 3d meshes with
    // variables whose SIL is changing with time
    //
    for (iter = 0; iter < 10 && !noCycles; iter++)
    {
        char tmpName[256];
        sprintf(tmpName, "hist_ucd3d_%04d", iter);
        fprintf(stderr, "creating %s\n", tmpName);

        if ((dbfile = DBCreate(tmpName, DB_CLOBBER, DB_LOCAL,
                               "multi-block ucd 3d test file", driver))
            == NULL)
        {
            fprintf(stderr, "Could not create '%s'.\n", tmpName);
        }
        else if (build_multi(dbfile, DB_UCDMESH, DB_UCDVAR, 3, 3, 4, 3, 0, 4*iter) == -1)
        {
            fprintf(stderr, "Error in creating '%s'.\n", tmpName);
            DBClose(dbfile);
        }
        else
        {
            DBClose(dbfile);
        }
    }

    //
    // don't do duplicate meshes for this test
    //
    noDups = 1;
    for (iter = 0; iter < 4 && !noTimeInvariantMultimesh; iter++)
    {
        char tmpName[256];
        char tmpStr[1024];
        if (iter == 0)
            sprintf(tmpName, "multi_ucd3d_ti_base");
        else
            sprintf(tmpName, "multi_ucd3d_ti_%04d", iter);
        fprintf(stderr, "creating %s\n", tmpName);

        if ((dbfile = DBCreate(tmpName, DB_CLOBBER, DB_LOCAL,
                               "multi-block ucd 3d test file with time-invariant mesh", driver))
            == NULL)
        {
            fprintf(stderr, "Could not create '%s'.\n", tmpName);
        }
        else if (build_multi(dbfile, DB_UCDMESH, DB_UCDVAR, 3, 3, 4, 3, 0, 100+iter) == -1)
        {
            fprintf(stderr, "Error in creating '%s'.\n", tmpName);
            DBClose(dbfile);
        }
        else
        {
            //
            // Add multivar to multimesh map information
            //
            if (iter > 0)
            {
                DBSetDir(dbfile,"/");
                int len;
                sprintf(tmpStr, "d;p;u;v;w;hist;mat1");
                len = strlen(tmpStr);
                DBWrite(dbfile, "MultivarToMultimeshMap_vars", tmpStr, &len, 1, DB_CHAR);
                sprintf(tmpStr, "mesh1;mesh1;mesh1;mesh1;mesh1;mesh1;mesh1");
                len = strlen(tmpStr);
                DBWrite(dbfile, "MultivarToMultimeshMap_meshes", tmpStr, &len, 1, DB_CHAR);
            }

            DBClose(dbfile);
        }
    }

    return (0);
}
