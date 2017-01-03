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

vtkStandardNewMacro(vtkVisItClipper);

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
// ****************************************************************************

vtkVisItClipper::FilterState::FilterState()
{ 
    this->CellList = NULL;
    this->CellListSize = 0;

    this->clipFunction = NULL;
    this->scalarArrayAsVTK = NULL;
    this->scalarCutoff = 0.;

    this->otherOutput = NULL;

    this->removeWholeCells = false;
    this->insideOut = false;
    this->useZeroCrossings = false;
    this->computeInsideAndOut = false;
    this->precomputeClipScalars = true;
}

vtkVisItClipper::FilterState::~FilterState()
{
    if (this->clipFunction != NULL)
        this->clipFunction->Delete();

    if (this->otherOutput)
        this->otherOutput->Delete();

    if (this->scalarArrayAsVTK != NULL)
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
    this->clipFunction->Register(NULL);

    // Clear the scalar array so we know to use the clip function
    if (scalarArrayAsVTK != NULL)
    {
        scalarArrayAsVTK->Delete();
        scalarArrayAsVTK = NULL;
    }
}

void
vtkVisItClipper::FilterState::SetClipScalars(vtkDataArray *array, double cutoff)
{
    // Clear the clip function so we know to use scalars
    if (this->clipFunction != NULL)
    {
        this->clipFunction->Delete();
        this->clipFunction = NULL;
    }

    if (scalarArrayAsVTK != NULL)
    {
        scalarArrayAsVTK->Delete();
        scalarArrayAsVTK = NULL;
    }

    // Set the scalar array
    scalarArrayAsVTK = array;
    scalarArrayAsVTK->Register(NULL);

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
        clipData->SetClipFunction(NULL);
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
    state.removeWholeCells = rwc;
}

vtkUnstructuredGrid*
vtkVisItClipper::GetOtherOutput()
{
    return state.otherOutput;
}

// ****************************************************************************
//  Class: CellPointsGetter
//
//  Purpose:
//    Provides a GetCellPoints method for structured meshes.
//
//  Notes:      This class is separated out so it can be used for both the
//              Structured and Rectilinear bridge classes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 29 14:35:21 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

class CellPointsGetter
{
public:
    CellPointsGetter()
    {
        int d[] = {1,1,1};
        Initialize(d);
    }

    CellPointsGetter(const int  *dims)
    {
        Initialize(dims);
    }

    void Initialize(const int *dims)
    {
        pt_dims[0] = dims[0];
        pt_dims[1] = dims[1];
        pt_dims[2] = dims[2];
        cell_dims[0] = (pt_dims[0]>1) ? (pt_dims[0]-1) : pt_dims[0];
        cell_dims[1] = (pt_dims[1]>1) ? (pt_dims[1]-1) : pt_dims[1];
        cell_dims[2] = (pt_dims[2]>1) ? (pt_dims[2]-1) : pt_dims[2];
        strideY = cell_dims[0];
        strideZ = cell_dims[0]*cell_dims[1];
        ptstrideY = pt_dims[0];
        ptstrideZ = pt_dims[0]*pt_dims[1];

        cellType = (dims[2] <= 1) ? VTK_QUAD : VTK_HEXAHEDRON;
        nCellPts = (dims[2] <= 1) ? 4 : 8;
    }

    inline vtkIdType *GetCellPoints(vtkIdType cellId, vtkIdType &nCellPoints)
    {
        const int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
        const int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
        const int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
        vtkIdType cellI = cellId % cell_dims[0];
        vtkIdType cellJ = (cellId/strideY) % cell_dims[1];
        vtkIdType cellK = (cellId/strideZ);
        for (int j = 0; j<nCellPts; j++)
        {
            ids[j] = (cellI + X_val[j]) +
                     (cellJ + Y_val[j])*ptstrideY +
                     (cellK + Z_val[j])*ptstrideZ;
        }
        nCellPoints = nCellPts;
        return ids;
    }

    inline const int *GetDimensions() const { return pt_dims; }
    inline int GetCellType(vtkIdType) const { return cellType; }
private:
    vtkIdType   ids[8];

    int         pt_dims[3];
    vtkIdType   cell_dims[3];
    vtkIdType   strideY, strideZ;
    vtkIdType   ptstrideY, ptstrideZ;
    vtkIdType   nCellPts;
    int         cellType;
};

// ****************************************************************************
//  Class: ClipperBridge
//
//  Purpose:
//    This class is bridge that provides methods to access cells and points for
//    classes that use vtkPoints for their points.
//
//  Notes:      The class is templated on PointGetter so we can access point
//              data directly as memory or using GetPoints calls.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Mar 26 13:38:48 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

template <typename PointGetter>
class ClipperBridge
{
public:
    ClipperBridge(vtkPointSet *d) : points(d->GetPoints()), ds(d)
    {
    }

    inline vtkIdType GetNumberOfCells()  { return ds->GetNumberOfCells(); }
    inline vtkIdType GetNumberOfPoints() { return ds->GetNumberOfPoints(); }
    inline vtkCellData *GetCellData()    { return ds->GetCellData(); }
    inline vtkPointData *GetPointData()  { return ds->GetPointData(); }

    inline void GetPoint(vtkIdType index, double pt[3]) const
    {
        points.GetPoint(index, pt);
    }

    void ConstructDataSet(vtkVolumeFromVolume &vfv,
                          vtkUnstructuredGrid *output)
    {
        vfv.ConstructDataSet(GetPointData(), GetCellData(), output, ds->GetPoints());
    }
private:
    PointGetter  points;
    vtkPointSet *ds;
};

//
// These subclasses are necessary because GetCellPoints is not implemented in vtkPointSet.
//
template <typename PointGetter>
class ClipperBridgePolyData : public ClipperBridge<PointGetter>
{
public:
    ClipperBridgePolyData(vtkPolyData *d) : ClipperBridge<PointGetter>(d)
    {
        pd = d;
    }

    inline int GetCellType(vtkIdType cellId) const
    {
        return pd->GetCellType(cellId);
    }

    inline vtkIdType *GetCellPoints(vtkIdType cellId, vtkIdType &nCellPts)
    {
        vtkIdType *cellPts = NULL;
        pd->GetCellPoints(cellId, nCellPts, cellPts);
        return cellPts;
    }
private:
    vtkPolyData *pd;
};

template <typename PointGetter>
class ClipperBridgeUnstructuredGrid : public ClipperBridge<PointGetter>
{
public:
    ClipperBridgeUnstructuredGrid(vtkUnstructuredGrid *d) : ClipperBridge<PointGetter>(d)
    {
        ug = d;
    }

    inline int GetCellType(vtkIdType cellId) const
    {
        return ug->GetCellType(cellId);
    }

    inline vtkIdType *GetCellPoints(vtkIdType cellId, vtkIdType &nCellPts)
    {
        vtkIdType *cellPts = NULL;
        ug->GetCellPoints(cellId, nCellPts, cellPts);
        return cellPts;
    }
private:
    vtkUnstructuredGrid *ug;
};

template <typename PointGetter>
class ClipperBridgeStructuredGrid : public ClipperBridge<PointGetter>
{
public:
    ClipperBridgeStructuredGrid(vtkStructuredGrid *d) : ClipperBridge<PointGetter>(d), 
        cellPoints()
    {
        sg = d;
        int dims[3];
        sg->GetDimensions(dims);
        cellPoints.Initialize(dims);
    }

    inline int GetCellType(vtkIdType cellId) const
    {
        return cellPoints.GetCellType(cellId);
    }

    inline vtkIdType *GetCellPoints(vtkIdType cellId, vtkIdType &nCellPts)
    {
        return cellPoints.GetCellPoints(cellId, nCellPts);
    }
private:
    vtkStructuredGrid *sg;
    CellPointsGetter   cellPoints;
};

// ****************************************************************************
//  Class: ClipperBridgeRectilinearGrid
//
//  Purpose:
//    This class is bridge that provides methods to access cells and points for
//    rectilinear grids.
//
//  Notes:      The class is templated on PointGetter so we can access point
//              data directly as memory or using GetTuple1 calls.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Mar 26 13:38:48 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

template <typename PointGetter>
class ClipperBridgeRectilinearGrid
{
public:
    ClipperBridgeRectilinearGrid(vtkRectilinearGrid *rg, const int *dims, 
        vtkDataArray *Xc, vtkDataArray *Yc, vtkDataArray *Zc) : 
        points(dims, Xc, Yc, Zc), cellPoints(dims), ds(rg), X(Xc), Y(Yc), Z(Zc)
    {
    }

    inline vtkIdType GetNumberOfCells()  { return ds->GetNumberOfCells(); }
    inline vtkIdType GetNumberOfPoints() { return ds->GetNumberOfPoints(); }
    inline vtkCellData *GetCellData()    { return ds->GetCellData(); }
    inline vtkPointData *GetPointData()  { return ds->GetPointData(); }

    inline int GetCellType(vtkIdType cellId) const
    {
        return cellPoints.GetCellType(cellId);
    }

    inline vtkIdType *GetCellPoints(vtkIdType cellId, vtkIdType &nCellPoints)
    {
        return cellPoints.GetCellPoints(cellId, nCellPoints);
    }

    inline void GetPoint(vtkIdType index, double pt[3]) const
    {
        points.GetPoint(index, pt);
    }

    inline void GetPoint(vtkIdType i, vtkIdType j, vtkIdType k, double pt[3]) const
    {
        points.GetPoint(i, j, k, pt);
    }

    void ConstructDataSet(vtkVolumeFromVolume &vfv,
                          vtkUnstructuredGrid *output)
    {
        vfv.ConstructDataSet(GetPointData(), GetCellData(), output, 
                             cellPoints.GetDimensions(), X, Y, Z);
    }
private:
    PointGetter      points;
    CellPointsGetter cellPoints;

    vtkRectilinearGrid *ds;
    vtkDataArray *X, *Y, *Z;
};

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
// ****************************************************************************

template <typename Bridge, typename ScalarAccess>
void
vtkVisItClipper_Algorithm(Bridge &bridge, ScalarAccess scalar,
    vtkVisItClipper::FilterState &state,
    vtkDataSet *input, vtkUnstructuredGrid *output,
    vtkUnstructuredGrid *stuff_I_cant_clip,
    vtkVisItClipper *clipper)
{
    int t1 = visitTimer->StartTimer();
    vtkIdType nCells = bridge.GetNumberOfCells();
    vtkIdType nPts = bridge.GetNumberOfPoints();

    //
    // Do the actual clipping here
    //
    vtkIdType ptSizeGuess = (state.CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : state.CellListSize*5 + 100);

    vtkVolumeFromVolume vfvIn(nPts, ptSizeGuess);
    vtkVolumeFromVolume vfvOut(nPts, ptSizeGuess);
    vtkVolumeFromVolume *useVFV = NULL;

    const int max_pts = 8;

    vtkIdType nToProcess = (state.CellList != NULL ? state.CellListSize : nCells);
    vtkIdType numIcantClip = 0;

    for (vtkIdType i = 0 ; i < nToProcess ; i++)
    {
        // Get the cell details
        vtkIdType cellId = (state.CellList != NULL ? state.CellList[i] : i);
        clipper->ModifyClip(input, cellId);
        int cellType = bridge.GetCellType(cellId);
        vtkIdType nCellPts = 0;
        vtkIdType *cellPts = bridge.GetCellPoints(cellId, nCellPts);

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
                                       CopyAllocate(bridge.GetCellData(), nCells);

                stuff_I_cant_clip->InsertNextCell(cellType, nCellPts, cellPts);
                stuff_I_cant_clip->GetCellData()->
                            CopyData(bridge.GetCellData(), cellId, numIcantClip);
                numIcantClip++;
                continue;
            }
            break;
        }

        // fill the dist functions and calculate lookup case
        int lookup_case = 0;
        double dist[max_pts];
        for (int j = nCellPts-1 ; j >= 0 ; j--)
        {
            if (state.precomputeClipScalars)
            {
                dist[j] = state.scalarCutoff - scalar.GetTuple1(cellPts[j]);
            }
            else
            {
                double pt[3];
                bridge.GetPoint(cellPts[j], pt);
                dist[j] = -state.clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }

            if (dist[j] >= 0)
                lookup_case++;
            if (j > 0)
                lookup_case *= 2;
        }

        if (state.removeWholeCells && lookup_case != 0)
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
                        double dir = dist[pt2] - dist[pt1];
                        double amt = 0. - dist[pt1];
                        double percent = 1. - (amt / dir);

                        // We may have physically (though not logically)
                        // degenerate cells if percent==0 or percent==1.
                        // We could pretty easily and mostly safely clamp
                        // percent to the range [1e-4, 1. - 1e-4] here.
                        vtkIdType ptId1 = cellPts[pt1];
                        vtkIdType ptId2 = cellPts[pt2];

                        // deal with exact zero crossings if requested
                        if (state.clipFunction != NULL && state.useZeroCrossings)
                        {
                            double p0[3], p1[3];
                            bridge.GetPoint(ptId1, p0);
                            bridge.GetPoint(ptId2, p1);

                            AdjustPercentToZeroCrossing(p0, p1,
                                                        state.clipFunction,
                                                        &percent);
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
                    if (state.computeInsideAndOut)
                        interpIDsOut[interpID] = vfvOut.AddCentroidPoint(npts, shape);
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
        bridge.ConstructDataSet(vfvIn, just_from_zoo);

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
            bridge.ConstructDataSet(vfvOut, just_from_zoo);

            appender->AddInputData(just_from_zoo);
            appender->Update();

            if (state.otherOutput != NULL)
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
        bridge.ConstructDataSet(vfvIn, output);

        if (state.computeInsideAndOut)
        {
            if (state.otherOutput)
                state.otherOutput->Delete();
            state.otherOutput = vtkUnstructuredGrid::New();
            bridge.ConstructDataSet(vfvOut, state.otherOutput);
        }
    }
    visitTimer->StopTimer(t2, "VFV Constructing data set");
}

// ****************************************************************************
//  Method: vtkVisItClipper_RectExecute
//
//  Purpose: 
//    Execute the clipping algorithm to different data via a bridge thet lets
//    us deal with different mesh types and coordinate precisions. This function
//    sets up structured meshes specially.
//
//  Arguments:
//
//  Returns:    
//
//  Note:       Same
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

template <typename Bridge>
void
vtkVisItClipper_RectExecute(Bridge bridge, const int *pt_dims,
    vtkVisItClipper::FilterState &state,
    vtkDataSet *input, vtkUnstructuredGrid *output,
    vtkUnstructuredGrid *stuff_I_cant_clip,
    vtkVisItClipper *clipper) 
{
    if(state.clipFunction != NULL)
    {
        double *scalar = new double[bridge.GetNumberOfPoints()];
        vtkAccessor<double> scalarAccess(scalar);
        vtkIdType id = 0;
        if (state.precomputeClipScalars)
        {
            for (vtkIdType k=0; k<pt_dims[2]; k++)
            {
                for (vtkIdType j=0; j<pt_dims[1]; j++)
                {
                    for (vtkIdType i=0; i<pt_dims[0]; i++, id++)
                    {
                        double pt[3];
                        bridge.GetPoint(i,j,k,pt);
                        scalar[id] = -state.clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
                    }
                }
            }
        }

        vtkVisItClipper_Algorithm(bridge, scalarAccess, state, input, output, 
            stuff_I_cant_clip, clipper);
        delete [] scalar;
    }
    else if(state.scalarArrayAsVTK != NULL)
    {
        int dt = state.scalarArrayAsVTK->GetDataType();
        if(dt == VTK_FLOAT)
            vtkVisItClipper_Algorithm(bridge, 
                vtkAccessor<float>(state.scalarArrayAsVTK),
                state, input, output, stuff_I_cant_clip, clipper);
        else if(dt == VTK_DOUBLE)
            vtkVisItClipper_Algorithm(bridge, 
                vtkAccessor<double>(state.scalarArrayAsVTK),
                state, input, output, stuff_I_cant_clip, clipper);
        else
            vtkVisItClipper_Algorithm(bridge, 
                vtkGeneralAccessor(state.scalarArrayAsVTK),
                state, input, output, stuff_I_cant_clip, clipper);
    }
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

template <typename Bridge>
void
vtkVisItClipper_Execute(Bridge bridge,
    vtkVisItClipper::FilterState &state,
    vtkDataSet *input, vtkUnstructuredGrid *output,
    vtkUnstructuredGrid *stuff_I_cant_clip,
    vtkVisItClipper *clipper)
{
    if(state.clipFunction != NULL)
    {
        vtkIdType nPts = bridge.GetNumberOfPoints();
        double *scalar = new double[nPts];
        if (state.precomputeClipScalars)
        {
            for (vtkIdType i=0; i < nPts; i++)
            {
                double pt[3];
                bridge.GetPoint(i, pt);
                scalar[i] = -state.clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
            }
        }

        vtkVisItClipper_Algorithm(bridge, vtkAccessor<double>(scalar), 
                                  state, input, output, stuff_I_cant_clip, clipper);
        delete [] scalar;
    }
    else if(state.scalarArrayAsVTK != NULL)
    {
        int dt = state.scalarArrayAsVTK->GetDataType();
        if(dt == VTK_FLOAT)
            vtkVisItClipper_Algorithm(bridge, 
                 vtkAccessor<float>(state.scalarArrayAsVTK),
                 state, input, output, stuff_I_cant_clip, clipper);
        else if(dt == VTK_DOUBLE)
            vtkVisItClipper_Algorithm(bridge, 
                 vtkAccessor<double>(state.scalarArrayAsVTK),
                 state, input, output, stuff_I_cant_clip, clipper);
        else
            vtkVisItClipper_Algorithm(bridge, 
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
    vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int t0 = visitTimer->StartTimer();
    int do_type = ds->GetDataObjectType();

    if (do_type == VTK_RECTILINEAR_GRID || do_type == VTK_STRUCTURED_GRID)
    {
        vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
        int pt_dims[3] = {0,0,0};
        if (do_type == VTK_RECTILINEAR_GRID)
        {
            vtkRectilinearGrid *rg = (vtkRectilinearGrid*)ds;
            rg->GetDimensions(pt_dims);

            vtkDataArray *X = rg->GetXCoordinates();
            vtkDataArray *Y = rg->GetYCoordinates();
            vtkDataArray *Z = rg->GetZCoordinates();
            int tx = X->GetDataType();
            int ty = Y->GetDataType();
            int tz = Z->GetDataType();
            bool same = (tx == ty) && (ty == tz);
            if(same && tx == VTK_FLOAT)
            {
                ClipperBridgeRectilinearGrid<vtkRectPointAccessor<float> > 
                    bridge(rg, pt_dims, X, Y, Z);
                vtkVisItClipper_RectExecute(bridge, pt_dims, 
                    this->state, ds, output, stuff_I_cant_clip, this);
            }
            else if(same && tx == VTK_DOUBLE)
            {
                ClipperBridgeRectilinearGrid<vtkRectPointAccessor<double> > 
                    bridge(rg, pt_dims, X, Y, Z);
                vtkVisItClipper_RectExecute(bridge, pt_dims, 
                    this->state, ds, output, stuff_I_cant_clip, this);
            }
            else
            {
                ClipperBridgeRectilinearGrid<vtkGeneralRectPointAccessor> 
                    bridge(rg, pt_dims, X, Y, Z);
                vtkVisItClipper_RectExecute(bridge, pt_dims, 
                    this->state, ds, output, stuff_I_cant_clip, this);
            }
        }
        else // do_type == VTK_STRUCTURED_GRID
        {
            vtkStructuredGrid *sg = (vtkStructuredGrid*)ds;
            sg->GetDimensions(pt_dims);
            if(sg->GetPoints()->GetDataType() == VTK_FLOAT)
            {
                ClipperBridgeStructuredGrid<vtkPointAccessor<float> > 
                    bridge(sg);
                vtkVisItClipper_Execute(bridge,  this->state, ds, output, 
                    stuff_I_cant_clip, this);
            }
            else if(sg->GetPoints()->GetDataType() == VTK_DOUBLE)
            {
                ClipperBridgeStructuredGrid<vtkPointAccessor<double> > bridge(sg);
                vtkVisItClipper_Execute(bridge,  this->state, ds, output, 
                    stuff_I_cant_clip, this);
            }
/* This case probably does not happen...
            else
            {
                ClipperBridge<vtkGeneralPointAccessor> bridge(sg);
                vtkVisItClipper_Execute(bridge, this->state, ds, output,
                    stuff_I_cant_clip, this);
            }
*/
        }

        stuff_I_cant_clip->Delete();
    }
    else if (do_type == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *)ds;

        vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
        stuff_I_cant_clip->SetPoints(ug->GetPoints());
        stuff_I_cant_clip->GetPointData()->ShallowCopy(ug->GetPointData());
        stuff_I_cant_clip->Allocate(ug->GetNumberOfCells());

        if(ug->GetPoints()->GetDataType() == VTK_FLOAT)
        {
            ClipperBridgeUnstructuredGrid<vtkPointAccessor<float> > bridge(ug);
            vtkVisItClipper_Execute(bridge, this->state, ds, output, 
                stuff_I_cant_clip, this);
        }
        else if(ug->GetPoints()->GetDataType() == VTK_DOUBLE)
        {
            ClipperBridgeUnstructuredGrid<vtkPointAccessor<double> > bridge(ug);
            vtkVisItClipper_Execute(bridge, this->state, ds, output, 
                stuff_I_cant_clip, this);
        }
/* This case probably does not happen...
        else
        {
            ClipperBridgeUnstructuredGrid<vtkGeneralPointAccessor> bridge(ug);
            vtkVisItClipper_UnstructuredExecute(bridge, this->state, ds,
                output, stuff_I_cant_clip, this);
        }
*/

        stuff_I_cant_clip->Delete();
    }
    else if (do_type == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *)ds;

        vtkUnstructuredGrid *stuff_I_cant_clip = vtkUnstructuredGrid::New();
        stuff_I_cant_clip->SetPoints(pd->GetPoints());
        stuff_I_cant_clip->GetPointData()->ShallowCopy(pd->GetPointData());
        stuff_I_cant_clip->Allocate(pd->GetNumberOfCells());

        if(pd->GetPoints()->GetDataType() == VTK_FLOAT)
        {
            ClipperBridgePolyData<vtkPointAccessor<float> > bridge(pd);
            vtkVisItClipper_Execute(bridge, this->state, ds, output, 
                stuff_I_cant_clip, this);
        }
        else if(pd->GetPoints()->GetDataType() == VTK_DOUBLE)
        {
            ClipperBridgePolyData<vtkPointAccessor<double> > bridge(pd);
            vtkVisItClipper_Execute(bridge, this->state, ds, output, 
                stuff_I_cant_clip, this);
        }
/* This case probably does not happen...
        else
        {
            ClipperBridgePolyData<vtkGeneralPointAccessor> bridge(pd);
            vtkVisItClipper_Execute(bridge, this->state, ds, output,
                stuff_I_cant_clip, this);
        }
*/

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
