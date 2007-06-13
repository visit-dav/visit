/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt denribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redenribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redenributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redenributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the denribution.
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

// ****************************************************************************
// File: thinplane.C
//
// Purpose:
//     This program creates a dataset that has a thin plane in it, to stress
//     material interface reconstruction.
//
// NOTE:    This file was copied from bigsil.C and took routines directly
//          from that file.
//
// Programmer: Hank Childs
// Creation:   June 13, 2007
//
// ****************************************************************************

#include <stdio.h>
#include <silo.h>
#include <math.h>
#include <vector>
#include <string>
#include <visitstream.h>

using std::vector;

void WriteOutMultivars(DBfile *);
void WriteOutDomain(DBfile *, int, int);
void WriteOutConnectivity(DBfile *);
void WriteOutDecomposition(DBfile *);

int nx = 8;
int ny = 8;
int nz = 8;
int ndomains_per_dim = 3;
int n_total_domains = ndomains_per_dim*ndomains_per_dim*ndomains_per_dim;
int ndomains_per_group = ndomains_per_dim*ndomains_per_dim;
int driver = DB_PDB;

int main(int argc, char *argv[])
{
    for (int args = 1 ; args < argc ; args++)
    {
        if (strcmp(argv[args], "-driver") == 0)
        {
            args++;

            if (strcmp(argv[args], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[args], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[args]);
            }
        }
    }
    n_total_domains = ndomains_per_dim*ndomains_per_dim*ndomains_per_dim;
    ndomains_per_group = ndomains_per_dim*ndomains_per_dim;

    DBfile *dbfile = DBCreate("thinplane.silo", DB_CLOBBER, DB_LOCAL,NULL,driver);

    WriteOutMultivars(dbfile);

    for (int i = 0 , groupNum = -1; i < n_total_domains ; i++)
    {
        if (i%(ndomains_per_group) == 0)
            groupNum++;

        WriteOutDomain(dbfile, i, groupNum);
    }

    WriteOutConnectivity(dbfile);
    WriteOutDecomposition(dbfile);

    DBClose(dbfile);
}

// ****************************************************************************
// Method: WriteOutDecomposition
//
// Purpose:
//   Writes out group information to a Decomposition directory. 
//
// Programmer: Hank Childs
// Creation:   June 13, 2007
// 
// ****************************************************************************

void
WriteOutDecomposition(DBfile *dbfile)
{
    int dims = 1;
    int ndims = 1;
    int val = ndomains_per_dim;
    if (DBMkDir(dbfile, "Decomposition") == -1)
        return;
    DBSetDir(dbfile, "Decomposition");
    DBWrite(dbfile, "NumBlocks", &val, &dims, ndims, DB_INT);
    char dirname[48];

    int *groupIds = new int[n_total_domains];
  
    for (int i = 0, groupNum = -1; i < n_total_domains; i++)
    {
        if (i%ndomains_per_group == 0)
            groupNum++;
        sprintf(dirname, "Domain_%d", i);
        DBMkDir(dbfile, dirname); 
        DBSetDir(dbfile, dirname); 
        DBWrite(dbfile, "BlockNum", &groupNum, &dims, ndims, DB_INT);
        DBSetDir(dbfile, "../"); 
        groupIds[i] = groupNum;
    }
    DBSetDir(dbfile, "../");

    dims = n_total_domains;
    DBWrite(dbfile, "_visit_domain_groups", groupIds, &dims, ndims, DB_INT);

    delete [] groupIds;
}


// ****************************************************************************
// Method: WriteOutConnectivity
//
// Purpose:
//   Writes out connectivity information.  
//
// Programmer: Hank Childs
// Creation:   June 13, 2007
//
// ****************************************************************************

void
WriteOutConnectivity(DBfile *dbfile)
{
    int dims  = 1;
    int ndims = 1;
    int val = n_total_domains;
    DBWrite(dbfile, "ConnectivityNumDomains", &val, &dims, ndims, DB_INT);

    int *nneighbors = new int[n_total_domains];
    int lneighbors = 0;
    for (int i = 0 ; i < ndomains_per_dim ; i++)
    {
        for (int j = 0 ; j < ndomains_per_dim ; j++)
        {
            for (int k = 0 ; k < ndomains_per_dim ; k++)
            {
                int index = k*ndomains_per_dim*ndomains_per_dim +
                            j*ndomains_per_dim + i;
                nneighbors[index] = 0;
                // FACES
                nneighbors[index] += (i==0 || i == ndomains_per_dim-1 ? 1 : 2);
                nneighbors[index] += (j==0 || j == ndomains_per_dim-1 ? 1 : 2);
                nneighbors[index] += (k==0 || k == ndomains_per_dim-1 ? 1 : 2);
                // EDGES
                int nedges = 0;
                nedges += (i==0 || i == ndomains_per_dim-1 ? 1 : 0);
                nedges += (j==0 || j == ndomains_per_dim-1 ? 1 : 0);
                nedges += (k==0 || k == ndomains_per_dim-1 ? 1 : 0);
                nneighbors[index] += (nedges == 0 ? 12 : (nedges == 1 ? 8 :
                                       (nedges == 2 ? 5 : 3)));
                // VERTICES
                int nverts = 0;
                nverts += (i==0 || i == ndomains_per_dim-1 ? 1 : 0);
                nverts += (j==0 || j == ndomains_per_dim-1 ? 1 : 0);
                nverts += (k==0 || k == ndomains_per_dim-1 ? 1 : 0);
                nneighbors[index] += (nverts == 0 ? 8 : (nverts == 1 ? 4 :
                                       (nverts == 2 ? 2 : 1)));
                lneighbors += nneighbors[index]*11;
            }
        }
    }
    dims = n_total_domains;
    DBWrite(dbfile, "ConnectivityNumNeighbors", nneighbors, &dims, ndims,
            DB_INT);
    dims = 1;
    DBWrite(dbfile, "ConnectivityLenNeighbors", &lneighbors, &dims, ndims,
            DB_INT);

    dims = n_total_domains*6;
    int *exts = new int[n_total_domains*6];
    for (int i = 0 ; i < ndomains_per_dim ; i++)
    {
        for (int j = 0 ; j < ndomains_per_dim ; j++)
        {
            for (int k = 0 ; k < ndomains_per_dim ; k++)
            {
                int index = k*ndomains_per_dim*ndomains_per_dim +
                            j*ndomains_per_dim + i;
                exts[6*index]   = 1;
                exts[6*index+1] = nx;
                exts[6*index+2] = 1;
                exts[6*index+3] = ny;
                exts[6*index+4] = 1;
                exts[6*index+5] = nz;
            }
        }
    }
    DBWrite(dbfile, "ConnectivityExtents", exts, &dims, ndims, DB_INT);
    delete [] exts;

    int *neighbors = new int[lneighbors*11];
    int offset = 0;
    int last = 0;
    int ndomains = ndomains_per_dim*ndomains_per_dim*ndomains_per_dim;
    vector<int> neighbor_index(ndomains);
    for (int n = 0 ; n < ndomains ; n++)
        neighbor_index[n] = 0;
    for (int k = 0 ; k < ndomains_per_dim ; k++)
    {
        for (int j = 0 ; j < ndomains_per_dim ; j++)
        {
            for (int i = 0 ; i < ndomains_per_dim ; i++)
            {
                int index = k*ndomains_per_dim*ndomains_per_dim +
                            j*ndomains_per_dim + i;
                // FRONT, BOTTOM, LEFT NODE
                if (i > 0 && j > 0 && k > 0)
                {
                    neighbors[offset+0] = index - 1 - ndomains_per_dim
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // FRONT, BOTTOM EDGE
                if (j > 0 && k > 0)
                {
                    neighbors[offset+0] = index - ndomains_per_dim
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // FRONT, BOTTOM, RIGHT NODE
                if (i < ndomains_per_dim-1 && j > 0 && k > 0)
                {
                    neighbors[offset+0] = index + 1 - ndomains_per_dim
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // LEFT, FRONT EDGE
                if (i > 0 && k > 0)
                {
                    neighbors[offset+0] = index - 1
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // FRONT FACE
                if (k > 0)
                {
                    neighbors[offset+0] = index -
                                          ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // RIGHT, FRONT EDGE
                if (i < ndomains_per_dim-1 && k > 0)
                {
                    neighbors[offset+0] = index + 1
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // FRONT, TOP, LEFT NODE
                if (i > 0 && j < ndomains_per_dim-1 && k > 0)
                {
                    neighbors[offset+0] = index - 1 + ndomains_per_dim
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // FRONT, TOP EDGE
                if (j < ndomains_per_dim-1 && k > 0)
                {
                    neighbors[offset+0] = index + ndomains_per_dim
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // FRONT, TOP, RIGHT NODE
                if (i < ndomains_per_dim-1 && j < ndomains_per_dim-1 && k > 0)
                {
                    neighbors[offset+0] = index + 1 + ndomains_per_dim
                                          - ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = 1;
                    offset += 11;
                }
                // BOTTOM, LEFT EDGE
                if (i > 0 && j > 0)
                {
                    neighbors[offset+0] = index - 1 - ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BOTTOM FACE
                if (j > 0)
                {
                    neighbors[offset+0] = index - ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BOTTOM, RIGHT EDGE
                if (i < ndomains_per_dim-1 && j > 0)
                {
                    neighbors[offset+0] = index + 1 - ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // LEFT FACE
                if (i > 0)
                {
                    neighbors[offset+0] = index - 1;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // RIGHT FACE
                if (i < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + 1;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // TOP, LEFT EDGE
                if (i > 0 && j < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index - 1 + ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // TOP FACE
                if (j < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // TOP, RIGHT EDGE
                if (i < ndomains_per_dim-1 && j < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + 1 + ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = 1;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BACK, BOTTOM, LEFT NODE
                if (i > 0 && j > 0 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index - 1 - ndomains_per_dim
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BACK, BOTTOM EDGE
                if (j > 0 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index - ndomains_per_dim
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BACK, BOTTOM, RIGHT NODE
                if (i < ndomains_per_dim-1 && j > 0 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + 1 - ndomains_per_dim
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = 1;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // LEFT, BACK EDGE
                if (i > 0 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index - 1
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BACK FACE
                if (k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index +
                                          ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // RIGHT, BACK EDGE
                if (i < ndomains_per_dim-1 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + 1
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = 1;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BACK, TOP, LEFT NODE
                if (i > 0 && j < ndomains_per_dim-1 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index - 1 + ndomains_per_dim
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = 1;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // FRONT, TOP EDGE
                if (j < ndomains_per_dim-1 && k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + ndomains_per_dim
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = 1;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                // BACK, TOP, RIGHT NODE
                if (i < ndomains_per_dim-1 && j < ndomains_per_dim-1 &&
                    k < ndomains_per_dim-1)
                {
                    neighbors[offset+0] = index + 1 + ndomains_per_dim
                                          + ndomains_per_dim*ndomains_per_dim;
                    neighbors[offset+1] = neighbor_index[neighbors[offset]];
                    neighbor_index[neighbors[offset]]++;
                    neighbors[offset+2] = 1;
                    neighbors[offset+3] = 2;
                    neighbors[offset+4] = 3;
                    neighbors[offset+5] = nx;
                    neighbors[offset+6] = nx;
                    neighbors[offset+7] = ny;
                    neighbors[offset+8] = ny;
                    neighbors[offset+9] = nz;
                    neighbors[offset+10] = nz;
                    offset += 11;
                }
                if (nneighbors[index] != ((offset-last)/11))
                    cerr << "Did not match for " << i << ", " << j << ", " <<k
                         << endl << "NNeiighbors = " << nneighbors[index]
                         << endl << "diff = " << ((offset-last)/11) << endl;
                last = offset;
            }
        }
    }
    dims = lneighbors;
    DBWrite(dbfile, "ConnectivityNeighbors", neighbors, &dims, ndims, DB_INT);
    delete [] neighbors;
    delete [] nneighbors;
}

// ****************************************************************************
// Method: WriteOutMultivars
//
// Purpose:
//   Writes out multivar information.  
//
// Programmer: Hank Childs
// Creation:   June 13, 2007
//
// ****************************************************************************

void
WriteOutMultivars(DBfile *dbfile)
{
    char **meshnames  = new char*[n_total_domains];
    char **matnames   = new char*[n_total_domains];
    char **dennames  = new char*[n_total_domains];
    int   *meshtypes  = new int[n_total_domains];
    int   *vartypes  = new int[n_total_domains];

    int nGroups  = ndomains_per_dim; 
    int groupNum = -1;
    for (int i = 0 ; i < n_total_domains ; i++)
    {
        char filename[128] = { '\0' };

        if (i%(ndomains_per_group) == 0)
            groupNum++;

        meshnames[i] = new char[128];
        sprintf(meshnames[i], "%s/block%d/mesh", filename, i);
        matnames[i] = new char[128];
        sprintf(matnames[i], "%s/block%d/mat", filename, i);
        dennames[i] = new char[128];
        sprintf(dennames[i], "%s/block%d/den", filename, i);
        meshtypes[i] = DB_QUADMESH;
        vartypes[i]  = DB_QUADVAR;
    }
    DBoptlist *optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_NGROUPS, &nGroups);
    DBPutMultimesh(dbfile, "mesh", n_total_domains, meshnames, meshtypes,optlist);
    DBPutMultivar(dbfile, "den", n_total_domains, dennames, vartypes,optlist);
    DBPutMultimat(dbfile, "mat", n_total_domains, matnames, optlist);
    DBFreeOptlist(optlist);
    for (int i = 0, groupNum = -1; i < n_total_domains ; i++)
    {
        delete [] meshnames[i];
        delete [] matnames[i];
        delete [] dennames[i];
    }
    delete [] meshnames;
    delete [] matnames;
    delete [] dennames;
    delete [] meshtypes;
    delete [] vartypes;
}

// ****************************************************************************
// Method: WriteOutDomain
//
// Purpose:
//   Creates and writes out one domain of the mesh. 
//
// Programmer: Hank Childs
// Creation:   June 13, 2007
//
// ****************************************************************************

void
WriteOutDomain(DBfile *dbfile, int index, int groupNum)
{
    char dirname[1024];
    sprintf(dirname, "block%d", index);
    DBMkDir(dbfile, dirname);
    DBSetDir(dbfile, dirname);

    char *meshname = "mesh";
    char *varname  = "den";
    int dims[3];
    dims[0] = nx;
    dims[1] = ny;
    dims[2] = nz;

    int npts = nx*ny*nz;
    int ncells = (nx-1)*(ny-1)*(nz-1);
    float *x = new float[npts];
    float *y = new float[npts];
    float *z = new float[npts];
    float *den = new float[ncells];

    int x_block = index % ndomains_per_dim;
    int y_block = (index/ndomains_per_dim) % ndomains_per_dim;
    int z_block = (index/(ndomains_per_dim*ndomains_per_dim));

    int baseIndex[3];
    baseIndex[0] = (x_block == 0 ? 1 : x_block *nx );
    baseIndex[1] = (y_block == 0 ? 1 : y_block *ny );
    baseIndex[2] = 1;

    float x_start = ((float) x_block) / ((float) ndomains_per_dim);
    float y_start = ((float) y_block) / ((float) ndomains_per_dim);
    float z_start = ((float) z_block) / ((float) ndomains_per_dim);

    float block_step = 1./ndomains_per_dim;
    float x_step = block_step / (nx-1);
    float y_step = block_step / (ny-1);
    float z_step = block_step / (nz-1);

    for (int i = 0 ; i < nx ; i++)
    {
        for (int j = 0 ; j < ny ; j++)
        {
            for (int k = 0 ; k < nz ; k++)
            {
                int index = k*ny*nx + j*nx + i;
                x[index] = x_start + i*x_step;
                y[index] = y_start + j*y_step;
                z[index] = z_start + k*z_step;
            }
        }
    }
    bool haveMixed = false;
    if (x[0] < 0.5 && x[nx-1] > 0.5)
        haveMixed = true;
    for (int i = 0 ; i < nx-1 ; i++)
    {
        for (int j = 0 ; j < ny-1 ; j++)
        {
            for (int k = 0 ; k < nz-1 ; k++)
            {
                int index2 = k*(ny)*(nx) + j*(nx) + i;
                int index  = k*(ny-1)*(nx-1) + j*(nx-1) + i;
                if (x[index2] <= 0.50 && x[index2+1] >= 0.50)
                    den[index] = 6.;
                else if (x[index2] <= 0.50)
                    den[index] = 3.;
                else
                    den[index] = 1.;
            }
        }
    }
    int nmixed = (haveMixed ? 3*(ny-1)*(nz-1) : 0);
    float *mixedvar = NULL;
    if (haveMixed)
    {
        mixedvar = new float[nmixed];
        int current = 0;
        for (int i = 0 ; i < ny-1 ; i++)
            for (int j = 0 ; j < nz-1 ; j++)
            {
                mixedvar[current+0] = 3.;
                mixedvar[current+1] = 10.;
                mixedvar[current+2] = 1.;
                current += 3;
            }
    }
  
    float *coords[3];
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    DBoptlist *optlist = DBMakeOptlist(2);
    DBAddOption(optlist, DBOPT_GROUPNUM, &groupNum);
    DBAddOption(optlist, DBOPT_BASEINDEX, baseIndex);

    DBPutQuadmesh(dbfile, meshname, NULL, coords, dims, 3, DB_FLOAT,
                  DB_NONCOLLINEAR, optlist);
    int zdims[3] = { dims[0]-1, dims[1]-1, dims[2]-1 };
    DBPutQuadvar1(dbfile, varname, meshname, den, zdims, 3, mixedvar, nmixed,
                  DB_FLOAT, DB_ZONECENT, NULL);

    int nmats = 3;
    int *matnos = new int[nmats];
    for (int i = 0 ; i < nmats ; i++)
    {
        matnos[i] = i+1;
    }

    int nzones = (nx-1)*(ny-1)*(nz-1);
    int *matlist = new int[nzones];
    dims[0] = nx-1;
    dims[1] = ny-1;
    dims[2] = nz-1;
    int mixlen = (haveMixed ? 3*dims[1]*dims[2] : 0);
    int   *mix_mat  = new int[mixlen];
    int   *mix_next = new int[mixlen];
    int   *mix_zone = new int[mixlen];
    float *mix_vf   = new float[mixlen];
    int    current  = 0;
    for (int i = 0 ; i < dims[0] ; i++)
        for (int j = 0 ; j < dims[1] ; j++)
            for (int k = 0 ; k < dims[2] ; k++)
            {
                int index2 = k*(ny-1)*(nx-1) + j*(nx-1) + i;
                int index = k*ny*nx + j*nx + i;
                if (x[index] <= 0.50 && x[index+1] >= 0.50)
                {
                    matlist[index2] = -(current+1);
                    mix_mat[current+0]  = 1;
                    mix_mat[current+1]  = 2;
                    mix_mat[current+2]  = 3;
                    mix_vf[current+0]   = 0.25;
                    mix_vf[current+1]   = 0.5;
                    mix_vf[current+2]   = 0.25;
                    mix_zone[current+0] = index2;
                    mix_zone[current+1] = index2;
                    mix_zone[current+2] = index2;
                    mix_next[current+0] = current+2;
                    mix_next[current+1] = current+3;
                    mix_next[current+2] = 0;
                    current += 3;
                }
                else if (x[index] <= 0.50)
                    matlist[index2] = 1;
                else
                    matlist[index2] = 3;
            }

    DBPutMaterial(dbfile, "mat", meshname, nmats, matnos, matlist, dims, 3,
                  mix_next, mix_mat, mix_zone, mix_vf, mixlen, DB_FLOAT, NULL);
    DBSetDir(dbfile, "..");
    DBFreeOptlist(optlist);

    delete [] x;
    delete [] y;
    delete [] z;
    delete [] den;
    delete [] matnos;
    delete [] matlist;
}

