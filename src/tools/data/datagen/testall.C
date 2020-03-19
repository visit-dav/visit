// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <silo.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// suppress the following since silo uses char * in its API
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wwritable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#define false   0
#define true    1

#define ALLOC_N(T,N)    ((T*)calloc((N),sizeof(T)))
#define FREE(M)         if(M){free(M);(M)=NULL;}

#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)<(B))?(B):(A))

// Local function prototypes
static void     fill_rect2d_bkgr(int matlist[], int nx, int ny, int matno);
static void     fill_rect2d_mat(float x[], float y[], int matlist[], int nx,
                                int ny, int mix_next[], int mix_mat[],
                                int mix_zone[], float mix_vf[], int *mixlen,
                                int matno, double radius);
static void     fill_rect3d_bkgr(int matlist[], int nx, int ny, int nz,
                                 int matno);
static void     fill_rect3d_mat(float x[], float y[], float z[], int matlist[],
                                int nx, int ny, int nz, int mix_next[],
                                int mix_mat[], int mix_zone[], float mix_vf[],
                                int *mixlen, int matno, double radius);
static void     build_rect2d(DBfile * dbfile, int size);
static void     build_curv2d(DBfile * dbfile, int size, int major_order);
static void     build_ucd2d(DBfile * dbfile, int size);
static void     build_ucd2d_lines(DBfile * dbfile, int size);
static void     build_rect3d(DBfile * dbfile, int size);
static void     build_curv3d(DBfile * dbfile, int size, int major_order);
static void     build_ucd3d(DBfile * dbfile, int size);
static void     build_ucd3d_lines(DBfile * dbfile, int size);
static void     build_poly3d(DBfile * dbfile, int size);
static void     MakeFiles(int size, int type);

static void
fill_rect2d_bkgr(int matlist[], int nx, int ny, int matno)
{
    int       i, j;

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            matlist[j * nx + i] = matno;
        }
    }
}

static void
fill_rect2d_mat(float x[], float y[], int matlist[], int nx, int ny, int
                mix_next[], int mix_mat[], int mix_zone[], float mix_vf[], int
                *mixlen, int matno, double radius)
{
    int       i, j, k, l;
    double    xcenter, ycenter;
    double    dist;
    int       cnt;
    int       mixlen2;
    int       *itemp;
    double    dx, dy;
    double    xx[20], yy[20];

    mixlen2 = *mixlen;

    itemp = ALLOC_N (int, (nx + 1) * (ny + 1));

    xcenter = .5;
    ycenter = .5;
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < ny + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            itemp[j * (nx + 1) + i] = (dist < radius) ? 1 : 0;
        }
    }
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            cnt = itemp[(j) * (nx + 1) + (i)] +
                itemp[(j + 1) * (nx + 1) + (i)] +
                itemp[(j + 1) * (nx + 1) + (i + 1)] +
                itemp[(j) * (nx + 1) + (i + 1)];
            if (cnt == 0)
            {
                /* EMPTY */
            }
            else if (cnt == 4)
            { 
                matlist[j * nx + i] = matno;
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
                        cnt += (dist < radius) ? 1 : 0;
                    }
                }
                mix_mat[mixlen2] = matlist[j * nx + i];
                mix_mat[mixlen2 + 1] = matno;
                mix_next[mixlen2] = mixlen2 + 2;
                mix_next[mixlen2 + 1] = 0;
                mix_zone[mixlen2] = j * nx + i;
                mix_zone[mixlen2 + 1] = j * nx + i;
                mix_vf[mixlen2] = 1. - (((double)cnt) / 400.);
                mix_vf[mixlen2 + 1] = ((double)cnt) / 400.;
                matlist[j * nx + i] = - (mixlen2 + 1);
                mixlen2 += 2;
            }
        }
    }

    FREE (itemp);

    *mixlen = mixlen2;
}

static void
fill_rect3d_bkgr(int matlist[], int nx, int ny, int nz, int matno)
{
    int       i, j, k;

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
                float mix_vf[], int *mixlen, int matno, double radius)
{
    int       i, j, k, l, m, n;
    double    xcenter, ycenter, zcenter;
    double    dist;
    int       cnt;
    int       mixlen2;
    int       *itemp;
    double    dx, dy, dz;
    double    xx[10], yy[10], zz[10];

    mixlen2 = *mixlen;

    itemp = ALLOC_N (int, (nx + 1) * (ny + 1) * (nz + 1));

    xcenter = .5;
    ycenter = .5;
    zcenter = .5;
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < ny + 1; j++)
        {
            for (k = 0; k < nz + 1; k++)
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
                    mix_mat[mixlen2] = matlist[i * nx * ny + j * nx + k];
                    mix_mat[mixlen2 + 1] = matno;
                    mix_next[mixlen2] = mixlen2 + 2;
                    mix_next[mixlen2 + 1] = 0;
                    mix_zone[mixlen2] = i * nx * ny + j * nx + k;
                    mix_zone[mixlen2 + 1] = i * nx * ny + j * nx + k;
                    mix_vf[mixlen2] = 1. - (((double)cnt) / 1000.);
                    mix_vf[mixlen2 + 1] = ((double)cnt) / 1000.;
                    matlist[i * nx * ny + j * nx + k] = -(mixlen2 + 1);
                    mixlen2 += 2;
                }
            }
        }
    }

    FREE (itemp);

    *mixlen = mixlen2;
}

static DBoptlist *
rect2d_var_optlist(const char *units, int *cycle, float *time,
    double *dtime)
{
    DBoptlist *optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, cycle);
    DBAddOption(optlist, DBOPT_TIME, time);
    DBAddOption(optlist, DBOPT_DTIME, dtime);
    DBAddOption(optlist, DBOPT_UNITS, (void *)units);
    return optlist;
}

// ****************************************************************************
//   Modifications:
//     Brad Whitlock, Wed Jul 21 14:20:06 PST 2004
//     Added variable units.
//
// ****************************************************************************

static void
build_rect2d(DBfile * dbfile, int size)
{
    int            cycle;
    float          time;
    double         dtime;
    int            ndims;
    int            nx=0, ny=0;
    int            dims[3], zdims[3];
    float         *coords[3];
    float         *x = NULL, *y = NULL;

    char          *meshname = NULL, *var1name = NULL, *var2name = NULL;
    char          *var3name = NULL, *var4name = NULL, *matname = NULL;

    float         *d=NULL, *p=NULL, *u=NULL, *v=NULL, *t=NULL, *ascii=NULL, *distarr=NULL;

    char          *asciiw=NULL;

    int            nmats;
    int            matnos[9];
    int           *matlist = NULL;
    int            dims2[3];
    int            mixlen;
    int           *mix_next = NULL, *mix_mat = NULL, *mix_zone = NULL;
    float         *mix_vf = NULL;

    DBoptlist     *optlist = NULL, *varOptList = NULL;

    int            i, j;
    double         xave, yave;
    double         xcenter, ycenter;
    double         dist;

    switch (size)
    {
        case 1:
            nx = 30;
            ny = 40;
            break;
        case 2:
            nx = 300;
            ny = 400;
            break;
        case 3:
            nx = 1000;
            ny = 1200;
            break;
    }

    x = ALLOC_N (float, nx + 1);
    y = ALLOC_N (float, ny + 1);
    d = ALLOC_N (float, nx * ny);
    p = ALLOC_N (float, nx * ny);
    u = ALLOC_N (float, (nx + 1) * (ny + 1));
    v = ALLOC_N (float, (nx + 1) * (ny + 1));
    t = ALLOC_N (float, (nx + 1) * (ny + 1));
    distarr = ALLOC_N (float, nx * ny);
    ascii = ALLOC_N (float, nx * ny);
    asciiw = ALLOC_N (char, nx * ny * 9);
    matlist = ALLOC_N (int, nx * ny);
    mix_next = ALLOC_N (int, 40 * ny);
    mix_mat  = ALLOC_N (int, 40 * ny);
    mix_zone = ALLOC_N (int, 40 * ny);
    mix_vf   = ALLOC_N (float, 40 * ny);

    //
    // Create the mesh.
    //
    meshname = "quadmesh2d";
    coords[0] = x;
    coords[1] = y;
    ndims = 2;
    dims[0] = nx + 1;
    dims[1] = ny + 1;
    for (i = 0; i < nx + 1; i++)
        x[i] = i * (1. / nx);
    for (i = 0; i < ny + 1; i++)
        y[i] = i * (1. / nx);

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var2name = "p";
    xcenter = .5;
    ycenter = .5;
    zdims[0] = nx;
    zdims[1] = ny;
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            xave = (x[i]+x[i+1]) / 2.0 ;
            yave = (y[j]+y[j+1]) / 2.0 ;

            dist = sqrt((xave - xcenter) * (xave - xcenter) +
                        (yave - ycenter) * (yave - ycenter));

            d[j * nx + i] = dist;
            p[j * nx + i] = 1 / (dist + 0.0001);
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var4name = "v";
    xcenter = 0.5;
    ycenter = 0.5;
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < ny + 1; j++)
        {
            dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                        (y[j] - ycenter) * (y[j] - ycenter));
            if (dist < 0.5/nx || dist < 0.5/ny)
            {
                u[j * (nx + 1) + i] = 1.0;
                v[j * (nx + 1) + i] = 0.0;
            }
            else
            {
                u[j * (nx + 1) + i] = (x[i] - xcenter) / dist;
                v[j * (nx + 1) + i] = (y[j] - ycenter) / dist;
            }
        }
    }

    //
    // Create the temperature array.
    //
    if (t != NULL)
    {
        double a,b,c,d;

        a = 1.;
        b = 3.;
        c = 5.;
        d = 7.;
        for (i = 0; i <= nx; ++i)
        {
            double x;

            x = i / (double) nx;
            for (j = 0; j <= ny; ++j)
            {
                double y;

                y = j / (double) ny;
                t[j*(nx+1) + i] = a + b*x + c*exp(d*y);
            }
        }
    }

    //
    // Create the ascii label array.  The center of the mesh is labeled
    // `A' and the furthest from the center is labeled `Z'.
    //
    if (ascii)
    {
       double maxdist = 0.0;
       xcenter = .5;
       ycenter = .5;
       for (i = 0; i < nx; i++)
       {
          for (j = 0; j < ny; j++)
          {
            distarr[j*nx+i] = sqrt ((x[i] - xcenter) * (x[i] - xcenter) +
                                    (y[j] - ycenter) * (y[j] - ycenter));
            if (distarr[j*nx+i]>maxdist) maxdist = distarr[j*nx+i];
          }
       }

#define PUT_LABEL(S)              \
{   char const *tmp = #S;         \
    for (int q = 0; q < 9; q++)   \
        asciiw[q*nx*ny+i] = '\0'; \
    for (int q = 0; q < 9 && q < (int) strlen(#S); q++)   \
        asciiw[q*nx*ny+i] = tmp[q]; \
    break;                        \
}

       for (i = 0; i < nx*ny; i++)
       {
          ascii[i] = 'A' + 26*distarr[i]/maxdist;
          switch ((int) ascii[i])
          {
              case 'A': case 'B':
              case 'C': case 'D':
                  PUT_LABEL(Leonard);
              case 'E': case 'F':
              case 'G': case 'H':
                  PUT_LABEL(Sheldon);
              case 'I': case 'J':
              case 'K': case 'L':
                  PUT_LABEL(Penny);
              case 'M': case 'N':
              case 'O': case 'P':
                  PUT_LABEL(Amy Farrah Fowler);
              case 'Q': case 'R':
              case 'S': case 'T':
                  PUT_LABEL(Bernadet);
              case 'U': case 'V':
              case 'W': case 'X':
                  PUT_LABEL(Kruthrapali);
              default: 
                  PUT_LABEL(Wolowitz);
           }
        }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    nmats = 9;
    matnos[0] = 1;
    matnos[1] = 3;
    matnos[2] = 6;
    matnos[3] = 7;
    matnos[4] = 8;
    matnos[5] = 11;
    matnos[6] = 12;
    matnos[7] = 16;
    matnos[8] = 18;
    dims2[0] = nx;
    dims2[1] = ny;
    mixlen = 0;

    //
    // Put in material 1 as the background then overlay materials
    // 2 through 9.
    //
    fill_rect2d_bkgr (matlist, nx, ny, 1);

    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 3, 0.8);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 6, 0.7);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 7, 0.6);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 8, 0.5);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 11, 0.4);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 12, 0.3);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 16, 0.2);
    fill_rect2d_mat (x, y, matlist, nx, ny, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 18, 0.1);

    if (mixlen > 40 * ny)
    {
        printf ("mixlen = %d\n", mixlen);
        exit (1);
    }

    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    char *info = "mesh quadmesh2d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite (dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v};speed scalar sqrt(u*u+v*v)";
    i = strlen(defvars) + 1;
    DBWrite (dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

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

    DBPutQuadmesh(dbfile, meshname, NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, optlist);

    varOptList = rect2d_var_optlist("g/cm^2", &cycle, &time, &dtime);
    DBPutQuadvar1(dbfile, var1name, meshname, d, zdims, ndims, NULL, 0,
                  DB_FLOAT, DB_ZONECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = rect2d_var_optlist("bars", &cycle, &time, &dtime);
    DBPutQuadvar1(dbfile, var2name, meshname, p, zdims, ndims, NULL, 0,
                  DB_FLOAT, DB_ZONECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = rect2d_var_optlist("cm", &cycle, &time, &dtime);
    DBPutQuadvar1(dbfile, var3name, meshname, u, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = rect2d_var_optlist("cm", &cycle, &time, &dtime);
    DBPutQuadvar1(dbfile, var4name, meshname, v, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = rect2d_var_optlist("Celsius", &cycle, &time, &dtime);
    DBPutQuadvar1(dbfile, "t", meshname, t, dims, ndims, NULL, 0, 
                  DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    DBPutMaterial(dbfile, matname, meshname, nmats, matnos, matlist, dims2,
                  ndims, mix_next, mix_mat, mix_zone, mix_vf, mixlen,
                  DB_FLOAT, optlist);

    // Create a symmetric tensor variable
    {
        void *arr[] = {u,v,t};
        char *arrnames[] = {"V0","V1","V2"};
        DBPutQuadvar(dbfile, "tensor", meshname, 3, arrnames, arr, dims, ndims, NULL, 0,
            DB_FLOAT, DB_NODECENT, NULL);
    }

    if (ascii)
    {
       j = true;
       void *arr[9];
       char *arrnames[] = {"L0","L1","L2","L3","L4","L5","L6","L7","L8"};

       varOptList = rect2d_var_optlist("Labels", &cycle, &time, &dtime);
       DBAddOption (varOptList, DBOPT_ASCII_LABEL, &j);

       DBPutQuadvar1(dbfile, "ascii", meshname, ascii, zdims, ndims, NULL, 0,
                     DB_FLOAT, DB_ZONECENT, varOptList);

       arr[0] = &asciiw[0*nx*ny];
       arr[1] = &asciiw[1*nx*ny];
       arr[2] = &asciiw[2*nx*ny];
       arr[3] = &asciiw[3*nx*ny];
       arr[4] = &asciiw[4*nx*ny];
       arr[5] = &asciiw[5*nx*ny];
       arr[6] = &asciiw[6*nx*ny];
       arr[7] = &asciiw[7*nx*ny];
       arr[8] = &asciiw[8*nx*ny];
       DBPutQuadvar(dbfile, "asciiw", meshname, 9, arrnames, arr, zdims, ndims, NULL, 0,
                     DB_CHAR, DB_ZONECENT, varOptList);

       DBFreeOptlist(varOptList);
    }

    DBFreeOptlist(optlist);

    //
    // Free the temporary storage.
    //
    FREE (x);
    FREE (y);
    FREE (d);
    FREE (p);
    FREE (u);
    FREE (v);
    FREE (t);
    FREE (distarr);
    FREE (ascii);
    FREE (asciiw);
    FREE (matlist);
    FREE (mix_next);
    FREE (mix_mat);
    FREE (mix_zone);
    FREE (mix_vf);
}

static void
build_curv2d(DBfile * dbfile, int size, int major_order)
{
    int            cycle;
    float          time;
    double         dtime;
    int            ndims;
    int            nx=0, ny=0;
    int            sn1, sn2, sz1, sz2;
    int            dims[3], zdims[3];
    int            lo_off[3], hi_off[3];
    float         *coords[3];
    float         *x, *y;

    char          *meshname, *var1name, *var2name, *var3name;
    char          *var4name, *matname;
    double        *d, *p;
    float         *u, *v;

    int            nmats;
    int            matnos[3];
    int           *matlist;
    int            dims2[3];
    int            mixlen;
    int           *mix_next, *mix_mat, *mix_zone;
    float         *mix_vf;

    DBoptlist     *optlist;

    int            i, j;
    double         xave, yave;
    double         xcenter, ycenter;
    double         theta, dtheta;
    double         r, dr;
    double         dist;
    float         *xx, *yy;

    switch (size)
    {
        case 1:
            nx = 30;
            ny = 40;
            break;
        case 2:
            nx = 300;
            ny = 400;
            break;
        case 3:
            nx = 1000;
            ny = 1200;
            break;
    }

    if (major_order == DB_ROWMAJOR)
    {
        sz1 = 1;
        sz2 = nx;
        sn1 = 1;
        sn2 = nx + 1;
    }
    else
    {
        sz1 = ny;
        sz2 = 1;
        sn1 = ny + 1;
        sn2 = 1;
    }

    x = ALLOC_N (float, (nx + 1) * (ny + 1));
    y = ALLOC_N (float, (nx + 1) * (ny + 1));
    d = ALLOC_N (double, nx * ny);
    p = ALLOC_N (double, nx * ny);
    u = ALLOC_N (float, (nx + 1) * (ny + 1));
    v = ALLOC_N (float, (nx + 1) * (ny + 1));
    xx = ALLOC_N (float, (nx + 1) * 2);
    yy = ALLOC_N (float, (nx + 1) * 2);
    matlist = ALLOC_N (int, nx * ny);
    mix_next = ALLOC_N (int, 2 * nx);
    mix_mat  = ALLOC_N (int, 2 * nx);
    mix_zone = ALLOC_N (int, 2 * nx);
    mix_vf   = ALLOC_N (float, 2 * nx);

    //
    // Create the mesh.
    //
    meshname = "curvmesh2d";
    coords[0] = x;
    coords[1] = y;
    ndims = 2;
    dims[0] = nx + 1;
    dims[1] = ny + 1;
    dtheta = (180. / nx) * (3.1415926 / 180.);
    dr = 3. / ny;
    theta = 0;
    for (i = 0; i < nx + 1; i++)
    {
        r = 2.;
        for (j = 0; j < ny + 1; j++)
        {
            x[j * sn2 + i * sn1] = r * cos(theta);
            y[j * sn2 + i * sn1] = r * sin(theta);
            r += dr;
        }
        theta += dtheta;
    }

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    zdims[0] = nx;
    zdims[1] = ny;
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            xave = (x[j * sn2 + i * sn1] +
                    x[j * sn2 + (i + 1) * sn1] +
                    x[(j + 1) * sn2 + (i + 1) * sn1] +
                    x[(j + 1) * sn2 + i * sn1]) / 4.;
            yave = (y[j * sn2 + i * sn1] +
                    y[j * sn2 + (i + 1) * sn1] +
                    y[(j + 1) * sn2 + (i + 1) * sn1] +
                    y[(j + 1) * sn2 + i * sn1]) / 4.;
            dist = sqrt((xave - xcenter) * (xave - xcenter) +
                        (yave - ycenter) * (yave - ycenter));
            d[j * sz2 + i * sz1] = dist;
            p[j * sz2 + i * sz1] = 1. / (dist + .0001);
        }
    }

    //
    // Create the velocity component arrays. The values from the
    // first 2 rows of the x and y arrays are first copied into the
    // arrays xx and yy which are stored in the fortran order.  The
    // indexing on the xx and yy arrays is then done for rectilinear
    // meshes. This generates an interesting vector field.
    //
    var3name = "u";
    var4name = "v";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < 2; j++)
        {
            xx [j * (nx + 1) + i] = x [j * sn2 + i * sn1];
            yy [j * (nx + 1) + i] = y [j * sn2 + i * sn1];
        }
    }
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < ny + 1; j++)
        {
            dist = sqrt((xx[i] - xcenter) * (xx[i] - xcenter) +
                        (yy[j] - ycenter) * (yy[j] - ycenter));
            u[j * sn2 + i * sn1] = (xx[i] - xcenter) / dist;
            v[j * sn2 + i * sn1] = (yy[j] - ycenter) / dist;
        }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = nx;
    dims2[1] = ny;
    mixlen = 0;

    //
    // Put in the material in 3 shells.
    //
    for (i = 0; i < nx; i++)
    {
        //
        // Add 9 layers of material 1.
        //
        for (j = 0; j < 9; j++)
        {
            matlist[j * sz2 + i * sz1] = 1;
        }

        //
        // Add a layer of mixed material zones.
        //
        j = 9;
        matlist[j * sz2 + i * sz1] = - mixlen - 1;
        mix_next [mixlen]     = mixlen + 2;
        mix_next [mixlen + 1] = 0;
        mix_mat  [mixlen]     = 1;
        mix_mat  [mixlen + 1] = 2;
        mix_zone [mixlen]     = j * sz2 + i * sz1;
        mix_zone [mixlen + 1] = j * sz2 + i * sz1;
        mix_vf   [mixlen]     = 0.4;
        mix_vf   [mixlen + 1] = 0.6;
        mixlen += 2;

        //
        // Add 10 layers of material 2.
        //
        for (j = 10; j < 20; j++)
        {
            matlist[j * sz2 + i * sz1] = 2;
        }

        //
        // Make the remainder of layers material 3.
        //
        for (j = 20; j < ny; j++)
        {
            matlist[j * sz2 + i * sz1] = 3;
        }
    }

    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;
    lo_off [0] = 1;
    lo_off [1] = 1;
    hi_off [0] = 3;
    hi_off [1] = 1;

    char *info = "mesh curvmesh2d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite (dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v};speed scalar sqrt(u*u+v*v)";
    i = strlen(defvars) + 1;
    DBWrite (dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

    optlist = DBMakeOptlist(15);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    char *xaxis="Z";
    char *yaxis="R";
    DBAddOption(optlist, DBOPT_XLABEL, xaxis);
    DBAddOption(optlist, DBOPT_YLABEL, yaxis);
    char *units="cm";
    DBAddOption(optlist, DBOPT_XUNITS, units);
    DBAddOption(optlist, DBOPT_YUNITS, units);
    DBAddOption(optlist, DBOPT_LO_OFFSET, &lo_off);
    DBAddOption(optlist, DBOPT_HI_OFFSET, &hi_off);
    DBAddOption(optlist, DBOPT_MAJORORDER, &major_order);
    int coordsys = DB_CYLINDRICAL;
    DBAddOption(optlist, DBOPT_COORDSYS, &coordsys);

    DBPutQuadmesh(dbfile, meshname, NULL, coords, dims, ndims, DB_FLOAT,
                  DB_NONCOLLINEAR, optlist);

    DBPutQuadvar1(dbfile, var1name, meshname, d, zdims, ndims, NULL,
                  0, DB_DOUBLE, DB_ZONECENT, optlist);

    DBPutQuadvar1(dbfile, var2name, meshname, p, zdims, ndims, NULL,
                  0, DB_DOUBLE, DB_ZONECENT, optlist);

    DBPutQuadvar1(dbfile, var3name, meshname, u, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, optlist);

    DBPutQuadvar1(dbfile, var4name, meshname, v, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, optlist);

    DBPutMaterial(dbfile, matname, meshname, nmats, matnos, matlist, dims2,
                  ndims, mix_next, mix_mat, mix_zone, mix_vf, mixlen,
                  DB_FLOAT, optlist);

    DBFreeOptlist(optlist);

    //
    // Free the temporary storage.
    //
    FREE (x);
    FREE (y);
    FREE (d);
    FREE (p);
    FREE (u);
    FREE (v);
    FREE (xx);
    FREE (yy);
    FREE (matlist);
    FREE (mix_vf);
    FREE (mix_zone);
    FREE (mix_mat);
    FREE (mix_next);
}

static DBoptlist *
ucd2d_var_optlist(const char *units, int *cycle, float *time,
    double *dtime)
{
    DBoptlist *optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, cycle);
    DBAddOption(optlist, DBOPT_TIME, time);
    DBAddOption(optlist, DBOPT_DTIME, dtime);
    DBAddOption(optlist, DBOPT_UNITS, (void *)units);
    return optlist;
}

// ****************************************************************************
// Modifications:
//   Brad Whitlock, Thu Jul 22 16:19:51 PST 2004
//   I added units and changed the labels to something that does not match
//   a default value in VisIt.
//
// ****************************************************************************

static void
build_ucd2d(DBfile * dbfile, int size)
{
    int             cycle;
    float           time;
    double          dtime;
    float          *coords[2];
    int             dims[1];
    float           x[13], y[13];
    float           d[13],u[13],v[13];
    float           p[12];
    int             matlist[12];
    int             materials[4];
    float           mix_vf[50];
    int             mix_next[50];
    int             mix_mat[50];
    int             mix_zone[50];
    int             current_mix;
    int             nzones, nnodes;
    int             zonelist[44];
    int             zonelist_length;
    int             shapesize[2];
    int             shapecount[2];
    DBoptlist      *optlist = NULL;

    int             i;

    //
    // Create the mesh.
    //
    coords[0] = x;
    coords[1] = y;

    nnodes = 13;
    nzones = 12;

    x[0] = 0.0;     y[0] = 2.0;
    x[1] = 2.0;     y[1] = 0.0;
    x[2] = 1.5;     y[2] = 1.5;
    x[3] = 2.5;     y[3] = 1.5;
    x[4] = 4.0;     y[4] = 2.0;
    x[5] = 2.0;     y[5] = 1.5;
    x[6] = 1.5;     y[6] = 2.0;
    x[7] = 2.0;     y[7] = 2.0;
    x[8] = 2.5;     y[8] = 2.0;
    x[9] = 1.5;     y[9] = 2.5;
    x[10] = 2.0;     y[10] = 2.5;
    x[11] = 2.5;     y[11] = 2.5;
    x[12] = 2.0;     y[12] = 4.0;

    i = 0;
    // Zone 0
    zonelist[i++] = 0;
    zonelist[i++] = 1;
    zonelist[i++] = 2;

    // Zone 1
    zonelist[i++] = 1;
    zonelist[i++] = 4;
    zonelist[i++] = 3;

    // Zone 2
    zonelist[i++] = 0;
    zonelist[i++] = 9;
    zonelist[i++] = 12;

    // Zone 3
    zonelist[i++] = 12;
    zonelist[i++] = 11;
    zonelist[i++] = 4;

    // Zone 4
    zonelist[i++] = 0;
    zonelist[i++] = 2;
    zonelist[i++] = 6;
    zonelist[i++] = 9;

    // Zone 5
    zonelist[i++] = 2;
    zonelist[i++] = 1;
    zonelist[i++] = 3;
    zonelist[i++] = 5;

    // Zone 6
    zonelist[i++] = 3;
    zonelist[i++] = 4;
    zonelist[i++] = 11;
    zonelist[i++] = 8;

    // Zone 7
    zonelist[i++] = 9;
    zonelist[i++] = 10;
    zonelist[i++] = 11;
    zonelist[i++] = 12;

    // Zone 8
    zonelist[i++] = 2;
    zonelist[i++] = 5;
    zonelist[i++] = 7;
    zonelist[i++] = 6;

    // Zone 9
    zonelist[i++] = 5;
    zonelist[i++] = 3;
    zonelist[i++] = 8;
    zonelist[i++] = 7;

    // Zone 10
    zonelist[i++] = 6;
    zonelist[i++] = 7;
    zonelist[i++] = 10;
    zonelist[i++] = 9;

    // Zone 11
    zonelist[i++] = 7;
    zonelist[i++] = 8;
    zonelist[i++] = 11;
    zonelist[i++] = 10;

    zonelist_length = i;

    shapesize[0] = 3;
    shapesize[1] = 4;

    shapecount[0] = 4;
    shapecount[1] = 8;

    //
    // Set up data variables
    //
    for (i = 0; i < nnodes; i++)
    {
        double xl,yl;

        xl= x[i] - 2.0;
        yl= y[i] - 2.0;

        d[i] = sqrt(xl*xl+yl*yl);
        u[i] = xl;
        v[i] = yl;
    }
    for (i = 0; i < nzones; i++)
    {
        if (i < shapecount[0])
        {
            int index;
            double xavg, yavg;

            index = (i*shapesize[0]);
            xavg = (x[zonelist[index]] + x[zonelist[index+1]] + x[zonelist[index+2]])/3;
            yavg = (y[zonelist[index]] + y[zonelist[index+1]] + y[zonelist[index+2]])/3;

            xavg -= 2.0;
            yavg -= 2.0;

            p[i] = sqrt(xavg*xavg+yavg*yavg);
        }
        else
        {
            int index;
            double xavg, yavg;

            index = shapecount[0]*shapesize[0]+(i-shapecount[0])*shapesize[1];

            xavg = (x[zonelist[index]] + x[zonelist[index+1]] +
                    x[zonelist[index+2]] + x[zonelist[index+3]])/4;
            yavg = (y[zonelist[index]] + y[zonelist[index+1]] +
                    y[zonelist[index+2]] + y[zonelist[index+3]])/4;

            xavg -= 2.0;
            yavg -= 2.0;

            p[i] = sqrt(xavg*xavg+yavg*yavg);
        }
    }

    //
    // Set up the materials.
    //
    materials[0] = 1;
    materials[1] = 2;
    materials[2] = 3;
    materials[3] = 4;

#define SET_MIX(mat1,mat2) \
    matlist[i] = -(current_mix+1); \
    mix_mat[current_mix] = mat1; \
    mix_vf[current_mix] = 0.5; \
    mix_zone[current_mix] = i; \
    mix_next[current_mix] = current_mix + 2; \
    current_mix++; \
    mix_mat[current_mix] = mat2; \
    mix_vf[current_mix] = 0.5; \
    mix_zone[current_mix] = i; \
    mix_next[current_mix] = 0; \
    current_mix++

    current_mix = 0;
    for(i=0;i<nzones;i++)
    {
        if (i==5)
        {
            matlist[i] = 1;
        }
        else if (i==4)
        {
            matlist[i] = 4;
        }
        else if (i==6)
        {
            matlist[i] = 2;
        }
        else if (i==7)
        {
            matlist[i] = 3;
        }
        else if ((i==0) || (i==8))
        {
            SET_MIX(1,4);
        }
        else if ((i==1) || (i==9))
        {
            SET_MIX(1,2);
        }
        else if ((i==3) || (i==11))
        {
            SET_MIX(2,3);
        }
        else if ((i==2) || (i==10))
        {
            SET_MIX(4,3);
        }
    }

    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    char *info = "mesh ucdmesh2d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite(dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v};speed scalar sqrt(u*u+v*v)";
    i = strlen(defvars) + 1;
    DBWrite(dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

    optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    char *xaxis="Width";
    char *yaxis="Height";
    DBAddOption(optlist, DBOPT_XLABEL, xaxis);
    DBAddOption(optlist, DBOPT_YLABEL, yaxis);
    char *units="Km";
    DBAddOption(optlist, DBOPT_XUNITS, units);
    DBAddOption(optlist, DBOPT_YUNITS, units);

    DBPutZonelist(dbfile, "ucd2d_zonelist", nzones, 2, zonelist,
                  zonelist_length, 0, shapesize, shapecount, 2);

    DBPutUcdmesh(dbfile, "ucdmesh2d", 2, NULL, coords, nnodes, nzones,
                 "ucd2d_zonelist", NULL, DB_FLOAT, optlist);

    dims[0] = nzones;
    DBPutMaterial(dbfile, "mat1", "ucdmesh2d", 4, materials, matlist, dims, 1,
                  mix_next, mix_mat, mix_zone, mix_vf, current_mix, DB_FLOAT,
                  NULL);

    DBoptlist *varOptList = ucd2d_var_optlist("g/Km^2", &cycle, &time, &dtime);
    DBPutUcdvar1(dbfile, "d", "ucdmesh2d", d, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = ucd2d_var_optlist("Km", &cycle, &time, &dtime);
    DBPutUcdvar1(dbfile, "u", "ucdmesh2d", u, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = ucd2d_var_optlist("Km", &cycle, &time, &dtime);
    DBPutUcdvar1(dbfile, "v", "ucdmesh2d", v, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    varOptList = ucd2d_var_optlist("bars", &cycle, &time, &dtime);
    DBPutUcdvar1(dbfile, "p", "ucdmesh2d", p, nzones, NULL, 0, DB_FLOAT,
                 DB_ZONECENT, varOptList);
    DBFreeOptlist(varOptList);

    //
    // Free the temporary storage.
    //
    DBFreeOptlist(optlist);
}

static void
build_ucd2d_lines(DBfile * dbfile, int size)
{
    int             cycle;
    float           time;
    double          dtime;
    int             tdim;
    float          *coords[2];
    int             dims[1];
    float           x[13], y[13];
    float           d[13],u[13],v[13];
    float           p[16];
    int             nzones, nnodes;
    int             zonelist[32];
    int             zonelist_length;
    int             shapetype[1];
    int             shapesize[1];
    int             shapecount[1];
    DBoptlist      *optlist = NULL;

    int             i;

    //
    // Create the mesh.
    //
    coords[0] = x;
    coords[1] = y;

    nnodes = 13;

    x[0] = 0.0;     y[0] = 2.0;
    x[1] = 2.0;     y[1] = 0.0;
    x[2] = 1.5;     y[2] = 1.5;
    x[3] = 2.5;     y[3] = 1.5;
    x[4] = 4.0;     y[4] = 2.0;
    x[5] = 2.0;     y[5] = 1.5;
    x[6] = 1.5;     y[6] = 2.0;
    x[7] = 2.0;     y[7] = 2.0;
    x[8] = 2.5;     y[8] = 2.0;
    x[9] = 1.5;     y[9] = 2.5;
    x[10] = 2.0;     y[10] = 2.5;
    x[11] = 2.5;     y[11] = 2.5;
    x[12] = 2.0;     y[12] = 4.0;

    i = 0;
    // line 0
    zonelist[i++] = 0;
    zonelist[i++] = 1;
    // line 1
    zonelist[i++] = 1;
    zonelist[i++] = 2;
    // line 2
    zonelist[i++] = 2;
    zonelist[i++] = 0;
    // line 3
    zonelist[i++] = 1;
    zonelist[i++] = 4;
    // line 4
    zonelist[i++] = 4;
    zonelist[i++] = 3;
    // line 5
    zonelist[i++] = 3;
    zonelist[i++] = 1;
    // line 6
    zonelist[i++] = 0;
    zonelist[i++] = 9;
    // line 7
    zonelist[i++] = 9;
    zonelist[i++] = 12;
    // line 8
    zonelist[i++] = 12;
    zonelist[i++] = 0;
    // line 9
    zonelist[i++] = 12;
    zonelist[i++] = 11;
    // line 10
    zonelist[i++] = 11;
    zonelist[i++] = 4;
    // line 11
    zonelist[i++] = 4;
    zonelist[i++] = 12;
    // line 12 
    zonelist[i++] = 9;
    zonelist[i++] = 11;
    // line 13 
    zonelist[i++] = 11;
    zonelist[i++] = 3;
    // line 14 
    zonelist[i++] = 3;
    zonelist[i++] = 2;
    // line 15
    zonelist[i++] = 2;
    zonelist[i++] = 9;

    zonelist_length = i;


    nzones        = 16;
    shapecount[0] = 16;
    shapesize[0]  = 2;
    shapetype[0]  = 10;

    //
    // Set up data variables
    //
    for (i = 0; i < nnodes; i++)
    {
        double xl,yl;

        xl= x[i] - 2.0;
        yl= y[i] - 2.0;

        d[i] = sqrt(xl*xl+yl*yl);
        u[i] = xl;
        v[i] = yl;
    }
    for (i = 0; i < nzones; i++)
    {
        int index;
        double xavg, yavg;

        index = (i*shapesize[0]);
        xavg = (x[zonelist[index]] + x[zonelist[index+1]])/2;
        yavg = (y[zonelist[index]] + y[zonelist[index+1]])/2;

        xavg -= 2.0;
        yavg -= 2.0;

        p[i] = sqrt(xavg*xavg+yavg*yavg);
    }


    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;
    tdim = 1;

    char *info = "mesh ucd_linesmesh2d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite(dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    optlist = DBMakeOptlist(4);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    DBAddOption(optlist, DBOPT_TOPO_DIM, &tdim);

    DBPutZonelist2(dbfile, "ucd2d_zonelist", nzones, 2, zonelist,
                  zonelist_length, 0, 0, 0, 
                  shapetype, shapesize, shapecount, 1,NULL);

    DBPutUcdmesh(dbfile, "ucd_linesmesh2d", 2, NULL, coords, nnodes, nzones,
                 "ucd2d_zonelist", NULL, DB_FLOAT, optlist);

    dims[0] = nzones;

    DBPutUcdvar1(dbfile, "d", "ucd_linesmesh2d", d, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    DBPutUcdvar1(dbfile, "u", "ucd_linesmesh2d", u, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    DBPutUcdvar1(dbfile, "v", "ucd_linesmesh2d", v, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    DBPutUcdvar1(dbfile, "p", "ucd_linesmesh2d", p, nzones, NULL, 0, DB_FLOAT,
                 DB_ZONECENT, NULL);

    //
    // Free the temporary storage.
    //
    DBFreeOptlist(optlist);
}


static void
build_rect3d(DBfile * dbfile, int size)
{
    int            cycle;
    float          time;
    double         dtime;
    int            nx=0, ny=0, nz=0;
    int            ndims;
    int            dims[3], zdims[3];
    float         *coords[3];
    float         *x, *y, *z;

    char          *meshname, *var1name, *var2name, *var3name, *var4name;
    char          *var5name, *matname;
    float         *d, *p;
    float         *u, *v, *w;
    float         *t;

    int            nmats;
    int            matnos[9];
    int           *matlist;
    int            dims2[3];
    int            mixlen;
    int           *mix_next, *mix_mat, *mix_zone;
    float         *mix_vf;

    DBoptlist     *optlist;

    int            i, j, k;
    double         xave, yave, zave;
    double         xcenter, ycenter, zcenter;
    double         dist;

    switch (size)
    {
        case 1:
            nx = 30;
            ny = 40;
            nz = 30;
            break;
        case 2:
            nx = 60;
            ny = 80;
            nz = 60;
            break;
        case 3:
            nx = 120;
            ny = 160;
            nz = 120;
            break;
    }

    x = ALLOC_N (float, nx + 1);
    y = ALLOC_N (float, ny + 1);
    z = ALLOC_N (float, nz + 1);
    d = ALLOC_N (float, nx * ny * nz);
    p = ALLOC_N (float, nx * ny * nz);
    u = ALLOC_N (float, (nx + 1) * (ny + 1) * (nz + 1));
    v = ALLOC_N (float, (nx + 1) * (ny + 1) * (nz + 1));
    w = ALLOC_N (float, (nx + 1) * (ny + 1) * (nz + 1));
    matlist = ALLOC_N (int, nx * ny * nz);
    mix_next = ALLOC_N (int, 50 * nx * ny);
    mix_mat  = ALLOC_N (int, 50 * nx * ny);
    mix_zone = ALLOC_N (int, 50 * nx * ny);
    mix_vf   = ALLOC_N (float, 50 * nx * ny);

    //
    // Create the mesh.
    //
    meshname = "quadmesh3d";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    ndims = 3;
    dims[0] = nx + 1;
    dims[1] = ny + 1;
    dims[2] = nz + 1;
    for (i = 0; i < nx + 1; i++)
        x[i] = i * (1. / nx);
    for (i = 0; i < ny + 1; i++)
        y[i] = i * (1. / ny);
    for (i = 0; i < nz + 1; i++)
        z[i] = i * (1. / nz);

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var2name = "p";
    xcenter = .5;
    ycenter = .5;
    zcenter = .5;
    zdims[0] = nx;
    zdims[1] = ny;
    zdims[2] = nz;
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz; k++)
            {
                xave = (x[i]+x[i+1]) / 2.0 ;
                yave = (y[j]+y[j+1]) / 2.0 ;
                zave = (z[k]+z[k+1]) / 2.0 ;

                dist = sqrt((xave - xcenter) * (xave - xcenter) +
                            (yave - ycenter) * (yave - ycenter) +
                            (zave - zcenter) * (zave - zcenter));

                d[k*nx*ny + j*nx + i] = dist;

                p[k*nx*ny + j*nx + i] = 1 / (dist + 0.0001);
            }
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var4name = "v";
    var5name = "w";
    xcenter = .5001;
    ycenter = .5001;
    zcenter = .5001;
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < ny + 1; j++)
        {
            for (k = 0; k < nz + 1; k++)
            {
                dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                            (y[j] - ycenter) * (y[j] - ycenter) +
                            (z[k] - zcenter) * (z[k] - zcenter));
                u[k * (nx + 1) * (ny + 1) + j * (nx + 1) + i] = (x[i] - xcenter) / dist;
                v[k * (nx + 1) * (ny + 1) + j * (nx + 1) + i] = (y[j] - ycenter) / dist;
                w[k * (nx + 1) * (ny + 1) + j * (nx + 1) + i] = (z[k] - zcenter) / dist;
            }
        }
    }


    //
    // Create the temperature array.
    //
    if ((t=ALLOC_N(float,(nx+1)*(ny+1)*(nz+1))) != NULL)
    {
       double a,b,c,d;

       a = 1.;
       b = 3.;
       c = 5.;
       d = 7.;
       for (i = 0; i < nx+1; i++)
          for (j = 0; j < ny+1; j++)
          {
             double y;

             y = j / (float) ny;
             for (k=0; k<nz+1; k++)
             {
                double z;

                z = k / (float) nz;
                t[k*(nx+1)*(ny+1) + j*(nx+1) + i] = a + b*y + c*exp(d*z);
             }
          }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    nmats = 8;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    matnos[3] = 4;
    matnos[4] = 5;
    matnos[5] = 6;
    matnos[6] = 7;
    matnos[7] = 8;
    matnos[8] = 9;
    dims2[0] = nx;
    dims2[1] = ny;
    dims2[2] = nz;
    mixlen = 0;

    //
    // Put in material 1 as the background then overlay materials
    // 2 through 9.
    //
    fill_rect3d_bkgr (matlist, nx, ny, nz, 1);

    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 2, 0.7);
    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 3, 0.6);
    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 4, 0.5);
    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 5, 0.4);
    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 6, 0.3);
    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 7, 0.2);
    fill_rect3d_mat (x, y, z, matlist, nx, ny, nz, mix_next, mix_mat, mix_zone,
                     mix_vf, &mixlen, 8, 0.1);

    if (mixlen > 50 * nx * ny)
    {
        printf ("mixlen = %d\n", mixlen);
        exit (1);
    }

    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    char *info = "mesh quadmesh3d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite (dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v,w};speed scalar sqrt(u*u+v*v+w*w)";
    i = strlen(defvars) + 1;
    DBWrite (dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

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

    DBPutQuadmesh(dbfile, meshname, NULL, coords, dims, ndims, DB_FLOAT,
                  DB_COLLINEAR, optlist);

    DBPutQuadvar1(dbfile, var1name, meshname, d, zdims, ndims, NULL, 0,
                  DB_FLOAT, DB_ZONECENT, optlist);

    DBPutQuadvar1(dbfile, var2name, meshname, p, zdims, ndims, NULL, 0,
                  DB_FLOAT, DB_ZONECENT, optlist);

    DBPutQuadvar1(dbfile, var3name, meshname, u, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, optlist);

    DBPutQuadvar1(dbfile, var4name, meshname, v, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, optlist);

    DBPutQuadvar1(dbfile, var5name, meshname, w, dims, ndims, NULL, 0,
                  DB_FLOAT, DB_NODECENT, optlist);

    if (t != NULL)
       DBPutQuadvar1(dbfile, "t", meshname, t
                    ,dims, ndims, NULL, 0, DB_FLOAT, DB_NODECENT
                    ,optlist
                    );

    DBPutMaterial(dbfile, matname, meshname, nmats, matnos, matlist, dims2,
                  ndims, mix_next, mix_mat, mix_zone, mix_vf, mixlen, DB_FLOAT,
                  optlist);

    DBFreeOptlist(optlist);

    //
    // Free the temporary storage.
    //
    FREE (x);
    FREE (y);
    FREE (z);
    FREE (d);
    FREE (p);
    FREE (u);
    FREE (v);
    FREE (w);
    FREE (t);
    FREE (matlist);
    FREE (mix_next);
    FREE (mix_mat);
    FREE (mix_zone);
    FREE (mix_vf);

}

static void
build_curv3d(DBfile * dbfile, int size, int major_order)
{
    int       cycle;
    float     time;
    double    dtime;
    float    *coords[3];

    float     *x = NULL;
    float     *y = NULL;
    float     *z = NULL;
    double    Xmin=0.0,Ymin=0.0,Zmin=0.0;
    double    Xmax=0.0,Ymax=0.0,Zmax=0.0;

    int       ndims, zdims[3];
    int       dims[3], dims2[3];

    char     *meshname, *var1name, *var2name, *var3name, *var4name;
    char     *var5name, *matname;

    float    *d = NULL, *p = NULL;
    float    *u = NULL;
    float    *v = NULL;
    float    *w = NULL;
    float    *t = NULL;

    int       first;
    int       nmats;
    int       matnos[3];
    int      *matlist = NULL;
    int       mixlen;
    int      *mix_next = NULL, *mix_mat = NULL;
    int      *mix_zone = NULL;
    float    *mix_vf = NULL;

    DBoptlist *optlist = NULL;

    int       i, j, k, l;

    double    xave, yave;
    double    xcenter, ycenter;
    int       nx=0, ny=0, nz=0;
    int       sn1, sn2, sn3, sz1, sz2, sz3;

    double    theta, dtheta;
    double    r, dr;
    double    h, dh;
    double    dist;
    float    *xx, *yy;

    switch (size)
    {
        case 1:
            nx = 30;
            ny = 40;
            nz = 30;
            break;
        case 2:
            nx = 60;
            ny = 80;
            nz = 60;
            break;
        case 3:
            nx = 120;
            ny = 160;
            nz = 120;
            break;
    }

    if (major_order == DB_ROWMAJOR)
    {
        sz1 = 1;
        sz2 = nx;
        sz3 = nx * ny;
        sn1 = 1;
        sn2 = nx + 1;
        sn3 = (nx + 1) * (ny + 1);
    }
    else
    {
        sz1 = ny * nz;
        sz2 = nz;
        sz3 = 1;
        sn1 = (ny + 1) * (nz + 1);
        sn2 = nz + 1;
        sn3 = 1;
    }

    x = ALLOC_N(float,(nx+1)*(ny+1)*(nz+1));
    y = ALLOC_N(float,(nx+1)*(ny+1)*(nz+1));
    z = ALLOC_N(float,(nx+1)*(ny+1)*(nz+1));
    d = ALLOC_N(float,nx*ny*nz);
    p = ALLOC_N(float,nx*ny*nz);
    u = ALLOC_N(float,(nx+1)*(ny+1)*(nz+1));
    v = ALLOC_N(float,(nx+1)*(ny+1)*(nz+1));
    w = ALLOC_N(float,(nx+1)*(ny+1)*(nz+1));
    xx = ALLOC_N(float,(nx+1)*2);
    yy = ALLOC_N(float,(nx+1)*2);
    matlist = ALLOC_N(int,nx*ny*nz);
    mix_next = ALLOC_N(int,50 * nx*ny*nz);
    mix_mat = ALLOC_N(int,50 * nx*ny*nz);
    mix_vf = ALLOC_N(float,50 * nx*ny*nz);

    //
    // Create the mesh.
    //
    meshname = "curvmesh3d";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    ndims = 3;
    dims[0] = nx+1;
    dims[1] = ny+1;
    dims[2] = nz+1;

    dtheta = -(180. / nx) * (3.1415926536 / 180.);
    dh = 1;
    dr = 3. / ny;
    theta = 3.1415926536;

    first = 1;
    for (i = 0; i < nx+1; i++)
    {
        r = 2.;
        for (j = 0; j < ny+1; j++)
        {
            h = 0.;
            for (k = 0; k < nz+1; k++)
            {
                double Xp,Yp,Zp;
                int    a;

                Xp = r * cos(theta);
                Yp = r * sin(theta);
                Zp = h;
                if (first)
                {
                    Xmin  = Xp;
                    Xmax  = Xp;
                    Ymin  = Yp;
                    Ymax  = Yp;
                    Zmin  = Zp;
                    Zmax  = Zp;
                    first = 0;
                }
                else
                {
                    Xmin  = Min(Xmin,Xp);
                    Xmax  = Max(Xmax,Xp);
                    Ymin  = Min(Ymin,Yp);
                    Ymax  = Max(Ymax,Yp);
                    Zmin  = Min(Zmin,Zp);
                    Zmax  = Max(Zmax,Zp);
                }
                a        = k*sn3 + j*sn2 + i*sn1;
                x[a]     = Xp;
                y[a]     = Yp;
                z[a]     = Zp;
                h       += dh;
            }
            r += dr;
        }
        theta += dtheta;
    }

    //
    // Force the y values to be greater than or equal to zero so that
    // the y values are all planar on the y = 0 symmetry boundary.
    //
    for (i = 0; i < nx+1; i += nx)
    {
        for (j = 0; j < ny+1; j++)
        {
            for (k = 0; k < nz+1; k++)
            {
                l = k*sn3 + j*sn2 + i*sn1;
                y[l] = 0.;
            }
        }
    }

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    zdims[0] = nx;
    zdims[1] = ny;
    zdims[2] = nz;

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz; k++)
            {
                xave = (x[k*sn3 +  j   *sn2 +  i   *sn1] +
                        x[k*sn3 +  j   *sn2 + (i+1)*sn1] +
                        x[k*sn3 + (j+1)*sn2 + (i+1)*sn1] +
                        x[k*sn3 + (j+1)*sn2 +  i   *sn1]) / 4.;

                yave = (y[k*sn3 +  j   *sn2 +  i   *sn1] +
                        y[k*sn3 +  j   *sn2 + (i+1)*sn1] +
                        y[k*sn3 + (j+1)*sn2 + (i+1)*sn1] +
                        y[k*sn3 + (j+1)*sn2 +  i   *sn1]) / 4.;

                dist = sqrt ((xave - xcenter) * (xave - xcenter) +
                             (yave - ycenter) * (yave - ycenter) );
                d[k*sz3 + j*sz2 + i*sz1] = dist;
                p[k*sz3 + j*sz2 + i*sz1] = 1. / (dist + .0001);
            }
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var4name = "v";
    var5name = "w";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < nx + 1; i++)
    {
        for (j = 0; j < 2; j++)
        {
            xx [j * (nx + 1) + i] = x [j * sn2 + i * sn1];
            yy [j * (nx + 1) + i] = y [j * sn2 + i * sn1];
        }
    }
    for (i = 0; i < nx+1; i++)
    {
        for (j = 0; j < ny+1; j++)
        {
            for (k = 0; k < nz+1; k++)
            {
                dist = sqrt ((xx[i] - xcenter) * (xx[i] - xcenter) +
                             (yy[j] - ycenter) * (yy[j] - ycenter));
                u[k*sn3 + j*sn2 + i*sn1] = (xx[i] - xcenter) / dist;
                v[k*sn3 + j*sn2 + i*sn1] = (yy[j] - ycenter) / dist;
                w[k*sn3 + j*sn2 + i*sn1] = 0.;
            }
        }
    }

    //
    // Create the temperature array.
    //
    if ((t=ALLOC_N(float,(nx+1)*(ny+1)*(nz+1))) != NULL)
    {
       double Ca,Cb,Cc,Cd;

       Ca = 1.;
       Cb = 3.;
       Cc = 5.;
       Cd = 7.;
       for (i = 0; i <= nx; ++i)
          for (j = 0; j <= ny; ++j)
             for (k = 0; k <= nz; ++k)
             {
                double Yv,Zv;
                int    a;

                a    = k*sn3 + j*sn2 + i*sn1;
                Yv   = (y[a]-Ymin) / (Ymax-Ymin);
                Zv   = (z[a]-Zmin) / (Zmax-Zmin);
                t[a] = Ca + Cb*Yv + Cc*exp(Cd*Zv);
             }
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    dims2[0] = nx;
    dims2[1] = ny;
    dims2[2] = nz;

    mixlen = 0;

    //
    // Put in the material in 3 shells.
    //
    for (i = 0; i < nx; i++)
    {
        for (k = 0; k < nz; k++)
        {
            for (j = 0; j < 10; j++)
            {
                matlist[k*sz3 + j*sz2 + i*sz1] = 1;
            }
            for (j = 10; j < 20; j++)
            {
                matlist[k*sz3 + j*sz2 + i*sz1] = 2;
            }
            for (j = 20; j < ny; j++)
            {
                matlist[k*sz3 + j*sz2 + i*sz1] = 3;
            }
        }
    }

    //
    // Write out the variables.
    //
    cycle = 48;
    time  = 4.8;
    dtime = 4.8;

    char *info = "mesh curvmesh3d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite (dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v,w};speed scalar sqrt(u*u+v*v+w*w)";
    i = strlen(defvars) + 1;
    DBWrite (dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

    optlist = DBMakeOptlist (10);
    DBAddOption (optlist, DBOPT_CYCLE, &cycle);
    DBAddOption (optlist, DBOPT_TIME , &time);
    DBAddOption (optlist, DBOPT_DTIME, &dtime);
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
    DBAddOption(optlist, DBOPT_MAJORORDER, &major_order);

    DBPutQuadmesh (dbfile, meshname, NULL, coords, dims, ndims,
                   DB_FLOAT, DB_NONCOLLINEAR, optlist);

    DBPutQuadvar1 (dbfile, var1name, meshname, d, zdims, ndims, NULL, 0,
                   DB_FLOAT, DB_ZONECENT, optlist);

    DBPutQuadvar1 (dbfile, var2name, meshname, p, zdims, ndims, NULL, 0,
                   DB_FLOAT, DB_ZONECENT, optlist);

    DBPutQuadvar1 (dbfile, var3name, meshname, u, dims, ndims, NULL, 0,
                   DB_FLOAT, DB_NODECENT, optlist);

    DBPutQuadvar1 (dbfile, var4name, meshname, v, dims, ndims, NULL, 0,
                   DB_FLOAT, DB_NODECENT, optlist);

    DBPutQuadvar1 (dbfile, var5name, meshname, w, dims, ndims, NULL, 0,
                   DB_FLOAT, DB_NODECENT, optlist);

    DBPutQuadvar1 (dbfile, "t", meshname, t, dims, ndims, NULL, 0, DB_FLOAT,
                   DB_NODECENT, optlist);

    DBPutMaterial (dbfile, matname, meshname, nmats, matnos, matlist, dims2,
                   ndims, mix_next, mix_mat, mix_zone, mix_vf, mixlen,
                   DB_FLOAT, optlist);

    DBFreeOptlist (optlist);

    //
    // Free the temporary storage.
    //
    FREE(x);
    FREE(y);
    FREE(z);
    FREE(d);
    FREE(p);
    FREE(u);
    FREE(v);
    FREE(w);
    FREE(t);
    FREE(xx);
    FREE(yy);
    FREE(matlist);
    FREE(mix_mat);
    FREE(mix_next);
    FREE(mix_zone);
    FREE(mix_vf);
}

static DBoptlist *
ucd3d_var_optlist(const char *units, int *cycle, float *time,
    double *dtime)
{
    DBoptlist *optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, cycle);
    DBAddOption(optlist, DBOPT_TIME, time);
    DBAddOption(optlist, DBOPT_DTIME, dtime);
    DBAddOption(optlist, DBOPT_UNITS, (void *)units);
    return optlist;
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Mon Jun 21 09:32:37 PDT 2004
//    Added exterior_faces. ('4335/'4337)
//
//    Brad Whitlock, Wed Jul 21 14:14:07 PST 2004
//    Added variable units and changed the axis labels to values that are
//    different from VisIt's defaults.
//
// **************************************************************************** 

static void
build_ucd3d(DBfile * dbfile, int size)
{
    int            cycle;
    float          time;
    double         dtime;
    float         *coords[3];
    float          x[2646], y[2646], z[2646];
    double         Xmin=0,Ymin=0,Zmin=0;
    double         Xmax=0,Ymax=0,Zmax=0;
    int            first;
    int            nfaces, nzones, nnodes;
    int            lfacelist, lzonelist;
    int            fshapesize, fshapecnt, zshapesize, zshapecnt;
    int            zonelist[16000];
    int            facelist[11200];
    int            zoneno[2800];

    char          *meshname, *var1name, *var2name, *var3name, *var4name;
    char          *var5name, *matname;
    float         *vars[1];
    char          *varnames[1];
    float          d[2646], p[2646], u[2646], v[2646], w[2646];
    float         *t;

    int            nmats;
    int            matnos[3];
    int            matlist[2000];
    int            mixlen;
    int            mix_next[900], mix_mat[900], mix_zone[900];
    float          mix_vf[900];

    int            nmatspec[3];
    float          specmf[5000];
    int            speclist[2000];
    int            mix_speclist[900];
    int            nspecmf;

    DBoptlist     *optlist = NULL, *varOptList = NULL;

    DBfacelist    *fl;

    int            i, j, k, l;
    int            iz;
    double         xcenter, ycenter;
    double         theta, dtheta;
    double         r, dr;
    double         h, dh;
    double         dist;

    //
    // Create the mesh.
    //
    meshname = "ucdmesh3d";
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    nnodes = 21 * 21 * 6;
    nzones = 20 * 20 * 5;

    lzonelist = (20 * 20 * 5) * 8;
    zshapecnt = nzones;
    zshapesize = 8;

    dh = 20. / 20.;
    dtheta = (180. / 20.) * (3.1415926 / 180.);
    dr = 3. / 5.;
    h = 0.;
    first = 1;
    for (i = 0; i < 21; i++)
    {
        theta = 0.;
        for (j = 0; j < 21; j++)
        {
            r = 2.;
            for (k = 0; k < 6; k++)
            {
                double Xv,Yv,Zv;
                int    a;

                Xv = r * cos(theta);
                Yv = r * sin(theta);
                Zv = h;
                if (first)
                {
                    Xmin  = Xv;
                    Xmax  = Xv;
                    Ymin  = Yv;
                    Ymax  = Yv;
                    Zmin  = Zv;
                    Zmax  = Zv;
                    first = 0;
                }
                else
                {
                    Xmin  = Min(Xmin,Xv);
                    Xmax  = Max(Xmax,Xv);
                    Ymin  = Min(Ymin,Yv);
                    Ymax  = Max(Ymax,Yv);
                    Zmin  = Min(Zmin,Zv);
                    Zmax  = Max(Zmax,Zv);
                }
                a        = i*126 + j*6 + k;
                x[a]     = Xv;
                y[a]     = Yv;
                z[a]     = Zv;
                r       += dr;
            }
            theta += dtheta;
        }
        h += dh;
    }

    //
    // Force the y values to be greater than or equal to zero so that
    // the y values are all planar on the y = 0 symmetry boundary.
    //
    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 21; j += 20)
        {
            for (k = 0; k < 6; k++)
            {
                l = i*126 + j*6 + k;
                y[l] = 0.;
            }
        }
    }

    iz = 0;
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 20; j++)
        {
            for (k = 0; k < 5; k++)
            {
                zonelist[iz    ] = (i + 0) * 126 + (j + 1) * 6 + k + 1;
                zonelist[iz + 1] = (i + 0) * 126 + (j + 0) * 6 + k + 1;
                zonelist[iz + 2] = (i + 1) * 126 + (j + 0) * 6 + k + 1;
                zonelist[iz + 3] = (i + 1) * 126 + (j + 1) * 6 + k + 1;
                zonelist[iz + 4] = (i + 0) * 126 + (j + 1) * 6 + k + 0;
                zonelist[iz + 5] = (i + 0) * 126 + (j + 0) * 6 + k + 0;
                zonelist[iz + 6] = (i + 1) * 126 + (j + 0) * 6 + k + 0;
                zonelist[iz + 7] = (i + 1) * 126 + (j + 1) * 6 + k + 0;
                iz += 8;
            }
        }
    }

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var2name = "p";
    xcenter = 0.;
    ycenter = 0.;
    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 21; j++)
        {
            for (k = 0; k < 6; k++)
            {
                dist = sqrt((x[i * 126 + j * 6 + k] - xcenter) *
                            (x[i * 126 + j * 6 + k] - xcenter) +
                            (y[i * 126 + j * 6 + k] - ycenter) *
                            (y[i * 126 + j * 6 + k] - ycenter));
                d[i * 126 + j * 6 + k] = dist;
                p[i * 126 + j * 6 + k] = 1. / (dist + .0001);
            }
        }
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var4name = "v";
    var5name = "w";
    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 21; j++)
        {
            for (k = 0; k < 6; k++)
            {
                dist = sqrt(x[i] * x[i] + y[j] * y[j]);
                u[i * 126 + j * 6 + k] = x[i] / dist;
                v[i * 126 + j * 6 + k] = y[j] / dist;
                w[i * 126 + j * 6 + k] = 0.;
            }
        }
    }

    //
    // Create the temperature array.
    //
    if ((t=ALLOC_N(float,21*21*6)) != NULL)
    {
       double Ca,Cb,Cc,Cd;

       Ca = 1.;
       Cb = 3.;
       Cc = 5.;
       Cd = 7.;
       for (i = 0; i < 21; ++i)
          for (j = 0; j < 21; ++j)
             for (k = 0; k < 6; ++k)
             {
                double Yv,Zv;
                int    a;

                a    = i*21*6 + j*6 + k;
                Yv   = (y[a]-Ymin) / (Ymax-Ymin);
                Zv   = (z[a]-Zmin) / (Zmax-Zmin);
                t[a] = Ca + Cb*Yv + Cc*exp(Cd*Zv);
             }
    }

    //
    // Create the material and species arrays.
    //
    matname = "mat1";
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 4;
    matnos[2] = 9;
    mixlen = 0;

    //
    // Material 1 has 2 species at static concentrations 0.1 and 0.9
    // Material 2 has 3 species at static concentrations 0.2, 0.3, and 0.5
    // Material 3 has a single species
    //
    nmatspec[0] = 2;
    nmatspec[1] = 3;
    nmatspec[2] = 1;
    nspecmf = 0;

    //
    // Put in the material in 3 shells.
    //
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 20; j++)
        {
            // Clean zones, material 1
            for (k = 0; k < 2; k++)
            {
                int zone = i * 100 + j * 5 + k;

                matlist[zone] = 1;
                speclist[zone] = nspecmf + 1;
                specmf[nspecmf++] = 0.1;
                specmf[nspecmf++] = 0.9;
            }
            
            // Mixed zones, materials 1 and 4
            for (k = 2; k < 3; k++)
            {
                int zone = i * 100 + j * 5 + k;

                matlist[zone] = -mixlen - 1;
                speclist[zone]= -mixlen - 1;

                // material 1
                mix_mat [mixlen]   = 1;
                mix_next[mixlen]   = mixlen+2;
                mix_zone[mixlen]   = zone + 1;
                mix_vf  [mixlen]   = (float)i/20.;

                mix_speclist[mixlen] = nspecmf + 1;
                specmf[nspecmf++] = 0.1;
                specmf[nspecmf++] = 0.9;

                mixlen++;

                // material 4
                mix_mat [mixlen] = 4;
                mix_next[mixlen] = 0;
                mix_zone[mixlen] = zone + 1;
                mix_vf  [mixlen] = 1.0 - ((float)i/20.);

                mix_speclist[mixlen] = nspecmf + 1;
                specmf[nspecmf++] = 0.2;
                specmf[nspecmf++] = 0.3;
                specmf[nspecmf++] = 0.5;

                mixlen++;
            }

            // Clean zones, material 4
            for (k = 3; k < 4; k++)
            {
                int zone = i * 100 + j * 5 + k;
                matlist[zone] = 4;
                speclist[zone] = nspecmf + 1;
                specmf[nspecmf++] = 0.2;
                specmf[nspecmf++] = 0.3;
                specmf[nspecmf++] = 0.5;
            }

            // Clean zones, material 9
            for (k = 4; k < 5; k++)
            {
                int zone = i * 100 + j * 5 + k;
                matlist[zone] = 9;
                speclist[zone] = 0;
            }
        }
    }

    //
    // Calculate the external face list.
    //
    fl = DBCalcExternalFacelist(zonelist, nnodes, 0, &zshapesize,
                                &zshapecnt, 1, matlist, 0);

    nfaces = fl->nfaces;
    fshapecnt = fl->nfaces;
    fshapesize = 4;
    lfacelist = fl->lnodelist;
    for (i = 0; i < lfacelist; i++)
        facelist[i] = fl->nodelist[i];
    for (i = 0; i < nfaces; i++)
        zoneno[i] = fl->zoneno[i];

    DBFreeFacelist(fl);

    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    char *info = "mesh ucdmesh3d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite (dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v,w};speed scalar sqrt(u*u+v*v+w*w)";
    i = strlen(defvars) + 1;
    DBWrite (dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

    optlist = DBMakeOptlist(10);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    char *xaxis="Width";
    char *yaxis="Height";
    char *zaxis="Depth";
    DBAddOption(optlist, DBOPT_XLABEL, xaxis);
    DBAddOption(optlist, DBOPT_YLABEL, yaxis);
    DBAddOption(optlist, DBOPT_ZLABEL, zaxis);
    char *units="cm";
    DBAddOption(optlist, DBOPT_XUNITS, units);
    DBAddOption(optlist, DBOPT_YUNITS, units);
    DBAddOption(optlist, DBOPT_ZUNITS, units);

    DBPutFacelist(dbfile, "fl1", nfaces, 3, facelist, lfacelist, 0, zoneno,
                  &fshapesize, &fshapecnt, 1, NULL, NULL, 0);

    DBPutZonelist(dbfile, "zl1", nzones, 3, zonelist, lzonelist, 0,
                  &zshapesize, &zshapecnt, 1);

    DBPutUcdmesh(dbfile, meshname, 3, NULL, coords, nnodes, nzones,
                 "zl1", "fl1", DB_FLOAT, optlist);

    vars[0] = d;
    varnames[0] = var1name;
    varOptList = ucd3d_var_optlist("g/cm^3", &cycle, &time, &dtime);
    DBPutUcdvar(dbfile, var1name, meshname, 1, varnames, vars, nnodes, NULL,
                0, DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    vars[0] = p;
    varnames[0] = var2name;
    varOptList = ucd3d_var_optlist("bars", &cycle, &time, &dtime);
    DBPutUcdvar(dbfile, var2name, meshname, 1, varnames, vars, nnodes, NULL,
                0, DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    vars[0] = u;
    varnames[0] = var3name;
    varOptList = ucd3d_var_optlist("cm", &cycle, &time, &dtime);
    DBPutUcdvar(dbfile, var3name, meshname, 1, varnames, vars, nnodes, NULL,
                0, DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    vars[0] = v;
    varnames[0] = var4name;
    varOptList = ucd3d_var_optlist("cm", &cycle, &time, &dtime);
    DBPutUcdvar(dbfile, var4name, meshname, 1, varnames, vars, nnodes, NULL,
                0, DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    vars[0] = w;
    varnames[0] = var5name;
    varOptList = ucd3d_var_optlist("cm", &cycle, &time, &dtime);
    DBPutUcdvar(dbfile, var5name, meshname, 1, varnames, vars, nnodes, NULL,
                0, DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    vars[0]     = t;
    varnames[0] = "t";
    varOptList = ucd3d_var_optlist("Celsius", &cycle, &time, &dtime);
    DBPutUcdvar(dbfile, "t", meshname, 1, varnames, vars, nnodes, NULL, 0,
                DB_FLOAT, DB_NODECENT, varOptList);
    DBFreeOptlist(varOptList);

    DBPutMaterial(dbfile, matname, meshname, nmats, matnos, matlist, &nzones,
                  1, mix_next, mix_mat, mix_zone, mix_vf, mixlen, DB_FLOAT,
                  optlist);

    DBPutMatspecies(dbfile, "species", matname, nmats, nmatspec, speclist,
                    &nzones, 1, nspecmf, specmf, mix_speclist, mixlen, 
                    DB_FLOAT, optlist);

    {
        // Now write out another mesh that is quads posing as hexes.  Use
        // the facelist to create this mesh.
        int nzones = nfaces;
        int zshapesize = 8;
        int zshapecnt = nzones;
        int lzonelist = zshapesize*nzones;
        int *zl2 = new int[lzonelist];
        for (i = 0 ; i < nzones ; i++)
        {
            zl2[8*i+0] = facelist[4*i+0];
            zl2[8*i+1] = facelist[4*i+1];
            zl2[8*i+2] = facelist[4*i+2];
            zl2[8*i+3] = facelist[4*i+3];
            zl2[8*i+4] = facelist[4*i+0];
            zl2[8*i+5] = facelist[4*i+1];
            zl2[8*i+6] = facelist[4*i+2];
            zl2[8*i+7] = facelist[4*i+3];
        }

        DBPutZonelist(dbfile, "zl2", nzones, 3, zl2, lzonelist, 0,
                      &zshapesize, &zshapecnt, 1);

        DBPutUcdmesh(dbfile, "exterior_faces", 3, NULL, coords, nnodes, nzones,
                     "zl2", NULL, DB_FLOAT, optlist);
    }

    DBFreeOptlist(optlist);

    //
    // Free the temporary storage.
    //
    FREE(t);
}

static void
build_ucd3d_lines(DBfile * dbfile, int size)
{
    int             cycle;
    float           time;
    double          dtime;
    int             tdim;
    float          *coords[3];
    int             dims[1];
    float           x[13], y[13], z[13];
    float           d[13];
    float           p[16];
    int             nzones, nnodes;
    int             zonelist[32];
    int             zonelist_length;
    int             shapetype[1];
    int             shapesize[1];
    int             shapecount[1];
    DBoptlist      *optlist = NULL;

    int             i;

    //
    // Create the mesh.
    //
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    nnodes = 13;

    x[0] = 0.0;     y[0] = 2.0;      z[0] = 0;
    x[1] = 2.0;     y[1] = 0.0;      z[1] = 0;
    x[2] = 1.5;     y[2] = 1.5;      z[2] = 4;
    x[3] = 2.5;     y[3] = 1.5;      z[3] = 4;
    x[4] = 4.0;     y[4] = 2.0;      z[4] = 0;
    x[5] = 2.0;     y[5] = 1.5;      z[5] = 2;
    x[6] = 1.5;     y[6] = 2.0;      z[6] = 2;
    x[7] = 2.0;     y[7] = 2.0;      z[7] = 1;
    x[8] = 2.5;     y[8] = 2.0;      z[8] = 2;
    x[9] = 1.5;     y[9] = 2.5;      z[9] = 4;
    x[10] = 2.0;     y[10] = 2.5;    z[10] = 2;
    x[11] = 2.5;     y[11] = 2.5;    z[11] = 4;
    x[12] = 2.0;     y[12] = 4.0;    z[12] = 0;

    i = 0;
    // line 0
    zonelist[i++] = 0;
    zonelist[i++] = 1;
    // line 1
    zonelist[i++] = 1;
    zonelist[i++] = 2;
    // line 2
    zonelist[i++] = 2;
    zonelist[i++] = 0;
    // line 3
    zonelist[i++] = 1;
    zonelist[i++] = 4;
    // line 4
    zonelist[i++] = 4;
    zonelist[i++] = 3;
    // line 5
    zonelist[i++] = 3;
    zonelist[i++] = 1;
    // line 6
    zonelist[i++] = 0;
    zonelist[i++] = 9;
    // line 7
    zonelist[i++] = 9;
    zonelist[i++] = 12;
    // line 8
    zonelist[i++] = 12;
    zonelist[i++] = 0;
    // line 9
    zonelist[i++] = 12;
    zonelist[i++] = 11;
    // line 10
    zonelist[i++] = 11;
    zonelist[i++] = 4;
    // line 11
    zonelist[i++] = 4;
    zonelist[i++] = 12;
    // line 12 
    zonelist[i++] = 9;
    zonelist[i++] = 11;
    // line 13 
    zonelist[i++] = 11;
    zonelist[i++] = 3;
    // line 14 
    zonelist[i++] = 3;
    zonelist[i++] = 2;
    // line 15
    zonelist[i++] = 2;
    zonelist[i++] = 9;

    zonelist_length = i;

    nzones        = 16;
    shapecount[0] = 16;
    shapesize[0]  = 2;
    shapetype[0]  = 10;

    //
    // Set up data variables
    //
    for (i = 0; i < nnodes; i++)
    {
        d[i] = i;
    }
    for (i = 0; i < nzones; i++)
    {
        p[i] = i; 
    }


    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;
    tdim = 1;

    char *info = "mesh ucd_linesmesh3d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite(dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    optlist = DBMakeOptlist(4);
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    DBAddOption(optlist, DBOPT_TIME, &time);
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    DBAddOption(optlist, DBOPT_TOPO_DIM, &tdim);

    DBPutZonelist2(dbfile, "ucd3d_zonelist", nzones, 3, zonelist,
                  zonelist_length, 0, 0, 0, 
                  shapetype, shapesize, shapecount, 1,NULL);

    DBPutUcdmesh(dbfile, "ucd_linesmesh3d", 3, NULL, coords, nnodes, nzones,
                 "ucd3d_zonelist", NULL, DB_FLOAT, optlist);


    DBPutUcdvar1(dbfile, "n", "ucd_linesmesh3d", d, nnodes, NULL, 0, DB_FLOAT,
                 DB_NODECENT, NULL);

    DBPutUcdvar1(dbfile, "z", "ucd_linesmesh3d", p, nzones, NULL, 0, DB_FLOAT,
                 DB_ZONECENT, NULL);

    //
    // Free the temporary storage.
    //
    DBFreeOptlist(optlist);
}


static void
build_poly3d(DBfile *dbfile, int size)
{
    int            cycle;
    float          time;
    double         dtime;
    char          *coordnames[3];
    float         *coords[3];
    float         *x, *y, *z;
    float          Ca, Cb, Cc, Cd;
    float          Xmin,Ymin,Zmin;
    float          Xmax,Ymax,Zmax;
    int            nx=0, ny=0, nz=0;
    int            nfaces, nzone1, nzone2, nzone3, nzones, nnodes;
    int            lo_offset, hi_offset;
    int            lfacelist, lzonelist;
    int            fshapesize[3], fshapecnt[3], nfshapes;
    int            zshapetype[3], zshapesize[3], zshapecnt[3], nzshapes;
    int           *zonelist;
    int           *facelist;
    int           *zoneno;

    char          *meshname, *var1name, *var2name, *var3name, *var4name;
    char          *var5name, *matname;
    float         *vars[1];
    char          *varnames[1];
    float         *d, *p, *u, *v, *w, *t;

    int            nmats;
    int            matnos[3];
    int           *matlist;
    int            mixlen;

    DBoptlist     *optlist;

    DBfacelist    *fl;

    int            i, j, k;
    int            in, iz;
    double         xcenter, ycenter, zcenter;
    double         theta, dtheta;
    double         r, dr;
    double         h, dh;
    double         dist;

    switch (size)
    {
        case 1:
            nx = 20;
            ny = 20;
            nz = 10;
            break;
        case 2:
            nx = 40;
            ny = 40;
            nz = 20;
            break;
        case 3:
            nx = 80;
            ny = 80;
            nz = 40;
            break;
    }

    //
    // Create the mesh.
    //
    meshname = "ucdmesh3d";
    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    nnodes = (nx + 1) * ((ny / 2 + 1) * (nz / 2) + (ny + 1) * (nz / 2 + 1));
    x = ALLOC_N (float, nnodes);
    y = ALLOC_N (float, nnodes);
    z = ALLOC_N (float, nnodes);
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    in = 0;

    dh = 40. / nx;
    dtheta = (180. / (ny / 2)) * (3.1415926 / 180.);
    dr = 3. / nz;
    h = 0.;
    for (i = 0; i < nx + 1; i++)
    {
        theta = 0.;
        for (j = 0; j < ny / 2 + 1; j++)
        {
            r = 2.;
            for (k = 0; k < nz / 2; k++)
            {
                x[in]    = r * cos(theta);
                y[in]    = r * sin(theta);
                z[in]    = h;
                in++;
                r       += dr;
            }
            theta += dtheta;
        }
        h += dh;
    }

    dh = 40. / nx;
    dtheta = (180. / ny) * (3.1415926 / 180.);
    dr = 3. / nz;
    h = 0.;
    for (i = 0; i < nx + 1; i++)
    {
        theta = 0.;
        for (j = 0; j < ny + 1; j++)
        {
            r = 2. + (3. / 2.);
            for (k = nz / 2; k < nz + 1; k++)
            {
                x[in]    = r * cos(theta);
                y[in]    = r * sin(theta);
                z[in]    = h;
                in++;
                r       += dr;
            }
            theta += dtheta;
        }
        h += dh;
    }

    Xmin = x[0];
    Xmin = x[0];
    Xmax = x[0];
    Ymin = y[0];
    Ymax = y[0];
    Zmin = z[0];
    Zmax = z[0];
    for (i = 0; i < nnodes; i++)
    {
        Xmin = Min(Xmin, x[i]);
        Xmax = Max(Xmax, x[i]);
        Ymin = Min(Ymin, y[i]);
        Ymax = Max(Ymax, y[i]);
        Zmin = Min(Zmin, z[i]);
        Zmax = Max(Zmax, z[i]);
    }

    nzone1 = nx * (ny / 2) * (nz / 2 - 1);
    nzone2 = nx * (ny / 2) * 1;
    nzone3 = nx * ny * (nz / 2);
    nzones = nzone1 + nzone2 + nzone3;
    lo_offset = 0;
    hi_offset = 0;
    lzonelist = nzone1 * 8 + nzone2 * 38 + nzone3 * 8;
    zonelist = ALLOC_N (int, lzonelist);
    zshapecnt[0] = nzone1;
    zshapecnt[1] = nzone2;
    zshapecnt[2] = nzone3;
    zshapesize[0] = 8;
    zshapesize[1] = 38 * nzone2;
    zshapesize[2] = 8;
    zshapetype[0] = DB_ZONETYPE_HEX;
    zshapetype[1] = DB_ZONETYPE_POLYHEDRON;
    zshapetype[2] = DB_ZONETYPE_HEX;
    nzshapes = 3;

    iz = 0;
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny / 2; j++)
        {
            for (k = 0; k < nz / 2 - 1; k++)
            {
                zonelist[iz]     = (i)     * (ny / 2 + 1) * (nz / 2) +
                                   (j)     * (nz / 2) + k;
                zonelist[iz + 1] = (i)     * (ny / 2 + 1) * (nz / 2) +
                                   (j + 1) * (nz / 2) + k;
                zonelist[iz + 2] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                   (j + 1) * (nz / 2) + k;
                zonelist[iz + 3] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                   (j)     * (nz / 2) + k;
                zonelist[iz + 4] = (i)     * (ny / 2 + 1) * (nz / 2) +
                                   (j)     * (nz / 2) + k + 1;
                zonelist[iz + 5] = (i)     * (ny / 2 + 1) * (nz / 2) +
                                   (j + 1) * (nz / 2) + k + 1;
                zonelist[iz + 6] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                   (j + 1) * (nz / 2) + k + 1;
                zonelist[iz + 7] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                   (j)     * (nz / 2) + k + 1;
                iz += 8;
            }
        }
    }

    in = (nx + 1) * (ny / 2 + 1) * (nz / 2);
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny / 2; j++)
        {
            // Number of faces
            zonelist[iz] = 7;

            // Bottom four sided face
            zonelist[iz + 1]  = 4;
            zonelist[iz + 2]  = (i)     * (ny / 2 + 1) * (nz / 2) +
                                (j)     * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 3]  = (i)     * (ny / 2 + 1) * (nz / 2) +
                                (j + 1) * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 4]  = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                (j + 1) * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 5]  = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                (j)     * (nz / 2) + (nz / 2) - 1;

            // Two five sided side faces
            zonelist[iz + 6]  = 5;
            zonelist[iz + 7]  = (i)     * (ny / 2 + 1) * (nz / 2) +
                                (j)     * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 8]  = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j))     * (nz / 2 + 1);
            zonelist[iz + 9]  = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j) + 1) * (nz / 2 + 1);
            zonelist[iz + 10] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1) + 1) * (nz / 2 + 1);
            zonelist[iz + 11] = (i)     * (ny / 2 + 1) * (nz / 2) +
                                (j + 1) * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 12] = 5;
            zonelist[iz + 13] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                (j)     * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 14] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                (j + 1) * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 15] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1) + 1) * (nz / 2 + 1);
            zonelist[iz + 16] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j) + 1) * (nz / 2 + 1);
            zonelist[iz + 17] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j))     * (nz / 2 + 1);

            // Two four sided side faces
            zonelist[iz + 18] = 4;
            zonelist[iz + 19] = (i)     * (ny / 2 + 1) * (nz / 2) +
                                (j)     * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 20] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                (j)     * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 21] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j))     * (nz / 2 + 1);
            zonelist[iz + 22] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j))     * (nz / 2 + 1);
            zonelist[iz + 23] = 4;
            zonelist[iz + 24] = (i)     * (ny / 2 + 1) * (nz / 2) +
                                (j + 1) * (nz / 2) + (nz / 2) - 1;
            zonelist[iz + 25] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1) + 1) * (nz / 2 + 1);
            zonelist[iz + 26] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1) + 1) * (nz / 2 + 1);
            zonelist[iz + 27] = (i + 1) * (ny / 2 + 1) * (nz / 2) +
                                (j + 1) * (nz / 2) + (nz / 2) - 1;

            // Two four sided top faces
            zonelist[iz + 28] = 4;
            zonelist[iz + 29] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j))     * (nz / 2 + 1);
            zonelist[iz + 30] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j))     * (nz / 2 + 1);
            zonelist[iz + 31] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j) + 1) * (nz / 2 + 1);
            zonelist[iz + 32] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j) + 1) * (nz / 2 + 1);
            zonelist[iz + 33] = 4;
            zonelist[iz + 34] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1))     * (nz / 2 + 1);
            zonelist[iz + 35] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1))     * (nz / 2 + 1);
            zonelist[iz + 36] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1) + 1) * (nz / 2 + 1);
            zonelist[iz + 37] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                     ((2 * j + 1) + 1) * (nz / 2 + 1);

            iz += 38;
        }
    }

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz / 2; k++)
            {
                zonelist[iz]     = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                        (j)     * (nz / 2 + 1) + k;
                zonelist[iz + 1] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                        (j + 1) * (nz / 2 + 1) + k;
                zonelist[iz + 2] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                        (j + 1) * (nz / 2 + 1) + k;
                zonelist[iz + 3] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                        (j)     * (nz / 2 + 1) + k;
                zonelist[iz + 4] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                        (j)     * (nz / 2 + 1) + k + 1;
                zonelist[iz + 5] = in + (i)     * (ny + 1) * (nz / 2 + 1) +
                                        (j + 1) * (nz / 2 + 1) + k + 1;
                zonelist[iz + 6] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                        (j + 1) * (nz / 2 + 1) + k + 1;
                zonelist[iz + 7] = in + (i + 1) * (ny + 1) * (nz / 2 + 1) +
                                        (j)     * (nz / 2 + 1) + k + 1;
                iz += 8;
            }
        }
    }

    //
    // Create the density and pressure arrays.
    //
    var1name = "d";
    var2name = "p";
    d = ALLOC_N (float, nzones);
    p = ALLOC_N (float, nzones);

    iz = 0;
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny / 2; j++)
        {
            for (k = 0; k < nz / 2 - 1; k++)
            {
                d[iz] = 2.15 + (k * 0.3);
                iz++;
            }
        }
    }
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny / 2; j++)
        {
            d[iz] = 3.35;
            iz++;
        }
    }
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = 0; k < nz / 2; k++)
            {
                d[iz] = 3.65 + (k * 0.3);
                iz++;
            }
        }
    }
    for (i = 0; i < nzones; i++)
    {
        p[i] = 1. / (d[i] + .0001);
    }

    //
    // Create the velocity component arrays.
    //
    var3name = "u";
    var4name = "v";
    var5name = "w";
    u = ALLOC_N (float, nnodes);
    v = ALLOC_N (float, nnodes);
    w = ALLOC_N (float, nnodes);

    xcenter = 0.;
    ycenter = 3.5;
    zcenter = 20.;
    for (i = 0; i < nnodes; i++)
    {
        dist = sqrt((x[i] - xcenter) * (x[i] - xcenter) +
                    (y[i] - ycenter) * (y[i] - ycenter) +
                    (z[i] - zcenter) * (z[i] - zcenter));
        u[i] = (x[i] - xcenter) / dist;
        v[i] = (y[i] - ycenter) / dist;
        w[i] = (z[i] - zcenter) / dist;
    }

    //
    // Create the temperature array.
    //
    t = ALLOC_N (float, nnodes);
    Ca = 1.;
    Cb = 3.;
    Cc = 5.;
    Cd = 7.;
    for (i = 0; i < nnodes; i++)
    {
        t[i] = Ca + Cb * ((y[i] - Ymin) / (Ymax - Ymin)) +
                Cc * exp(Cd * ((z[i] - Zmin) / (Zmax - Zmin)));
    }

    //
    // Create the material array.
    //
    matname = "mat1";
    matlist = ALLOC_N (int, nzones);
    nmats = 3;
    matnos[0] = 1;
    matnos[1] = 2;
    matnos[2] = 3;
    mixlen = 0;

    //
    // Put in the material in 3 shells.
    //
    in = 0;
    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny / 2; j++)
        {
            for (k = 0; k < nz / 2 - 1; k++)
            {
                matlist[in] = 1;
                in++;
            }
        }
    }

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny / 2; j++)
        {
            matlist[in] = 2;
            in++;
        }
    }

    for (i = 0; i < nx; i++)
    {
        for (j = 0; j < ny; j++)
        {
            for (k = nz / 2; k < nz; k++)
            {
                matlist[in] = 3;
                in++;
            }
        }
    }

    //
    // Calculate the external face list.
    //
    fl = DBCalcExternalFacelist2(zonelist, nnodes, lo_offset, hi_offset, 0,
                                 zshapetype, zshapesize, zshapecnt, nzshapes,
                                 matlist, 1);

    nfaces = fl->nfaces;
    nfshapes = fl->nshapes;
    for (i = 0; i < nfshapes; i++)
    {
        fshapecnt[i]  = fl->shapecnt[i];
        fshapesize[i] = fl->shapesize[i];
    }
    lfacelist = fl->lnodelist;
    facelist = ALLOC_N (int, lfacelist);
    for (i = 0; i < lfacelist; i++)
        facelist[i] = fl->nodelist[i];
    zoneno = ALLOC_N (int, nfaces);
    for (i = 0; i < nfaces; i++)
        zoneno[i] = fl->zoneno[i];

    DBFreeFacelist(fl);

    //
    // Write out the variables.
    //
    cycle = 48;
    time = 4.8;
    dtime = 4.8;

    char *info = "mesh ucdmesh3d;pseudocolor d";
    i = strlen(info) + 1;
    DBWrite (dbfile, "_meshtvinfo", info, &i, 1, DB_CHAR);

    char *defvars = "vel vector {u,v,w};speed scalar sqrt(u*u+v*v+w*w)";
    i = strlen(defvars) + 1;
    DBWrite (dbfile, "_meshtv_defvars", defvars, &i, 1, DB_CHAR);

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

    (void)DBPutFacelist(dbfile, "fl1", nfaces, 3, facelist, lfacelist, 0,
                        zoneno, fshapesize, fshapecnt, nfshapes,
                        NULL, NULL, 0);

    (void)DBPutZonelist2(dbfile, "zl1", nzones, 3, zonelist, lzonelist, 0,
                         lo_offset, hi_offset, zshapetype, zshapesize,
                         zshapecnt, nzshapes, NULL);

    (void)DBPutUcdmesh(dbfile, meshname, 3, coordnames, coords,
                       nnodes, nzones, "zl1", "fl1", DB_FLOAT, optlist);

    vars[0] = d;
    varnames[0] = var1name;

    (void)DBPutUcdvar(dbfile, var1name, meshname, 1, varnames, vars,
                      nzones, NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

    vars[0] = p;
    varnames[0] = var2name;

    (void)DBPutUcdvar(dbfile, var2name, meshname, 1, varnames, vars,
                      nzones, NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);

    vars[0] = u;
    varnames[0] = var3name;

    (void)DBPutUcdvar(dbfile, var3name, meshname, 1, varnames, vars,
                      nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

    vars[0] = v;
    varnames[0] = var4name;

    (void)DBPutUcdvar(dbfile, var4name, meshname, 1, varnames, vars,
                      nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

    vars[0] = w;
    varnames[0] = var5name;

    (void)DBPutUcdvar(dbfile, var5name, meshname, 1, varnames, vars,
                      nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

    vars[0]     = t;
    varnames[0] = "t";
    (void)DBPutUcdvar(dbfile, "t", meshname, 1, varnames, vars,
                      nnodes, NULL, 0, DB_FLOAT, DB_NODECENT, optlist);

    (void)DBPutMaterial(dbfile, matname, meshname, nmats, matnos,
                        matlist, &nzones, 1, NULL, NULL, NULL,
                        NULL, mixlen, DB_FLOAT, optlist);

    DBFreeOptlist(optlist);

    //
    // Free the temporary storage.
    //
    FREE (x);
    FREE (y);
    FREE (z);
    FREE (d);
    FREE (p);
    FREE (u);
    FREE (v);
    FREE (w);
    FREE (t);
    FREE (matlist);
    FREE (zonelist);
    FREE (facelist);
    FREE (zoneno);
}

static void
MakeFiles(int size, int type)
{
    DBfile        *dbfile;
    char           filename[256];

    //
    // Create the 2D rectilinear data file.
    //
    sprintf(filename, "rect2d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "2D rectilinear test file", type);
    build_rect2d(dbfile, size);
    DBClose(dbfile);

    //
    // Create the 2D curvilinear data file.
    //
    sprintf(filename, "curv2d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "2D curvilinear test file", type);
    build_curv2d(dbfile, size, DB_ROWMAJOR);
    DBClose(dbfile);

    //
    // Create a column major 2D curvilinear data file.
    //
    sprintf(filename, "curv2d_colmajor.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "2D curvilinear test file", type);
    build_curv2d(dbfile, size, DB_COLMAJOR);
    DBClose(dbfile);

    //
    // Create the 2D ucd data file.
    //
    sprintf(filename, "ucd2d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "2D ucd test file", type);
    build_ucd2d(dbfile, size);
    DBClose(dbfile);

    //
    // Create the 2D ucd lines data file.
    //
    sprintf(filename, "ucd_lines2d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "2D ucd lines test file", type);
    build_ucd2d_lines(dbfile, size);
    DBClose(dbfile);

    //
    // Create the 3D rectilinear data file.
    //
    sprintf(filename, "rect3d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "3D rectilinear test file", type);
    build_rect3d(dbfile, size);
    DBClose(dbfile);

    //
    // Create the 3D curvillinear data file.
    //
    sprintf (filename, "curv3d.silo") ;
    printf ("   %s\n", filename) ;
    dbfile = DBCreate(filename, 0, DB_LOCAL, "3D curvilinear test file", type);
    build_curv3d (dbfile, size, DB_ROWMAJOR) ;
    DBClose (dbfile) ;

    //
    // Create the column major 3D curvillinear data file.
    //
    sprintf (filename, "curv3d_colmajor.silo") ;
    printf ("   %s\n", filename) ;
    dbfile = DBCreate(filename, 0, DB_LOCAL, "3D curvilinear test file", type);
    build_curv3d (dbfile, size, DB_COLMAJOR) ;
    DBClose (dbfile) ;

    //
    // Create the 3D ucd data file.
    //
    sprintf(filename, "ucd3d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "3D ucd test file", type);
    build_ucd3d(dbfile, size);
    DBClose(dbfile);

    //
    // Create the 3D ucd data file.
    //
    sprintf(filename, "ucd_lines3d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "3D ucd lines test file", type);
    build_ucd3d_lines(dbfile, size);
    DBClose(dbfile);


    //
    // Create the 3d arbitrary polyhedra data file.
    //
    sprintf(filename, "poly3d.silo");
    printf("   %s\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "3D polyhedra test file", type);
    build_poly3d(dbfile, size);
    DBClose(dbfile);
}

int
main(int argc, char *argv[])
{
    int            i;
    int            size;
    int            ntests = 0;
    int            driver = DB_PDB;

    //
    // parse the execute line.
    //
    size = 1;
    for (i = 1; i < argc; i++)
    {
        if (strcmp (argv[i], "-small") == 0)
            size = 1;
        else if (strcmp (argv[i], "-medium") == 0)
            size = 2;
        else if (strcmp (argv[i], "-large") == 0)
            size = 3;
        else if (strcmp(argv[i], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[i], "DB_PDB") == 0)
            driver = DB_PDB;
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[i]);
    }

    //
    // Abort on any errors.
    //
    DBShowErrors(DB_ABORT, NULL);

    MakeFiles(size, driver);
    ntests++;

    if (!ntests)
        printf("No tests performed.\n");

    return(0);
}
