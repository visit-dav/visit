// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "IsovolumeCellReconstructor.h"

#include <ClipCases.h>
#include <vtkCellTypes.h>
#include <avtMaterial.h>
#include <ZooMIR.h>
#include <ImproperUseException.h>
#include <vtkTriangulationTables.h>

#include "BitUtils.h"
#include "ResampledMat.h"

// ****************************************************************************
//  Constructor:  IsovolumeCellReconstructor::IsovolumeCellReconstructor
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 18, 2005
//
//  Modifications:
//    Kathleen Biagas, Thu Aug 11 2022
//    Removed MIRConnectivity from constructor, as it isn't used.
//
// ****************************************************************************
IsovolumeCellReconstructor::IsovolumeCellReconstructor(vtkDataSet *d,
                                                       avtMaterial *m,
                                                       ResampledMat &r,
                                                       int np, int nc,
                                                       ZooMIR &z)
    : CellReconstructor(d,m,r,np,nc,false,z)
{
}


// ****************************************************************************
//  Method:  IsovolumeCellReconstructor::ReconstructCell
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
//  Creation:    August 18, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 18 17:09:36 PDT 2005
//    Initial version.  Lifted heavily from the Recursive version.
//
//    Jeremy Meredith, Fri Feb 13 11:05:06 EST 2009
//    Added calculation of output vf's per material, if requested.
//
//    Jeremy Meredith, Tue Jun 18 11:56:22 EDT 2013
//    Output actual volumes/areas, not VF's, and return total vol/area.
//
// ****************************************************************************
double
IsovolumeCellReconstructor::ReconstructCell(int cellid_, int celltype_,
                                            int nids_, vtkIdType *ids_,
                                            double *outputvols)
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
        int n;
        for (n = 0 ; n < nids ; n++)
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

        //
        // Put in a fake background material
        //

        //
        // Fill the VF's array and calculate the clip case
        //
        int lookup_case = 0;
        for (n = nids-1 ; n >= 0 ; n--)
        {
            // node is from original mesh
            newVFs[n] = nodeVFs[n][matno];
            vfDiff[n] = newVFs[n] - mir.options.isovolumeVF;

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

        switch (celltype)
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
            if (lookup_case != 0)
            {
                TempCell cell;
                for (n = 0 ; n < nids ; n++)
                {
                    cell.localids[n] = n;
                    cell.ids[n] = ids[n];
                    cell.vfs[n] = newVFs[n];
                }
                cell.mat = matno;
                cell.celltype = celltype;
                cell.nnodes = nids;
                cell.mix_index = mix_index[matno];

                outlist.push_back(cell);
            }
            continue;
        }

        //
        // Fake an old cell for now
        //
        TempCell old;
        for (n = 0 ; n < nids ; n++)
        {
            old.localids[n] = n;
            old.ids[n] = ids[n];
            old.vfs[n] = newVFs[n];
        }
        old.mat = matno;
        old.celltype = celltype;
        old.nnodes = nids;
        old.mix_index = mix_index[matno];


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

                CreateCentroidPoint(old, interpID, color, matno,
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

                cell.mat = matno;
                cell.nnodes = npts;
                cell.mix_index = mix_index[cell.mat];

                int pts[MAX_NODES_PER_ZONE];
                for (int p = 0 ; p < npts ; p++)
                    pts[p] = *splitCase++;

                if (color != COLOR1)
                    continue;

                CreateOutputShape(old, cell, color, npts, pts, matno);
            }
        }
    }

    // If we're going to calculate actual volume fractions, first
    // zero them out, then accumulate the output cell partial contributions.
    double totalvol = 0;
    if (outputvols)
    {
        for (int matno=0; matno < nMaterials; matno++)
        {
            outputvols[matno] = 0.0;
        }
    }

    // Spit the reconstructed cells into the output zone list
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

        if (outputvols)
        {
            double coords[MAX_NODES_PER_ZONE][3];
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
            }
            double vol = CalculateVolumeOrAreaHelper(outcell.celltype, coords);
            totalvol += vol;
            outputvols[outcell.mat] += vol;
        }
    }

    return totalvol;
}

