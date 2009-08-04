/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include "Tetrahedralizer.h"
#include "mat3d_tet.h"
#include <vtkCell.h>
#include <VisItException.h>

#include <vector>
using std::vector;

#define MIN(A,B)  (((A) < (B)) ? (A) : (B))
#define SWAP(A,B) { int swtmp; swtmp=A; A=B; B=swtmp; }

// ****************************************************************************
//  Constructor: Tetrahedralizer::Tetrahedralizer
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2002
//
// ****************************************************************************
Tetrahedralizer::Tetrahedralizer(int nmat_)
{
    nmat = nmat_;
    for (int i=0; i<MAX_TETS_PER_CELL; i++)
        tet[i] = MaterialTetrahedron(nmat);
}

// ****************************************************************************
//  Method:  Tetrahedralizer::GetLowTetNodesForTet
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a tetrahedron.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattet's get re-initialized
//    every time.  This lets us re-use the Tetrahedralizer across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output tets.
//
//    Jeremy Meredith, Tue Jan 14 15:03:22 PST 2003
//    Extracted to a public static class function and made it fill in an
//    array instead of creating all the tets, so that it could be used
//    in more contexts.
//
// ****************************************************************************
int
Tetrahedralizer::GetLowTetNodesForTet(int nnodes,const int *nodes, int *tetnodes)
{
    int ntet = 1;
    tetnodes[0] = 0;
    tetnodes[1] = 1;
    tetnodes[2] = 2;
    tetnodes[3] = 3;
    return ntet;
}

// ****************************************************************************
//  Method:  Tetrahedralizer::GetLowTetNodesForVox
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a voxel.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattet's get re-initialized
//    every time.  This lets us re-use the Tetrahedralizer across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output tets.
//
//    Jeremy Meredith, Mon Jan  6 10:07:21 PST 2003
//    VTK tets are inverted from Silo tets, so I flipped the ordering of
//    all created tets.
//
//    Jeremy Meredith, Tue Jan 14 15:03:22 PST 2003
//    Extracted to a public static class function and made it fill in an
//    array instead of creating all the tets, so that it could be used
//    in more contexts.
//
// ****************************************************************************
int
Tetrahedralizer::GetLowTetNodesForVox(int nnodes,const int *nodes, int *tetnodes)
{
    int n;

    // find the node with the lowest index
    int minv = nodes[0];
    int minn = 0;
    for (n=1; n<nnodes; n++)
    {
        if (minv > nodes[n])
        {
            minv = nodes[n];
            minn = n;
        }
    }

    // rearrange the zone so that node is now at the zero position
    int flipx = (minn==0 || minn==4 || minn==2 || minn==6) ? 0 : 1;
    int flipy = (minn==4 || minn==5 || minn==6 || minn==7) ? 0 : 1;
    int flipz = (minn==0 || minn==1 || minn==4 || minn==5) ? 0 : 1;
    
    // create the old-node to new-node cross-index
    int *map  = mat3d_vox2tet_subdiv1_flip[flipx][flipy][flipz];

    // after flipping, is the zone going to be inside-out?
    bool invert = ((flipx + flipy + flipz) % 2) == 0;

    // which diagonals will the tets cross...
    int diagx0 = MIN(nodes[map[1]],nodes[map[6]]);
    int diagx1 = MIN(nodes[map[2]],nodes[map[5]]);
    int diagy0 = MIN(nodes[map[1]],nodes[map[3]]);
    int diagy1 = MIN(nodes[map[2]],nodes[map[0]]);
    int diagz0 = MIN(nodes[map[6]],nodes[map[3]]);
    int diagz1 = MIN(nodes[map[5]],nodes[map[0]]);

    int diagx = (diagx0 < diagx1) ? 0 : 1;
    int diagy = (diagy0 < diagy1) ? 0 : 1;
    int diagz = (diagz0 < diagz1) ? 0 : 1;

    // ... and which case do these correspond to (after flipping)
    int tetcase = (diagx<<2) + (diagy<<1) + (diagz);
    
    int ntet = ntet_mat3d_vox2tet_subdiv1[tetcase];

    for (int t=0; t<ntet; t++)
    {
        if (!invert)
        {
            tetnodes[t*4+0] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][2]];
            tetnodes[t*4+3] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][3]];
        }
        else
        {
            tetnodes[t*4+0] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][3]];
            tetnodes[t*4+3] = map[mat3d_vox2tet_subdiv1_node[tetcase][t][2]];
        }
    }
    return ntet;
}

// ****************************************************************************
//  Method:  Tetrahedralizer::GetLowTetNodesForHex
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a hexahedron.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 12 17:11:36 PST 2001
//    Fixed a numbering problem causing adjacent faces to be tetrahedralized
//    differently.
//
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattet's get re-initialized
//    every time.  This lets us re-use the Tetrahedralizer across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output tets.
//
//    Jeremy Meredith, Mon Jan  6 10:07:21 PST 2003
//    VTK tets are inverted from Silo tets, so I flipped the ordering of
//    all created tets.
//
//    Jeremy Meredith, Tue Jan 14 15:03:22 PST 2003
//    Extracted to a public static class function and made it fill in an
//    array instead of creating all the tets, so that it could be used
//    in more contexts.
//
// ****************************************************************************
int
Tetrahedralizer::GetLowTetNodesForHex(int nnodes,const int *nodes, int *tetnodes)
{
    int n;

    // find the node with the lowest index
    int minv = nodes[0];
    int minn = 0;
    for (n=1; n<nnodes; n++)
    {
        if (minv > nodes[n])
        {
            minv = nodes[n];
            minn = n;
        }
    }

    // rearrange the zone so that node is now at the zero position
    int flipx = (minn==0 || minn==4 || minn==7 || minn==3) ? 0 : 1;
    int flipy = (minn==5 || minn==4 || minn==7 || minn==6) ? 0 : 1;
    int flipz = (minn==0 || minn==1 || minn==4 || minn==5) ? 0 : 1;
    
    // create the old-node to new-node cross-index
    int *map  = mat3d_hex2tet_subdiv1_flip[flipx][flipy][flipz];

    // after flipping, is the zone going to be inside-out?
    bool invert = ((flipx + flipy + flipz) % 2) == 0;

    // which diagonals will the tets cross...
    int diagx0 = MIN(nodes[map[1]],nodes[map[6]]);
    int diagx1 = MIN(nodes[map[2]],nodes[map[5]]);
    int diagy0 = MIN(nodes[map[1]],nodes[map[3]]);
    int diagy1 = MIN(nodes[map[2]],nodes[map[0]]);
    int diagz0 = MIN(nodes[map[6]],nodes[map[3]]);
    int diagz1 = MIN(nodes[map[2]],nodes[map[7]]);

    int diagx = (diagx0 < diagx1) ? 0 : 1;
    int diagy = (diagy0 < diagy1) ? 0 : 1;
    int diagz = (diagz0 < diagz1) ? 0 : 1;

    // ... and which case do these correspond to (after flipping)
    int tetcase = (diagx<<2) + (diagy<<1) + (diagz);
    
    int ntet = ntet_mat3d_hex2tet_subdiv1[tetcase];

    for (int t=0; t<ntet; t++)
    {
        if (!invert)
        {
            tetnodes[t*4+0] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][2]];
            tetnodes[t*4+3] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][3]];
        }
        else
        {
            tetnodes[t*4+0] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][3]];
            tetnodes[t*4+3] = map[mat3d_hex2tet_subdiv1_node[tetcase][t][2]];
        }
    }
    return ntet;
}

// ****************************************************************************
//  Method:  Tetrahedralizer::GetLowTetNodesForWdg
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a wedge.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattet's get re-initialized
//    every time.  This lets us re-use the Tetrahedralizer across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output tets.
//
//    Jeremy Meredith, Mon Jan  6 10:07:21 PST 2003
//    VTK tets are inverted from Silo tets, so I flipped the ordering of
//    all created tets.
//
//    Jeremy Meredith, Tue Jan 14 15:03:22 PST 2003
//    Extracted to a public static class function and made it fill in an
//    array instead of creating all the tets, so that it could be used
//    in more contexts.
//
// ****************************************************************************
int
Tetrahedralizer::GetLowTetNodesForWdg(int nnodes,const int *nodes, int *tetnodes)
{
    int n;

    // find the node with the lowest index
    int minv = nodes[0];
    int minn = 0;
    for (n=1; n<nnodes; n++)
    {
        if (minv > nodes[n])
        {
            minv = nodes[n];
            minn = n;
        }
    }

    // rearrange the zone so that node is now at the zero position
    int flip  = (minn==0 || minn==1 || minn==2) ? 0 : 1;
    int rot   = (minn==0 || minn==3) ? 0 : ((minn==1 || minn==4) ? 1 : 2);
    
    // create the old-node to new-node cross-index
    int *map  = mat3d_wedge2tet_subdiv1_fliprot[flip][rot];

    // after flipping, is the zone going to be inside-out?
    bool invert = (flip % 2) == 0;

    // which diagonals will the tets cross...
    int diag0  = MIN(nodes[map[2]],nodes[map[4]]);
    int diag1  = MIN(nodes[map[1]],nodes[map[5]]);

    int diag   = (diag0 < diag1) ? 0 : 1;

    // ... and which case does this correspond to (after flipping)
    int tetcase = diag;
    
    int ntet = ntet_mat3d_wedge2tet_subdiv1;

    for (int t=0; t<ntet; t++)
    {
        if (!invert)
        {
            tetnodes[t*4+0] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][2]];
            tetnodes[t*4+3] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][3]];
        }
        else
        {
            tetnodes[t*4+0] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][3]];
            tetnodes[t*4+3] = map[mat3d_wedge2tet_subdiv1_node[tetcase][t][2]];
        }
    }
    return ntet;
}

// ****************************************************************************
//  Method:  Tetrahedralizer::GetLowTetNodesForPyr
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a pyramid.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattet's get re-initialized
//    every time.  This lets us re-use the Tetrahedralizer across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output tets.
//
//    Jeremy Meredith, Mon Jan  6 10:07:21 PST 2003
//    VTK tets are inverted from Silo tets, so I flipped the ordering of
//    all created tets.
//
//    Jeremy Meredith, Tue Jan 14 15:03:22 PST 2003
//    Extracted to a public static class function and made it fill in an
//    array instead of creating all the tets, so that it could be used
//    in more contexts.
//
// ****************************************************************************
int
Tetrahedralizer::GetLowTetNodesForPyr(int nnodes,const int *nodes, int *tetnodes)
{
    int n;

    // find the node with the lowest index
    int minv = nodes[0];
    int minn = 0;
    for (n=1; n<nnodes; n++)
    {
        if (minv > nodes[n])
        {
            minv = nodes[n];
            minn = n;
        }
    }

    // rearrange the zone so that node is now at the zero position
    int flip  = (minn==0 || minn==2) ? 0 : 1;
    
    // create the old-node to new-node cross-index
    int *map  = mat3d_pyr2tet_subdiv1_flip[flip];

    // after flipping, is the zone going to be inside-out?
    bool invert = (flip % 2) == 0;

    // which diagonals will the tets cross...
    // -- no question for a pyramid
    // ... and which case this must correspond (after flipping)
    int tetcase = 0;
    
    int ntet = ntet_mat3d_pyr2tet_subdiv1;

    for (int t=0; t<ntet; t++)
    {
        if (!invert)
        {
            tetnodes[t*4+0] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][2]];
            tetnodes[t*4+3] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][3]];
        }
        else
        {
            tetnodes[t*4+0] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][0]];
            tetnodes[t*4+1] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][1]];
            tetnodes[t*4+2] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][3]];
            tetnodes[t*4+3] = map[mat3d_pyr2tet_subdiv1_node[tetcase][t][2]];
        }
    }
    return ntet;
}


// ****************************************************************************
//  Method:  Tetrahedralizer::calc_tet_low
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a tetrahedron and
//    fill in the tets.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 14, 2003
//
//  Modifications:
//
// ****************************************************************************
void
Tetrahedralizer::calc_tet_low()
{
    ntet=GetLowTetNodesForTet(nnodes,nodes,tetnodes_low);
    fill_tets_low();
}

// ****************************************************************************
//  Method:  Tetrahedralizer::calc_vox_low
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a voxel and
//    fill in the tets.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 14, 2003
//
//  Modifications:
//
// ****************************************************************************
void
Tetrahedralizer::calc_vox_low()
{
    ntet=GetLowTetNodesForVox(nnodes,nodes,tetnodes_low);
    fill_tets_low();
}

// ****************************************************************************
//  Method:  Tetrahedralizer::calc_hex_low
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a hexahedron and
//    fill in the tets.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 14, 2003
//
//  Modifications:
//
// ****************************************************************************
void
Tetrahedralizer::calc_hex_low()
{
    ntet=GetLowTetNodesForHex(nnodes,nodes,tetnodes_low);
    fill_tets_low();
}

// ****************************************************************************
//  Method:  Tetrahedralizer::calc_wdg_low
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a wedge and
//    fill in the tets.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 14, 2003
//
//  Modifications:
//
// ****************************************************************************
void
Tetrahedralizer::calc_wdg_low()
{
    ntet=GetLowTetNodesForWdg(nnodes,nodes,tetnodes_low);
    fill_tets_low();
}

// ****************************************************************************
//  Method:  Tetrahedralizer::calc_pyr_low
//
//  Purpose:
//    Calculate the low-subdivision tetrahedralization for a pyramid and
//    fill in the tets.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 14, 2003
//
//  Modifications:
//
// ****************************************************************************
void
Tetrahedralizer::calc_pyr_low()
{
    ntet=GetLowTetNodesForPyr(nnodes,nodes,tetnodes_low);    
    fill_tets_low();
}

// ****************************************************************************
//  Method:  fill_tets_low
//
//  Purpose:
//    Fills in the tetrahedron array from the precalculated
//    ntet & tetnodes_low arrays.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 14, 2003
//
// ****************************************************************************
void
Tetrahedralizer::fill_tets_low()
{
    for (int t=0; t<ntet; t++)
    {
        tet[t].reset();

        int a = tetnodes_low[t*4+0];
        int b = tetnodes_low[t*4+1];
        int c = tetnodes_low[t*4+2];
        int d = tetnodes_low[t*4+3];

        tet[t].node[0].origindex = nodes[a];
        tet[t].node[1].origindex = nodes[b];
        tet[t].node[2].origindex = nodes[c];
        tet[t].node[3].origindex = nodes[d];

        tet[t].node[0].weight[a] = 1.0;
        tet[t].node[1].weight[b] = 1.0;
        tet[t].node[2].weight[c] = 1.0;
        tet[t].node[3].weight[d] = 1.0;

        for (int m=0; m<nmat; m++)
        {
            tet[t].node[0].matvf[m] = vf_node[a][m];
            tet[t].node[1].matvf[m] = vf_node[b][m];
            tet[t].node[2].matvf[m] = vf_node[c][m];
            tet[t].node[3].matvf[m] = vf_node[d][m];
        }
    }
}

// ****************************************************************************
//  Method:  Tetrahedralizer::Tetrahedralize
//
//  Purpose:
//    Create the tetrahedralization using the given parameters.
//
//  Arguments:
//    s          the subivision level
//    c          the cell
//    nmat_      the number of materials
//    vf_xxxx_   the locally indexed VFs for the zone/node/face/edge
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Hank Childs, Mon Oct  7 17:43:41 PDT 2002 
//    Remove costly VTK calls.
//
//    Hank Childs, Fri Jan 28 15:43:04 PST 2005
//    Use exception macros.
//
// ****************************************************************************
void
Tetrahedralizer::Tetrahedralize(MIROptions::SubdivisionLevel s, int celltype,
                                int npts, const int *c_ptr,
                                vector<float>   vf_zone_,
                                vector<float>  *vf_node_,
                                vector<float>  *vf_face_,
                                vector<float>  *vf_edge_)
{
    subdiv = s;
    nnodes = npts;
    nodes  = c_ptr;

    vf_zone = vf_zone_;
    vf_node = vf_node_;
    vf_face = vf_face_;
    vf_edge = vf_edge_;

    switch (subdiv)
    {
      case MIROptions::Low:
        switch (celltype)
        {
          case VTK_TETRA:       calc_tet_low();   break;
          case VTK_VOXEL:       calc_vox_low();   break;
          case VTK_HEXAHEDRON:  calc_hex_low();   break;
          case VTK_WEDGE:       calc_wdg_low();   break;
          case VTK_PYRAMID:     calc_pyr_low();   break;
          default:              EXCEPTION0(VisItException);
        }
        break;
      case MIROptions::Med:
        //
        // If you implement subdivision level medium you must change
        // the code in MIR::ReconstructTet.
        //
        switch (celltype)
        {
          case VTK_TETRA:       ntet = 12;     break;
          case VTK_VOXEL:       ntet = 24;     break;
          case VTK_HEXAHEDRON:  ntet = 24;     break;
          case VTK_WEDGE:       ntet = 18;     break;
          case VTK_PYRAMID:     ntet = 13;     break;
          default:              EXCEPTION0(VisItException);
        }
        break;
      case MIROptions::High:
        //
        // If you implement subdivision level high you must change
        // the code in MIR::ReconstructTet.
        //
        switch (celltype)
        {
          case VTK_TETRA:       ntet = 24;     break;
          case VTK_VOXEL:       ntet = 48;     break;
          case VTK_HEXAHEDRON:  ntet = 48;     break;
          case VTK_WEDGE:       ntet = 36;     break;
          case VTK_PYRAMID:     ntet = 26;     break;
          default:              EXCEPTION0(VisItException);
        }
        break;
      default:
        EXCEPTION0(VisItException);
    }
}
