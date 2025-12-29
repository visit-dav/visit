// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItClipper.h"

#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkClipDataSet.h>
#include <vtkFloatArray.h>
#include <vtkImplicitFunction.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkQuadric.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVolumeFromVolume.h>

#include <vtkAccessors.h>

#include <ImproperUseException.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <math.h>
#include <vector>

#include <ClipCases.h>
#include <vtkTriangulationTables.h>

vtkStandardNewMacro(vtkVisItClipper)

// ****************************************************************************
//  Function: AdjustPercentToZeroCrossing
//
//    Purpose: Given coordinate array, point ids and linear estimate of
//    a cut, use quadric to compute actual zero crossing and adjust the
//    percent value to hit the zero crossing
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006
//
//  Modifications:
//    Brad Whitlock, Fri Mar 23 17:13:49 PDT 2012
//    Pass in points.
//
// ****************************************************************************
static void
AdjustPercentToZeroCrossing(double p0[3], double p1[3], 
    vtkImplicitFunction *func, double *percent)
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
    if(quadric == nullptr)
        return;
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
//  Modifications:
//    Kathleen Biagas, Tue Aug 14 11:24:22 MST 2012
//    Added precomputeClipScalars.
//
//    Alister Maguire, Fri Nov 13 14:07:54 PST 2020
//    replaced removeWholeCells with cellClipStrategy.
//
// ****************************************************************************

vtkVisItClipper::FilterState::FilterState()
{ 
    this->CellList = nullptr;
    this->CellListSize = 0;

    this->clipFunction = nullptr;
    this->scalarArrayAsVTK = nullptr;
    this->scalarCutoff = 0.;

    this->otherOutput = nullptr;

    this->cellStrategy = REMOVE_PARTIAL_CELL;
    this->insideOut = false;
    this->useZeroCrossings = false;
    this->computeInsideAndOut = false;
    this->precomputeClipScalars = true;
}

vtkVisItClipper::FilterState::~FilterState()
{
    if (this->clipFunction != nullptr)
        this->clipFunction->Delete();

    if (this->otherOutput)
        this->otherOutput->Delete();

    if (this->scalarArrayAsVTK != nullptr)
        this->scalarArrayAsVTK->Delete();
}

void
vtkVisItClipper::FilterState::SetCellList(const vtkIdType *cl, vtkIdType size)
{
    this->CellList = cl;
    this->CellListSize = size;
}

void
vtkVisItClipper::FilterState::SetClipFunction(vtkImplicitFunction *func)
{
    // Set the clip function
    this->clipFunction = func;
    this->clipFunction->Register(nullptr);

    // Clear the scalar array so we know to use the clip function
    if (scalarArrayAsVTK != nullptr)
    {
        scalarArrayAsVTK->Delete();
        scalarArrayAsVTK = nullptr;
    }
}

void
vtkVisItClipper::FilterState::SetClipScalars(vtkDataArray *array, double cutoff)
{
    // Clear the clip function so we know to use scalars
    if (this->clipFunction != nullptr)
    {
        this->clipFunction->Delete();
        this->clipFunction = nullptr;
    }

    if (scalarArrayAsVTK != nullptr)
    {
        scalarArrayAsVTK->Delete();
        scalarArrayAsVTK = nullptr;
    }

    // Set the scalar array
    scalarArrayAsVTK = array;
    scalarArrayAsVTK->Register(nullptr);

    // Set the cutoff
    scalarCutoff     = cutoff;
}

void
vtkVisItClipper::FilterState::ClipDataset(vtkDataSet *in_ds,
                                          vtkUnstructuredGrid *out_ds)
{
    vtkClipDataSet *clipData = vtkClipDataSet::New();
    clipData->SetInputData(in_ds);
    if (this->clipFunction)
    {
        clipData->SetClipFunction(this->clipFunction);
        clipData->GenerateClipScalarsOff();
        clipData->SetInsideOut(this->insideOut);
    }
    else
    {
        clipData->SetClipFunction(nullptr);
        in_ds->GetPointData()->SetScalars(this->scalarArrayAsVTK);
        clipData->GenerateClipScalarsOff();
        clipData->SetValue(this->scalarCutoff);
        clipData->SetInsideOut(!this->insideOut);
    }
    clipData->Update();
    out_ds->ShallowCopy(clipData->GetOutput());
    clipData->Delete();
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
//    Brad Whitlock, Tue Mar 27 12:02:00 PDT 2012
//    Move members to state.
//
// ****************************************************************************

vtkVisItClipper::vtkVisItClipper() : vtkUnstructuredGridAlgorithm(), state()
{
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
//    Brad Whitlock, Tue Mar 27 12:04:39 PDT 2012
//    Deletion is handled in FilterState now.
//
// ****************************************************************************
vtkVisItClipper::~vtkVisItClipper()
{
}

void
vtkVisItClipper::SetUseZeroCrossings(bool use)
{
    if (use && state.clipFunction && 
        (strcmp(state.clipFunction->GetClassName(), "vtkQuadric") != 0))
    {
        vtkErrorMacro("UseZeroCrossings set to true allowed only with "
                      "vtkQuadric implicit functions");
        return;
    }

    state.useZeroCrossings = use;
}

void
vtkVisItClipper::SetComputeInsideAndOut(bool compute)
{
    state.computeInsideAndOut = compute;
}

void
vtkVisItClipper::SetCellList(const vtkIdType *cl, vtkIdType size)
{
    state.SetCellList(cl, size);
}

void
vtkVisItClipper::SetClipFunction(vtkImplicitFunction *func)
{
    if (state.useZeroCrossings && (strcmp(func->GetClassName(), "vtkQuadric") != 0))
    {
        vtkErrorMacro("Only vtkQuadric implicit functions "
                      "allowed with UseZeroCrossings set to true");
        return;
    }

    state.SetClipFunction(func);
}

// ****************************************************************************
// Method:  vtkVisItClipper::SetClipScalars
//
//  Purpose:
//    Sets a flag that when false, allows postponment of clipFunction 
//    evaluation until stepping through cells.
//
// Modifications:
//   Kathleen Biagas, Tue Aug 14 11:24:22 MST 2012
//   Added precomputeClipScalars.
//
// ****************************************************************************

void
vtkVisItClipper::SetPrecomputeClipScalars(const bool v)
{
    state.precomputeClipScalars = v;
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
//    Jeremy Meredith, Wed May  5 14:48:23 PDT 2004
//    Made it allow only a single cutoff, and use the "insideOut"
//    value to determine if this is a min or max value.
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Change the array argument to be a vtk data type.  Also added support
//    for data types besides "float".
//
//    Brad Whitlock, Tue Mar 27 11:51:35 PDT 2012
//    Move to FilterState.
//
// ****************************************************************************

void
vtkVisItClipper::SetClipScalars(vtkDataArray *array, float cutoff)
{
    state.SetClipScalars(array, cutoff);
}

void
vtkVisItClipper::SetInsideOut(bool io)
{
    state.insideOut = io;
}

// ****************************************************************************
//  Method:  vtkVisItClipper::SetCellClipStrategy
//
//  Purpose:
//    Set the cell clip strategy. This is the strategy used when the clip
//    boundary intersects with the cell.
//
//      1. REMOVE_PARTIAL_CELL
//         Remove the section of the cell that exists outside of the clip
//         boundary, and keep the rest intact.
//
//      2. REMOVE_WHOLE_CELL
//         Remove the entire cell.
//
//      3. KEEP_WHOLE_CELL
//         Keep the entire cell.
//
//  Arguments:
//    strategy        The cell clip strategy.
//
//  Programmer:  Alister Maguire
//  Creation:    November 13, 2020
//
// ****************************************************************************
void
vtkVisItClipper::SetCellClipStrategy(cellClipStrategy strategy)
{
    state.cellStrategy = strategy;
}

// ****************************************************************************
//  Method:  vtkVisItClipper::SetCellClipStrategyToRemovePartial
//
//  Purpose:
//    Set the cell clip strategy to REMOVE_PARTIAL_CELL.
//    When a clip boundary intersects a cell, remove the section of
//    the cell that exists outside of the clip boundary, and keep
//    the rest intact.
//
//  Programmer:  Alister Maguire
//  Creation:    November 13, 2020
//
// ****************************************************************************
void
vtkVisItClipper::SetCellClipStrategyToRemovePartial()
{
    state.cellStrategy = REMOVE_PARTIAL_CELL;
}

// ****************************************************************************
//  Method:  vtkVisItClipper::SetCellClipStrategyToRemoveWhole
//
//  Purpose:
//    Set the cell clip strategy to REMOVE_WHOLE_CELL.
//    When a clip boundary intersects a cell, remove the entire cell.
//
//  Programmer:  Alister Maguire
//  Creation:    November 13, 2020
//
// ****************************************************************************
void
vtkVisItClipper::SetCellClipStrategyToRemoveWhole()
{
    state.cellStrategy = REMOVE_WHOLE_CELL;
}

// ****************************************************************************
//  Method:  vtkVisItClipper::SetCellClipStrategyToKeepWhole
//
//  Purpose:
//    Set the cell clip strategy to KEEP_WHOLE_CELL.
//    When a clip boundary intersects a cell, keep the entire cell.
//
//  Programmer:  Alister Maguire
//  Creation:    November 13, 2020
//
// ****************************************************************************
void
vtkVisItClipper::SetCellClipStrategyToKeepWhole()
{
    state.cellStrategy = KEEP_WHOLE_CELL;
}

vtkUnstructuredGrid*
vtkVisItClipper::GetOtherOutput()
{
    return state.otherOutput;
}


// ****************************************************************************
//  Method: vtkVisItClipper_Algorithm
//
//  Purpose: 
//    This is the main algorithm that is applied when we clip. It used to be 
//    in the vtkVisItClipper::Execute method but I templated it so we could
//    support float/double values and coordinates.
//
//  Arguments:
//
//  Returns:    
//
//  Note:       The code to get coordinates and such from the mesh is delegated
//              to a bridge class that implements inline methods for returning
//              the values we want. This is done so we can abstract out the
//              mesh-related things and focus on the clipping algorithm.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 24, 2010
//
//  Modifications:
//    Brad Whitlock, Mon Mar 26 13:43:04 PDT 2012
//    I cut this code out from the Execute() method and templated it on Bridge
//    and ScalarAccess so we can get data from different meshes and precisions
//    without if statements.
//
//    Jeremy Meredith, Mon Jul  9 16:53:41 EDT 2012
//    Added support for 5- through 8-sided polygons.
//
//    Kathleen Biagas, Tue Aug 14 11:28:31 MST 2012 
//    Added clipper argument, for access to the ModifyClip method.
//    Evaluation clip function if precomputeClipScalars is false.
//
//    Alister Maguire, Fri Nov 13 14:07:54 PST 2020
//    Updated to clip cells based on the cellClipStrategy.
//
// ****************************************************************************

template <typename ScalarAccess>
void
vtkVisItClipper_Algorithm(ScalarAccess scalar,
    vtkVisItClipper::FilterState &state,
    vtkDataSet *input, vtkUnstructuredGrid *output,
    vtkUnstructuredGrid *stuff_I_cant_clip,
    vtkVisItClipper *clipper)
{
    int t1 = visitTimer->StartTimer();
    vtkIdType nCells = input->GetNumberOfCells();
    vtkIdType nPts = input->GetNumberOfPoints();

    //
    // Do the actual clipping here
    //
    size_t ptSizeGuess = (state.CellList == nullptr
                         ? static_cast<size_t>(pow(static_cast<float>(nCells), 0.6667f)) * 5 + 100
                         : static_cast<size_t>(state.CellListSize*5 + 100));

    vtkVolumeFromVolume vfvIn(nPts, ptSizeGuess);
    vtkVolumeFromVolume vfvOut(nPts, ptSizeGuess);
    vtkVolumeFromVolume *useVFV = nullptr;

    const int max_pts = 8;

    vtkIdType nToProcess = (state.CellList != nullptr ? state.CellListSize : nCells);
    vtkIdType numIcantClip = 0;

    vtkIdList *cellPts = vtkIdList::New();
    for (vtkIdType i = 0 ; i < nToProcess ; i++)
    {
        // Get the cell details
        vtkIdType cellId = (state.CellList != nullptr ? state.CellList[i] : i);
        clipper->ModifyClip(input, cellId);
        int cellType = input->GetCellType(cellId);
        cellPts->Reset();
        input->GetCellPoints(cellId, cellPts);
        vtkIdType nCellPts = cellPts->GetNumberOfIds();

        // If it's something we can't clip, save it for later
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
            // we now handle 5-8 sided polygons as well
            if (cellType == VTK_POLYGON &&
                nCellPts >= 5 && nCellPts <= 8)
            {
                break;
            }

            // everything else; defer for other clipper algorithm
            {
                if (numIcantClip == 0)
                    stuff_I_cant_clip->GetCellData()->
                                       CopyAllocate(input->GetCellData(), nCells);

                stuff_I_cant_clip->InsertNextCell(cellType, cellPts);
                stuff_I_cant_clip->GetCellData()->
                            CopyData(input->GetCellData(), cellId, numIcantClip);
                numIcantClip++;
                continue;
            }
            break;
        }

        // fill the dist functions and calculate lookup case
        int lookup_case = 0;
        double dist[max_pts];
        for (vtkIdType j = nCellPts-1 ; j >= 0 ; j--)
        {
            if (state.precomputeClipScalars)
            {
                dist[j] = state.scalarCutoff - scalar.GetTuple1(cellPts->GetId(j));
            }
            else
            {
                double pt[3];
                input->GetPoint(cellPts->GetId(j), pt);
                dist[j] = -state.clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }

            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        //
        // Determine how to handle clipping this cell. This is
        // really only relevant when the clip boundary intersects
        // the cell.
        //
        switch(state.cellStrategy)
        {
            case (vtkVisItClipper::REMOVE_PARTIAL_CELL):
            {
                //
                // No extra work needed for this case.
                //
                break;
            }
            case (vtkVisItClipper::REMOVE_WHOLE_CELL):
            {
                //
                // Choose the last case, which removes the entire cell.
                //
                if (lookup_case != 0)
                {
                    lookup_case = ((1 << nCellPts) - 1);
                }
                break;
            }
            case (vtkVisItClipper::KEEP_WHOLE_CELL):
            {
                //
                // A lookup_case of 0 means that we're inside the "keep"
                // portion of the clip boundary, and a lookup_case of
                // ((1 << nCellPts)-1) means that we're inside the "remove"
                // section. Anything in between is a cell that's instersected
                // by the clip boundary. We're keeping these intact here.
                //
                if (lookup_case > 0 && lookup_case < ((1 << nCellPts) - 1))
                {
                    lookup_case = 0;
                }
                break;
            }
        }

        unsigned char  *splitCase = nullptr;
        int             numOutput = 0;
        typedef int     edgeIndices[2];
        edgeIndices    *vertices_from_edges = nullptr;

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
            vertices_from_edges = nullptr;
            break;
          case VTK_POLYGON:
            switch (nCellPts)
            {
              case 5:
                startIndex = startClipShapesPoly5[lookup_case];
                splitCase  = &clipShapesPoly5[startIndex];
                numOutput  = numClipShapesPoly5[lookup_case];
                vertices_from_edges = poly5VerticesFromEdges;
                break;
              case 6:
                startIndex = startClipShapesPoly6[lookup_case];
                splitCase  = &clipShapesPoly6[startIndex];
                numOutput  = numClipShapesPoly6[lookup_case];
                vertices_from_edges = poly6VerticesFromEdges;
                break;
              case 7:
                startIndex = startClipShapesPoly7[lookup_case];
                splitCase  = &clipShapesPoly7[startIndex];
                numOutput  = numClipShapesPoly7[lookup_case];
                vertices_from_edges = poly7VerticesFromEdges;
                break;
              case 8:
                startIndex = startClipShapesPoly8[lookup_case];
                splitCase  = &clipShapesPoly8[startIndex];
                numOutput  = numClipShapesPoly8[lookup_case];
                vertices_from_edges = poly8VerticesFromEdges;
                break;
            }
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

                bool out = ((!state.insideOut && color == COLOR0) ||
                            ( state.insideOut && color == COLOR1));
                useVFV = &vfvIn;
                if (out)
                {
                    if (state.computeInsideAndOut)
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

                vtkIdType shape[8];
                for (int p = 0 ; p < npts ; p++)
                {
                    unsigned char pt = *splitCase++;
                    if (pt <= P7)
                    {
                        // We know pt P0 must be >P0 since we already
                        // assume P0 == 0.  This is why we do not
                        // bother subtracting P0 from pt here.
                        shape[p] = cellPts->GetId(pt);
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
                        double dir = dist[pt2] - dist[pt1];
                        double amt = 0. - dist[pt1];
                        double percent = 1. - (amt / dir);

                        // We may have physically (though not logically)
                        // degenerate cells if percent==0 or percent==1.
                        // We could pretty easily and mostly safely clamp
                        // percent to the range [1e-4, 1. - 1e-4] here.
                        vtkIdType ptId1 = cellPts->GetId(pt1);
                        vtkIdType ptId2 = cellPts->GetId(pt2);

                        // deal with exact zero crossings if requested
                        if (state.clipFunction != nullptr && state.useZeroCrossings)
                        {
                            double p0[3], p1[3];
                            input->GetPoint(ptId1, p0);
                            input->GetPoint(ptId2, p1);

                            AdjustPercentToZeroCrossing(p0, p1,
                                                        state.clipFunction,
                                                        &percent);
                        }
                                
                        shape[p] = useVFV->AddPoint(ptId1, ptId2, static_cast<float>(percent));
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
                    interpIDsIn[interpID] = vfvIn.AddCentroidPoint(static_cast<size_t>(npts), shape);
                    if (state.computeInsideAndOut)
                        interpIDsOut[interpID] = vfvOut.AddCentroidPoint(static_cast<size_t>(npts), shape);
                    break;
                }
            }
        }
    }
    visitTimer->StopTimer(t1, "Performing clipping");

    //
    // Construct the output data set.
    //
    int t2 = visitTimer->StartTimer();
    if (numIcantClip > 0)
    {
        int t3 = visitTimer->StartTimer();
        vtkUnstructuredGrid *not_from_zoo  = vtkUnstructuredGrid::New();
        state.ClipDataset(stuff_I_cant_clip, not_from_zoo);
        visitTimer->StopTimer(t3, "Clipping non-zoo cells");

        vtkUnstructuredGrid *just_from_zoo = vtkUnstructuredGrid::New();
        vfvIn.ConstructDataSet(input->GetPointData(),
                               input->GetCellData(),
                               just_from_zoo,
                               input->GetPoints());

        vtkAppendFilter *appender = vtkAppendFilter::New();
        appender->AddInputData(not_from_zoo);
        appender->AddInputData(just_from_zoo);
        appender->Update();

        output->ShallowCopy(appender->GetOutput());

        if (state.computeInsideAndOut)
        {
            appender->RemoveInputData(just_from_zoo);
            just_from_zoo->Delete();

            just_from_zoo = vtkUnstructuredGrid::New();
            vfvOut.ConstructDataSet(input->GetPointData(),
                                    input->GetCellData(),
                                    just_from_zoo,
                                    input->GetPoints());

            appender->AddInputData(just_from_zoo);
            appender->Update();

            if (state.otherOutput != nullptr)
                state.otherOutput->Delete();
            state.otherOutput = vtkUnstructuredGrid::New();
            state.otherOutput->ShallowCopy(appender->GetOutput());
        }

        appender->Delete();
        just_from_zoo->Delete();
        not_from_zoo->Delete();
    }
    else
    {
        vfvIn.ConstructDataSet(input->GetPointData(),
                               input->GetCellData(),
                               output,
                               input->GetPoints());

        if (state.computeInsideAndOut)
        {
            if (state.otherOutput)
                state.otherOutput->Delete();
            state.otherOutput = vtkUnstructuredGrid::New();
            vfvOut.ConstructDataSet(input->GetPointData(),
                                    input->GetCellData(),
                                    state.otherOutput,
                                    input->GetPoints());
        }
    }
    visitTimer->StopTimer(t2, "VFV Constructing data set");
}


// ****************************************************************************
//  Method: vtkVisItClipper_Execute
//
//  Purpose: 
//    Execute the clipping algorithm to different data via a bridge thet lets
//    us deal with different mesh types and coordinate precisions. This function
//    sets up unstructured meshes specially.
//
//  Arguments:
//
//  Returns:    
//
//  Note:       Same as structured except that point traversal is 0..N.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Mar 26 13:52:27 PDT 2012
//
//  Modifications:
//    Kathleen Biagas, Tue Aug 14 11:28:31 MST 2012 
//    Added clipper argument, for access to the ModifyClip method.
//    Only evaluate clip function if precomputeClipScalars is true.
//
// ****************************************************************************

void
vtkVisItClipper_Execute(
    vtkVisItClipper::FilterState &state,
    vtkDataSet *input, vtkUnstructuredGrid *output,
    vtkUnstructuredGrid *stuff_I_cant_clip,
    vtkVisItClipper *clipper)
{
    if(state.clipFunction != nullptr)
    {
        vtkIdType nPts = input->GetNumberOfPoints();
        double *scalar = new double[static_cast<size_t>(nPts)];
        if (state.precomputeClipScalars)
        {
            for (vtkIdType i=0; i < nPts; i++)
            {
                double pt[3];
                input->GetPoint(i, pt);
                scalar[i] = -state.clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }
        }

        vtkVisItClipper_Algorithm(vtkAccessor<double>(scalar), 
                                  state, input, output, stuff_I_cant_clip, clipper);
        delete [] scalar;
    }
    else if(state.scalarArrayAsVTK != nullptr)
    {
        int dt = state.scalarArrayAsVTK->GetDataType();
        if(dt == VTK_FLOAT)
            vtkVisItClipper_Algorithm(
                 vtkAccessor<float>(state.scalarArrayAsVTK),
                 state, input, output, stuff_I_cant_clip, clipper);
        else if(dt == VTK_DOUBLE)
            vtkVisItClipper_Algorithm(
                 vtkAccessor<double>(state.scalarArrayAsVTK),
                 state, input, output, stuff_I_cant_clip, clipper);
        else
            vtkVisItClipper_Algorithm(
                 vtkGeneralAccessor(state.scalarArrayAsVTK),
                  state, input, output, stuff_I_cant_clip, clipper);
    }
}

// ****************************************************************************
//  Method:  vtkVisItClipper::RequestData
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
//    Jeremy Meredith, Thu Feb 25 11:08:03 EST 2010
//    Don't forget to exit early if we have a dataset we can't understand.
//
//    Jeremy Meredith, Thu Feb 25 15:14:28 EST 2010
//    Allowing clipFunction usage to precalculate (most) values.  This
//    saves a good chunk of time in this mode since we were re-calculating
//    these values a number of times.
//
//    Brad Whitlock, Mon Mar 26 13:26:44 PDT 2012
//    Massive refactor to support float/double values and coordinates.
//
//    Eric Brugger, Thu Jul 18 17:03:54 PDT 2013
//    I corrected a bug that caused the clipping of double precision
//    structured grids to return zero cells.
//
// ****************************************************************************

int
vtkVisItClipper::RequestData(
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
    vtkDataSet *ds = vtkDataSet::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));

    if(ds == nullptr)
    {
        vtkErrorMacro("No input.");
        return 1;
    }

    vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    if(output == nullptr)
    {
        vtkErrorMacro("No output.");
        return 1;
    }

    int t0 = visitTimer->StartTimer();
    int do_type = ds->GetDataObjectType();

    if(do_type == VTK_RECTILINEAR_GRID  ||
       do_type == VTK_STRUCTURED_GRID   ||
       do_type == VTK_UNSTRUCTURED_GRID ||
       do_type == VTK_POLY_DATA)
    {
        vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
        if (do_type == VTK_UNSTRUCTURED_GRID)
        {
            vtkUnstructuredGrid *ug = static_cast<vtkUnstructuredGrid*>(ds);
            stuff_I_cant_clip->SetPoints(ug->GetPoints());
            stuff_I_cant_clip->GetPointData()->ShallowCopy(ug->GetPointData());
            stuff_I_cant_clip->Allocate(ug->GetNumberOfCells());
        }
        else if (do_type == VTK_POLY_DATA)
        {
            vtkPolyData *pd = static_cast<vtkPolyData*>(ds);
            stuff_I_cant_clip->SetPoints(pd->GetPoints());
            stuff_I_cant_clip->GetPointData()->ShallowCopy(pd->GetPointData());
            stuff_I_cant_clip->Allocate(pd->GetNumberOfCells());
        }

        vtkVisItClipper_Execute(this->state, ds, output, 
                                stuff_I_cant_clip, this);
        stuff_I_cant_clip->Delete();
    }
    else
    {
        debug1 << "vtkVisItClipper: Can't operate on this dataset,\n";
        debug1 << "                 reverting to raw VTK code.\n";
        ClipDataset(ds, output);
    }
    visitTimer->StopTimer(t0, "vtkVisItClipper");

    return 1;
}

// ****************************************************************************
//  Method: vtkVisItClipper::FillInputPortInformation
//
// ****************************************************************************

int
vtkVisItClipper::FillInputPortInformation(int, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

void vtkVisItClipper::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}


// ****************************************************************************
//  Modifications:
//    Hank Childs, Sat Mar 27 10:56:08 PST 2004
//    Work-around some funniness with VTK memory management.  (the funniness
//    is a bug with the vtkClipDataSet filter.)
//
//    Hank Childs, Sat Oct  6 15:37:11 PDT 2007
//    Fix bug with setting "inverse" for isovoluming.
//
// ****************************************************************************
void
vtkVisItClipper::ClipDataset(vtkDataSet *in_ds,
                             vtkUnstructuredGrid *out_ds)
{
    state.ClipDataset(in_ds, out_ds);
}
