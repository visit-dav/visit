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

#include <vtkVisItSplitter.h>

#include <vtkAppendFilter.h>
#include <vtkBinaryPartitionVolumeFromVolume.h>
#include <vtkCellData.h>
#include <vtkCSGFixedLengthBitField.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
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

#include <vtkAccessors.h>

#include <ImproperUseException.h>

#include <DebugStream.h>

#include <math.h>
#include <vector>

#include <ClipCases.h>
#include <vtkTriangulationTables.h>

#include <TimingsManager.h>

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
// Modifications:
//   Brad Whitlock, Fri Mar 23 17:13:49 PDT 2012
//   Pass in points.
//
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

vtkVisItSplitter::FilterState::FilterState()
{ 
    this->CellList = NULL;
    this->CellListSize = 0;

    this->clipFunction = NULL;
    this->scalarArrayAsVTK = NULL;
    this->scalarCutoff = 0.;

    this->removeWholeCells = false;
    this->insideOut = false;
    this->useZeroCrossings = false;

    this->oldTags = NULL;
    this->newTags = NULL;
    this->newTagBit = 0;
}

vtkVisItSplitter::FilterState::~FilterState()
{
    if (this->clipFunction != NULL)
        this->clipFunction->Delete();

    if (this->scalarArrayAsVTK != NULL)
        this->scalarArrayAsVTK->Delete();
}

void
vtkVisItSplitter::FilterState::SetCellList(const vtkIdType *cl, vtkIdType size)
{
    this->CellList = cl;
    this->CellListSize = size;
}

void
vtkVisItSplitter::FilterState::SetClipFunction(vtkImplicitFunction *func)
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
vtkVisItSplitter::FilterState::SetClipScalars(vtkDataArray *array, double cutoff)
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

// ****************************************************************************
//  Constructor:  vtkVisItSplitter::vtkVisItSplitter
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 26, 2010
//
//  Modifications:
//
// ****************************************************************************

vtkVisItSplitter::vtkVisItSplitter() : vtkUnstructuredGridAlgorithm(),
    state()
{
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
}

void
vtkVisItSplitter::SetUseZeroCrossings(bool use)
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
vtkVisItSplitter::SetCellList(const vtkIdType *cl, vtkIdType size)
{
    state.SetCellList(cl, size);
}

void
vtkVisItSplitter::SetClipFunction(vtkImplicitFunction *func)
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
//    Jeremy Meredith, Fri Feb 26 13:31:31 EST 2010
//    Initial creation, copied from vtkVisItSplitter.
//
//    Brad Whitlock, Tue Mar 27 11:51:35 PDT 2012
//    Move to FilterState.
//
//    Eric Brugger, Wed Jul 25 10:10:44 PDT 2012
//    Increase the number of boundaries that can be handled by the mulit-pass
//    CSG discretization from 128 to 512.
//
//    Eric Brugger, Thu Apr  3 08:27:15 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

void
vtkVisItSplitter::SetClipScalars(vtkDataArray *array, float cutoff)
{
    state.SetClipScalars(array, cutoff);
}

void
vtkVisItSplitter::SetInsideOut(bool io)
{
    state.insideOut = io;
}


void
vtkVisItSplitter::SetOldTagBitField(std::vector<vtkCSGFixedLengthBitField> *tags)
{
    state.oldTags = tags;
}

void
vtkVisItSplitter::SetNewTagBitField(std::vector<vtkCSGFixedLengthBitField> *tags)
{
    state.newTags = tags;
}

void
vtkVisItSplitter::SetNewTagBit(int bit)
{
    state.newTagBit = bit;
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
    state.removeWholeCells = rwc;
}

// ****************************************************************************
// Class: CellPointsGetter
//
// Purpose:
//   Provides a GetCellPoints method for structured meshes.
//
// Notes:      This class is separated out so it can be used for both the
//             Structured and Rectilinear bridge classes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 29 14:35:21 PDT 2012
//
// Modifications:
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
// Class: SplitterBridge
//
// Purpose:
//   This class is bridge that provides methods to access cells and points for
//   classes that use vtkPoints for their points.
//
// Notes:      The class is templated on PointGetter so we can access point
//             data directly as memory or using GetPoints calls.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 13:38:48 PDT 2012
//
// Modifications:
//    Eric Brugger, Wed Jul 25 10:10:44 PDT 2012
//    Increase the number of boundaries that can be handled by the mulit-pass
//    CSG discretization from 128 to 512.
//   
//    Eric Brugger, Thu Apr  3 08:27:15 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

template <typename PointGetter>
class SplitterBridge
{
public:
    SplitterBridge(vtkPointSet *d) : points(d->GetPoints()), ds(d)
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

    void ConstructDataSet(vtkBinaryPartitionVolumeFromVolume &vfv,
                          vtkUnstructuredGrid *output,
                          std::vector<vtkCSGFixedLengthBitField> *oldTags,
                          std::vector<vtkCSGFixedLengthBitField> *newTags,
                          int                  newTagBit)
    {
        vfv.ConstructDataSet(GetPointData(), GetCellData(),
                             output, ds->GetPoints(),
                             oldTags, newTags, newTagBit);
    }
private:
    PointGetter  points;
    vtkPointSet *ds;
};

//
// These subclasses are necessary because GetCellPoints is not implemented in vtkPointSet.
//
template <typename PointGetter>
class SplitterBridgePolyData : public SplitterBridge<PointGetter>
{
public:
    SplitterBridgePolyData(vtkPolyData *d) : SplitterBridge<PointGetter>(d)
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
class SplitterBridgeUnstructuredGrid : public SplitterBridge<PointGetter>
{
public:
    SplitterBridgeUnstructuredGrid(vtkUnstructuredGrid *d) : SplitterBridge<PointGetter>(d)
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
class SplitterBridgeStructuredGrid : public SplitterBridge<PointGetter>
{
public:
    SplitterBridgeStructuredGrid(vtkStructuredGrid *d) : SplitterBridge<PointGetter>(d)
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
// Class: SplitterBridgeRectilinearGrid
//
// Purpose:
//   This class is bridge that provides methods to access cells and points for
//   rectilinear grids.
//
// Notes:      The class is templated on PointGetter so we can access point
//             data directly as memory or using GetTuple1 calls.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 13:38:48 PDT 2012
//
// Modifications:
//    Eric Brugger, Wed Jul 25 10:10:44 PDT 2012
//    Increase the number of boundaries that can be handled by the mulit-pass
//    CSG discretization from 128 to 512.
//   
//    Eric Brugger, Thu Apr  3 08:27:15 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

template <typename PointGetter>
class SplitterBridgeRectilinearGrid
{
public:
    SplitterBridgeRectilinearGrid(vtkRectilinearGrid *rg, const int *dims, 
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

    void ConstructDataSet(vtkBinaryPartitionVolumeFromVolume &vfv,
                          vtkUnstructuredGrid *output,
                          std::vector<vtkCSGFixedLengthBitField> *oldTags,
                          std::vector<vtkCSGFixedLengthBitField> *newTags,
                          int                  newTagBit)
    {
        vfv.ConstructDataSet(GetPointData(), GetCellData(),
                             output, cellPoints.GetDimensions(),
                             X, Y, Z,
                             oldTags, newTags, 
                             newTagBit);
    }
private:
    PointGetter      points;
    CellPointsGetter cellPoints;

    vtkRectilinearGrid *ds;
    vtkDataArray *X, *Y, *Z;
};

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
//    Initial creation, copied from vtkVisItSplitter and modified to
//    merely split+tag output, not remove cells.
//
//    Brad Whitlock, Thu Mar 29 12:40:29 PDT 2012
//    I templated this code on Bridge and ScalarAccess so we can pass in 
//    different bridges to access various mesh types.
//
// ****************************************************************************

template <typename Bridge, typename ScalarAccess>
void
vtkVisItSplitter_Algorithm(Bridge bridge, ScalarAccess scalar,
    vtkVisItSplitter::FilterState &state,
    vtkUnstructuredGrid *output)
{
    int th1 = visitTimer->StartTimer();
    vtkIdType nCells = bridge.GetNumberOfCells();

    vtkIdType ptSizeGuess = (state.CellList == NULL
                         ? (int) pow(float(nCells), 0.6667f) * 5 + 100
                         : state.CellListSize*5 + 100);

    vtkBinaryPartitionVolumeFromVolume vfv(bridge.GetNumberOfPoints(),
                                           ptSizeGuess);

    const int max_pts = 8;

    vtkIdType nToProcess = (state.CellList != NULL ? state.CellListSize : nCells);
    for (vtkIdType i = 0 ; i < nToProcess ; i++)
    {
        // Get the cell details
        vtkIdType cellId = (state.CellList != NULL ? state.CellList[i] : i);
        int cellType = bridge.GetCellType(cellId);
        vtkIdType nCellPts = 0;
        vtkIdType *cellPts = bridge.GetCellPoints(cellId, nCellPts);

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
        double dist[max_pts];
        for (int j = nCellPts-1 ; j >= 0 ; j--)
        {
            dist[j] = state.scalarCutoff - scalar.GetTuple1(cellPts[j]);

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

                bool out = ((!state.insideOut && color == COLOR0) ||
                            ( state.insideOut && color == COLOR1));

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
    visitTimer->StopTimer(th1, "Performing splitting");

    //
    // Construct the output data set.
    //
    int th2 = visitTimer->StartTimer();
    bridge.ConstructDataSet(vfv, output, state.oldTags, state.newTags, state.newTagBit);
    visitTimer->StopTimer(th2, "VFV Constructing data set");
}

// ****************************************************************************
// Method: vtkVisItSplitter_RectExecute
//
// Purpose: 
//   Execute the clipping algorithm to different data via a bridge thet lets
//   us deal with different mesh types and coordinate precisions. This function
//   sets up structured meshes specially.
//
// Arguments:
//
// Returns:    
//
// Note:       Same
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 13:52:27 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

template <typename Bridge>
void
vtkVisItSplitter_RectExecute(Bridge bridge, const int *pt_dims,
    vtkVisItSplitter::FilterState &state,
    vtkUnstructuredGrid *output)
{
    if(state.clipFunction != NULL)
    {
        int t1 = visitTimer->StartTimer();
        double *scalar = new double[bridge.GetNumberOfPoints()];
        vtkIdType id = 0;
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
        visitTimer->StopTimer(t1, "Evaluate clip function");

        vtkVisItSplitter_Algorithm(bridge, vtkAccessor<double>(scalar),
                                   state, output);
        delete [] scalar;
    }
    else if(state.scalarArrayAsVTK != NULL)
    {
        int dt = state.scalarArrayAsVTK->GetDataType();
        if(dt == VTK_FLOAT)
            vtkVisItSplitter_Algorithm(bridge, vtkAccessor<float>(state.scalarArrayAsVTK),
                                       state, output);
        else if(dt == VTK_DOUBLE)
            vtkVisItSplitter_Algorithm(bridge, vtkAccessor<double>(state.scalarArrayAsVTK),
                                       state, output);
        else
            vtkVisItSplitter_Algorithm(bridge, vtkGeneralAccessor(state.scalarArrayAsVTK),
                                       state, output);
    }
}

// ****************************************************************************
// Method: vtkVisItSplitter_Execute
//
// Purpose: 
//   Execute the clipping algorithm to different data via a bridge thet lets
//   us deal with different mesh types and coordinate precisions. This function
//   sets up unstructured meshes specially.
//
// Arguments:
//
// Returns:    
//
// Note:       Same as structured except that point traversal is 0..N.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 13:52:27 PDT 2012
//
// Modifications:
//   
// ****************************************************************************
template <typename Bridge>
void
vtkVisItSplitter_Execute(Bridge bridge,
    vtkVisItSplitter::FilterState &state,
    vtkUnstructuredGrid *output)
{
    if(state.clipFunction != NULL)
    {
        int t1 = visitTimer->StartTimer();
        vtkIdType nPts = bridge.GetNumberOfPoints();
        double *scalar = new double[nPts];
        for (vtkIdType i=0; i < nPts; i++)
        {
            double pt[3];
            bridge.GetPoint(i, pt);
            scalar[i] = -state.clipFunction->EvaluateFunction(pt[0],pt[1],pt[2]);
        }
        visitTimer->StopTimer(t1, "Evaluate clip function");

        vtkVisItSplitter_Algorithm(bridge, vtkAccessor<double>(scalar), 
                                   state, output);
        delete [] scalar;
    }
    else if(state.scalarArrayAsVTK != NULL)
    {
        int dt = state.scalarArrayAsVTK->GetDataType();
        if(dt == VTK_FLOAT)
            vtkVisItSplitter_Algorithm(bridge, vtkAccessor<float>(state.scalarArrayAsVTK),
                                       state, output);
        else if(dt == VTK_DOUBLE)
            vtkVisItSplitter_Algorithm(bridge, vtkAccessor<double>(state.scalarArrayAsVTK),
                                       state, output);
        else
            vtkVisItSplitter_Algorithm(bridge, vtkGeneralAccessor(state.scalarArrayAsVTK),
                                       state, output);
    }
}

// ****************************************************************************
//  Method:  vtkVisItSplitter::RequestData
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
// ****************************************************************************

int
vtkVisItSplitter::RequestData(
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

    int do_type = ds->GetDataObjectType();

    // Set general input/output data
    int t0 = visitTimer->StartTimer();
    if (do_type == VTK_RECTILINEAR_GRID || do_type == VTK_STRUCTURED_GRID)
    {
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
                SplitterBridgeRectilinearGrid<vtkRectPointAccessor<float> > bridge(rg, pt_dims, X, Y, Z);
                vtkVisItSplitter_RectExecute(bridge, pt_dims, 
                    this->state, output);
            }
            else if(same && tx == VTK_DOUBLE)
            {
                SplitterBridgeRectilinearGrid<vtkRectPointAccessor<double> > bridge(rg, pt_dims, X, Y, Z);
                vtkVisItSplitter_RectExecute(bridge, pt_dims, 
                    this->state, output);
            }
            else
            {
                SplitterBridgeRectilinearGrid<vtkGeneralRectPointAccessor> bridge(rg, pt_dims, X, Y, Z);
                vtkVisItSplitter_RectExecute(bridge, pt_dims, 
                    this->state, output);
            }
        }
        else // do_type == VTK_STRUCTURED_GRID
        {
            vtkStructuredGrid *sg = (vtkStructuredGrid*)ds;
            sg->GetDimensions(pt_dims);
            if(sg->GetPoints()->GetDataType() == VTK_FLOAT)
            {
                SplitterBridgeStructuredGrid<vtkPointAccessor<float> > bridge(sg);
                vtkVisItSplitter_Execute(bridge,  
                    this->state, output);
            }
            else if(sg->GetPoints()->GetDataType() == VTK_FLOAT)
            {
                SplitterBridgeStructuredGrid<vtkPointAccessor<double> > bridge(sg);
                vtkVisItSplitter_Execute(bridge,  
                    this->state, output);
            }
/* This case probably does not happen...
            else
            {
                SplitterBridge<vtkGeneralPointAccessor> bridge(sg);
                vtkVisItSplitter_Execute(bridge, 
                    this->state, output);
            }
*/
        }
    }
    else if (do_type == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *)ds;

        if(ug->GetPoints()->GetDataType() == VTK_FLOAT)
        {
            SplitterBridgeUnstructuredGrid<vtkPointAccessor<float> > bridge(ug);
            vtkVisItSplitter_Execute(bridge,
                this->state, output);
        }
        else if(ug->GetPoints()->GetDataType() == VTK_DOUBLE)
        {
            SplitterBridgeUnstructuredGrid<vtkPointAccessor<double> > bridge(ug);
            vtkVisItSplitter_Execute(bridge,
                this->state, output);
        }
/* This case probably does not happen...
        else
        {
            SplitterBridgeUnstructuredGrid<vtkGeneralPointAccessor> bridge(ug);
            vtkVisItSplitter_UnstructuredExecute(bridge, 
                this->state, output);
        }
*/
    }
    else if (do_type == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *)ds;

        if(pd->GetPoints()->GetDataType() == VTK_FLOAT)
        {
            SplitterBridgePolyData<vtkPointAccessor<float> > bridge(pd);
            vtkVisItSplitter_Execute(bridge,
                this->state, output);
        }
        else if(pd->GetPoints()->GetDataType() == VTK_DOUBLE)
        {
            SplitterBridgePolyData<vtkPointAccessor<double> > bridge(pd);
            vtkVisItSplitter_Execute(bridge,
                this->state, output);
        }
/* This case probably does not happen...
        else
        {
            SplitterBridgePolyData<vtkGeneralPointAccessor> bridge(pd);
            vtkVisItSplitter_Execute(bridge, 
                this->state, output);
        }
*/
    }
    else
    {
        debug1 << "vtkVisItSplitter: Can't operate on this dataset.\n";
    }
    visitTimer->StopTimer(t0, "Splitter");

    return 1;
}

// ****************************************************************************
//  Method: vtkVisItSplitter::FillInputPortInformation
//
// ****************************************************************************

int
vtkVisItSplitter::FillInputPortInformation(int, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

// ****************************************************************************
//  Method: vtkVisItSplitter::PrintSelf
//
// ****************************************************************************

void
vtkVisItSplitter::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os,indent);
}
