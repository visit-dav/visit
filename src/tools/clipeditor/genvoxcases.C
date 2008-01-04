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

#include <visitstream.h>

// ****************************************************************************
//  File:  genvoxcases.cpp
//
//  Purpose:
//    Generates the 48 transforms of a voxel.
//    Points are 0-8, edges are A-L.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 24, 2004
//
// ****************************************************************************


int voxVerticesFromEdges[12][2] =
{
    { 0, 1 },   /* Edge 0 */
    { 1, 3 },   /* Edge 1 */
    { 2, 3 },   /* Edge 2 */
    { 0, 2 },   /* Edge 3 */
    { 4, 5 },   /* Edge 4 */
    { 5, 7 },   /* Edge 5 */
    { 6, 7 },   /* Edge 6 */
    { 4, 6 },   /* Edge 7 */
    { 0, 4 },   /* Edge 8 */
    { 1, 5 },   /* Edge 9 */
    { 2, 6 },   /* Edge 10 */
    { 3, 7 }    /* Edge 11 */
};


void
SetNode(int node[2][2][2], int id,
        int ox, int oy, int oz, int dx[3], int dy[3], int dz[3],
        int mx, int my, int mz)
{
    ox += dx[0]*mx + dy[0]*my + dz[0]*mz;
    oy += dx[1]*mx + dy[1]*my + dz[1]*mz;
    oz += dx[2]*mx + dy[2]*my + dz[2]*mz;
    node[ox][oy][oz] = id;
}

void
GenCases(int ox, int oy, int oz, int dx[3], int dy[3], int dz[3])
{
    int node[2][2][2];

    SetNode(node, 0, ox, oy, oz, dx,dy,dz, 0, 0, 0);
    SetNode(node, 1, ox, oy, oz, dx,dy,dz, 1, 0, 0);
    SetNode(node, 2, ox, oy, oz, dx,dy,dz, 0, 0, 1);
    SetNode(node, 3, ox, oy, oz, dx,dy,dz, 1, 0, 1);

    SetNode(node, 4, ox, oy, oz, dx,dy,dz, 0, 1, 0);
    SetNode(node, 5, ox, oy, oz, dx,dy,dz, 1, 1, 0);
    SetNode(node, 6, ox, oy, oz, dx,dy,dz, 0, 1, 1);
    SetNode(node, 7, ox, oy, oz, dx,dy,dz, 1, 1, 1);

    int allnodes[8] = {
        node[0][0][0],
        node[1][0][0],
        node[0][0][1],
        node[1][0][1],
        node[0][1][0],
        node[1][1][0],
        node[0][1][1],
        node[1][1][1]
    };
    cerr << "{{";
    cerr << allnodes[0] << ",";
    cerr << allnodes[1] << ",";
    cerr << allnodes[2] << ",";
    cerr << allnodes[3] << ",";
    cerr << allnodes[4] << ",";
    cerr << allnodes[5] << ",";
    cerr << allnodes[6] << ",";
    cerr << allnodes[7] << "},{";
    for (int i=0; i<12; i++)
    {
        int e0 = voxVerticesFromEdges[i][0];
        int e1 = voxVerticesFromEdges[i][1];

        int n0 = allnodes[e0];
        int n1 = allnodes[e1];

        for (int j=0; j<12; j++)
        {
            int j0 = voxVerticesFromEdges[j][0];
            int j1 = voxVerticesFromEdges[j][1];

            if (n0==j0 && n1==j1)
            {
                cerr << "'" << char(int('a')+j) << "'";
            }
            else if (n0==j1 && n1==j0)
            {
                cerr << "'" << char(int('a')+j) << "'";
            }

        }
        if (i<11) cerr << ",";
    }
    cerr << "},";

    int test_dy[3] = 
    {
        dx[1]*dz[2] - dx[2]*dz[1],
        dx[2]*dz[0] - dx[0]*dz[2],
        dx[0]*dz[1] - dx[1]*dz[0]
    };

    bool flip;
    if (test_dy[0]==dy[0] &&
        test_dy[1]==dy[1] &&
        test_dy[2]==dy[2])
    {
        flip = true;
    }
    else 
    {
        flip = false;
    }

    cerr << (flip ? "true " : "false");
    cerr << " }," << endl;
}

int main()
{
    for (int ox=0; ox<=1; ox++)
    {
      for (int oy=0; oy<=1; oy++)
      {
        for (int oz=0; oz<=1; oz++)
        {
            int xaxis[] = {ox==0 ? 1 : -1, 0, 0};
            int yaxis[] = {0, oy==0 ? 1 : -1, 0};
            int zaxis[] = {0, 0, oz==0 ? 1 : -1};

            GenCases(ox,oy,oz, xaxis, yaxis, zaxis);
            GenCases(ox,oy,oz, xaxis, zaxis, yaxis);
            GenCases(ox,oy,oz, yaxis, xaxis, zaxis);
            GenCases(ox,oy,oz, yaxis, zaxis, xaxis);
            GenCases(ox,oy,oz, zaxis, xaxis, yaxis);
            GenCases(ox,oy,oz, zaxis, yaxis, xaxis);
        }
      }
    }
}
