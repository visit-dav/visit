/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
    // there is an assemtry in it where the coefficient of the a[5] term does
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
//    Main execution method.  Delegate to mesh-specific functions.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 16 19:07:02 PST 2004
//    Added PolyData execution path.
//
// ****************************************************************************
void
vtkVisItClipper::Execute()
{
    vtkDataSet *input  = GetInput();

    int do_type = input->GetDataObjectType();
    if (do_type == VTK_RECTILINEAR_GRID)
    {
        RectilinearGridExecute();
    }
    else if (do_type == VTK_STRUCTURED_GRID)
    {
        StructuredGridExecute();
    }
    else if (do_type == VTK_UNSTRUCTURED_GRID)
    {
        UnstructuredGridExecute();
    }
    else if (do_type == VTK_POLY_DATA)
    {
        PolyDataExecute();
    }
    else
    {
        debug1 << "vtkVisItClipper: Can't operate on this dataset\n";
        GeneralExecute();
    }
}


// ****************************************************************************
//  Method:  vtkVisItClipper::StructuredGridExecute
//
//  Purpose:
//    Clips a structured grid.
//
//  Arguments:
//    none
//
//  Note:  most of the setup logic came from Hank's new vtkSlicer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 15 17:33:03 PDT 2003
//    Added ability for centroid-points to have an associated color.
//    This was needed for material interface reconstruction when it was
//    important to know if we should interpolate COLOR0's material or
//    COLOR1's material to come up with a material volume fraction for
//    the new point; it was not needed here, but we must skip over it.
//
//    Jeremy Meredith, Fri Jan 30 17:27:23 PST 2004
//    Added support for using a scalar array to clip against.
//
//    Jeremy Meredith, Mon Feb 16 19:07:24 PST 2004
//    Added polygonal cell support.
//
//    Jeremy Meredith, Wed May  5 14:49:55 PDT 2004
//    Made it support 2d cases as well.  Changed it to a single cutoff
//    for scalars to make the math more robust.
//
//    Brad Whitlock, Thu Aug 12 14:48:46 PST 2004
//    Added float casts to pow() arguments so it builds on MSVC7.Net.
//
//    Kathleen Bonnell, Tue Sep  6 08:45:16 PDT 2005
//    Added call to SetUpClipFunction. 
//
//    Jeremy Meredith, Tue Aug 29 16:20:25 EDT 2006
//    Added support for "atomic" cells that must be removed
//    entirely if they cannot be left whole.
//    Added support for line and vertex output shapes (though
//    structured grids shouldn't be outputting any, of course).
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to adjust percent to new percent consistent with zero
//    crossing of implicit func.
// ****************************************************************************

void
vtkVisItClipper::StructuredGridExecute(void)
{
    int  i, j;

    vtkStructuredGrid *sg = (vtkStructuredGrid *) GetInput();
    int pt_dims[3];
    sg->GetDimensions(pt_dims);
    bool twoD = (pt_dims[2] <= 1);

    int                nCells = sg->GetNumberOfCells();
    vtkPoints         *inPts  = sg->GetPoints();
    vtkCellData       *inCD   = sg->GetCellData();
    vtkPointData      *inPD   = sg->GetPointData();
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(sg->GetNumberOfPoints(), ptSizeGuess);

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    int cell_dims[3];
    cell_dims[0] = pt_dims[0]-1;
    cell_dims[1] = pt_dims[1]-1;
    cell_dims[2] = pt_dims[2]-1;
    int strideY = cell_dims[0];
    int strideZ = cell_dims[0]*cell_dims[1];
    int ptstrideY = pt_dims[0];
    int ptstrideZ = pt_dims[0]*pt_dims[1];
    int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    for (i = 0 ; i < nToProcess ; i++)
    {
        int cellId = (CellList != NULL ? CellList[i] : i);
        int cellI = cellId % cell_dims[0];
        int cellJ = (cellId/strideY) % cell_dims[1];
        int cellK = (cellId/strideZ);
        int lookup_case = 0;
        float dist[8];
        int nCellPts = twoD ? 4 : 8;
        SetUpClipFunction(cellId);
        for (j = nCellPts-1 ; j >= 0 ; j--)
        {
            int ptId = (cellI + X_val[j]) + (cellJ + Y_val[j])*ptstrideY +
                       (cellK + Z_val[j])*ptstrideZ;

            if (clipFunction)
            {
                float *pt = pts_ptr + 3*ptId;
                dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }
            else // if (scalarArray)
            {
                float val = scalarArray[ptId];
                dist[j] = scalarCutoff - val;
            }

            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        if (removeWholeCells && lookup_case != 0)
            lookup_case = ((1 << nCellPts) - 1);

        unsigned char *splitCase;
        int            numOutput;
        int            interpIDs[4];
        if (twoD)
        {
            splitCase = &clipShapesQua[startClipShapesQua[lookup_case]];
            numOutput = numClipShapesQua[lookup_case];
        }
        else
        {
            splitCase = &clipShapesHex[startClipShapesHex[lookup_case]];
            numOutput = numClipShapesHex[lookup_case];
        }

        for (j = 0 ; j < numOutput ; j++)
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

                if ((!insideOut && color == COLOR0) ||
                    ( insideOut && color == COLOR1))
                {
                    // We don't want this one; it's the wrong side.
                    splitCase += npts;
                    continue;
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
                        shape[p] = ((cellI + X_val[pt]) +
                                    (cellJ + Y_val[pt])*ptstrideY +
                                    (cellK + Z_val[pt])*ptstrideZ);
                    }
                    else if (pt >= EA && pt <= EL)
                    {
                        int pt1 = hexVerticesFromEdges[pt-EA][0];
                        int pt2 = hexVerticesFromEdges[pt-EA][1];
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
                        // percent to the range [1e-4, 1. - 1e-4] right here.
                        int ptId1 = ((cellI + X_val[pt1]) +
                                     (cellJ + Y_val[pt1])*ptstrideY +
                                     (cellK + Z_val[pt1])*ptstrideZ);
                        int ptId2 = ((cellI + X_val[pt2]) +
                                     (cellJ + Y_val[pt2])*ptstrideY +
                                     (cellK + Z_val[pt2])*ptstrideZ);

                        // deal with exact zero crossings if requested
                        if (clipFunction && useZeroCrossings)
                            AdjustPercentToZeroCrossing(pts_ptr, ptId1, ptId2,
                                clipFunction, &percent);
                                
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
                               shape[4], shape[5], shape[6], shape[7]);
                    break;
                  case ST_WDG:
                    vfv.AddWedge(cellId,
                                 shape[0], shape[1], shape[2],
                                 shape[3], shape[4], shape[5]);
                    break;
                  case ST_PYR:
                    vfv.AddPyramid(cellId, shape[0], shape[1],
                                   shape[2], shape[3], shape[4]);
                    break;
                  case ST_TET:
                    vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_QUA:
                    vfv.AddQuad(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_TRI:
                    vfv.AddTri(cellId, shape[0], shape[1], shape[2]);
                    break;
                  case ST_LIN:
                    vfv.AddLine(cellId, shape[0], shape[1]);
                    break;
                  case ST_VTX:
                    vfv.AddVertex(cellId, shape[0]);
                    break;
                  case ST_PNT:
                    interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                    break;
                }
            }

        }
    }

    vfv.ConstructDataSet(inPD, inCD, output, pts_ptr);
}

// ****************************************************************************
//  Method:  vtkVisItClipper::RectilinearGridExecute
//
//  Purpose:
//    Clips a rectilinear grid.
//
//  Arguments:
//    none
//
//  Note:  most of the setup logic came from Hank's new vtkSlicer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 15 17:33:03 PDT 2003
//    Added ability for centroid-points to have an associated color.
//    This was needed for material interface reconstruction when it was
//    important to know if we should interpolate COLOR0's material or
//    COLOR1's material to come up with a material volume fraction for
//    the new point; it was not needed here, but we must skip over it.
//
//    Jeremy Meredith, Fri Jan 30 17:27:23 PST 2004
//    Added support for using a scalar array to clip against.
//
//    Jeremy Meredith, Mon Feb 16 19:07:24 PST 2004
//    Added polygonal cell support.
//
//    Jeremy Meredith, Wed May  5 14:49:55 PDT 2004
//    Made it support 2d cases as well.  Changed it to a single cutoff
//    for scalars to make the math more robust.
//
//    Brad Whitlock, Thu Aug 12 14:49:24 PST 2004
//    Added float casts to the pow() arguments so it builds on MSVC7.Net.
//
//    Kathleen Bonnell, Tue Sep  6 08:45:16 PDT 2005
//    Added call to SetUpClipFunction. 
//
//    Jeremy Meredith, Tue Aug 29 16:20:25 EDT 2006
//    Added support for "atomic" cells that must be removed
//    entirely if they cannot be left whole.
//    Added support for line and vertex output shapes (though
//    rectilinear grids shouldn't be outputting any, of course).
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to adjust percent to new percent consistent with zero
//    crossing of implicit func.
//
//    Cyrus Harrison, Tue Aug 21 08:34:29 PDT 2007
//    Fixed case where new points were created but not required resulting
//    in duplicate points and bad connectivity.
//
// ****************************************************************************

void vtkVisItClipper::RectilinearGridExecute(void)
{
    int  i, j;

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) GetInput();
    int pt_dims[3];
    rg->GetDimensions(pt_dims);
    bool twoD = (pt_dims[2] <= 1);

    int           nCells = rg->GetNumberOfCells();
    float        *X      = (float* ) rg->GetXCoordinates()->GetVoidPointer(0);
    float        *Y      = (float* ) rg->GetYCoordinates()->GetVoidPointer(0);
    float        *Z      = (float* ) rg->GetZCoordinates()->GetVoidPointer(0);
    vtkCellData  *inCD   = rg->GetCellData();
    vtkPointData *inPD   = rg->GetPointData();
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(rg->GetNumberOfPoints(), ptSizeGuess);

    int cell_dims[3];
    cell_dims[0] = pt_dims[0]-1;
    cell_dims[1] = pt_dims[1]-1;
    cell_dims[2] = pt_dims[2]-1;
    int strideY = cell_dims[0];
    int strideZ = cell_dims[0]*cell_dims[1];
    int ptstrideY = pt_dims[0];
    int ptstrideZ = pt_dims[0]*pt_dims[1];
    int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    for (i = 0 ; i < nToProcess ; i++)
    {
        int cellId = (CellList != NULL ? CellList[i] : i);
        int cellI = cellId % cell_dims[0];
        int cellJ = (cellId/strideY) % cell_dims[1];
        int cellK = (cellId/strideZ);
        int lookup_case = 0;
        float dist[8];
        int nCellPts = twoD ? 4 : 8;
        SetUpClipFunction(cellId);
        for (j = nCellPts-1 ; j >= 0 ; j--)
        {
            if (clipFunction)
            {
                float pt[3];
                pt[0] = X[cellI + X_val[j]];
                pt[1] = Y[cellJ + Y_val[j]];
                pt[2] = Z[cellK + Z_val[j]];
                dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }
            else // if (scalarArray)
            {
                float val = scalarArray[(cellK + Z_val[j])*ptstrideZ +
                                        (cellJ + Y_val[j])*ptstrideY +
                                        (cellI + X_val[j])];
                dist[j] = scalarCutoff - val;
            }

            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        if (removeWholeCells && lookup_case != 0)
            lookup_case = ((1 << nCellPts) - 1);

        unsigned char *splitCase;
        int            numOutput;
        int            interpIDs[4];
        if (twoD)
        {
            splitCase = &clipShapesQua[startClipShapesQua[lookup_case]];
            numOutput = numClipShapesQua[lookup_case];
        }
        else
        {
            splitCase = &clipShapesHex[startClipShapesHex[lookup_case]];
            numOutput = numClipShapesHex[lookup_case];
        }

        for (j = 0 ; j < numOutput ; j++)
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

                if ((!insideOut && color == COLOR0) ||
                    ( insideOut && color == COLOR1))
                {
                    // We don't want this one; it's the wrong side.
                    splitCase += npts;
                    continue;
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
                        shape[p] = ((cellI + X_val[pt]) +
                                    (cellJ + Y_val[pt])*ptstrideY +
                                    (cellK + Z_val[pt])*ptstrideZ);
                    }
                    else if (pt >= EA && pt <= EL)
                    {
                        int pt1 = hexVerticesFromEdges[pt-EA][0];
                        int pt2 = hexVerticesFromEdges[pt-EA][1];
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
                        // percent to the range [1e-4, 1. - 1e-4] right here.
                        int ptId1 = ((cellI + X_val[pt1]) +
                                     (cellJ + Y_val[pt1])*ptstrideY +
                                     (cellK + Z_val[pt1])*ptstrideZ);
                        int ptId2 = ((cellI + X_val[pt2]) +
                                     (cellJ + Y_val[pt2])*ptstrideY +
                                     (cellK + Z_val[pt2])*ptstrideZ);

                        // deal with exact zero crossings if requested
                        if (clipFunction && useZeroCrossings)
                        {
                            float pt[6];
                            pt[0] = X[cellI + X_val[pt1]];
                            pt[1] = Y[cellJ + Y_val[pt1]];
                            pt[2] = Z[cellK + Z_val[pt1]];
                            pt[3] = X[cellI + X_val[pt2]];
                            pt[4] = Y[cellJ + Y_val[pt2]];
                            pt[5] = Z[cellK + Z_val[pt2]];
                            AdjustPercentToZeroCrossing(pt, 0, 1,
                                clipFunction, &percent);
                        }

                        if( percent == 1.0)
                            shape[p] = ptId1;
                        else if( percent == 0.0)
                            shape[p] = ptId2;
                        else
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
                               shape[4], shape[5], shape[6], shape[7]);
                    break;
                  case ST_WDG:
                    vfv.AddWedge(cellId,
                                 shape[0], shape[1], shape[2],
                                 shape[3], shape[4], shape[5]);
                    break;
                  case ST_PYR:
                    vfv.AddPyramid(cellId, shape[0], shape[1],
                                   shape[2], shape[3], shape[4]);
                    break;
                  case ST_TET:
                    vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_QUA:
                    vfv.AddQuad(cellId, shape[0], shape[1], shape[2], shape[3]);
                    break;
                  case ST_TRI:
                    vfv.AddTri(cellId, shape[0], shape[1], shape[2]);
                    break;
                  case ST_LIN:
                    vfv.AddLine(cellId, shape[0], shape[1]);
                    break;
                  case ST_VTX:
                    vfv.AddVertex(cellId, shape[0]);
                    break;
                  case ST_PNT:
                    interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                    break;
                }
            }
        }
    }

    vfv.ConstructDataSet(inPD, inCD, output, pt_dims, X, Y, Z);
}

// ****************************************************************************
//  Method:  vtkVisItClipper::UnstructuredGridExecute
//
//  Purpose:
//    Clips an unstructured grid.
//
//  Arguments:
//    none
//
//  Note:  most of the setup logic came from Hank's new vtkSlicer.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 15 17:33:03 PDT 2003
//    Added ability for centroid-points to have an associated color.
//    This was needed for material interface reconstruction when it was
//    important to know if we should interpolate COLOR0's material or
//    COLOR1's material to come up with a material volume fraction for
//    the new point; it was not needed here, but we must skip over it.
//
//    Jeremy Meredith, Fri Jan 30 17:27:23 PST 2004
//    Added support for using a scalar array to clip against.
//
//    Jeremy Meredith, Mon Feb 16 19:07:24 PST 2004
//    Added polygonal cell support.
//
//    Jeremy Meredith, Wed May  5 14:49:55 PDT 2004
//    Changed it to a single cutoff for scalars to make the math more robust.
//
//    Jeremy Meredith, Thu Jun 24 09:39:31 PDT 2004
//    Added support for unstructured voxels and pixels, letting us use this
//    fast algorithm on thresholded/onionpeeled structured meshes.
//
//    Kathleen Bonnell, Tue Sep  6 08:45:16 PDT 2005
//    Added call to SetUpClipFunction. 
//
//    Jeremy Meredith, Tue Aug 29 16:20:25 EDT 2006
//    Added support for polydata types (line and vertex).
//    Added support for "atomic" cells that must be removed
//    entirely if they cannot be left whole.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to compute both sides of clip in one execute. Added code
//    to adjust percent to zero crossings if requested.
//
// ****************************************************************************

void vtkVisItClipper::UnstructuredGridExecute(void)
{
    // The routine here is a bit trickier than for the Rectilinear or
    // Structured grids.  We want to clip an unstructured grid -- but that
    // could mean any cell type.  We only have triangulation tables for
    // the finite element zoo.  So the gameplan is to clip any of the
    // elements of the finite element zoo.  If there are more elements left
    // over, clip them using the conventional VTK filters.  Finally,
    // append together the clips from the zoo with the clips from the
    // non-zoo elements.  If all the elements are from the zoo, then just
    // clip them with no appending.

    int   i, j;

    vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *) GetInput();

    int                nCells = ug->GetNumberOfCells();
    vtkPoints         *inPts  = ug->GetPoints();
    vtkCellData       *inCD   = ug->GetCellData();
    vtkPointData      *inPD   = ug->GetPointData();
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(ug->GetNumberOfPoints(), ptSizeGuess);
    vtkVolumeFromVolume vfvOut(ug->GetNumberOfPoints(), ptSizeGuess);
    vtkVolumeFromVolume *useVFV;

    vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
    stuff_I_cant_clip->SetPoints(ug->GetPoints());
    stuff_I_cant_clip->GetPointData()->ShallowCopy(ug->GetPointData());
    stuff_I_cant_clip->Allocate(nCells);

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantClip = 0;
    for (i = 0 ; i < nToProcess ; i++)
    {
        int        cellId = (CellList != NULL ? CellList[i] : i);
        int        cellType = ug->GetCellType(cellId);
        int        npts;
        vtkIdType *pts;
        ug->GetCellPoints(cellId, npts, pts);
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
 
        if (canClip)
        {
            SetUpClipFunction(cellId);
            const int max_pts = 8;
            float dist[max_pts];
            int lookup_case = 0;
            for (j = npts-1 ; j >= 0 ; j--)
            {
                if (clipFunction)
                {
                    float *pt = pts_ptr + 3*pts[j];
                    dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
                }
                else // if (scalarArray)
                {
                    float val = scalarArray[pts[j]];
                    dist[j] = scalarCutoff - val;
                }

                if (dist[j] >= 0)
                    lookup_case++;
                if (j > 0)
                    lookup_case *= 2;
            }

            if (removeWholeCells && lookup_case != 0)
                lookup_case = ((1 << npts) - 1);

            int             startIndex = 0;
            unsigned char  *splitCase = NULL;
            int             numOutput = 0;
            typedef int     edgeIndices[2];
            edgeIndices    *vertices_from_edges = NULL;

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

            int            interpIDs[4];
            int            interpIDsOut[4];
            for (j = 0 ; j < numOutput ; j++)
            {
                unsigned char shapeType = *splitCase++;
                {
                    int npts;
                    int interpID = -1;
                    int interpIDOut = -1;
                    int interpIDtmp;
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
                        interpIDtmp = *splitCase++;
                        color    = *splitCase++;
                        npts     = *splitCase++;
                        break;
                      default:
                        EXCEPTION1(ImproperUseException,
                                   "An invalid output shape was found in "
                                   "the ClipCases.");
                    }

                    useVFV = &vfv;
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
                    if (useVFV == &vfv)
                        interpID = interpIDtmp;
                    else
                        interpIDOut = interpIDtmp;

                    int shape[8];
                    for (int p = 0 ; p < npts ; p++)
                    {
                        unsigned char pt = *splitCase++;
                        if (pt <= P7)
                        {
                            // We know pt P0 must be >P0 since we already
                            // assume P0 == 0.  This is why we do not
                            // bother subtracting P0 from pt here.
                            shape[p] = pts[pt];
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
                            int ptId1 = pts[pt1];
                            int ptId2 = pts[pt2];

                            // deal with exact zero crossings if requested
                            if (clipFunction && useZeroCrossings)
                                AdjustPercentToZeroCrossing(pts_ptr, ptId1, ptId2,
                                    clipFunction, &percent);
                                
                            shape[p] = useVFV->AddPoint(ptId1, ptId2, percent);
                        }
                        else if (pt >= N0 && pt <= N3)
                        {
                            if (useVFV == &vfv)
                                shape[p] = interpIDs[pt - N0];
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
                        if (useVFV == &vfv)
                            interpIDs[interpID] = useVFV->AddCentroidPoint(npts, shape);
                        else
                            interpIDsOut[interpIDOut] = useVFV->AddCentroidPoint(npts, shape);
                        break;
                    }
                }
            }
        }
        else
        {
            if (numIcantClip == 0)
                stuff_I_cant_clip->GetCellData()->
                                       CopyAllocate(ug->GetCellData(), nCells);

            stuff_I_cant_clip->InsertNextCell(cellType, npts, pts);
            stuff_I_cant_clip->GetCellData()->
                            CopyData(ug->GetCellData(), cellId, numIcantClip);
            numIcantClip++;
        }
    }

    if (numIcantClip > 0)
    {
        vtkUnstructuredGrid *not_from_zoo  = vtkUnstructuredGrid::New();
        ClipDataset(stuff_I_cant_clip, not_from_zoo);
        
        vtkUnstructuredGrid *just_from_zoo = vtkUnstructuredGrid::New();
        vfv.ConstructDataSet(inPD, inCD, just_from_zoo, pts_ptr);

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
            vfvOut.ConstructDataSet(inPD, inCD, just_from_zoo, pts_ptr);

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
        vfv.ConstructDataSet(inPD, inCD, output, pts_ptr);
        if (computeInsideAndOut)
        {
            if (otherOutput) otherOutput->Delete();
            otherOutput = vtkUnstructuredGrid::New();
            vfvOut.ConstructDataSet(inPD, inCD, otherOutput, pts_ptr);
        }
    }

    stuff_I_cant_clip->Delete();
}

// ****************************************************************************
//  Method:  vtkVisItClipper::PolyDataExecute
//
//  Purpose:
//    Clips a polydata object.
//
//  Arguments:
//    none
//
//  Note:  Copied from UnstructuredGridExecute.  Probably could unify
//         them for ease of maintenance.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 16, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed May  5 14:49:55 PDT 2004
//    Changed it to a single cutoff for scalars to make the math more robust.
//
//    Kathleen Bonnell, Tue Sep  6 08:45:16 PDT 2005
//    Added call to SetUpClipFunction. 
//
//    Jeremy Meredith, Tue Aug 29 16:20:25 EDT 2006
//    Added support for polydata types (line and vertex).
//    Added support for "atomic" cells that must be removed
//    entirely if they cannot be left whole.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to adjust percent to new percent consistent with zero
//    crossing of implicit func.
//
// ****************************************************************************

void vtkVisItClipper::PolyDataExecute(void)
{
    // The routine here is a bit trickier than for the Rectilinear or
    // Structured grids.  We want to clip an unstructured grid -- but that
    // could mean any cell type.  We only have triangulation tables for
    // the finite element zoo.  So the gameplan is to clip any of the
    // elements of the finite element zoo.  If there are more elements left
    // over, clip them using the conventional VTK filters.  Finally,
    // append together the clips from the zoo with the clips from the
    // non-zoo elements.  If all the elements are from the zoo, then just
    // clip them with no appending.

    int   i, j;

    vtkPolyData *pd = (vtkPolyData *) GetInput();

    int                nCells = pd->GetNumberOfCells();
    vtkPoints         *inPts  = pd->GetPoints();
    vtkCellData       *inCD   = pd->GetCellData();
    vtkPointData      *inPD   = pd->GetPointData();
    vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)GetOutput();

    int ptSizeGuess = (CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : CellListSize*5 + 100);

    vtkVolumeFromVolume vfv(pd->GetNumberOfPoints(), ptSizeGuess);

    vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
    stuff_I_cant_clip->SetPoints(pd->GetPoints());
    stuff_I_cant_clip->GetPointData()->ShallowCopy(pd->GetPointData());
    stuff_I_cant_clip->Allocate(nCells);

    float *pts_ptr = (float *) inPts->GetVoidPointer(0);

    int nToProcess = (CellList != NULL ? CellListSize : nCells);
    int numIcantClip = 0;
    for (i = 0 ; i < nToProcess ; i++)
    {
        int        cellId = (CellList != NULL ? CellList[i] : i);
        int        cellType = pd->GetCellType(cellId);
        int        npts;
        vtkIdType *pts;
        pd->GetCellPoints(cellId, npts, pts);
        bool canClip = false;
        switch (cellType)
        {
          case VTK_TETRA:
          case VTK_PYRAMID:
          case VTK_WEDGE:
          case VTK_HEXAHEDRON:
          case VTK_TRIANGLE:
          case VTK_QUAD:
          case VTK_LINE:
          case VTK_VERTEX:
            canClip = true;
            break;

          default:
            canClip = false;
            break;
        }
 
        if (canClip)
        {
            const int max_pts = 8;
            float dist[max_pts];
            int lookup_case = 0;
            SetUpClipFunction(cellId);
            for (j = npts-1 ; j >= 0 ; j--)
            {
                if (clipFunction)
                {
                    float *pt = pts_ptr + 3*pts[j];
                    dist[j] = clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
                }
                else // if (scalarArray)
                {
                    float val = scalarArray[pts[j]];
                    dist[j] = scalarCutoff - val;
                }

                if (dist[j] >= 0)
                    lookup_case++;
                if (j > 0)
                    lookup_case *= 2;
            }

            if (removeWholeCells && lookup_case != 0)
                lookup_case = ((1 << npts) - 1);

            int             startIndex = 0;
            unsigned char  *splitCase = NULL;
            int             numOutput = 0;
            typedef int     edgeIndices[2];
            edgeIndices    *vertices_from_edges = NULL;

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

            int            interpIDs[4];
            for (j = 0 ; j < numOutput ; j++)
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

                    if ((!insideOut && color == COLOR0) ||
                        ( insideOut && color == COLOR1))
                    {
                        // We don't want this one; it's the wrong side.
                        splitCase += npts;
                        continue;
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
                            shape[p] = pts[pt];
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
                            int ptId1 = pts[pt1];
                            int ptId2 = pts[pt2];

                            // deal with exact zero crossings if requested
                            if (clipFunction && useZeroCrossings)
                                AdjustPercentToZeroCrossing(pts_ptr, ptId1, ptId2,
                                    clipFunction, &percent);
                                
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
                                   shape[4], shape[5], shape[6], shape[7]);
                        break;
                      case ST_WDG:
                        vfv.AddWedge(cellId,
                                     shape[0], shape[1], shape[2],
                                     shape[3], shape[4], shape[5]);
                        break;
                      case ST_PYR:
                        vfv.AddPyramid(cellId, shape[0], shape[1],
                                       shape[2], shape[3], shape[4]);
                        break;
                      case ST_TET:
                        vfv.AddTet(cellId, shape[0], shape[1], shape[2], shape[3]);
                        break;
                      case ST_QUA:
                        vfv.AddQuad(cellId, shape[0], shape[1], shape[2], shape[3]);
                        break;
                      case ST_TRI:
                        vfv.AddTri(cellId, shape[0], shape[1], shape[2]);
                        break;
                      case ST_LIN:
                        vfv.AddLine(cellId, shape[0], shape[1]);
                        break;
                      case ST_VTX:
                        vfv.AddVertex(cellId, shape[0]);
                        break;
                      case ST_PNT:
                        interpIDs[interpID] = vfv.AddCentroidPoint(npts, shape);
                        break;
                    }
                }
            }
        }
        else
        {
            if (numIcantClip == 0)
                stuff_I_cant_clip->GetCellData()->
                                       CopyAllocate(pd->GetCellData(), nCells);

            stuff_I_cant_clip->InsertNextCell(cellType, npts, pts);
            stuff_I_cant_clip->GetCellData()->
                            CopyData(pd->GetCellData(), cellId, numIcantClip);
            numIcantClip++;
        }
    }

    if (numIcantClip > 0)
    {
        vtkUnstructuredGrid *not_from_zoo  = vtkUnstructuredGrid::New();
        ClipDataset(stuff_I_cant_clip, not_from_zoo);
        
        vtkUnstructuredGrid *just_from_zoo = vtkUnstructuredGrid::New();
        vfv.ConstructDataSet(inPD, inCD, just_from_zoo, pts_ptr);

        vtkAppendFilter *appender = vtkAppendFilter::New();
        appender->AddInput(not_from_zoo);
        appender->AddInput(just_from_zoo);
        appender->GetOutput()->Update();

        output->ShallowCopy(appender->GetOutput());
        appender->Delete();
        not_from_zoo->Delete();
        just_from_zoo->Delete();
    }
    else
    {
        vfv.ConstructDataSet(inPD, inCD, output, pts_ptr);
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

