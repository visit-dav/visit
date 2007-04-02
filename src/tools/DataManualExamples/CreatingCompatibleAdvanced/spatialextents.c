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

#include <silo.h>
#include <stdio.h>
#include <string.h>

#define NZDOMS 5
#define NYDOMS 5
#define NXDOMS 5

#define NZ 20
#define NY 15
#define NX 10

#define XSIZE 5.
#define YSIZE 5.
#define ZSIZE 5.

void
write_domains(DBfile *dbfile, double *spatial_extents, double *data_extents)
{
    float var[(NX-1) * (NY-1) * (NZ-1)];
    int dims[] = {NX, NY, NZ};
    int vardims[] = {NX-1, NY-1, NZ-1};
    int dom, zdom, ydom, xdom;
    int i, j, k, ndims = 3;

    /* Create each of the domain meshes in a separate directory. */
    dom = 0;
    for(zdom = 0; zdom < NZDOMS; ++zdom)
    for(ydom = 0; ydom < NYDOMS; ++ydom)
    for(xdom = 0; xdom < NXDOMS; ++xdom, ++dom)
    {
        float xc[NX], yc[NY], zc[NZ];
        float *coords[] = {xc, yc, zc};
        int index = 0;
        float xstart, xend, ystart, yend, zstart, zend;
        int xzones, yzones, zzones, nzones;
        int xnodes, ynodes, znodes;

        double *sdomextents = spatial_extents + 6 * dom;
        double *ddomextents = data_extents + 2 * dom;

        /* Create a new directory. */
        char dirname[100];
        sprintf(dirname, "Domain%03d", dom);
        DBMkDir(dbfile, dirname);
        DBSetDir(dbfile, dirname);

        /* Determine default start, end coordinates */
        xstart = (float)xdom * XSIZE;
        xend   = (float)(xdom+1) * XSIZE;
        xzones = NX-1;
        ystart = (float)ydom * YSIZE;
        yend   = (float)(ydom+1) * YSIZE;
        yzones = NY-1;
        zstart = (float)zdom * ZSIZE;
        zend   = (float)(zdom+1) * ZSIZE;
        zzones = NZ-1;

        xnodes = xzones + 1;
        ynodes = yzones + 1;
        znodes = zzones + 1;

        nzones = xzones*yzones*zzones;
        dims[0] = xnodes;
        dims[1] = ynodes;
        dims[2] = znodes;

        vardims[0] = xzones;
        vardims[1] = yzones;
        vardims[2] = zzones;

        /* Create the mesh coordinates. */
        for(i = 0; i < xnodes; ++i)
        {
            float t = (float)i / (float)(xnodes-1);
            xc[i] = (1.-t)*xstart + t*xend;
        }
        for(i = 0; i < ynodes; ++i)
        {
            float t = (float)i / (float)(ynodes-1);
            yc[i] = (1.-t)*ystart + t*yend;
        }
        for(i = 0; i < znodes; ++i)
        {
            float t = (float)i / (float)(znodes-1);
            zc[i] = (1.-t)*zstart + t*zend;
        }

        /* Create the variable value. */
        for(k = 0; k < zzones; ++k)
        {
            for(j = 0; j < yzones; ++j)
            {
                for(i = 0; i < xzones; ++i)
                {
                    float dx, dy, dz;
                    dx = xc[i] - 5.;
                    dy = yc[j] - 5.;
                    dz = zc[k] - 5.;
                    var[index++] = (float)sqrt(dx*dx + dy*dy + dz*dz);
                }
            }
        }

        /* Figure out the spatial extents for the domain. */
        sdomextents[0] = xc[0];        /* min x */
        sdomextents[1] = yc[0];        /* min y */
        sdomextents[2] = zc[0];        /* min z */
        sdomextents[3] = xc[xnodes-1]; /* max x */
        sdomextents[4] = yc[ynodes-1]; /* max y */
        sdomextents[5] = zc[znodes-1]; /* max z */

        /* Figure out the data extents for the domain. */
        ddomextents[0] = ddomextents[1] = var[0];
        for(i = 1; i < (NX-1)*(NY-1)*(NZ-1); ++i)
        {
            ddomextents[0] = (var[i] < ddomextents[0]) ? var[i] : ddomextents[0];
            ddomextents[1] = (var[i] > ddomextents[1]) ? var[i] : ddomextents[1];
        }

        /* Write a rectilinear mesh. */
        DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, NULL);

        /* Write a node-centered var. */
        DBPutQuadvar1(dbfile, "var", "quadmesh", var, vardims,
                      ndims, NULL, 0, DB_FLOAT, DB_ZONECENT, NULL);

        /* Go back to the top directory. */
        DBSetDir(dbfile, "..");
    }
}

void
write_multimesh(DBfile *dbfile, double *spatial_extents)
{
    char **meshnames = NULL;
    int dom, nmesh = NXDOMS*NYDOMS*NZDOMS, *meshtypes = NULL;
    DBoptlist *optlist = NULL;
    const int six = 6;

    /* Create the list of mesh names. */
    meshnames = (char **)malloc(nmesh * sizeof(char *));
    for(dom = 0; dom < nmesh; ++dom)
    {
        char tmp[100];
        sprintf(tmp, "Domain%03d/quadmesh", dom);
        meshnames[dom] = strdup(tmp);
    }
    /* Create the list of mesh types. */
    meshtypes = (int *)malloc(nmesh * sizeof(int));
    for(dom = 0; dom < nmesh; ++dom)
        meshtypes[dom] = DB_QUAD_RECT;

    /* Write the multimesh. */
    optlist = DBMakeOptlist(2);
    DBAddOption(optlist, DBOPT_EXTENTS_SIZE, (void *)&six);
    DBAddOption(optlist, DBOPT_EXTENTS, (void *)spatial_extents);
    DBPutMultimesh(dbfile, "quadmesh", nmesh, meshnames, meshtypes, optlist);
    DBFreeOptlist(optlist);

    /* Free the memory*/
    for(dom = 0; dom < nmesh; ++dom)
        free(meshnames[dom]);
    free(meshnames);
    free(meshtypes);
}

void
write_multivar(DBfile *dbfile, double *data_extents)
{
    char **varnames = NULL;
    int dom, nvar = NXDOMS*NYDOMS*NZDOMS, *vartypes = NULL;
    DBoptlist *optlist = NULL;
    const int two = 2;

    /* Create the list of var names. */
    varnames = (char **)malloc(nvar * sizeof(char *));
    for(dom = 0; dom < nvar; ++dom)
    {
        char tmp[100];
        sprintf(tmp, "Domain%03d/var", dom);
        varnames[dom] = strdup(tmp);
    }
    /* Create the list of var types. */
    vartypes = (int *)malloc(nvar * sizeof(int));
    for(dom = 0; dom < nvar; ++dom)
        vartypes[dom] = DB_QUADVAR;

    /* Write the multivar. */
    optlist = DBMakeOptlist(2);
    DBAddOption(optlist, DBOPT_EXTENTS_SIZE, (void *)&two);
    DBAddOption(optlist, DBOPT_EXTENTS, (void *)data_extents);
    DBPutMultivar(dbfile, "var", nvar, varnames, vartypes, optlist);
    DBFreeOptlist(optlist);

    /* Free the memory*/
    for(dom = 0; dom < nvar; ++dom)
        free(varnames[dom]);
    free(varnames);
    free(vartypes);
}

int
main(int argc, char *argv[])
{
    /* Array to contain the spatial dimensions. */
    double spatial_extents[NZDOMS][NYDOMS][NXDOMS][6];
    double data_extents[NZDOMS][NYDOMS][NXDOMS][2];

    /* Open the Silo file */
    DBfile *dbfile = DBCreate("spatialextents.silo", DB_CLOBBER, DB_LOCAL,
    "Master file with spatial extents", DB_HDF5);    

    /* Add other Silo calls here. */
    write_domains(dbfile, (double *)spatial_extents, (double *)data_extents);
    write_multimesh(dbfile, (double *)spatial_extents);
    write_multivar(dbfile, (double *)data_extents);

    /* Close the Silo file. */
    DBClose(dbfile);

    return 0;
}
