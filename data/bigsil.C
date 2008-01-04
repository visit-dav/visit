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
// File: bigsil.C
//
// Purpose:
//     This program creates a dataset that has a large SIL, but very few zones
//     per domain.
//
// Programmer: Hank Childs
// Creation:   November 13, 2002
//
// Modifications:
//
//  Hank Childs, Wed Mar 26 16:35:29 PST 2003
//  Updated variables when '-big' flag is issued.
//
//  Kathleen Bonnell, Wed Sep 10 16:10:25 PDT 2003
//  Added method 'WriteOutDecomposition' and ndomains_per_group.
//
//  Mark C. Miller, Thu Nov 10 12:19:46 PST 2005
//  Added ability to set ndomains_per_dim and nmats on command line.
//  Added ability to write two styles of multi-part files. The "normal"
//  method and the unusual method where a Multi-XXX entry refers to a member
//  in the root file using the filename, see bug #6790 
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
int nmats = 40;
int multipart = 0;
int multiparta = 0; // flag to generate odd-ball multi-XXX objects

int driver = DB_PDB;

int main(int argc, char *argv[])
{
    for (int args = 1 ; args < argc ; args++)
    {
        if (strcmp(argv[args], "-big") == 0)
        {
            ndomains_per_dim = 10;
            nmats = 128;
        }
        else if (strcmp(argv[args], "-ndoms") == 0)
        {
            args++;
            ndomains_per_dim = atoi(argv[args]);
        }
        else if (strcmp(argv[args], "-nmats") == 0)
        {
            args++;
            nmats = atoi(argv[args]);
        }
        else if (strcmp(argv[args], "-multipart") == 0)
        {
            multipart = 1;
        }
        else if (strcmp(argv[args], "-multiparta") == 0)
        {
            multiparta = 1;
        }
        else if (strcmp(argv[args], "-driver") == 0)
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

    DBfile *dbfile;
    if (multipart)
        dbfile = DBCreate("bigsil_0000.silo", DB_CLOBBER, DB_LOCAL,NULL,driver);
    else
        dbfile = DBCreate("bigsil.silo", DB_CLOBBER, DB_LOCAL,NULL,driver);

    WriteOutMultivars(dbfile);

    DBfile *dbfiletmp = dbfile;
    for (int i = 0 , groupNum = -1; i < n_total_domains ; i++)
    {
        if (i%(ndomains_per_group) == 0)
        {
            groupNum++;

            if (multipart && groupNum > 0)
            {
                if (groupNum > 1)
                    DBClose(dbfiletmp);

                char filename[128];
                sprintf(filename, "bigsil_%04d.silo", groupNum);
                dbfiletmp = DBCreate(filename, DB_CLOBBER, DB_LOCAL,NULL,driver);
            }
        }

        WriteOutDomain(dbfiletmp, i, groupNum);
    }

    if (multipart)
        DBClose(dbfiletmp);

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
// Programmer: Kathleen Bonnell 
// Creation:   September 10, 2003
// 
// Modifications:
//
//   Mark C. Miller, Tue Nov 22 13:59:08 PST 2005
//   Added alternative group output
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
// Creation:   November 13, 2002
//
// Modifications:
//
//   Hank Childs, Tue Jul  5 16:40:13 PDT 2005
//   Properly set up "match" portion of "neighbors" array.
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
// Creation:   November 13, 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 10 16:16:35 PDT 2003
//   Added a DBoptlist to store numGroups.
//
// ****************************************************************************

void
WriteOutMultivars(DBfile *dbfile)
{
    char **meshnames  = new char*[n_total_domains];
    char **matnames   = new char*[n_total_domains];
    char **distnames  = new char*[n_total_domains];
    int   *meshtypes  = new int[n_total_domains];
    int   *vartypes  = new int[n_total_domains];

    int nGroups = ndomains_per_dim; 
    for (int i = 0, groupNum = -1; i < n_total_domains ; i++)
    {
        char filename[128];

        if (i%(ndomains_per_group) == 0)
            groupNum++;

        if (multiparta || (multipart && groupNum > 0))
            sprintf(filename, "bigsil_%04d.silo:", groupNum);
        else
            filename[0] = '\0';

        meshnames[i] = new char[128];
        sprintf(meshnames[i], "%s/block%d/mesh", filename, i);
        matnames[i] = new char[128];
        sprintf(matnames[i], "%s/block%d/mat", filename, i);
        distnames[i] = new char[128];
        sprintf(distnames[i], "%s/block%d/dist", filename, i);
        meshtypes[i] = DB_QUADMESH;
        vartypes[i]  = DB_QUADVAR;
    }
    DBoptlist *optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_NGROUPS, &nGroups);
    DBPutMultimesh(dbfile, "mesh", n_total_domains, meshnames, meshtypes,optlist);
    DBPutMultivar(dbfile, "dist", n_total_domains, distnames, vartypes,optlist);
    DBPutMultimat(dbfile, "mat", n_total_domains, matnames, optlist);
    DBFreeOptlist(optlist);
    for (int i = 0, groupNum = -1; i < n_total_domains ; i++)
    {
        delete [] meshnames[i];
        delete [] matnames[i];
        delete [] distnames[i];
    }
    delete [] meshnames;
    delete [] matnames;
    delete [] distnames;
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
// Creation:   November 13, 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 10 16:16:35 PDT 2003
//   Added groupNum argument.  Created DBoptlist to store groupNum in mesh.
//
//   Kathleen Bonnell, Tue Dec 23 14:55:16 PST 2003
//   Added baseIndex to option litst.
//
//   Kathleen Bonnell, Thu Jul 21 07:52:49 PDT 2005
//   Made baseIndex 1-indexed to match Connectivity Extents.
//
//   Kathleen Bonnell, Fri Aug 19 15:45:46 PDT 2005 
//   Removed the '-1' from the baseIndex calculation.
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
    char *varname  = "dist";
    int dims[3];
    dims[0] = nx;
    dims[1] = ny;
    dims[2] = nz;

    int npts = nx*ny*nz;
    float *x = new float[npts];
    float *y = new float[npts];
    float *z = new float[npts];
    float *dist = new float[npts];

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
                dist[index] = (x[index]-0.5)*(x[index]-0.5)
                            + (y[index]-0.5)*(y[index]-0.5)
                            + (z[index]-0.5)*(z[index]-0.5);
                dist[index] = sqrt(dist[index]);
            }
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
    DBPutQuadvar1(dbfile, varname, meshname, dist, dims, 3, NULL, 0,
                  DB_FLOAT, DB_NODECENT, NULL);

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
    DBFreeOptlist(optlist);

    delete [] x;
    delete [] y;
    delete [] z;
    delete [] dist;
    delete [] matnos;
    delete [] matlist;
}

