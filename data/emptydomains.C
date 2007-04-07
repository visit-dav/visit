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

// Programmer: Jeremy Meredith
// Date:       June  7, 2005
//
// Purpose:    Make a dataset with empty domains, using the
//             new "EMPTY" keyword for the multi-objects.
//
//
// Modifications:
//
#include <silo.h>
#include <math.h>
#include <stdlib.h>

#define NX 12
#define NY 4
#define NZ 8
#define NPTS (NX*NY*NZ)
#define NZONES ((NX-1)*(NY-1)*(NZ-1))

int main()
{
    char *meshnames[2] = { "EMPTY", "domain1/mesh" };
    int meshtypes[2] = { DB_QUADMESH, DB_QUADMESH };

    char *varnames[2] = { "EMPTY", "domain1/density" };
    int vartypes[2] = { DB_QUADVAR, DB_QUADVAR };

    char *matnames[2] = { "EMPTY", "domain1/mat" };

    char *specnames[2] = { "EMPTY", "domain1/spec" };

    int matnos[2] = {1, 5};
    int nmatspec[2] = {1, 2};
    float specmf[4] = {.1, .9, .4, .6};
    int *matlist;
    int *speclist;
    float *x, *y, *z, *v;
    float *coords[3];
    DBfile *db;
    int dims[3], zdims[3];
    int ndims;
    int i;
    DBoptlist *opt;

    x = new float[NX];
    y = new float[NY];
    z = new float[NZ];
    v = new float[NPTS];
    matlist = new int[NZONES];
    speclist = new int[NZONES];
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    for (i = 0 ; i < NX; i++)
        x[i] = i;

    for (i = 0 ; i < NY; i++)
        y[i] = i;

    for (i = 0 ; i < NZ; i++)
        z[i] = i;
    
    for (i = 0 ; i < NPTS; i++)
        v[i] = sqrt((float)(i));

    for (i = 0 ; i < NZONES; i++)
    {
        int matindex = (i/17)%2;
        matlist[i] = matnos[matindex];
        speclist[i] = matindex == 0 ? 0 : ((i > (NZONES/2)) ? 1 : 3);
    }

    dims[0] = NX;
    dims[1] = NY;
    dims[2] = NZ;
    zdims[0] = NX-1;
    zdims[1] = NY-1;
    zdims[2] = NZ-1;
    ndims = 3;

    db = DBCreate("emptydomains.silo", DB_CLOBBER, DB_LOCAL, "test empty domains", DB_PDB);

    DBMkDir(db, "domain1");
    DBSetDir(db, "domain1");
    DBPutQuadmesh(db, "mesh", NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, NULL);

    DBPutQuadvar1(db, "density", "mesh", v, dims, ndims, NULL,0,
                  DB_FLOAT, DB_NODECENT, NULL);
    DBPutMaterial(db, "mat", "mesh", 2,matnos, matlist, zdims, ndims, NULL,NULL,NULL,NULL,0,
                  DB_FLOAT, NULL);

    DBPutMatspecies(db, "spec", "mat", 2,nmatspec, speclist, zdims, ndims, 4,specmf, NULL,0,
                    DB_FLOAT, NULL);
    DBSetDir(db, "..");

#if 1
    DBPutMultimesh(db, "mesh", 2, meshnames, meshtypes, NULL);
    DBPutMultivar(db, "density", 2, varnames, vartypes, NULL);
    DBPutMultimat(db, "mat", 2, matnames, NULL);

    opt = DBMakeOptlist(1);
    char *matname = "mat";
    DBAddOption(opt, DBOPT_MATNAME, matname);
    DBPutMultimatspecies(db, "spec", 2, specnames, opt);
#endif

    DBClose(db);

    return 0;
}
