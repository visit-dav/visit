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

#include "RecursiveCellReconstructor.h"

#include <ClipCases.h>
#include <vtkCellTypes.h>
#include <avtMaterial.h>
#include <ZooMIR.h>
#include <VisItArray.h>
#include <ImproperUseException.h>
#include <vtkTriangulationTables.h>

#include "BitUtils.h"
#include "ResampledMat.h"


#include <verdict.h>

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

static double CalculateVolume(int type, double coords[][3])
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

    cerr << "ERROR; SHOULDN'T GET HERE!!!!\n";
    return -99999;
}



// ****************************************************************************
//  Constructor:  RecursiveCellReconstructor::RecursiveCellReconstructor
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 18, 2005
//
// ****************************************************************************
RecursiveCellReconstructor::RecursiveCellReconstructor(vtkDataSet *d,
                                                       avtMaterial *m,
                                                       ResampledMat &r,
                                                       int np, int nc,
                                                       MIRConnectivity &mc,
                                                       ZooMIR &z)
    : CellReconstructor(d,m,r,np,nc,true,mc,z)
{
}

// ****************************************************************************
//  Method:  RecursiveCellReconstructor::ReconstructCell
//
//  Purpose:
//    Split a cell into its component material pieces
//
//  Arguments:
//    cellid     the old cellid
//    celltype   the cell type
//    nids       number of points in the cells
//    ids        the points from the cell
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 16 10:44:59 PDT 2003
//    Fixed a bug with material selecting mixvars.
//
//    Jeremy Meredith, Thu Sep 18 11:13:41 PDT 2003
//    1. Added VTK_TRIANGLE and VTK_QUAD input cases for 2D MIR.
//    2. Added ST_TRI and ST_QUA output shape types for 2D MIR.
//    3. Fixed a bug with material interface smoothing; a test was inverted.
//    4. Had to remove the faster way to calculate the variable "newexists",
//    as it was blatantly wrong.  The faster way was using matsAtCellOneAway, 
//    but it needed to be using matsAtCellOrig.
//
//    Jeremy Meredith, Thu Jun 24 10:38:05 PDT 2004
//    Added Voxel and Pixel input shapes.
//
//    Jeremy Meredith, Thu Aug 18 13:27:25 PDT 2005
//    There was a test for a "real clean" zone that assumed the material
//    that wound up being clean would always have a positive (clean) entry
//    in the original matlist array.  It turns out that occasionally there
//    will be cells marked as mixed (negative) in the matlist array where
//    all but one material have zero mix_vf's.  I changed the test slightly
//    to account for this.
//
//    Jeremy Meredith, Thu Aug 18 17:10:02 PDT 2005
//    Created a base CellReconstructor class and moved this method down
//    into this derived class.
//
//    Mark C. Miller, Thu Feb  9 21:06:10 PST 2006
//    Renamed Array class to VisItArray to avoid name collisions with
//    third-party libs
// ****************************************************************************
void
RecursiveCellReconstructor::ReconstructCell(int cellid_, int celltype_,
                                            int nids_, int *ids_,
                                            double *vfs)
{
    cellid = cellid_;
    celltype = celltype_;
    nids = nids_;
    ids = ids_;

    // extract mix_index for each material
    mat->ExtractCellMatInfo(cellid, mix_index);

    // get the index into the material arrays for this cell
    for (int n = 0 ; n < nids ; n++)
    {
        nodeIndices[n]    = rm.nodeIndexInMatArray[ids[n]];
        nodeIndexLimit[n] = rm.nodeIndexInMatArray[ids[n]+1];
    }

    outlist.clear();
    tmplist.clear();

    // Count the number of materials adjacent to this cell
    // Note: there it probably a faster way to do this, as it
    // may be slowing down clean-cell reconstruction
    int nm = 0;
    for (int i=0; i<rm.nBPE; i++)
        nm += nBitsSet[rm.matsAtCellOneAway[cellid*rm.nBPE + i]];

    // Loop over all materials in this cell
    bool nextOneAllZeros = !(rm.matsAtCellOneAway[cellid*rm.nBPE] & 0x01);
    for (int matno = 0 ; matno < nMaterials; matno++)
    {
        bool allZeros = nextOneAllZeros;
        nextOneAllZeros = (matno+1 >= nMaterials) || !(rm.matsAtCellOneAway[cellid*rm.nBPE + byteForBit(matno+1)] & bitForBit(matno+1));

        // If only one material is at this cell's nodes, then
        // the VFs are 1.0 at all nodes, so no node has a VF
        // of 0.0, so we don't have to worry about other
        // materials intersecting with this one.  In other words,
        // these are the "real clean zones".
        if (nm==1 && allZeros)
            continue;

        if (allZeros && nextOneAllZeros)
            continue;


        // Fill the volume fractions for the current material
        for (int n = 0 ; n < nids ; n++)
        {
            nodeVFs[n][matno] = 0.;
            if (!allZeros &&
                nodeIndices[n] < nodeIndexLimit[n] &&
                rm.matArrayMatNo[nodeIndices[n]] == matno)
            {
                nodeVFs[n][matno] = rm.matArrayNodeVF[nodeIndices[n]];
                nodeIndices[n]++;
            }
        }

        if (outlist.empty())
        {
            //
            // First material in
            //

            TempCell cell;
            for (int n = 0 ; n < nids ; n++)
            {
                cell.localids[n] = n;
                cell.ids[n] = ids[n];
                cell.vfs[n] = nodeVFs[n][matno];
            }
            cell.mat = matno;
            cell.celltype = celltype;
            cell.nnodes = nids;
            cell.mix_index = mix_index[matno];

            outlist.push_back(cell);
        }
        else
        {
            //
            // There's already background material; merge new one
            //

            // make the new list the old list
            VisItArray<TempCell>::Swap(outlist, tmplist);
            outlist.clear();

            // loop over all old temporary cells
            for (int t = 0 ; t < tmplist.size(); t++)
            {
                TempCell &old = tmplist[t];

                bool oldexists = rm.matsAtCellOrig[cellid*rm.nBPE + byteForBit(old.mat)] & bitForBit(old.mat);
                bool newexists = rm.matsAtCellOrig[cellid*rm.nBPE + byteForBit(matno)]   & bitForBit(matno);

                //
                // Fill the VF's array and calculate the clip case
                //
                int lookup_case = 0;
                for (int n = old.nnodes-1 ; n >= 0 ; n--)
                {
                    if (old.ids[n] < nPoints)
                    {
                        // node is from original mesh
                        newVFs[n] = nodeVFs[old.localids[n]][matno];
                    }
                    else // if (old.ids[n] >= nPoints)
                    {
                        ZooMIR::ReconstructedCoord &coord = mir.coordsList[old.ids[n]-nPoints];
                        if (coord.origzone != cellid)
                        {
                            EXCEPTION1(ImproperUseException,
                                       "An old edge point was encountered "
                                       "tagged at a previous cell ID, and was "
                                       "never updated to be relative to the "
                                       "current cell.");
                        }

                        newVFs[n] = 0;
                        for (int w=0; w<nids; w++)
                        {
                            newVFs[n] += nodeVFs[w][matno] * coord.weight[w];
                        }
                    }

                    vfDiff[n] = newVFs[n] - old.vfs[n];

                    if (vfDiff[n] > 0)
                        lookup_case++;
                    if (n>0)
                        lookup_case *= 2;
                }

                //
                // We know which case we correspond to; find out how
                // to split the old temp cell
                //
                startIndex = 0;
                splitCase = NULL;
                numOutput = 0;
                vertices_from_edges = NULL;

                switch (old.celltype)
                {
                  case VTK_TETRA:
                    startIndex = startClipShapesTet[lookup_case];
                    splitCase  = &clipShapesTet[startIndex];
                    numOutput  = numClipShapesTet[lookup_case];
                    vertices_from_edges = tetVerticesFromEdges;
                    break;
                  case VTK_PYRAMID:
                    startIndex = startClipShapesPyr[lookup_case];
                    splitCase  = &clipShapesPyr[startIndex];
                    numOutput  = numClipShapesPyr[lookup_case];
                    vertices_from_edges = pyramidVerticesFromEdges;
                    break;
                  case VTK_WEDGE:
                    startIndex = startClipShapesWdg[lookup_case];
                    splitCase  = &clipShapesWdg[startIndex];
                    numOutput  = numClipShapesWdg[lookup_case];
                    vertices_from_edges = wedgeVerticesFromEdges;
                    break;
                  case VTK_HEXAHEDRON:
                    startIndex = startClipShapesHex[lookup_case];
                    splitCase  = &clipShapesHex[startIndex];
                    numOutput  = numClipShapesHex[lookup_case];
                    vertices_from_edges = hexVerticesFromEdges;
                    break;
                  case VTK_VOXEL:
                    startIndex = startClipShapesVox[lookup_case];
                    splitCase  = &clipShapesVox[startIndex];
                    numOutput  = numClipShapesVox[lookup_case];
                    vertices_from_edges = voxVerticesFromEdges;
                    break;
                  case VTK_TRIANGLE:
                    startIndex = startClipShapesTri[lookup_case];
                    splitCase  = &clipShapesTri[startIndex];
                    numOutput  = numClipShapesTri[lookup_case];
                    vertices_from_edges = triVerticesFromEdges;
                    break;
                  case VTK_QUAD:
                    startIndex = startClipShapesQua[lookup_case];
                    splitCase  = &clipShapesQua[startIndex];
                    numOutput  = numClipShapesQua[lookup_case];
                    vertices_from_edges = quadVerticesFromEdges;
                    break;
                  case VTK_PIXEL:
                    startIndex = startClipShapesPix[lookup_case];
                    splitCase  = &clipShapesPix[startIndex];
                    numOutput  = numClipShapesPix[lookup_case];
                    vertices_from_edges = pixelVerticesFromEdges;
                    break;
                }

                //
                // This is a full replacement; take a shortcut
                //
                if (numOutput == 1)
                {
                    int newmat = (lookup_case==0) ? old.mat : matno;
                    
                    // Worry about that "artificial smoothing" toggle
                    if (mir.options.smoothing)
                    {
                        old.mat = newmat;
                    }
                    else
                    {
                        if (newexists && !oldexists)
                            old.mat = matno;
                        else if (oldexists && !newexists)
                            old.mat = old.mat;
                        else
                            old.mat = newmat;
                    }

                    if (lookup_case != 0)
                    {
                        old.mix_index = mix_index[old.mat];
                        for (int n=0; n<old.nnodes; n++)
                            old.vfs[n] = newVFs[n];
                    }
                    outlist.push_back(old);
                    continue;
                }

                //
                // Create the output shapes (and centroid points)
                //
                for (int j = 0 ; j < numOutput ; j++)
                {
                    unsigned char shapeType = *splitCase++;

                    if (shapeType == ST_PNT)
                    {
                        int interpID = *splitCase++;
                        int color    = *splitCase++;
                        int npts     = *splitCase++;
                        int pts[MAX_NODES_PER_ZONE];
                        for (int p = 0 ; p < npts ; p++)
                            pts[p] = *splitCase++;

                        CreateCentroidPoint(old, interpID, color, old.mat,
                                            matno, npts, pts);
                    }
                    else
                    {
                        int npts;
                        int color;
                        TempCell cell;

                        switch (shapeType)
                        {
                          case ST_HEX:
                            npts = 8;
                            color = *splitCase++;
                            cell.celltype = VTK_HEXAHEDRON;
                            break;
                          case ST_WDG:
                            npts = 6;
                            color = *splitCase++;
                            cell.celltype = VTK_WEDGE;
                            break;
                          case ST_PYR:
                            npts = 5;
                            color = *splitCase++;
                            cell.celltype = VTK_PYRAMID;
                            break;
                          case ST_TET:
                            npts = 4;
                            color = *splitCase++;
                            cell.celltype = VTK_TETRA;
                            break;
                          case ST_QUA:
                            npts = 4;
                            color = *splitCase++;
                            cell.celltype = VTK_QUAD;
                            break;
                          case ST_TRI:
                            npts = 3;
                            color = *splitCase++;
                            cell.celltype = VTK_TRIANGLE;
                            break;
                          default:
                            EXCEPTION1(ImproperUseException,
                                       "An invalid output shape was found in "
                                       "the ClipCases.");
                        }

                        if (mir.options.smoothing)
                        {
                            cell.mat = (color==COLOR0 ? old.mat : matno);
                        }
                        else
                        {
                            if (newexists && !oldexists)
                                cell.mat = matno;
                            else if (oldexists && !newexists)
                                cell.mat = old.mat;
                            else
                                cell.mat = (color==COLOR0 ? old.mat : matno);
                        }

                        cell.nnodes = npts;
                        cell.mix_index = mix_index[cell.mat];

                        int pts[MAX_NODES_PER_ZONE];
                        for (int p = 0 ; p < npts ; p++)
                            pts[p] = *splitCase++;

                        CreateOutputShape(old, cell, color, npts, pts, matno);
                    }
                }
            }
        }
    }

    // If we'r going to calculate actual volume fractions, first
    // zero them out, then accumulate the output cell partial contributions.
    if (vfs)
    {
        for (int matno=0; matno < nMaterials; matno++)
        {
            vfs[matno] = 0.0;
        }
    }


    // Spit the reconstructed cells into the output zone list
    double totalvol = 0;
    for (int out = 0 ; out < outlist.size() ; out++)
    {
        TempCell &outcell = outlist[out];
        ZooMIR::ReconstructedZone zone;
        zone.origzone   = cellid;
        zone.mat        = outcell.mat;
        zone.celltype   = outcell.celltype;
        zone.nnodes     = outcell.nnodes;
        zone.startindex = mir.indexList.size();
        zone.mix_index  = outcell.mix_index;

        mir.zonesList.push_back(zone);

        for (int n=0; n<outcell.nnodes; n++)
            mir.indexList.push_back(outcell.ids[n]);

        
        if (vfs)
        {
            double coords[MAX_NODES_PER_ZONE][3];
            //cerr << "output cell, orig="<<zone.origzone<<", mat="<<zone.mat<<", #"<<out<<":\n";
            for (int n=0; n<outcell.nnodes; n++)
            {
                int id = outcell.ids[n];
                if (id < mir.origNPoints)
                {
                    coords[n][0] = mir.origXCoords[id];
                    coords[n][1] = mir.origYCoords[id];
                    coords[n][2] = mir.origZCoords[id];
                }
                else
                {
                    coords[n][0] = mir.coordsList[id-mir.origNPoints].x;
                    coords[n][1] = mir.coordsList[id-mir.origNPoints].y;
                    coords[n][2] = mir.coordsList[id-mir.origNPoints].z;
                }
                //cerr <<"   coord = "<<coords[n][0]<<" , "<<coords[n][1]<<" , "<<coords[n][2]<<"\n";
            }
            double vol = CalculateVolume(outcell.celltype, coords);
            totalvol += vol;
            //cerr << "   Volume = "<< vol<<endl;
            vfs[outcell.mat] += vol;
        }
    }

    if (vfs)
    {
        for (int matno=0; matno < nMaterials; matno++)
        {
            vfs[matno] /= totalvol;
        }
    }
}

