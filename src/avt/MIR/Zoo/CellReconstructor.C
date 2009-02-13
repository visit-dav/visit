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

#include "CellReconstructor.h"

#include <ClipCases.h>
#include <vtkCellTypes.h>
#include <avtMaterial.h>
#include <ZooMIR.h>
#include <ImproperUseException.h>
#include <vtkTriangulationTables.h>

#include "BitUtils.h"
#include "ResampledMat.h"

#include <verdict.h>

// ****************************************************************************
//  Constructor: CellReconstructor::CellReconstructor
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 18 18:02:38 PDT 2005
//    I was able to re-use most of this class for a new isovolume based
//    reconstruction algorithm.  Everything stayed except I made
//    ReconstructCell pure-virtual, and I needed to keep track of whether
//    or not edge points were shared across materials.
//
//    Hank Childs, Fri Jun  9 15:05:25 PDT 2006
//    Reorder initializers to match declaration order in class definition.
//
// ****************************************************************************
CellReconstructor::CellReconstructor(vtkDataSet *d,
                                     avtMaterial *m,
                                     ResampledMat &r,
                                     int np, int nc,
                                     bool matsSharePoints,
                                     MIRConnectivity &mc,
                                     ZooMIR &z)
    : mesh(d), mat(m), rm(r), nPoints(np), nCells(nc),
      conn(mc), mir(z),
      nMaterials(mat->GetNMaterials()),
      allMaterialsSharePoints(matsSharePoints),
      edges(nPoints/2),
      outlist(200),
      tmplist(200)
{
    mix_index = new int[mat->GetNMaterials()];
    for (int i=0; i<MAX_NODES_PER_ZONE; i++)
        nodeVFs[i] = new float[mat->GetNMaterials()];
}

// ****************************************************************************
//  Destructor:  CellReconstructor::~CellReconstructor
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
CellReconstructor::~CellReconstructor()
{
    delete[] mix_index;
    for (int i=0; i<MAX_NODES_PER_ZONE; i++)
        delete[] nodeVFs[i];
}

// ****************************************************************************
//  Method:  CellReconstructor::CreateCentroidPoint
//
//  Purpose:
//    Create a new point needed to triangulate the output shapes
//    from this cell.  Make sure the interpolation is all done
//    correctly.
//
//  Arguments:
//    old        the old TempCell
//    interpID   where to stick it when it's done (0-3)
//    color      COLOR0, COLOR1, or NOCOLOR: how to interpolate the VF's
//    mat0,mat1  the old and new materials
//    npts,pts   the ids from the original cell
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Brad Whitlock, Tue Sep 23 09:48:36 PDT 2003
//    Made it build on Windows.
//
// ****************************************************************************
void
CellReconstructor::CreateCentroidPoint(TempCell &old, int interpID,
                                       int color, int mat0, int mat1,
                                       int npts, int *pts)
{
    ZooMIR::ReconstructedCoord newCoord;
    newCoord.origzone = cellid;
    newCoord.x = 0;
    newCoord.y = 0;
    newCoord.z = 0;
    for (int id = 0 ; id < nids ; id++)
        newCoord.weight[id] = 0.;
    float weight = 1. / float(npts);

    for (int p = 0 ; p < npts ; p++)
    {
        unsigned char pt = pts[p];
        if (pt <= P7)
        {
            int ptId = old.ids[pt];
            if (ptId < nPoints)
            {
                // it's an original
                newCoord.weight[old.localids[pt]] += weight;
            }
            else
            {
                ZooMIR::ReconstructedCoord &oldCoord =
                                                 mir.coordsList[ptId-nPoints];
                for (int q = 0 ; q < nids ; q++)
                    newCoord.weight[q] += weight * oldCoord.weight[q];
            }
        }
        else if (pt >= EA && pt <= EL)
        {
            int pt1 = vertices_from_edges[pt-EA][0];
            int pt2 = vertices_from_edges[pt-EA][1];
            if (pt2 < pt1)
            {
                int tmp = pt2;
                pt2 = pt1;
                pt1 = tmp;
            }
            double dir = vfDiff[pt2] - vfDiff[pt1];
            double amt = 0. - vfDiff[pt1];
            double percent = 1. - (amt / dir);

            // We may have physically (though not logically)
            // degenerate cells if percent==0 or percent==1.
            int ptId1 = old.ids[pt1];
            int ptId2 = old.ids[pt2];

            if (ptId1 < nPoints)
            {
                // it's an original
                newCoord.weight[old.localids[pt1]] += weight * percent;
            }
            else
            {
                ZooMIR::ReconstructedCoord &oldCoord = mir.coordsList[ptId1-nPoints];
                for (int q = 0 ; q < nids ; q++)
                    newCoord.weight[q] += weight * oldCoord.weight[q] * percent;
            }

            if (ptId2 < nPoints)
            {
                // it's an original
                newCoord.weight[old.localids[pt2]] += weight * (1.0 - percent);
            }
            else
            {
                ZooMIR::ReconstructedCoord &oldCoord = mir.coordsList[ptId2-nPoints];
                for (int q = 0 ; q < nids ; q++)
                    newCoord.weight[q] += weight * oldCoord.weight[q] * (1.0 - percent);
            }
        }
        else if (pt >= N0 && pt <= N3)
        {
            int ptId = interpIDs[pt - N0];
            if (ptId < nPoints)
            {
                // it's an original
                newCoord.weight[old.localids[pt]] += weight;
            }
            else
            {
                ZooMIR::ReconstructedCoord &oldCoord = mir.coordsList[ptId-nPoints];
                for (int q = 0 ; q < nids ; q++)
                    newCoord.weight[q] += weight * oldCoord.weight[q];
            }
        }
        else
        {
            EXCEPTION1(ImproperUseException,
                       "An invalid output point value "
                       "was found in the ClipCases.");
        }
    }

    interpVFs[interpID] = 0;
    for (int q = 0 ; q < nids ; q++)
    {
        newCoord.x += newCoord.weight[q] * mir.origXCoords[ids[q]];
        newCoord.y += newCoord.weight[q] * mir.origYCoords[ids[q]];
        newCoord.z += newCoord.weight[q] * mir.origZCoords[ids[q]];
        if (color==COLOR0)
            interpVFs[interpID] += newCoord.weight[q] * nodeVFs[q][mat0];
        else if (color==COLOR1)
            interpVFs[interpID] += newCoord.weight[q] * nodeVFs[q][mat1];
        else // color==NOCOLOR
            interpVFs[interpID] += newCoord.weight[q] * 0.5 * (nodeVFs[q][mat0] + nodeVFs[q][mat1]);
    }
    interpIDs[interpID] = mir.coordsList.size()+nPoints;

    mir.coordsList.push_back(newCoord);
}

// ****************************************************************************
//  Method:  CellReconstructor::CreateOutputShape
//
//  Purpose:
//    Create a new piece of the original cell
//
//  Arguments:
//    old        the old TempCell
//    cell       the new TempCell
//    color      COLOR0 or COLOR1 (cannot be NOCOLOR)
//    npts,pts   the original cell nodes
//    matno      the actual material number (not just what's in cell.mat)
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 18 18:02:38 PDT 2005
//    I was able to re-use most of this class for a new isovolume based
//    reconstruction algorithm.  Everything stayed except I made
//    ReconstructCell pure-virtual, and I needed to keep track of whether
//    or not edge points were shared across materials.
//
// ****************************************************************************
void
CellReconstructor::CreateOutputShape(TempCell &old,
                                     TempCell &cell,
                                     int color,
                                     int npts, int *pts,
                                     int matno)
{
    for (int p = 0 ; p < npts ; p++)
    {
        unsigned char pt = pts[p];
        if (pt <= P7)
        {
            cell.ids[p] = old.ids[pt];
            cell.localids[p] = old.localids[pt];
            if (color==COLOR0)
                cell.vfs[p] = old.vfs[pt];
            else
                cell.vfs[p] = newVFs[pt];
        }
        else if (pt >= EA && pt <= EL)
        {
            int pt1 = vertices_from_edges[pt-EA][0];
            int pt2 = vertices_from_edges[pt-EA][1];
            if (pt2 < pt1)
            {
                int tmp = pt2;
                pt2 = pt1;
                pt1 = tmp;
            }
            float dir = vfDiff[pt2] - vfDiff[pt1];
            float amt = 0. - vfDiff[pt1];
            float percent = 1. - (amt / dir);

            // We may have physically (though not logically)
            // degenerate cells if percent==0 or percent==1.
            int ptId1 = old.ids[pt1];
            int ptId2 = old.ids[pt2];

            // Look for the edge to exist already
            ZooMIR::EdgeHashEntry *edge;
            if (allMaterialsSharePoints)
            {
                edge = edges.GetEdge(ptId1,ptId2,0);
            }
            else
            {
                edge = edges.GetEdge(ptId1,ptId2,matno);
            }

            if (edge->GetPointId() >= 0)
            {
                // found it!
                cell.ids[p] = edge->GetPointId();

                // We need to update edge points shared between cells to match the current cell
                
                ZooMIR::ReconstructedCoord &newCoord =
                                 mir.coordsList[edge->GetPointId() - nPoints];
                if (newCoord.origzone != cellid)
                {
                    newCoord.origzone = cellid;

                    for (int q = 0 ; q < nids ; q++)
                        newCoord.weight[q] = 0;

                    if (ptId1 < nPoints)
                    {
                        newCoord.weight[old.localids[pt1]] += percent;
                    }
                    else
                    {
                        for (int q = 0 ; q < nids ; q++)
                            newCoord.weight[q] +=
                                mir.coordsList[ptId1-nPoints].weight[q] * percent;
                    }

                    if (ptId2 < nPoints)
                    {
                        newCoord.weight[old.localids[pt2]] += (1. - percent);
                    }
                    else
                    {
                        for (int q = 0 ; q < nids ; q++)
                            newCoord.weight[q] +=
                                mir.coordsList[ptId2-nPoints].weight[q] * (1. - percent);
                    }
                }
            }
            else
            {
                // didn't find it; make a new one
                ZooMIR::ReconstructedCoord newCoord;

                newCoord.origzone = cellid;

                newCoord.x = 0;
                newCoord.y = 0;
                newCoord.z = 0;
                for (int q = 0 ; q < nids ; q++)
                    newCoord.weight[q] = 0;

                if (ptId1 < nPoints)
                {
                    newCoord.x += mir.origXCoords[ptId1] * percent;
                    newCoord.y += mir.origYCoords[ptId1] * percent;
                    newCoord.z += mir.origZCoords[ptId1] * percent;
                    newCoord.weight[old.localids[pt1]] += percent;
                }
                else
                {
                    newCoord.x += mir.coordsList[ptId1-nPoints].x * percent;
                    newCoord.y += mir.coordsList[ptId1-nPoints].y * percent;
                    newCoord.z += mir.coordsList[ptId1-nPoints].z * percent;
                    for (int q = 0 ; q < nids ; q++)
                        newCoord.weight[q] +=
                            mir.coordsList[ptId1-nPoints].weight[q] * percent;
                }
                if (ptId2 < nPoints)
                {
                    newCoord.x += mir.origXCoords[ptId2] * (1. - percent);
                    newCoord.y += mir.origYCoords[ptId2] * (1. - percent);
                    newCoord.z += mir.origZCoords[ptId2] * (1. - percent);
                    newCoord.weight[old.localids[pt2]] += (1. - percent);
                }
                else
                {
                    newCoord.x += mir.coordsList[ptId2-nPoints].x * (1. - percent);
                    newCoord.y += mir.coordsList[ptId2-nPoints].y * (1. - percent);
                    newCoord.z += mir.coordsList[ptId2-nPoints].z * (1. - percent);
                    for (int q = 0 ; q < nids ; q++)
                        newCoord.weight[q] +=
                            mir.coordsList[ptId2-nPoints].weight[q] * (1. - percent);
                }
                int newid = mir.coordsList.size()+nPoints;
                cell.ids[p] = newid;
                edge->SetPointId(newid);
                mir.coordsList.push_back(newCoord);
            }
                                        
            // Doesn't matter whether or not we use the old or new VFs;
            // they should should match at the intersection.  Just use
            // the new ones arbitrarily.
            cell.vfs[p] = newVFs[pt1]*percent + newVFs[pt2]*(1.-percent);
        }
        else if (pt >= N0 && pt <= N3)
        {
            cell.ids[p] = interpIDs[pt - N0];
            cell.vfs[p] = interpVFs[pt - N0];
        }
        else
        {
            EXCEPTION1(ImproperUseException,
                       "An invalid output point value "
                       "was found in the ClipCases.");
        }
    }

    outlist.push_back(cell);
}

// ****************************************************************************
//  Method:  CellReconstructor::CalculateVolumeOrAreaHelper
//
//  Purpose:
//    Calculate the area or volume of a VTK cell.  Logic taken
//    from the mesh quality expressions.
//
//  Arguments:
//    type       vtk cell type
//    coords     the coordinates in VTK order
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 13, 2009
//
// ****************************************************************************

inline void
Swap1(double &a, double &b)
{
    double tmp = a;
    a = b;
    b = tmp;
}

inline void
Swap3(double c[][3], int a, int b)
{
    Swap1(c[a][0], c[b][0]);
    Swap1(c[a][1], c[b][1]);
    Swap1(c[a][2], c[b][2]);
}

inline
void Copy3(double coords[][3], double a[], int i)
{
    a[0] = coords[i][0];
    a[1] = coords[i][1];
    a[2] = coords[i][2];
}

double
CellReconstructor::CalculateVolumeOrAreaHelper(int type, double coords[][3])
{
    switch (type)
    {
      case VTK_TRIANGLE:
          return v_tri_area(3, coords);
        
      case VTK_QUAD:
        return v_quad_area(4, coords);
        
      case VTK_PIXEL:
        Swap3(coords, 2, 3);
        return v_quad_area(4, coords);

      case VTK_VOXEL:
        Swap3(coords, 2,3);
        Swap3(coords, 6,7);
        return v_hex_volume(8,coords);

      case VTK_HEXAHEDRON:
        return v_hex_volume(8,coords);

      case VTK_TETRA:
        return v_tet_volume(4,coords);

      case VTK_WEDGE:
        {
        int   subdiv[3][4] = { {0,5,4,3}, {0,2,1,4}, {0,4,5,2} };
        double tet_coords[4][3];
        double vol = 0;
        for (int i = 0 ; i < 3 ; i++)
        {
            for (int j = 0 ; j < 4 ; j++)
                for (int k = 0 ; k < 3 ; k++)
                    tet_coords[j][k] = coords[subdiv[i][j]][k];
            vol += v_tet_volume(4, tet_coords);
        }
        return vol;
        }
        
      // The verdict metric for pyramid I have yet to figure out how to work.
      // However, it does the same thing that we do here: Divide the pyramid
      // into two tetrahedrons.
      case VTK_PYRAMID:
        {
        double one[4][3];
        double two[4][3];
            
        Copy3(coords,one[0], 0);
        Copy3(coords,one[1], 1);
        Copy3(coords,one[2], 2);
        Copy3(coords,one[3], 4);

        Copy3(coords,two[0], 0);
        Copy3(coords,two[1], 2);
        Copy3(coords,two[2], 3);
        Copy3(coords,two[3], 4);

        return v_tet_volume(4,one) + v_tet_volume(4,two);
        }
    }

    // ERROR; SHOULDN'T GET HERE!!!!
    return -99999;
}
