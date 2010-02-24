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

#include "vtkVisItClipper.h"
#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkClipDataSet.h>
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
#include <vtkVolumeFromVolume.h>

#include <ImproperUseException.h>

#include <DebugStream.h>

#include <math.h>
#include <vector>

#include <ClipCases.h>
#include <vtkTriangulationTables.h>

vtkCxxRevisionMacro(vtkVisItClipper, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkVisItClipper);

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
//  Constructor:  vtkVisItClipper::vtkVisItClipper
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Aug 29 13:38:08 EDT 2006
//    Added support for leaving cells whole.
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Initialize new data members.
//
// ****************************************************************************

vtkVisItClipper::vtkVisItClipper()
{
    CellList = NULL;
    CellListSize = 0;
    insideOut = false;
    clipFunction = NULL;
    removeWholeCells = false;
    useZeroCrossings = false;
    computeInsideAndOut = false;
    otherOutput = NULL;
    scalarArrayAsVTK = NULL;
    iOwnData = false;
}

// ****************************************************************************
//  Destructor:  vtkVisItClipper::~vtkVisItClipper
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Clean up new data members.
//
// ****************************************************************************
vtkVisItClipper::~vtkVisItClipper()
{
    if (otherOutput)
        otherOutput->Delete();
    if (iOwnData)
        delete [] scalarArray;
    if (scalarArrayAsVTK != NULL)
        scalarArrayAsVTK->Delete();
}

void
vtkVisItClipper::SetUseZeroCrossings(bool use)
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
vtkVisItClipper::SetComputeInsideAndOut(bool compute)
{
    computeInsideAndOut = compute;
}

void
vtkVisItClipper::SetCellList(int *cl, int size)
{
    CellList = cl;
    CellListSize = size;
}

void
vtkVisItClipper::SetClipFunction(vtkImplicitFunction *func)
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
//  Method:  vtkVisItClipper::SetClipScalars
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
//  Creation:    January 30, 2004
//
//  Modifications:
//
//    Jeremy Meredith, Wed May  5 14:48:23 PDT 2004
//    Made it allow only a single cutoff, and use the "insideOut"
//    value to determine if this is a min or max value.
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Change the array argument to be a vtk data type.  Also added support
//    for data types besides "float".
//
// ****************************************************************************

void
vtkVisItClipper::SetClipScalars(vtkDataArray *array, float cutoff)
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
vtkVisItClipper::SetInsideOut(bool io)
{
    insideOut = io;
}

// ****************************************************************************
//  Method:  vtkVisItClipper::SetRemoveWholeCells
//
//  Purpose:
//    Tell the clipper if you want it to treat cells as atomic, and
//    simply remove any cell not entirely within the region.
//
//  Arguments:
//    lcw        the new setting
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
void
vtkVisItClipper::SetRemoveWholeCells(bool rwc)
{
    removeWholeCells = rwc;
}

vtkUnstructuredGrid*
vtkVisItClipper::GetOtherOutput()
{
    return otherOutput;
}

// ****************************************************************************
//  Method:  vtkVisItClipper::Execute
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
//    Jeremy Meredith, Wed Feb 24 10:18:33 EST 2010
//    Initial creation: unified the old rectilinear, structured, unstructured,
//    and polydata execution functions into this single function.
//
// ****************************************************************************
void
vtkVisItClipper::Execute()
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
    vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();

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
        stuff_I_cant_clip->SetPoints(ug->GetPoints());
        stuff_I_cant_clip->GetPointData()->ShallowCopy(ug->GetPointData());
        stuff_I_cant_clip->Allocate(nCells);
    }
    else if (do_type == VTK_POLY_DATA)
    {
        pg = (vtkPolyData*)ds;
        pts_ptr = (float*)pg->GetPoints()->GetVoidPointer(0);
        stuff_I_cant_clip->SetPoints(pg->GetPoints());
        stuff_I_cant_clip->GetPointData()->ShallowCopy(pg->GetPointData());
        stuff_I_cant_clip->Allocate(nCells);

    }
    else
    {
        debug1 << "vtkVisItClipper: Can't operate on this dataset,\n";
        debug1 << "                 reverting to raw VTK code.\n";
        GeneralExecute();
    }

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkVolumeFromVolume vfvIn(ds->GetNumberOfPoints(), ptSizeGuess);
    vtkVolumeFromVolume vfvOut(ds->GetNumberOfPoints(), ptSizeGuess);
    vtkVolumeFromVolume *useVFV;

    const int max_pts = 8;
    int cellType = twoD ? VTK_QUAD : VTK_HEXAHEDRON; // constant for struct grd
    int nCellPts = twoD ? 4 : 8;                     // constant for struct grd
    vtkIdType cellPtsStruct[8];
    vtkIdType *cellPts = cellPtsStruct; // for struct grd, we'll fill it

    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantClip = 0;
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

        // If it's something we can't clip, save it for later
        bool canClip = false;
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
            canClip = true;
            break;

          default:
            canClip = false;
            break;
        }
        if (!canClip)
        {
            if (numIcantClip == 0)
                stuff_I_cant_clip->GetCellData()->
                                       CopyAllocate(ds->GetCellData(), nCells);

            stuff_I_cant_clip->InsertNextCell(cellType, nCellPts, cellPts);
            stuff_I_cant_clip->GetCellData()->
                            CopyData(ds->GetCellData(), cellId, numIcantClip);
            numIcantClip++;
            continue;
        }

        // fill the dist functions and calculate lookup case
        int lookup_case = 0;
        float dist[max_pts];
        for (int j = nCellPts-1 ; j >= 0 ; j--)
        {
            if (clipFunction)
            {
                float ptRect[3];
                float *pt = ptRect;
                if (pts_ptr)
                {
                    pt = pts_ptr + 3*cellPts[j];
                }
                else
                {
                    pt[0] = X[cellI + X_val[j]];
                    pt[1] = Y[cellJ + Y_val[j]];
                    pt[2] = Z[cellK + Z_val[j]];
                }
                dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }
            else // if (scalarArray)
            {
                float val = scalarArray[cellPts[j]];
                dist[j] = scalarCutoff - val;
            }

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

        int            interpIDsIn[4];
        int            interpIDsOut[4];
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

                useVFV = &vfvIn;
                if ((!insideOut && color == COLOR0) ||
                    ( insideOut && color == COLOR1))
                {
                    if (computeInsideAndOut)
                    {
                        useVFV = &vfvOut;
                    }
                    else
                    {
                        // We don't want this one; it's the wrong side.
                        splitCase += npts;
                        continue;
                    }
                }

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
                                
                        shape[p] = useVFV->AddPoint(ptId1, ptId2, percent);
                    }
                    else if (pt >= N0 && pt <= N3)
                    {
                        if (useVFV == &vfvIn)
                            shape[p] = interpIDsIn[pt - N0];
                        else
                            shape[p] = interpIDsOut[pt - N0];
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
                    useVFV->AddHex(cellId,
                                   shape[0], shape[1], shape[2], shape[3],
                                   shape[4], shape[5], shape[6], shape[7]);
                    break;
                  case ST_WDG:
                    useVFV->AddWedge(cellId,
                                     shape[0], shape[1], shape[2],
                                     shape[3], shape[4], shape[5]);
                    break;
                  case ST_PYR:
                    useVFV->AddPyramid(cellId, shape[0], shape[1],
                                       shape[2], shape[3], shape[4]);
                    break;
                  case ST_TET:
                    useVFV->AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_QUA:
                    useVFV->AddQuad(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_TRI:
                    useVFV->AddTri(cellId, shape[0], shape[1], shape[2]);
                    break;
                  case ST_LIN:
                    useVFV->AddLine(cellId, shape[0], shape[1]);
                    break;
                  case ST_VTX:
                    useVFV->AddVertex(cellId, shape[0]);
                    break;
                  case ST_PNT:
                    interpIDsIn[interpID] = vfvIn.AddCentroidPoint(npts, shape);
                    if (computeInsideAndOut)
                        interpIDsOut[interpID] = vfvOut.AddCentroidPoint(npts, shape);
                    break;
                }
            }
        }
    }

    if (numIcantClip > 0)
    {
        vtkUnstructuredGrid *not_from_zoo  = vtkUnstructuredGrid::New();
        ClipDataset(stuff_I_cant_clip, not_from_zoo);
        
        vtkUnstructuredGrid *just_from_zoo = vtkUnstructuredGrid::New();
        if (pts_ptr)
            vfvIn.ConstructDataSet(inPD, inCD, just_from_zoo, pts_ptr);
        else
            vfvIn.ConstructDataSet(inPD, inCD, just_from_zoo, pt_dims,X,Y,Z);

        vtkAppendFilter *appender = vtkAppendFilter::New();
        appender->AddInput(not_from_zoo);
        appender->AddInput(just_from_zoo);
        appender->GetOutput()->Update();

        output->ShallowCopy(appender->GetOutput());

        if (computeInsideAndOut)
        {
            appender->RemoveInput(just_from_zoo);
            just_from_zoo->Delete();

            just_from_zoo = vtkUnstructuredGrid::New();
            if (pts_ptr)
                vfvOut.ConstructDataSet(inPD, inCD, just_from_zoo, pts_ptr);
            else
                vfvOut.ConstructDataSet(inPD, inCD, just_from_zoo, pt_dims,X,Y,Z);

            appender->AddInput(just_from_zoo);
            appender->GetOutput()->Update();

            if (otherOutput) otherOutput->Delete();
            otherOutput = vtkUnstructuredGrid::New();
            otherOutput->ShallowCopy(appender->GetOutput());
        }

        appender->Delete();
        just_from_zoo->Delete();
        not_from_zoo->Delete();
    }
    else
    {
        if (pts_ptr)
            vfvIn.ConstructDataSet(inPD, inCD, output, pts_ptr);
        else
            vfvIn.ConstructDataSet(inPD, inCD, output, pt_dims,X,Y,Z);

        if (computeInsideAndOut)
        {
            if (otherOutput) otherOutput->Delete();
            otherOutput = vtkUnstructuredGrid::New();
            if (pts_ptr)
                vfvOut.ConstructDataSet(inPD, inCD, otherOutput, pts_ptr);
            else
                vfvOut.ConstructDataSet(inPD, inCD, otherOutput, pt_dims,X,Y,Z);
        }
    }

    stuff_I_cant_clip->Delete();
}


void vtkVisItClipper::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}

void vtkVisItClipper::GeneralExecute(void)
{
    ClipDataset(GetInput(), (vtkUnstructuredGrid*)GetOutput());
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sat Mar 27 10:56:08 PST 2004
//    Work-around some funniness with VTK memory management.  (the funniness
//    is a bug with the vtkClipDataSet filter.)
//
//    Hank Childs, Sat Oct  6 15:37:11 PDT 2007
//    Fix bug with setting "inverse" for isovoluming.
//
// ****************************************************************************

void vtkVisItClipper::ClipDataset(vtkDataSet *in_ds,
                                  vtkUnstructuredGrid *out_ds)
{
    vtkClipDataSet *clipData = vtkClipDataSet::New();
    clipData->SetInput(in_ds);
    if (clipFunction)
    {
        clipData->SetClipFunction(clipFunction);
        clipData->GenerateClipScalarsOff();
        clipData->SetInsideOut(insideOut);
    }
    else
    {
        clipData->SetClipFunction(NULL);
        in_ds->GetPointData()->SetScalars(scalarArrayAsVTK);
        clipData->GenerateClipScalarsOff();
        clipData->SetValue(scalarCutoff);
        clipData->SetInsideOut(!insideOut);
    }
    clipData->Update();
    out_ds->ShallowCopy(clipData->GetOutput());
    clipData->Delete();
}

