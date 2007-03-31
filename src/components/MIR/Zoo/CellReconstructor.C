#include "CellReconstructor.h"

#include <ClipCases.h>
#include <vtkCellTypes.h>
#include <avtMaterial.h>
#include <ZooMIR.h>
#include <ImproperUseException.h>
#include <vtkTriangulationTables.h>

#include "BitUtils.h"
#include "ResampledMat.h"

// ****************************************************************************
//  Constructor: CellReconstructor::CellReconstructor
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
CellReconstructor::CellReconstructor(vtkDataSet *d,
                                     avtMaterial *m,
                                     ResampledMat &r,
                                     int np, int nc,
                                     MIRConnectivity &mc,
                                     ZooMIR &z)
    : mesh(d), mat(m), rm(r), nPoints(np), nCells(nc), conn(mc), mir(z),
      nMaterials(mat->GetNMaterials()),
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
//  Method:  CellReconstructor::ReconstructCell
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
// ****************************************************************************
void
CellReconstructor::ReconstructCell(int cellid_, int celltype_,
                                   int nids_, int *ids_)
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
        if (nm==1 && matno!=mat->GetMatlist()[cellid])
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
            Array<TempCell>::Swap(outlist, tmplist);
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
    }
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
            ZooMIR::EdgeHashEntry *edge = edges.GetEdge(ptId1,ptId2);
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
