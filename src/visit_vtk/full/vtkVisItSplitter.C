/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include "vtkVisItSplitter.h"
#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkImplicitFunction.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkQuadric.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkBinaryPartitionVolumeFromVolume.h>

#include <ImproperUseException.h>

#include <DebugStream.h>

#include <math.h>
#include <vector>

#include <ClipCases.h>
#include <vtkTriangulationTables.h>

#include <FixedLengthBitField.h>
#include <TimingsManager.h>

vtkCxxRevisionMacro(vtkVisItSplitter, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkVisItSplitter);

//
// Function: AdjustPercentToZeroCrossing
//
// Purpose: Given coordinate array, point ids and linear estimate of
// a cut, use quadric to compute actual zero crossing and adjust the
// percent value to hit the zero crossing
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
//
static void
AdjustPercentToZeroCrossing(const float *const pts, int ptId1, int ptId2,
    vtkImplicitFunction *func, float *percent)
{
    if (func == 0)
        return;

    // we only handle general quadrics at the moment
    if (strcmp(func->GetClassName(), "vtkQuadric") != 0)
        return;

    //
    // quadric equation coefficient array indexing...
    // x^2   y^2   z^2    xy    xz    yz    x    y    z    1
    //  0     1     2     3     4     5     6    7    8    9
    //
    vtkQuadric *quadric = vtkQuadric::SafeDownCast(func);
    const double *a = quadric->GetCoefficients();

    // quick check for planar functions. They're linear and so
    // 'percent' is already correct
    if (a[0] == 0.0 && a[1] == 0.0 && a[2] == 0.0 &&
        a[3] == 0.0 && a[4] == 0.0 && a[5] == 0.0)
        return;

    //
    // We'll define a "ray" between points p0 and p1 such that a
    // point along it is defined by p(t) = p0 + t * (p1 - p0).
    // When t==0, p(t)==p0 and when t==1, p(t)==p1. So, along
    // the edge between the points p0 and p1, 0<=t<=1
    //
    const float *const p0 = pts + 3*ptId1;
    const float *const p1 = pts + 3*ptId2;

    // origin of "ray" to intersect against the quadric surface
    double x0 = p0[0];
    double y0 = p0[1];
    double z0 = p0[2];

    // direction (non-normalized) of ray to intersect quadric surface
    double xd = p1[0] - x0;
    double yd = p1[1] - y0;
    double zd = p1[2] - z0;

    //
    // compute quadratic equation coefficients for ray/quadric intersection
    // At^2 + Bt + C = 0
    //
    // These equations were obtained from various web resources. However,
    // I am suspect of the equation for the B coefficient as cited on the
    // web. Several sources cite the equation with the commented line. However,
    // there is an asymmetry in it where the coefficient of the a[5] term does
    // not include a xd*z0 contribution analagous to the a[3] and a[4] terms.
    // Empirical results from its use have shown that indeed it is in error.
    // The commented line and this comment is left here in case anyone
    // bothers to check this math against available sources.
    //
    double A = a[0]*xd*xd + a[1]*yd*yd + a[2]*zd*zd +
               a[3]*xd*yd + a[4]*yd*zd + a[5]*xd*zd;
    double B = 2*a[0]*x0*xd + 2*a[1]*y0*yd + 2*a[2]*z0*zd +
               //a[3]*(x0*yd+y0*xd) + a[4]*(y0*zd+yd*z0) + a[5]*x0*zd +
               a[3]*(x0*yd+xd*y0) + a[4]*(y0*zd+yd*z0) + a[5]*(x0*zd+xd*z0) +
               a[6]*xd + a[7]*yd +a[8]*zd;
    double C = a[0]*x0*x0 + a[1]*y0*y0 + a[2]*z0*z0 +
               a[3]*x0*y0 + a[4]*y0*z0 + a[5]*x0*z0 +
               a[6]*x0 + a[7]*y0 + a[8]*z0 + a[9];

    //
    // compute the root(s) of the quadratic equation
    //
    double t = 0.0;
    if (A == 0)
    {
        //
        // We get here if the quadric is really just linear
        //
        if (B == 0)
            t = 0.0;
        else
            t = -C / B;
    }
    else
    {
        //
        // We get here only when the quadric is indeed non-linear
        //
        double disc = B*B - 4*A*C;
        if (disc >= 0.0)
        {
            t = (-B - sqrt(disc)) / (2*A);
            if (t < 0)
                t = (-B + sqrt(disc)) / (2*A);
        }
    }

    if (t > 0.0 && t <= 1.0)
        *percent = 1.0-t;
}

// ****************************************************************************
//  Constructor:  vtkVisItSplitter::vtkVisItSplitter
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 26, 2010
//
//  Modifications:
//
// ****************************************************************************

vtkVisItSplitter::vtkVisItSplitter()
{
    CellList = NULL;
    CellListSize = 0;
    insideOut = false;
    clipFunction = NULL;
    removeWholeCells = false;
    useZeroCrossings = false;
    scalarArrayAsVTK = NULL;
    iOwnData = false;
    oldTags = NULL;
    newTags = NULL;
    newTagBit = 0;
}

// ****************************************************************************
//  Destructor:  vtkVisItSplitter::~vtkVisItSplitter
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 26, 2010
//
//  Modifications:
//
// ****************************************************************************
vtkVisItSplitter::~vtkVisItSplitter()
{
    if (iOwnData)
        delete [] scalarArray;
    if (scalarArrayAsVTK != NULL)
        scalarArrayAsVTK->Delete();
}

void
vtkVisItSplitter::SetUseZeroCrossings(bool use)
{
    if (use && clipFunction && 
        (strcmp(clipFunction->GetClassName(), "vtkQuadric") != 0))
    {
        vtkErrorMacro("UseZeroCrossings set to true allowed only with "
                      "vtkQuadric implicit functions");
        return;
    }

    useZeroCrossings = use;
}

void
vtkVisItSplitter::SetCellList(int *cl, int size)
{
    CellList = cl;
    CellListSize = size;
}

void
vtkVisItSplitter::SetClipFunction(vtkImplicitFunction *func)
{
    if (useZeroCrossings && (strcmp(func->GetClassName(), "vtkQuadric") != 0))
    {
        vtkErrorMacro("Only vtkQuadric implicit functions "
                      "allowed with UseZeroCrossings set to true");
        return;
    }

    // Set the clip function
    clipFunction = func;

    // Clear the scalar array so we know to use the clip function
    scalarArray = NULL;
}

// ****************************************************************************
//  Method:  vtkVisItSplitter::SetClipScalars
//
//  Purpose:
//    Set the scalar array used for clipping, and the cutoff.
//    To clip to a range, execute this filter once for the minimum
//    and once for the maximum.
//
//  Arguments:
//    array      the scalar array
//    cutoff     the cutoff
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 26, 2010
//
//  Modifications:
//   Jeremy Meredith, Fri Feb 26 13:31:31 EST 2010
//   Initial creation, copied from vtkVisItClipper.
//
// ****************************************************************************

void
vtkVisItSplitter::SetClipScalars(vtkDataArray *array, float cutoff)
{
    if (iOwnData)
    {
        delete [] scalarArray;
        iOwnData = false;
    }
    if (scalarArrayAsVTK != NULL)
    {
        scalarArrayAsVTK->Delete();
        scalarArrayAsVTK = NULL;
    }

    // Clear the clip function so we know to use scalars
    clipFunction = NULL;

    // Set the scalar array
    scalarArrayAsVTK = array;
    scalarArrayAsVTK->Register(NULL);
    if (array->GetDataType() == VTK_FLOAT)
    {
        scalarArray = (float *) array->GetVoidPointer(0);
    }
    else
    {
        iOwnData = true;
        int nTuples = array->GetNumberOfTuples();
        scalarArray = new float[nTuples];
        for (int i = 0 ; i < nTuples ; i++)
        {
            scalarArray[i] = array->GetTuple1(i);
        }
    }

    // Set the cutoff
    scalarCutoff     = cutoff;
}

void
vtkVisItSplitter::SetInsideOut(bool io)
{
    insideOut = io;
}


void
vtkVisItSplitter::SetOldTagBitField(std::vector<FixedLengthBitField<16> > *tags)
{
    oldTags = tags;
}

void
vtkVisItSplitter::SetNewTagBitField(std::vector<FixedLengthBitField<16> > *tags)
{
    newTags = tags;
}

void
vtkVisItSplitter::SetNewTagBit(int bit)
{
    newTagBit = bit;
}

// ****************************************************************************
//  Method:  vtkVisItSplitter::SetRemoveWholeCells
//
//  Purpose:
//    Tell the clipper if you want it to treat cells as atomic, and
//    simply remove any cell not entirely within the region.
//
//  Arguments:
//    lcw        the new setting
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 26, 2010
//
// ****************************************************************************
void
vtkVisItSplitter::SetRemoveWholeCells(bool rwc)
{
    removeWholeCells = rwc;
}

// ****************************************************************************
//  Method:  vtkVisItSplitter::Execute
//
//  Purpose:
//    Main execution method.  
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 24, 2010
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 26 13:31:31 EST 2010
//    Initial creation, copied from vtkVisItClipper and modified to
//    merely split+tag output, not remove cells.
//
// ****************************************************************************
void
vtkVisItSplitter::Execute()
{
    vtkDataSet *ds = GetInput();

    int do_type = ds->GetDataObjectType();
    vtkRectilinearGrid   *rg = NULL;
    vtkStructuredGrid    *sg = NULL;
    vtkUnstructuredGrid  *ug = NULL;
    vtkPolyData          *pg = NULL;

    // coordinate arrays for any mesh type
    float      *X       = NULL;
    float      *Y       = NULL;
    float      *Z       = NULL;
    float      *pts_ptr = NULL;

    // dimensions for structured grids
    int pt_dims[3];
    int cell_dims[3];
    int strideY;
    int strideZ;
    int ptstrideY;
    int ptstrideZ;

    // indices to convert structured grid cells to hexahedron/quadrilateral
    const int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    const int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    const int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };

    // Set general input/output data
    int                  nCells = ds->GetNumberOfCells();
    vtkCellData         *inCD   = ds->GetCellData();
    vtkPointData        *inPD   = ds->GetPointData();
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    bool twoD = false;
    if (do_type == VTK_RECTILINEAR_GRID || do_type == VTK_STRUCTURED_GRID)
    {
        if (do_type == VTK_RECTILINEAR_GRID)
        {
            rg = (vtkRectilinearGrid*)ds;
            rg->GetDimensions(pt_dims);
            X = (float* ) rg->GetXCoordinates()->GetVoidPointer(0);
            Y = (float* ) rg->GetYCoordinates()->GetVoidPointer(0);
            Z = (float* ) rg->GetZCoordinates()->GetVoidPointer(0);
        }
        else // do_type == VTK_STRUCTURED_GRID
        {
            sg = (vtkStructuredGrid*)ds;
            sg->GetDimensions(pt_dims);
            pts_ptr = (float*)sg->GetPoints()->GetVoidPointer(0);
        }

        twoD = (pt_dims[2] <= 1);
        cell_dims[0] = pt_dims[0]-1;
        cell_dims[1] = pt_dims[1]-1;
        cell_dims[2] = pt_dims[2]-1;
        strideY = cell_dims[0];
        strideZ = cell_dims[0]*cell_dims[1];
        ptstrideY = pt_dims[0];
        ptstrideZ = pt_dims[0]*pt_dims[1];
    }
    else if (do_type == VTK_UNSTRUCTURED_GRID)
    {
        ug = (vtkUnstructuredGrid*)ds;
        pts_ptr = (float*)ug->GetPoints()->GetVoidPointer(0);
    }
    else if (do_type == VTK_POLY_DATA)
    {
        pg = (vtkPolyData*)ds;
        pts_ptr = (float*)pg->GetPoints()->GetVoidPointer(0);
    }
    else
    {
        debug1 << "vtkVisItSplitter: Can't operate on this dataset.\n";
    }

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    if (clipFunction)
    {
        int th = visitTimer->StartTimer();
        int npts = ds->GetNumberOfPoints();
        scalarArray = new float[npts];
        scalarCutoff = 0;
        if (pts_ptr)
        {
            for (int i=0; i<npts; i++)
            {
                float *pt =  pts_ptr + 3*i;
                scalarArray[i] = 
                   -clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }
        }
        else
        {
            int ctr = 0;
            for (int k=0; k<pt_dims[2]; k++)
            {
                for (int j=0; j<pt_dims[1]; j++)
                {
                    for (int i=0; i<pt_dims[0]; i++)
                    {
                        scalarArray[ctr++] = 
                           -clipFunction->EvaluateFunction(X[i],Y[j],Z[k]);
                        //cerr << "i="<<i<<" j="<<j<<" k="<<k<<" val="<<scalarArray[ctr-1]<<endl;
                    }
                }
            }
        }
        visitTimer->StopTimer(th, "Calculating clipfunction values");
    }

    vtkBinaryPartitionVolumeFromVolume vfv(ds->GetNumberOfPoints(),
                                           ptSizeGuess);

    const int max_pts = 8;
    int cellType = twoD ? VTK_QUAD : VTK_HEXAHEDRON; // constant for struct grd
    int nCellPts = twoD ? 4 : 8;                     // constant for struct grd
    vtkIdType cellPtsStruct[8];
    vtkIdType *cellPts = cellPtsStruct; // for struct grd, we'll fill it

    int th1 = visitTimer->StartTimer();
    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    for (int i = 0 ; i < nToProcess ; i++)
    {
        // Get the cell details
        int cellId = (CellList != NULL ? CellList[i] : i);
        int cellI = -1;
        int cellJ = -1;
        int cellK = -1;
        if (ug)
        {
            cellType = ug->GetCellType(cellId);
            ug->GetCellPoints(cellId, nCellPts, cellPts);
            // don't need cellI/J/K
        }
        else if (pg)
        {
            cellType = pg->GetCellType(cellId);
            pg->GetCellPoints(cellId, nCellPts, cellPts);
            // don't need cellI/J/K
        }
        else // structured grid
        {
            // cellType already set
            // nCellPts already set
            cellI = cellId % cell_dims[0];
            cellJ = (cellId/strideY) % cell_dims[1];
            cellK = (cellId/strideZ);
            for (int j = 0; j<nCellPts; j++)
            {
                cellPts[j] = (cellI + X_val[j]) +
                             (cellJ + Y_val[j])*ptstrideY +
                             (cellK + Z_val[j])*ptstrideZ;
            }
        }

        // If it's something we can't clip, skip this output
        switch (cellType)
        {
          case VTK_TETRA:
          case VTK_PYRAMID:
          case VTK_WEDGE:
          case VTK_HEXAHEDRON:
          case VTK_VOXEL:
          case VTK_TRIANGLE:
          case VTK_QUAD:
          case VTK_PIXEL:
          case VTK_LINE:
          case VTK_VERTEX:
            break;

          default:
            continue;
            break;
        }

        // fill the dist functions and calculate lookup case
        int lookup_case = 0;
        float dist[max_pts];
        for (int j = nCellPts-1 ; j >= 0 ; j--)
        {
            float val = scalarArray[cellPts[j]];
            dist[j] = scalarCutoff - val;

            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        if (removeWholeCells && lookup_case != 0)
            lookup_case = ((1 << nCellPts) - 1);

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
                int npts;
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

                bool out = ((!insideOut && color == COLOR0) ||
                            ( insideOut && color == COLOR1));

                int shape[8];
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

                        // deal with exact zero crossings if requested
                        if (clipFunction && useZeroCrossings)
                        {
                            if (pts_ptr)
                            {
                                AdjustPercentToZeroCrossing(pts_ptr,
                                                            ptId1, ptId2,
                                                            clipFunction,
                                                            &percent);
                            }
                            else
                            {
                                // fake a little points array for rgrids
                                float pt[6];
                                pt[0] = X[cellI + X_val[pt1]];
                                pt[1] = Y[cellJ + Y_val[pt1]];
                                pt[2] = Z[cellK + Z_val[pt1]];
                                pt[3] = X[cellI + X_val[pt2]];
                                pt[4] = Y[cellJ + Y_val[pt2]];
                                pt[5] = Z[cellK + Z_val[pt2]];
                                AdjustPercentToZeroCrossing(pt,
                                                            0, 1,
                                                            clipFunction,
                                                            &percent);
                            }
                        }
                                
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

                switch (shapeType)
                {
                  case ST_HEX:
                    vfv.AddHex(cellId,
                               shape[0], shape[1], shape[2], shape[3],
                               shape[4], shape[5], shape[6], shape[7],
                               !out);
                    break;
                  case ST_WDG:
                    vfv.AddWedge(cellId,
                                 shape[0], shape[1], shape[2],
                                 shape[3], shape[4], shape[5],
                                 !out);
                    break;
                  case ST_PYR:
                    vfv.AddPyramid(cellId, shape[0], shape[1],
                                   shape[2], shape[3], shape[4],
                                   !out);
                    break;
                  case ST_TET:
                    vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3],
                               !out);
                    break;
                  case ST_QUA:
                    vfv.AddQuad(cellId, shape[0], shape[1], shape[2], shape[3],
                                !out);
                    break;
                  case ST_TRI:
                    vfv.AddTri(cellId, shape[0], shape[1], shape[2],
                               !out);
                    break;
                  case ST_LIN:
                    vfv.AddLine(cellId, shape[0], shape[1],
                                !out);
                    break;
                  case ST_VTX:
                    vfv.AddVertex(cellId, shape[0],
                                  !out);
                    break;
                  case ST_PNT:
                    interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                    break;
                }
            }
        }
    }
    visitTimer->StopTimer(th1, "Performing clipping");

    if (clipFunction)
    {
        delete[] scalarArray;
        scalarArray = NULL;
    }

    int th2 = visitTimer->StartTimer();
    if (pts_ptr)
        vfv.ConstructDataSet(inPD, inCD, output, pts_ptr,
                             oldTags, newTags, newTagBit);
    else
        vfv.ConstructDataSet(inPD, inCD, output, pt_dims,X,Y,Z,
                             oldTags, newTags, newTagBit);
    visitTimer->StopTimer(th2, "VFV Constructing data set");
}


void vtkVisItSplitter::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}
