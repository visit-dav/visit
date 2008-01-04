/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ****************************************************************************
// File: global_node.C
//
// Purpose:
//     This program creates a dataset that has global nodes, but not ghost 
//     zones.
//
// Programmer: Hank Childs
// Creation:   October 5, 2004
//
// ****************************************************************************

#include <stdio.h>
#include <silo.h>
#include <math.h>
#include <vector>
#include <string>
#include <visitstream.h>

void WriteOutMultivars(DBfile *);
void WriteOutDomain(DBfile *, int, int, int);

int NDOMI = 4;
int NNODEI = 64;
int NDOMJ = 4;
int NNODEJ = 64;
int NDOMK = 4;
int NNODEK = 64;

int nmats = 8;

int driver = DB_PDB;

int main(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++)
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
            }
        }
    }

    DBfile *dbfile = DBCreate("global_node.silo", DB_CLOBBER, DB_LOCAL,NULL,driver);

    WriteOutMultivars(dbfile);
    for (int i = 0 ; i < NDOMI ; i++)
        for (int j = 0 ; j < NDOMJ ; j++)
            for (int k = 0 ; k < NDOMK ; k++)
                WriteOutDomain(dbfile, i, j, k);

    DBClose(dbfile);
}

// ****************************************************************************
// Method: WriteOutMultivars
//
// Purpose:
//   Writes out multivar information.  
//
// Programmer: Hank Childs
// Creation:   October 5, 2004
//
// Modifications:
//   Kathleen Bonnell, Wed May 11 17:50:53 PDT 2005
//   Added support for zonal var 'p'.
//
// ****************************************************************************

void
WriteOutMultivars(DBfile *dbfile)
{
    int n_total_domains = NDOMI*NDOMJ*NDOMK;
    char **meshnames  = new char*[n_total_domains];
    char **matnames   = new char*[n_total_domains];
    char **distnames  = new char*[n_total_domains];
    char **pnames     = new char*[n_total_domains];
    int   *meshtypes  = new int[n_total_domains];
    int   *vartypes   = new int[n_total_domains];
   
    for (int i = 0 ; i < n_total_domains ; i++)
    {
        meshnames[i] = new char[128];
        sprintf(meshnames[i], "/block%d/mesh", i);
        matnames[i] = new char[128];
        sprintf(matnames[i], "/block%d/mat", i);
        distnames[i] = new char[128];
        sprintf(distnames[i], "/block%d/dist", i);
        pnames[i] = new char[128];
        sprintf(pnames[i], "/block%d/p", i);
        meshtypes[i] = DB_UCDMESH;
        vartypes[i]  = DB_UCDVAR;
    }
    DBPutMultimesh(dbfile, "mesh", n_total_domains, meshnames, meshtypes,NULL);
    DBPutMultivar(dbfile, "dist", n_total_domains, distnames, vartypes,NULL);
    DBPutMultivar(dbfile, "p", n_total_domains, pnames, vartypes,NULL);
    DBPutMultimat(dbfile, "mat", n_total_domains, matnames, NULL);
}

// ****************************************************************************
// Method: WriteOutDomain
//
// Purpose:
//   Creates and writes out one domain of the mesh. 
//
// Programmer: Hank Childs
// Creation:   October 5, 2004
// 
// Modifications:
//   Kathleen Bonnell, Wed Dec 15 15:03:42 PST 2004
//   Added gzoneno.
//
//   Kathleen Bonnell, Wed May 11 17:50:53 PDT 2005
//   Added support for zonal var 'p'.
//
// ****************************************************************************

void
WriteOutDomain(DBfile *dbfile, int dI, int dJ, int dK)
{
    int  i, j, k;

    int dom = dK*NDOMI*NDOMJ + dJ*NDOMI + dI;
    char dirname[1024];
    sprintf(dirname, "block%d", dom);
    DBMkDir(dbfile, dirname);
    DBSetDir(dbfile, dirname);

    int nx = NNODEI / NDOMI;
    int ny = NNODEJ / NDOMJ;
    int nz = NNODEK / NDOMK;

    float x_start = ((float) dI) / ((float) NDOMI);
    float y_start = ((float) dJ) / ((float) NDOMJ);
    float z_start = ((float) dK) / ((float) NDOMK);
    float rangeX = 1./NDOMI;
    float rangeY = 1./NDOMJ;
    float rangeZ = 1./NDOMK;
    float x_step = rangeX / (nx-1);
    float y_step = rangeY / (ny-1);
    float z_step = rangeZ / (nz-1);

    char *meshname = "mesh";
    char *varname  = "dist";
    char *zvarname  = "p";
    int dims[3];
    dims[0] = nx-1;
    dims[1] = ny-1;
    dims[2] = nz-1;
    int nzones = (nx-1)*(ny-1)*(nz-1);
    int *gzoneno = new int[nzones];
    float *p = new float[nzones];
    int g_istart = dI * (nx-1);
    int g_jstart = dJ * (ny-1);
    int g_kstart = dK * (nz-1);

    //
    // Create zonelist.
    //
    int lzonelist = 8*nzones;
    int *zonelist = new int[lzonelist];
    int zshapetype[1] = { DB_ZONETYPE_HEX };
    int zshapesize[1] = { 8 };
    int zshapecnt[1] = { nzones };
    int idx = 0;
    int gidx = 0;
    for (k = 0 ; k < dims[2] ; k++)
         for (j = 0 ; j < dims[1] ; j++)
            for (i = 0 ; i < dims[0] ; i++)
            {
                int base = k*nx*ny + j*nx + i;


                zonelist[idx++] = base;
                zonelist[idx++] = base+1;
                zonelist[idx++] = base+1+nx;
                zonelist[idx++] = base+nx;
                zonelist[idx++] = base+nx*ny;
                zonelist[idx++] = base+1+nx*ny;
                zonelist[idx++] = base+1+nx+nx*ny;
                zonelist[idx++] = base+nx+nx*ny;

                p[gidx] = i*35.45;

                gzoneno[gidx++] = (k+g_kstart)*(NNODEI-1)*(NNODEJ-1) +
                             (j+g_jstart)*(NNODEI-1) + (i+g_istart);
            }    

    
    DBoptlist *zoptlist = DBMakeOptlist(1);
    DBAddOption(zoptlist, DBOPT_ZONENUM, gzoneno);
    DBPutZonelist2(dbfile, "zl1", nzones, 3, zonelist, lzonelist, 
                  0, 0, 0, zshapetype, zshapesize, zshapecnt, 1, zoptlist);
    DBFreeOptlist(zoptlist);
    delete [] gzoneno;
    delete [] zonelist;

    // 
    // Create the points and the single variable.
    //
    int npts = nx*ny*nz;
    float *x = new float[npts];
    float *y = new float[npts];
    float *z = new float[npts];
    float *dist = new float[npts];
    int *gnodeno = new int[npts];



    for (k = 0 ; k < nz ; k++)
    {
        for (j = 0 ; j < ny ; j++)
        {
            for (i = 0 ; i < nx ; i++)
            {
                int index = k*ny*nx + j*nx + i;
                x[index] = x_start + i*x_step;
                y[index] = y_start + j*y_step;
                z[index] = z_start + k*z_step;
                dist[index] = (x[index]-0.5)*(x[index]-0.5)
                            + (y[index]-0.5)*(y[index]-0.5)
                            + (z[index]-0.5)*(z[index]-0.5);
                dist[index] = sqrt(dist[index]);
                gnodeno[index] = (k+g_kstart)*NNODEI*NNODEJ +
                                 (j+g_jstart)*NNODEI + (i+g_istart);
            }
        }
    }
  
    float *coords[3];
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    DBoptlist *optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_NODENUM, gnodeno);
    DBPutUcdmesh(dbfile, meshname, 3, NULL, coords, npts, nzones, "zl1",
                  NULL, DB_FLOAT, optlist);
    DBPutUcdvar1(dbfile, varname, meshname, dist, npts, NULL, 0,
                  DB_FLOAT, DB_NODECENT, NULL);
    DBPutUcdvar1(dbfile, zvarname, meshname, p, nzones, NULL, 0,
                  DB_FLOAT, DB_ZONECENT, NULL);
    DBFreeOptlist(optlist);
    delete [] gnodeno;

    //
    // Create the materials.
    //
    int *matnos = new int[nmats];
    for (int i = 0 ; i < nmats ; i++)
    {
        matnos[i] = i+1;
    }

    int *matlist = new int[nzones];
    for (int i = 0 ; i < dims[0] ; i++)
        for (int j = 0 ; j < dims[1] ; j++)
            for (int k = 0 ; k < dims[2] ; k++)
            {
                int index = k*ny*nx + j*nx + i;
                float pt[3];
                pt[0] = fabs(0.5-x[index]);
                pt[1] = fabs(0.5-y[index]);
                pt[2] = fabs(0.5-z[index]);
                float prod = pt[0]*pt[1]*pt[2];
                // Max value of prod is 1/8, so multiply by 8.
                float frac = prod * 8. * nmats;
                int mat = (int) frac;
                if (mat >= nmats)
                   mat = nmats-1;
                if (mat <= 0)
                   mat = 1;
                int index2 = k*(ny-1)*(nx-1) + j*(nx-1) + i;
                matlist[index2] = mat;
            }

    DBPutMaterial(dbfile, "mat", meshname, nmats, matnos, matlist, dims, 3,
                  NULL, NULL, NULL, NULL, 0, DB_FLOAT, NULL);
    DBSetDir(dbfile, "..");

    delete [] x;
    delete [] y;
    delete [] z;
    delete [] dist;
    delete [] p;
    delete [] matnos;
    delete [] matlist;
}

