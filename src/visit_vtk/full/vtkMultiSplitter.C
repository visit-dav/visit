/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <vtkMultiSplitter.h>

#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkCSGFixedLengthBitField.h>
#include <vtkFloatArray.h>
#include <vtkImplicitFunction.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkQuadric.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVolumeFromCSGVolume.h>

#include <ImproperUseException.h>

#include <DebugStream.h>

#include <math.h>
#include <vector>

#include <ClipCases.h>
#include <vtkTriangulationTables.h>

#include <TimingsManager.h>

vtkStandardNewMacro(vtkMultiSplitter);

// ****************************************************************************
//  Constructor:  vtkMultiSplitter::vtkMultiSplitter
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//
// ****************************************************************************

vtkMultiSplitter::vtkMultiSplitter()
{
    bounds  = NULL;
    nBounds = 0;
    newTags = NULL;
}

// ****************************************************************************
//  Destructor:  vtkMultiSplitter::~vtkMultiSplitter
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//
// ****************************************************************************
vtkMultiSplitter::~vtkMultiSplitter()
{
}

// ****************************************************************************
//  Method:  vtkMultiSplitter::SetClipFunctions
//
//  Purpose:
//    Set the functions to clip against.  
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//
// ****************************************************************************

void
vtkMultiSplitter::SetClipFunctions(double *bnds, int nBnds)
{
    bounds = bnds;
    nBounds = nBnds;
}

// ****************************************************************************
//  Method:  vtkMultiSplitter::SetClipFunctions
//
//  Purpose:
//    Set the tag bits.  
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:22:21 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

void
vtkMultiSplitter::SetTagBitField(std::vector<vtkCSGFixedLengthBitField> *tags)
{
    newTags = tags;
}

// ****************************************************************************
//  Method:  vtkMultiSplitter::RequestData
//
//  Purpose:
//    Main execution method.  
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:22:21 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

int
vtkMultiSplitter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    vtkRectilinearGrid *rg = vtkRectilinearGrid::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    //
    // Set general input/output data
    //
    vtkCellData *inCD = rg->GetCellData();

    //
    // Populate the vfv with all the hexes.
    //
    int        dims[3];
    rg->GetDimensions(dims);

    float *X   = (float* ) rg->GetXCoordinates()->GetVoidPointer(0);
    float *Y   = (float* ) rg->GetYCoordinates()->GetVoidPointer(0);
    float *Z   = (float* ) rg->GetZCoordinates()->GetVoidPointer(0);

    int npts = dims[0] * dims[1] * dims[2];
    std::vector<float> pts;
    pts.reserve(npts*3);
    int ndx = 0;
    for (int k = 0; k < dims[2]; k++)
    {
        for (int j = 0; j < dims[1]; j++)
        {
            for (int i = 0; i < dims[0]; i++)
            {
                 pts.push_back(X[i]);
                 pts.push_back(Y[j]);
                 pts.push_back(Z[k]);
            }
        }
    }

    int ptSizeGuess = (int) pow(float(npts), 0.6667f) * 5 + 100;
    vtkVolumeFromCSGVolume vfv(npts, ptSizeGuess);

    ndx = 0;
    int nx = dims[0];
    int ny = dims[1];
    vtkCSGFixedLengthBitField bf_zero;
    for (int k = 0; k < dims[2] - 1; k++)
    {
        for (int j = 0; j < dims[1] - 1; j++)
        {
            for (int i = 0; i < dims[0] - 1; i++)
            {
                vfv.AddHex(ndx, 
                           (i)   + (j)  *nx + (k)  *nx*ny,
                           (i+1) + (j)  *nx + (k)  *nx*ny,
                           (i+1) + (j+1)*nx + (k)  *nx*ny,
                           (i)   + (j+1)*nx + (k)  *nx*ny,
                           (i)   + (j)  *nx + (k+1)*nx*ny,
                           (i+1) + (j)  *nx + (k+1)*nx*ny,
                           (i+1) + (j+1)*nx + (k+1)*nx*ny,
                           (i)   + (j+1)*nx + (k+1)*nx*ny,
                           bf_zero);
                ndx++;
            }
        }
    }
    
    //
    // Loop over the boundaries.
    //
    debug1 << "vtkMultiSplitter: nBounds = " << nBounds;
    int outCases[9] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
    vtkQuadric *clipFunction =  vtkQuadric::New();
    for (int iBnd = 0; iBnd < nBounds; iBnd++)
    {
        //
        // Create the array of the clip values for the current boundary.
        //
        clipFunction->SetCoefficients(&bounds[iBnd*10]);
        npts = (int)pts.size() / 3;
        float *clipArray = new float[npts];
        for (int i = 0; i < npts; i++)
        {
            float *pt = &pts[3*i];
            clipArray[i] = 
               -clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
        }

        vfv.InitTraversal();
    
        int nCells = vfv.GetNumberOfCells();
        for (int iCell = 0; iCell < nCells; iCell++)
        {
            const vtkIdType *cellPts = vfv.GetCell();
            if (cellPts[0] == -1)
            {
                vfv.NextCell();
                continue;
            }
            int nCellPts = vfv.GetCellSize();
            int out_case = outCases[nCellPts];
            int cellType = vfv.GetCellVTKType();
            int cellId = cellPts[0];
            cellPts++;

            // fill the dist functions and calculate lookup case
            int lookup_case = 0;
            const int max_pts = 8;
            float dist[max_pts];
            for (int j = nCellPts-1 ; j >= 0 ; j--)
            {
                float val = clipArray[cellPts[j]];
                dist[j] = - val;

                if (dist[j] >= 0)
                    lookup_case++;
                if (j > 0)
                    lookup_case *= 2;
            }

            if (lookup_case == 0)
            {
                vfv.SetTagBit(iBnd);
                vfv.NextCell();
                continue;
            }
            else if (lookup_case == out_case)
            {
                vfv.NextCell();
                continue;
            }

            unsigned char  *splitCase = NULL;
            int             numOutput = 0;
            typedef int     edgeIndices[2];
            edgeIndices    *vertices_from_edges = NULL;

            int startIndex;
            switch (cellType)
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
              case VTK_LINE:
                startIndex = startClipShapesLin[lookup_case];
                splitCase  = &clipShapesLin[startIndex];
                numOutput  = numClipShapesLin[lookup_case];
                vertices_from_edges = lineVerticesFromEdges;
                break;
              case VTK_VERTEX:
                startIndex = startClipShapesVtx[lookup_case];
                splitCase  = &clipShapesVtx[startIndex];
                numOutput  = numClipShapesVtx[lookup_case];
                vertices_from_edges = NULL;
                break;
            }

            int interpIDs[4];
            for (int j = 0 ; j < numOutput ; j++)
            {
                unsigned char shapeType = *splitCase++;
                {
                    vtkIdType npts;
                    int interpID = -1;
                    int color    = -1;
                    switch (shapeType)
                    {
                      case ST_HEX:
                        npts = 8;
                        color = *splitCase++;
                        break;
                      case ST_WDG:
                        npts = 6;
                        color = *splitCase++;
                        break;
                      case ST_PYR:
                        npts = 5;
                        color = *splitCase++;
                        break;
                      case ST_TET:
                        npts = 4;
                        color = *splitCase++;
                        break;
                      case ST_QUA:
                        npts = 4;
                        color = *splitCase++;
                        break;
                      case ST_TRI:
                        npts = 3;
                        color = *splitCase++;
                        break;
                      case ST_LIN:
                        npts = 2;
                        color = *splitCase++;
                        break;
                      case ST_VTX:
                        npts = 1;
                        color = *splitCase++;
                        break;
                      case ST_PNT:
                        interpID = *splitCase++;
                        color    = *splitCase++;
                        npts     = *splitCase++;
                        break;
                      default:
                        EXCEPTION1(ImproperUseException,
                                   "An invalid output shape was found in "
                                   "the ClipCases.");
                    }

                    bool out = (color == COLOR0);

                    vtkIdType shape[8];
                    for (int p = 0 ; p < npts ; p++)
                    {
                        unsigned char pt = *splitCase++;
                        if (pt <= P7)
                        {
                            // We know pt P0 must be >P0 since we already
                            // assume P0 == 0.  This is why we do not
                            // bother subtracting P0 from pt here.
                            shape[p] = cellPts[pt];
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
                            float dir = dist[pt2] - dist[pt1];
                            float amt = 0. - dist[pt1];
                            float percent = 1. - (amt / dir);

                            // We may have physically (though not logically)
                            // degenerate cells if percent==0 or percent==1.
                            // We could pretty easily and mostly safely clamp
                            // percent to the range [1e-4, 1. - 1e-4] here.
                            int ptId1 = cellPts[pt1];
                            int ptId2 = cellPts[pt2];

                            shape[p] = vfv.AddPoint(ptId1, ptId2, percent);
                        }
                        else if (pt >= N0 && pt <= N3)
                        {
                            shape[p] = interpIDs[pt - N0];
                        }
                        else
                        {
                            EXCEPTION1(ImproperUseException,
                                       "An invalid output point value "
                                       "was found in the ClipCases.");
                        }
                    }

                    vtkCSGFixedLengthBitField bf;
                    if (shapeType != ST_PNT)
                    {
                        bf = vfv.GetTag();
                        if (out)
                            bf.SetBit(iBnd);
                    }
                    switch (shapeType)
                    {
                      case ST_HEX:
                        vfv.AddHex(cellId,
                                   shape[0], shape[1], shape[2], shape[3],
                                   shape[4], shape[5], shape[6], shape[7], bf);
                        break;
                      case ST_WDG:
                        vfv.AddWedge(cellId,
                                     shape[0], shape[1], shape[2],
                                     shape[3], shape[4], shape[5], bf);
                        break;
                      case ST_PYR:
                        vfv.AddPyramid(cellId, shape[0], shape[1],
                                       shape[2], shape[3], shape[4], bf);
                        break;
                      case ST_TET:
                        vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3], bf);
                        break;
                      case ST_QUA:
                        vfv.AddQuad(cellId, shape[0], shape[1], shape[2], shape[3], bf);
                        break;
                      case ST_TRI:
                        vfv.AddTri(cellId, shape[0], shape[1], shape[2], bf);
                        break;
                      case ST_LIN:
                        vfv.AddLine(cellId, shape[0], shape[1], bf);
                        break;
                      case ST_VTX:
                        vfv.AddVertex(cellId, shape[0], bf);
                        break;
                      case ST_PNT:
                        interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                        break;
                    }
                }
            }
            vfv.InvalidateCell();
            vfv.NextCell();
        }
        delete [] clipArray;
        vfv.UpdatePoints(pts);
    }

    vfv.ConstructDataSet(inCD, output, &pts[0], (int)pts.size()/3, newTags);

    return 1;
}

// ****************************************************************************
//  Method: vtkMultiSplitter::FillInputPortInformation
//
// ****************************************************************************

int
vtkMultiSplitter::FillInputPortInformation(int, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkRectilinearGrid");
    return 1;
}

// ****************************************************************************
//  Method: vtkMultiSplitter::PrintSelf
//
// ****************************************************************************

void vtkMultiSplitter::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}
