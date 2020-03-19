// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTransparencyActor.C                          //
// ************************************************************************* //

#include <avtTransparencyActor.h>

#include <float.h>
#include <cstring>
#include <limits>
#include <algorithm>
#include <cmath>

#include <avtParallel.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAxisDepthSort.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkDepthSortPolyData2.h>
#include <vtkDoubleArray.h>
#include <vtkGeometryFilter.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkParallelImageSpaceRedistributor.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkVisItPolyDataNormals.h>

#include <ColorAttribute.h>

#include <DebugStream.h>
#include <BadIndexException.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

#ifdef PARALLEL
#include "mpi.h"
#endif

using std::vector;
using std::numeric_limits;

#ifdef _MSC_VER
template <typename num_t>
num_t log2(num_t num)
{ return log(num)/log(num_t(2)); }
#endif

// ****************************************************************************
//  Class: Bounds
//
//  Purpose:
//      internal helper class that encapsulate bounds and various
//      common manipulations
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

class Bounds
{
public:
    Bounds();
    Bounds(double *p, int i);
    Bounds(double lx, double hx, double ly, double hy, double lz, double hz, int i);
    Bounds(const Bounds &o);
    ~Bounds();

    Bounds &operator=(const Bounds &o);

    bool IsEmpty() const
    { return (bds[1] < bds[0]) || (bds[3] < bds[2])
      || (bds[5] < bds[4]); }

    operator bool() const
    { return !IsEmpty(); }

    static
    void MakeEmpty(double *b);

    bool IntersectionIsEmpty(const Bounds &o) const;

    double *GetBounds(){ return bds; }
    const double *GetBounds() const { return bds; }
    void GetBounds(double *b) const;
    void SetBounds(double *p, bool take);

    Bounds &ShrinkByEpsInPlace();

    void SetId(int i){ id = i; }
    int GetId() const { return id; }

    void UpdateCentroidDistance(const double *udir, const double *cam);
    double GetCentroidDistance() const { return dist; }

private:
    friend bool greater(const Bounds &l, const Bounds &r);
    friend ostream &operator<<(ostream &os, const Bounds &b);

private:
    double *bds; // ptr to 6 doubles xlo xhi ylo yhi zlo zhi
    double dist; // distance to the centroid along some direction
    int id; // process id of the owner
    bool own; // flag indicates if we need ot delete the array
};

// ****************************************************************************
//  helper functions for Bounds
//      greater -- predicate used to sort the bounds
//      operator<< -- used to print bounds, vectors of bounds,
//          and vectors of doubles
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
bool greater(const Bounds &l, const Bounds &r)
{ return l.dist > r.dist; }

#ifdef avtTransparencyActorDEBUG
ostream &operator<<(ostream &os, const Bounds &b)
{
    os << "id=" << b.id << " dist=" << b.dist << " bds=" << " ["
        << b.bds[0] << ", " << b.bds[1] << b.bds[2] << ", " << b.bds[3]
        << b.bds[4] << ", " << b.bds[5] << "]";
    return os;
}

ostream &operator<<(ostream &os, const vector<Bounds> &vb)
{
    size_t n = vb.size();
    for (size_t i = 0; i < n; ++i)
        os << vb[i] << endl;
    return os;
}

template <typename T>
ostream &operator<<(ostream &os, const vector<T> &vb)
{
    size_t n = vb.size();
    if (n)
    {
        os << vb[0];
        for (size_t i = 1; i < n; ++i)
            os << ", " << vb[i];
    }
    return os;
}
#endif

// ****************************************************************************
//  Method Bounds::Bounds
//
//  Purpose:
//      constructor
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds::Bounds() :
  bds(static_cast<double*>(malloc(6*sizeof(double)))),
  dist(numeric_limits<double>::max()), id(0), own(true)
{
    MakeEmpty(bds);
}

// ****************************************************************************
//  Method Bounds::~Bounds
//
//  Purpose:
//      destructor
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds::~Bounds()
{
    if (own)
        free(bds);
}

// ****************************************************************************
//  Method Bounds::Bounds
//
//  Purpose:
//      zero copy construct from exsiting data
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds::Bounds(double *p, int i) :
  bds(p), dist(numeric_limits<double>::max()), id(i), own(false)
{}

// ****************************************************************************
//  Method Bounds::Bounds
//
//  Purpose:
//      construct, allocate and copy the data
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds::Bounds(double lx, double hx, double ly,
    double hy, double lz, double hz, int i) :
  bds(static_cast<double*>(malloc(6*sizeof(double)))),
  dist(numeric_limits<double>::max()), id(i), own(true)
{
    bds[0] = lx; bds[1] = hx;
    bds[2] = ly; bds[3] = hy;
    bds[4] = lz; bds[5] = hz;
}

// ****************************************************************************
//  Method Bounds::Bounds
//
//  Purpose:
//      copy constructor, a deep copy is made
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds::Bounds(const Bounds &o) :
  bds(static_cast<double*>(malloc(6*sizeof(double)))),
  dist(o.dist), id(o.id), own(true)
{
    memcpy(bds, o.bds, 6*sizeof(double));
}

// ****************************************************************************
//  Method Bounds::operator=
//
//  Purpose:
//      assignment operator
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds &Bounds::operator=(const Bounds &o)
{
    if (this == &o)
        return *this;
    SetBounds(o.bds, true);
    id = o.id;
    dist = o.dist;
    return *this;
}

// ****************************************************************************
//  Method Bounds::IntersectionIsEmpty
//
//  Purpose:
//      test to determine if this bounds overlaps the other one
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

bool Bounds::IntersectionIsEmpty(const Bounds &o) const
{
    if (IsEmpty() || o.IsEmpty())
        return true;

    double lo = bds[0] > o.bds[0] ? bds[0] : o.bds[0];
    double hi = bds[1] < o.bds[1] ? bds[1] : o.bds[1];
    if (lo > hi)
        return true;

    lo = bds[2] > o.bds[2] ? bds[2] : o.bds[2];
    hi = bds[3] < o.bds[3] ? bds[3] : o.bds[3];
    if (lo > hi)
        return true;

    lo = bds[4] > o.bds[4] ? bds[4] : o.bds[4];
    hi = bds[5] < o.bds[5] ? bds[5] : o.bds[5];
    if (lo > hi)
        return true;

    return false;
}

// ****************************************************************************
//  Method Bounds::MakeEmpty
//
//  Purpose:
//      initializes the bounds
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void Bounds::MakeEmpty(double *b)
{
    b[0] =  numeric_limits<double>::max();
    b[1] = -numeric_limits<double>::max();
    b[2] =  numeric_limits<double>::max();
    b[3] = -numeric_limits<double>::max();
    b[4] =  numeric_limits<double>::max();
    b[5] = -numeric_limits<double>::max();
}

// ****************************************************************************
//  Method Bounds::GetBounds
//
//  Purpose:
//      copy the data into the proivided array
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void Bounds::GetBounds(double *p) const
{
    memcpy(p, bds, 6*sizeof(double));
}

// ****************************************************************************
//  Method Bounds::SetBounds
//
//  Purpose:
//      shallow/deep copy6 the data from the provided array
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void Bounds::SetBounds(double *p, bool deep)
{
    if (deep)
    {
        if (!bds || !own)
            bds = static_cast<double*>(malloc(6*sizeof(double)));
        memcpy(bds, p, 6*sizeof(double));
        own = true;
    }
    else
    {
        if (own)
            free(bds);
        bds = p;
        own = false;
    }
}

// ****************************************************************************
//  Method Bounds::ShrinkByEpsInPlace
//
//  Purpose:
//      shrinks this bounds by a very small amount. eps is the value
//      of the least significant bit that we care about. machine
//      eps is in bit position 53 for double. we will use bit position 10
//      to avoid rounding issues and it's precise enough.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

Bounds &Bounds::ShrinkByEpsInPlace()
{
    double prec = 10.0;
    double ulpx = pow(2.0, log2(bds[1] - bds[0]) - prec);
    bds[0] += ulpx;
    bds[1] -= ulpx;

    double ulpy = pow(2.0, log2(bds[3] - bds[2]) - prec);
    bds[2] += ulpy;
    bds[3] -= ulpy;

    double ulpz = pow(2.0, log2(bds[5] - bds[4]) - prec);
    bds[4] += ulpz;
    bds[5] -= ulpz;

    return *this;
}

// ****************************************************************************
//  Method Bounds::UpdateCentroidDistance
//
//  Purpose:
//      Compute the distance to the centroid of the bounds in a
//      given direction. bounds can be sorted by this distance.
//
//  Programmer: Burlen Loring
//  Creation:   Mon Aug 17 15:35:23 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void Bounds::UpdateCentroidDistance(const double *udir, const double *cam)
{
    if (!IsEmpty())
    {
        double cen[3] = {
            cam[0] - (bds[0] + bds[1])/2.0,
            cam[1] - (bds[2] + bds[3])/2.0,
            cam[2] - (bds[4] + bds[5])/2.0
            };

        dist = udir[0]*cen[0] + udir[1]*cen[1] + udir[2]*cen[2];
    }
}






// ****************************************************************************
//  Method: avtTransparencyActor constructor
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Initialize perfectSort.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six sorts.
//
//    Jeremy Meredith, Fri Jul 26 14:30:40 PDT 2002
//    Default perfect sorting to true.
//
//    Chris Wojtan, Wed Jul 7 10:17 PDT 2004
//    Added parallel support
//
//    Jeremy Meredith, Thu Oct 21 12:16:38 PDT 2004
//    Enhanced parallel support.  Made it use avtParallel code instead of
//    ifdefs so we didn't have to build a parallel version of this library.
//
//    Brad Whitlock, Mon Nov 1 15:33:56 PST 2004
//    Passed rank and size into the image space redistributor so that
//    library could be built on MacOS X.
//
//    Kathleen Bonnell, Tue Jan 11 16:06:33 PST 2005 
//    Initialize is2Dimensional.
//
//    Brad Whitlock, Fri Jan 23 15:27:34 PST 2009
//    Pass the communicator to vtkParallelImageSpaceRedistributor.
//
//    Tom Fogal, Sun May 24 21:31:20 MDT 2009
//    Initialize transparenciesExist && cachedTransparencies.
//
//    Burlen Loring, Fri Aug 14 11:53:08 PDT 2015
//    Addded skip sort flag
//
//    Burlen Loring, Wed Aug 19 14:00:39 PDT 2015
//    Added new sort pipelines in support of ordered compositing
//    and depth peeling
//
// ****************************************************************************

avtTransparencyActor::avtTransparencyActor() :
    inputModified(true),
    actorMTime(0),
    appender(NULL),
    distribute(NULL),
    myActor(NULL),
    myMapper(NULL),
    axisSort(NULL),
    distributeDepthSort(NULL),
    depthSort(NULL),
    usePerfectSort(true),
    is2Dimensional(false),
    lastCamera(NULL),
    renderingSuspended(false),
    transparenciesExist(false),
    cachedTransparencies(false),
    sortOp(SORT_NONE)
{
    appender = vtkAppendPolyData::New();
    myMapper = vtkPolyDataMapper::New();

    myActor = vtkActor::New();
    myActor->SetMapper(myMapper);

    // Tell the mapper that we are going to set up an RGBA field ourselves.
    myMapper->SetColorModeToDefault();
    myMapper->ColorByArrayComponent("Colors", 0);
    myMapper->SetScalarModeToUsePointFieldData();

    axisSort = vtkAxisDepthSort::New();
    axisSort->SetInputConnection(appender->GetOutputPort());

    depthSort = vtkDepthSortPolyData2::New();
    depthSort->SetDepthSortModeToBoundsCenter();
    depthSort->SetInputConnection(appender->GetOutputPort());

#ifdef PARALLEL
    distribute = vtkParallelImageSpaceRedistributor::New();
    distribute->SetRankAndSize(PAR_Rank(), PAR_Size());
    distribute->SetCommunicator(VISIT_MPI_COMM);
    distribute->SetInputConnection(appender->GetOutputPort());

    distributeDepthSort = vtkDepthSortPolyData2::New();
    distributeDepthSort->SetDepthSortModeToBoundsCenter();
    distributeDepthSort->SetInputConnection(distribute->GetOutputPort());
#endif

    lastCamera = vtkMatrix4x4::New();
}


// ****************************************************************************
//  Method: avtTransparencyActor destructor
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Delete perfectSort.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six sorts.
//
//    Chris Wojtan, Wed Jul 7 10:17 PDT 2004
//    Added parallel support
//
// ****************************************************************************

avtTransparencyActor::~avtTransparencyActor()
{
    appender->Delete();
    myActor->Delete();
    myMapper->Delete();
    axisSort->Delete();
    depthSort->Delete();
#ifdef PARALLEL
    distribute->Delete();
    distributeDepthSort->Delete();
#endif
    lastCamera->Delete();
}


// ****************************************************************************
//  Method: avtTransparencyActor::InputWasModified
//
//  Purpose:
//      This is a hint to the transparency actor that one of its inputs was
//      modified.  It will not look through the inputs when rendering each
//      frame, so this is the mechanism to indicate that it should recalculate.
//
//  Arguments:
//      <unused>   The index of the input that was modified.
//
//  Programmer:  Hank Childs
//  Creation:    July 8, 2002
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Added opacity arg. Added code to stick opacity value in map
//
// ****************************************************************************

void
avtTransparencyActor::InputWasModified(int transparencyIndex, double opacity)
{
    inputModified = true;
    if (opacity != -1.0)
        inputsOpacities[transparencyIndex] = opacity;
}


// ****************************************************************************
//  Method: avtTransparencyActor::UsePerfectSort
//
//  Purpose:
//      Tells the transparency actor to use the best possible sort for the
//      next frame.
//
//  Arguments:
//      perfect    True if future updates should sort perfectly
//
//  Programmer:  Hank Childs
//  Creation:    July 11, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jul 26 14:29:06 PDT 2002
//    Made it take a bool, since we can turn it on or off as a permanent
//    mode of operation now.
//
//    Jeremy Meredith, Thu Oct 21 12:18:23 PDT 2004
//    Call TransparenciesExist now because the logic has become more complex.
//    Specifically, it may have needed an update, and it needed unification
//    in parallel.
//
// ****************************************************************************

bool
avtTransparencyActor::UsePerfectSort(bool perfect)
{
    usePerfectSort = perfect;
    return TransparenciesExist();
}


// ****************************************************************************
//  Method: avtTransparencyActor::AddInput
//
//  Purpose:
//      Adds the VTK constructs for a plot.
//
//  Arguments:
//      d       A vector of datasets.
//      m       A vector of mappers.
//      a       A vector of actors.
//
//  Returns:    The index of the newly added input.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 16:02:45 PDT 2002
//    Add visibility.
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Initialized inputsOpacities
//
//    Kathleen Biagas, Tue Jul 12 13:43:22 MST 2016
//    Store actor's current visibility state.
//
//    Kathleen Biagas, Tue Nov  5 12:35:13 PST 2019
//    Don't allow vtkPointGlyphMapper to participate in the sorting.
//
// ****************************************************************************

int
avtTransparencyActor::AddInput(vector<vtkDataSet *> &d, 
                          vector<vtkDataSetMapper *> &m, vector<vtkActor *> &a)
{
    int index = (int)datasets.size();

    vector<vtkDataSet *> d2; 
    vector<vtkDataSetMapper *> m2;
    vector<vtkActor *> a2;
    for (size_t i = 0; i < d.size(); ++i)
    {
        if (m[i] != NULL && m[i]->IsA("vtkPointGlyphMapper"))
            continue;

        d2.push_back(d[i]);
        m2.push_back(m[i]);
        a2.push_back(a[i]);
    }
    datasets.push_back(d2);
    mappers.push_back(m2);
    actors.push_back(a2);
    useActor.push_back(true);
    visibility.push_back(true);

    size_t size = d2.size();
    vector<vtkPolyData *> pd;
    for (size_t i = 0 ; i < size ; ++i)
        pd.push_back(NULL);
    preparedDataset.push_back(pd);

    vector<int> vis;
    for (size_t i = 0 ; i < a2.size(); ++i)
    {
        if (a2[i] != NULL)
            vis.push_back(a2[i]->GetVisibility());
        else
            vis.push_back(1);
    }
    actorVis.push_back(vis);

    inputModified = true;

    inputsOpacities[index] = 1.0;

    return index;
}


// ****************************************************************************
//  Method: avtTransparencyActor::ReplaceInput
//
//  Purpose:
//      Allows you to replace the input for the i'th input with a new set of
//      VTK constructs.
//
//  Arguments:
//      ind     The index of these VTK constructs.
//      d       A vector of datasets.
//      m       A vector of mappers.
//      a       A vector of actors.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//    Kathleen Biagas, Tue Jul 12 13:43:22 MST 2016
//    Store actor's current visibility state.
//
//    Kathleen Biagas, Tue Nov  5 12:35:13 PST 2019
//    Don't allow vtkPointGlyphMapper to participate in the sorting.
//
// ****************************************************************************

void
avtTransparencyActor::ReplaceInput(int ind, vector<vtkDataSet *> &d, 
                          vector<vtkDataSetMapper *> &m, vector<vtkActor *> &a)
{
    if (ind < 0 || (size_t)ind >= datasets.size())
    {
        EXCEPTION2(BadIndexException, ind, (int)datasets.size());
    }

    vector<vtkDataSet *> d2; 
    vector<vtkDataSetMapper *> m2;
    vector<vtkActor *> a2;
    for (size_t i = 0; i < d.size(); ++i)
    {
        if (m[i] != NULL && m[i]->IsA("vtkPointGlyphMapper"))
            continue;

        d2.push_back(d[i]);
        m2.push_back(m[i]);
        a2.push_back(a[i]);
    }

    datasets[ind] = d2;
    mappers[ind]  = m2;
    actors[ind]   = a2;

    vector<int> vis;
    for (size_t i = 0 ; i < a2.size() ; i++)
    {
        if (a2[i] != NULL)
            vis.push_back(a2[i]->GetVisibility());
        else
            vis.push_back(1);
    }
    actorVis[ind] = vis;

    for (size_t i = 0 ; i < preparedDataset[ind].size() ; i++)
    {
        if (preparedDataset[ind][i] != NULL)
        {
            preparedDataset[ind][i]->Delete();
            preparedDataset[ind][i] = NULL;
        }
    }

    inputModified = true;
}


// ****************************************************************************
//  Method: avtTransparencyActor::RemoveInput
//
//  Purpose:
//      Removes the i'th input.  This means it will free up any memory
//      associated with the index.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2003
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Set inputsOpacities for associated input to zero
//
// ****************************************************************************

void
avtTransparencyActor::RemoveInput(int ind)
{
    if (ind < 0 || (size_t)ind >= useActor.size())
    {
        EXCEPTION2(BadIndexException, ind, (int)useActor.size());
    }

    for (size_t i = 0 ; i < preparedDataset[ind].size() ; i++)
    {
        if (preparedDataset[ind][i] != NULL)
        {
            preparedDataset[ind][i]->Delete();
            preparedDataset[ind][i] = NULL;
        }
    }

    inputsOpacities[ind] = 0.0;

    useActor[ind] = false;
}


// ****************************************************************************
//  Method: avtTransparencyActor::TurnOffInput
//
//  Purpose:
//      Turns off the i'th input.  This means it will not render.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
// ****************************************************************************

void
avtTransparencyActor::TurnOffInput(int ind)
{
    if ((ind < 0) || ((size_t)ind >= useActor.size()))
    {
        EXCEPTION2(BadIndexException, ind, (int)useActor.size());
    }

    useActor[ind] = false;
}


// ****************************************************************************
//  Method: avtTransparencyActor::TurnOnInput
//
//  Purpose:
//      Turns on the i'th input.  This means it will render.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
// ****************************************************************************

void
avtTransparencyActor::TurnOnInput(int ind)
{
    if (ind < 0 || (size_t)ind >= useActor.size())
    {
        EXCEPTION2(BadIndexException, ind, (int)useActor.size());
    }

    useActor[ind] = true;
}


// ****************************************************************************
//  Method: avtTransparencyActor::SetVisibility
//
//  Purpose:
//      Sets the visibility of a certain actor.
//
//  Arguments:
//      ind     The index of the VTK constructs.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
// ****************************************************************************

void
avtTransparencyActor::SetVisibility(int ind, bool val)
{
    if (ind < 0 || (size_t)ind >= useActor.size())
    {
        EXCEPTION2(BadIndexException, ind, (int)useActor.size());
    }

    visibility[ind] = val;
}


// ****************************************************************************
//  Method: avtTransparencyActor::VisibilityOff
//
//  Purpose:
//      Tells the transparency actor not to render (this is used for navigate
//      bounding box mode).
//
//  Programmer: Hank Childs
//  Creation:   July 19, 2002
//
// ****************************************************************************

void
avtTransparencyActor::VisibilityOff(void)
{
    myActor->SetVisibility(0);
}


// ****************************************************************************
//  Method: avtTransparencyActor::VisibilityOn
//
//  Purpose:
//      Tells the transparency actor that it should render (this is used for 
//      navigate bounding box mode).
//
//  Programmer: Hank Childs
//  Creation:   July 19, 2002
//
//  Modifications:
//
//    Hank Childs, Wed May  7 13:50:00 PDT 2003
//    Only turn on the actor conditionally.  This will prevent an error msg.
//
//    Jeremy Meredith, Thu Oct 21 12:18:23 PDT 2004
//    Call TransparenciesExist now because the logic has become more complex.
//    Specifically, it may have needed an update, and it needed unification
//    in parallel.
//
// ****************************************************************************

void
avtTransparencyActor::VisibilityOn(void)
{
    if (TransparenciesExist())
        myActor->SetVisibility(1);
}



// ****************************************************************************
//  Method: avtTransparencyActor::ComputeCompositingOrder
//
//  Purpose:
//      Check if a compositing order can be determined. If so compute
//      the order of the ranks to correctly render translucent geometry
//      without doing a global parallel camera order sort. The ordering
//      is returned in a vector where ranks are sorterd furthest to
//      nearest from the camera. My approach verifies that each rank's
//      data is disjoint. If that is the case then the order can be
//      computed. This works for both structured and unstructured data.
//
//  Programmer: Burlen Loring
//  Creation:   Fri Aug 14 11:58:17 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
bool avtTransparencyActor::ComputeCompositingOrder(
    vtkCamera *cam, std::vector<int> &order)
{
#ifdef avtTransparencyActorDEBUG
    debug2 << "avtTransparencyActor::ComputeCompositingOrder" << endl;
#endif
    int rank = 0;
    int nranks = 1;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    MPI_Comm_size(VISIT_MPI_COMM, &nranks);
#endif

    // handle the serial case. you have the order but
    // it doesn't really matter as no compositing is
    // needed
    if (nranks < 2)
        return true;

    // the bounds of the data I have
    vector<double> buf(6*nranks, 0.0);
    appender->Update();
    vtkPolyData *data = appender->GetOutput();
    if (data->GetNumberOfCells())
        data->GetBounds(&buf[6*rank]);
    else
        Bounds::MakeEmpty(&buf[6*rank]);

    // share it with all other ranks
#ifdef PARALLEL
    MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
        &buf[0], 6, MPI_DOUBLE, VISIT_MPI_COMM);
#endif

    // get the camera position and view direction
    double pos[3];
    cam->GetPosition(pos);

    double foc[3];
    cam->GetFocalPoint(foc);

    double dir[3] = {pos[0] - foc[0], pos[1] - foc[1], pos[2] - foc[2]};
    double len = sqrt(dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]);
    dir[0] /= len;
    dir[1] /= len;
    dir[2] /= len;

    // compute the distance from the camera to/ bounds centroid
    // projected onto the view vector. drop empty bounds now,
    // we don't need those ranks to participate in compositing.
    vector<Bounds> bounds;
    bounds.reserve(nranks);
    for (int i = 0; i < nranks; ++i)
    {
        Bounds b(&buf[6*i], i);
        if (b.IsEmpty())
            continue; // drop it
        b.UpdateCentroidDistance(dir, pos);
        b.ShrinkByEpsInPlace();
        bounds.push_back(b);
    }
#ifdef avtTransparencyActorDEBUG
    debug2 << "bounds = {" << bounds << "}" << endl;
#endif

    // do any of the non-empty bounds intersect? if
    // not then we can compute the ordering.
    size_t nbounds = bounds.size();
    for (size_t j = 0; j < nbounds; ++j)
    {
        for (size_t i = 0; i < j; ++i)
        {
            if (!bounds[j].IntersectionIsEmpty(bounds[i]))
                return false;
        }
    }

    // sort the bounds according to distance from camera along
    // view dir. the first process in the list is the one
    // farthest away (ie descending order) and will be the root
    // of the ordered compositing tree.
    std::sort(bounds.begin(), bounds.end(), greater);

    // return the sorted ranks
    order.resize(nbounds);
    for (size_t i = 0; i < nbounds; ++i)
        order[i] =  bounds[i].GetId();

#ifdef avtTransparencyActorDEBUG
    debug2 << "order = {" << order << "}" << endl;
#endif

    return true;
}

// ****************************************************************************
//  Method: avtTransparencyActor::PrepareForRender
//
//  Purpose:
//      Gets ready for a render -- this means making sure that the poly data is
//      all set and that we are using the correct sort.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Allow for using a better sort in some instances.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six axis sorts.
//
//    Jeremy Meredith, Fri Jul 26 14:29:50 PDT 2002
//    No longer disable perfect sorting.  Our clients will disable it
//    when they determine it is okay.
//
//    Chris Wojtan, Fri Jun 25 16:13 PDT 2004
//    Ignore sorting and other unnecessary computation if the data is
//    2-dimensional
//
//    Chris Wojtan, Thurs Jul 8 16:18 PDT 2004
//    Force recalculation if parallel rendering is enabled
//
//    Jeremy Meredith, Thu Oct 21 15:05:29 PDT 2004
//    Use PAR_Size instead of an ifdef so we don't have to build a parallel
//    library.  Disable axis sort in parallel because it's only slowing us
//    down (never use it).
//
//    Hank Childs, Thu Mar  8 13:08:34 PST 2007
//    Make sure that the transparent geometry gets turned off when we render
//    something 2D.
//
//    Burlen Loring, Wed Aug 19 13:53:08 PDT 2015
//    Refactor some of the control logic. Added new sorting modes and
//    a mask to select amongs them. This enables us to use depth peeling
//    and/or ordered compositing mode.
//
// ****************************************************************************

void
avtTransparencyActor::PrepareForRender(vtkCamera *cam)
{
#ifdef avtTransparencyActorDEBUG
    debug2 << "avtTransparencyActor::PrepareForRender" << endl;
#endif
    // if this is a 2D plot, we don't need to sort anything
    if (is2Dimensional)
    {
        if (!TransparenciesExist())
            myActor->SetVisibility(0);
        return;
    }

    SetUpActor();

    // If we don't have anything to render, don't have our actor draw.
    if (!TransparenciesExist() || renderingSuspended)
    {
        myActor->SetVisibility(0);
    }
    else
    {
        myActor->SetVisibility(1);

        // do the selcted sort operation and pass the data into
        // the mapper. 
        if (sortOp == SORT_NONE)
        {
#ifdef avtTransparencyActorDEBUG
            debug2 << "skipping geometry sort" << endl;
#endif
            appender->Update();
            myMapper->SetInputData(appender->GetOutput());
        }
#ifdef PARALLEL
        else if ((sortOp & SORT_DISTRIBUTE) && (sortOp & SORT_DEPTH))
        {
#ifdef avtTransparencyActorDEBUG
            debug2 << "redistributed data followed by camera order sort" << endl;
#endif
            distributeDepthSort->SetCamera(cam);
            distributeDepthSort->Update();
            myMapper->SetInputData(distributeDepthSort->GetOutput());
        }
        else if (sortOp & SORT_DISTRIBUTE)
        {
#ifdef avtTransparencyActorDEBUG
            debug2 << "redistributed data" << endl;
#endif
            distribute->Update();
            myMapper->SetInputData(distribute->GetOutput());
        }
#endif
        else if (sortOp & SORT_DEPTH)
        {
            if (usePerfectSort)
            {
#ifdef avtTransparencyActorDEBUG
                debug2 << "local camera order sort" << endl;
#endif                
                depthSort->SetCamera(cam);
                depthSort->Update();
                myMapper->SetInputData(depthSort->GetOutput());
            }
            else
            {
#ifdef avtTransparencyActorDEBUG
                debug2 << "local axis order sort" << endl;
#endif
                axisSort->Update();
                if (CameraChanged(cam))
                    myMapper->SetInputData(GetAxisSortOutput(cam));
            }
        }
        else
        {
            EXCEPTION1(ImproperUseException, "invalid sortOp!");
        }
    }
}


// ****************************************************************************
//  Method: avtTransparencyActor::CameraChanged
//
//  Purpose:
//      Return true if the camera matrix has changed
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//      Burlen Loring, Wed Aug 19 13:38:39 PDT 2015
//      I factored this into its own function
//
// ****************************************************************************

bool
avtTransparencyActor::CameraChanged(vtkCamera *cam)
{
    vtkMatrix4x4 *mat = cam->GetViewTransformMatrix();
    bool equal = true;
    for (int i = 0 ; i < 16 ; i++)
    {
        if (mat->Element[i/4][i%4] != lastCamera->Element[i/4][i%4])
        {
           equal = false;
           vtkMatrix4x4 *mat = cam->GetViewTransformMatrix();
           lastCamera->DeepCopy(mat);
           break;
        }
    }
    return equal;
}

// ****************************************************************************
//  Method: avtTransparencyActor::GetAxisSortOutput
//
//  Purpose:
//      Get the correct output for the given camera orientation
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//      Burlen Loring, Wed Aug 19 13:38:39 PDT 2015
//      I factored this into its own function
//
// ****************************************************************************

vtkPolyData *
avtTransparencyActor::GetAxisSortOutput(vtkCamera *cam)
{
    // Based on what the direction of project is, set up the best
    // sorting.
    double proj[3];
    cam->GetDirectionOfProjection(proj);
    int biggest = 0;
    if (fabs(proj[biggest]) < fabs(proj[1]))
        biggest = 1;
    if (fabs(proj[biggest]) < fabs(proj[2]))
        biggest = 2;
    biggest += 1;
    if (proj[biggest-1] < 0.)
    {
        biggest *= -1;
    }
    switch (biggest)
    {
      case -3:
        return axisSort->GetMinusZOutput();
        break;
      case -2:
        return axisSort->GetMinusYOutput();
        break;
      case -1:
        return axisSort->GetMinusXOutput();
        break;
      case 1:
        return axisSort->GetPlusXOutput();
        break;
      case 2:
        return axisSort->GetPlusYOutput();
        break;
      case 3:
        return axisSort->GetPlusZOutput();
        break;
    }
    debug1 << "failed to determine which axis sort output to use!" << endl;
    return NULL;
}

// ****************************************************************************
//  Method: avtTransparencyActor::AddToRenderer
//
//  Purpose:
//      Adds the VTK actor for this AVT actor to the specified renderer.
//
//  Arguments:
//      ren     The renderer to add this actor to.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//    Chris Wojtan, Wed Jul 7 10:01 PDT 2004
//    Pass renderer into parallel transparency filter.
//
//    Jeremy Meredith, Thu Oct 21 15:08:16 PDT 2004
//    Use PAR_Size instead of an ifdef.
//
//    Burlen Loring, Wed Aug 19 13:16:09 PDT 2015
//    Go back to the ifdef to simplify logic elsewhere.
//    distributed sorting won't be requested if there's only
//    1 rank now.
//
// ****************************************************************************

void
avtTransparencyActor::AddToRenderer(vtkRenderer *ren)
{
    ren->AddActor(myActor);
#ifdef PARALLEL
    distribute->SetRenderer(ren);
#endif
}


// ****************************************************************************
//  Method: avtTransparencyActor::RemoveFromRenderer
//
//  Purpose:
//      Removes the VTK actor for this AVT actor from the specified renderer.
//
//  Arguments:
//      ren     The renderer to remove this actor from.
//
//  Programmer: Hank Childs
//  Creation:   July 9, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 15:08:39 PDT 2006
//    GetProps->RemoveItem has been deprecated, use GetViewProps.
//
// ****************************************************************************

void
avtTransparencyActor::RemoveFromRenderer(vtkRenderer *ren)
{
    //
    // This is supposed to approximate the RemoveActor call of
    // vtkRenderer.  That call also tells the actor to release its
    // graphics resources, which does not work well for us, since
    // we remove the actors every time we add new plots (the viewer
    // does a ClearPlots) and also when the vis window re-orders the
    // actors.
    //
    // THIS IS A MAINTENANCE ISSUE.  This routine should be the same
    // as vtkRenderer::RemoveActor, but does not call
    // ReleaseGraphicsResources (which is actually called indirectly
    // through vtkViewport::RemoveProp).
    //
    //ren->RemoveActor(actors[i]);
    //
    ren->GetActors()->RemoveItem(myActor);
    myActor->RemoveConsumer(ren);
    ren->GetViewProps()->RemoveItem(myActor);
}

// ****************************************************************************
//  Method: avtTransparencyActor::SyncProps
//
//  Purpose:
//      Synchronize properties across MPI ranks. Ranks that
//      initially have no data need to have rendering props
//      synchronized after depth sort.
//
//      This calls a worker method that does the communication
//      after first determining the representative source
//      from the list of input data.
//
//      This methods only needs to be called for the global
//      parallel redistribution of data. it has to be called
//      by all ranks.
//
//  Programmer: Burlen Loring
//  Creation:  Tue Sep  8 13:58:01 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void avtTransparencyActor::SyncProps()
{
    // the (local) representative actor, the one we take the props
    // from and apply to the appended dataset, is the the one from
    // the last non-empty input.
    vtkActor *repActor = NULL;
    size_t numActors = datasets.size();
    for (size_t i = 0 ; i < numActors ; ++i)
    {
        if (useActor[i] && visibility[i] == true)
        {
            size_t numParts = datasets[i].size();
            for (size_t j = 0; j < numParts; ++j)
            {
                if (preparedDataset[i][j])
                    repActor = actors[i][j];
            }
        }
    }
    if (TransparenciesExist() && !renderingSuspended)
    {
        vtkProperty *dest  = myActor->GetProperty();
        vtkProperty *source = repActor ? repActor->GetProperty() : NULL;
        SyncProps(dest, source);
    }
}

// ****************************************************************************
//  Method: avtTransparencyActor::SyncProps
//
//  Purpose:
//      Synchronize properties across MPI ranks. Ranks that
//      initially have no data need to have rendering props
//      synchronized after depth sort because they then will
//      have data to render.
//
//  Programmer: Burlen Loring
//  Creation:   Thu Jun 25 12:34:39 PDT 2015
//
//  Modifications:
//    Kathleen Biagas, Mon Sep 18 18:52:22 MST 2017
//    Changed curMTime from 'unsigned long long' to 'unsigned long', because
//    MPI_MAX is not defined for MPI_UNSIGNED_LONG_LONG in the MSMPI we use on
//    Windows, and the actual type returned by GetMTime is 'unsigned long'.
//
//    Eric Brugger, Thu Apr  4 08:26:51 PDT 2019
//    I corrected a memory overwrite where an MPI broadcast was being done
//    with 22 doubles but the buffer was only 20 doubles long. I changed
//    the broadcast to only send 20 doubles.
//
//    Kathleen Biagas, Tue Oct 29 12:14:42 PDT 2019
//    Added EdgeColor.
//
// ****************************************************************************

int avtTransparencyActor::SyncProps(vtkProperty *dest, vtkProperty *source)
{
#ifdef avtTransparencyActorDEBUG
    debug2 << "avtTransparencyActor::SyncProps " << dest << " " << source << endl;
#endif
    int rank = PAR_Rank();
    int size = PAR_Size();

    // skip a bunch of communication of props haven't changed
    unsigned long curMTime = source ? source->GetMTime() : 0;
#ifdef PARALLEL
    MPI_Allreduce(MPI_IN_PLACE, &curMTime, 1, MPI_UNSIGNED_LONG,
        MPI_MAX, VISIT_MPI_COMM);
#endif
#ifdef avtTransparencyActorDEBUG
    debug2 << "a mtime = " << actorMTime << " cur mtime = " << curMTime << endl;
#endif
    if (actorMTime && (curMTime <= actorMTime))
    {
#ifdef avtTransparencyActorDEBUG
        debug2 << "skiped sync!" << endl;
#endif
        return 0;
    }
    actorMTime = curMTime;

    // find the first rank that has valid props
    // he will be the source of the props for
    // the others
    vector<unsigned char> have_source(size, 0);
    have_source[rank] = source ? 1 : 0;

#ifdef PARALLEL
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_BYTE,
        &have_source[0], 1, MPI_BYTE, VISIT_MPI_COMM);
#endif

    int root = -1;
    int n_have = 0;
    for (int i = 0; i < size; ++i)
    {
        if (have_source[i])
        {
            // count the ranks with valid props. none or all
            // are special cases
            n_have += 1;

            // first rank who has valid props will serialize
            // and send
            if (root < 0)
                root = i;
        }
    }

    // none have valid props, nothing to do.
    if (n_have == 0)
        return -1;

    // serialize the props into a buffer
    double buf[23] = {0.0};
    if ((rank == root) || (n_have == size))
    {
        buf[0] = source->GetInterpolation();
        buf[1] = source->GetRepresentation();
        buf[2] = source->GetAmbient();
        buf[3] = source->GetDiffuse();
        buf[4] = source->GetSpecular();
        buf[5] = source->GetSpecularPower();
        double *c = source->GetAmbientColor();
        buf[6] = c[0];
        buf[7] = c[1];
        buf[8] = c[2];
        c = source->GetDiffuseColor();
        buf[9] = c[0];
        buf[10] = c[1];
        buf[11] = c[2];
        c = source->GetSpecularColor();
        buf[12] = c[0];
        buf[13] = c[1];
        buf[14] = c[2];
        buf[15] = source->GetEdgeVisibility();
        buf[16] = source->GetLineWidth();
        buf[17] = source->GetPointSize();
        buf[18] = source->GetBackfaceCulling();
        buf[19] = source->GetFrontfaceCulling();
        c = source->GetEdgeColor();
        buf[20] = c[0];
        buf[21] = c[1];
        buf[22] = c[2];
    }

#ifdef PARALLEL
    // if any processes don't have valid props send
    if (n_have != size)
        MPI_Bcast(buf, 23, MPI_DOUBLE, root, VISIT_MPI_COMM);
#endif

    // deserialize
    dest->SetInterpolation(buf[0]);
    dest->SetRepresentation(buf[1]);
    // If we copy over lighting, odd things happen.  This is in the
    // system as HYPer4112.
    //dest->SetAmbient(buf[2]);
    //dest->SetDiffuse(buf[3]);
    dest->SetSpecular(buf[4]);
    dest->SetSpecularPower(buf[5]);
    dest->SetAmbientColor(&buf[6]);
    dest->SetDiffuseColor(&buf[9]);
    dest->SetSpecularColor(&buf[12]);
    dest->SetEdgeVisibility(buf[15]);
    dest->SetLineWidth(buf[16]);
    dest->SetPointSize(buf[17]);
    dest->SetBackfaceCulling(buf[18]);
    dest->SetFrontfaceCulling(buf[19]);
    dest->SetEdgeColor(&buf[20]);

    return 0;
}

// ****************************************************************************
//  Method: avtTransparency::SetUpActor
//
//  Purpose:
//      This is the big routine to pull all the poly data together into one
//      poly data that can then be sorted.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jul 11 16:07:53 PDT 2002
//    Incorporate the idea of visibility.  Also remove actor from the scene if
//    it has nothing to draw.
//
//    Hank Childs, Sat Jul 13 13:26:10 PDT 2002
//    Do a better job of copying over the input actors' attributes.
//
//    Hank Childs, Sun Jul 14 15:49:58 PDT 2002
//    Use new VTK module to do all six axis sorts.
//
//    Jeremy Meredith, Thu Oct 21 15:11:19 PDT 2004
//    Force a re-execution of the appender so all processors do the
//    same thing in parallel.  Call TransparenciesExist now because
//    the logic has become more complex.  Specifically, it may have
//    needed an update, and it needed unification in parallel.  Honor
//    the suspension of transparent rendering for two-pass mode.
//
//    Kathleen Bonnell, Wed May 17 15:08:39 PDT 2006
//    Ensure that appender has non-NULL input (can have isempty input, not NULL).
//
//    Tom Fogal, Thu Aug 14 14:14:54 EDT 2008
//    Match size types.
//
//    Burlen Loring, Wed Aug 19 13:19:05 PDT 2015
//    Move the sorting actions back into PrepareForRender so that
//    they are all in one spot.
//
//    Burlen Loring, Tue Sep  8 13:06:25 PDT 2015
//    Don't recalculate unecessarily in parallel. specifically remove the
//    PAR_Size > 1 condition while keeping the appender update.
//
//    Kathleen Biagas, Tue Jul 12 13:45:40 MST 2016
//    Added early return if there are no datasets.
//
// ****************************************************************************

void
avtTransparencyActor::SetUpActor(void)
{
    if(datasets.empty())
        return;

#ifdef avtTransparencyActorDEBUG
    debug2 << "avtTransparencyActor::SetUpActor" << endl;
#endif

    // Determine if our poly-data input is up-to-date.
    bool needToRecalculate = inputModified
        || (useActor.size() != lastExecutionActorList.size())
        || (actorMTime == 0);

    size_t n = useActor.size();
    for (size_t i = 0; (i < n) && !needToRecalculate; ++i)
    {
        if (useActor[i] != lastExecutionActorList[i])
            needToRecalculate = true;
    }

    if (needToRecalculate)
    {
        // Maintain our internal state for next time.
        lastExecutionActorList = useActor;

        appender->RemoveAllInputs();
        size_t numActors = datasets.size();
        //vtkActor *repActor = NULL;
        bool addedInput = false;
        for (size_t i = 0 ; i < numActors ; ++i)
        {
            if (useActor[i] && visibility[i] == true)
            {
                size_t numParts = datasets[i].size();
                for (size_t j = 0 ; j < numParts ; j++)
                {
                    PrepareDataset(i, j);
                    if (preparedDataset[i][j] != NULL)
                    {
                        addedInput = true;
                        appender->AddInputData(preparedDataset[i][j]);
                    }
                }
            }
        }
    
        //  VTK pipeline requires filters to have non-null inputs.
        if (!addedInput)
        {
            vtkPolyData *pd = vtkPolyData::New();
            appender->AddInputData(pd);
            pd->Delete();
        }

        if (actorMTime == 0)
            SyncProps();
    }
#ifdef avtTransparencyActorDEBUG
    else
    {
        debug2 << "using cached setup!" << endl;
    }
#endif

    // Force the appender to update; this is needed in parallel SR mode
    // because all processors need to re-execute the pipeline, and if
    // not all processors have data then they might not re-execute.
    // See VisIt00005467.
    appender->Modified();
}


// ****************************************************************************
//  Method: avtTransparencyActor::PrepareDataset
//
//  Purpose:
//      This routine will take a dataset potentially convert to polydata
//      compute normals and map colors onto it.  This will allow it to be
//      merged later.
//
//  Arguments:
//      input       The input index.
//      subinput    The piece of the input.
//
//  Programmer:   Hank Childs
//  Creation:     July 8, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Nov 20 14:29:21 PST 2002
//    Re-order setting of normals.  They were getting removed by subsequent
//    calls.
//
//    Hank Childs, Thu May  6 08:37:25 PDT 2004
//    Do a better job of handling normals for cell-based normals.  This is more
//    important because the poly data mapper no longer calculates them for us.
//
//    Hank Childs, Fri Dec 29 09:53:13 PST 2006
//    Accomodate situations where rectilinear and curvilinear grids are
//    getting shipped down as is (i.e. not poly data).  This means we must
//    add normals and remove ghost data.
//
//    Hank Childs, Mon Feb 12 14:02:16 PST 2007
//    Make sure that the colors that get generated are directly related
//    to the poly data we are rendering.  Some mappers are linked to 
//    vtkRectilinearGrids or vtkStructuredGrids.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    All rectilinear grids that made it here get converted to polydata now --
//    the difference with these is that they need an extra transform applied.
//    These should all be 2D grids (i.e. external polygons) anyway, so the
//    performance penalty is small.
//
//    Brad Whitlock, Tue Aug 16 16:12:44 PDT 2011
//    Change to new SetInputConnection, GetOutputPort style. Also disable
//    color texturing on the mapper prior to calling MapScalars since when the
//    mapper is doing color texturing, it will not return a colors array from
//    MapScalars.
//
//    Kathleen Biagas, Fri Jul 27 10:44:24 PDT 2012
//    Only create filters if needed.  Break upstream vtk pipeline by setting
//    in_ds source to NULL. (Fixes crash/no data for rgrids on Windows).
//
//    Burlen Loring, Sat Sep 12 08:55:19 PDT 2015
//    Eliminate an unessary memcpy of scalar colors. Fix leak of vtk transform
//
//    Kathleen Biagas, Tue Jul 12 13:46:14 MST 2016
//    Retrieved and preserve stored actor's visibility state.
//
// ****************************************************************************

void
avtTransparencyActor::PrepareDataset(size_t input, size_t subinput)
{
#ifdef avtTransparencyActorDEBUG
    debug2 << "avtTransparencyActor::PrepareDataset" << endl;
#endif

    inputModified = false;

    vtkDataSet       *in_ds  = datasets[input][subinput];
    vtkActor         *actor  = actors[input][subinput];
    vtkDataSetMapper *mapper = mappers[input][subinput];
    int               avis   = actorVis[input][subinput];
    //
    // If we don't have valid input, there isn't a lot we can do.
    //
    if (in_ds == NULL || actor == NULL || mapper == NULL)
    {
        preparedDataset[input][subinput] = NULL; // should be already, but...
        return;
    }

    //
    // Check to see if we have already done all the necessary calculations.
    //
    vtkDataSet *ds = preparedDataset[input][subinput];
    if (ds != NULL && in_ds->GetMTime() < ds->GetMTime() &&
        actor->GetMTime() < ds->GetMTime() &&
        mapper->GetMTime() < ds->GetMTime())
    {
        //
        // Our last preparation of this dataset is still good.  No need to redo
        // the same work twice.
        //
#ifdef avtTransparencyActorDEBUG
        debug2 << "preparation of this dataset is still good!" << endl;
#endif
        return;
    }

    //
    // If we have made it this far, we need to recalculate.  Go ahead and clean
    // up from any previous executions.
    //
    if (preparedDataset[input][subinput] != NULL)
    {
        preparedDataset[input][subinput]->Delete();
        preparedDataset[input][subinput] = NULL;
    }

    //
    // If this actor is fully opaque, then we are not needed.
    //
    if (actor->GetProperty()->GetOpacity() >= 1.)
    {
        actor->SetVisibility(avis);
        return;
    }
    else
    {
        //
        // We will be drawing this, so turn theirs off.
        //
        actor->SetVisibility(0);
    }

    //
    // If the actor is fully transparent, there's no need for us to process it.
    //
    if (actor->GetProperty()->GetOpacity() <= 0. || avis == 0)
    {
        return;
    }

    //
    // Okay, the real recalculation work starts here.  Start by confirming
    // that we are actually dealing with polydata.
    //
    vtkGeometryFilter *gf = NULL;
    vtkDataSetRemoveGhostCells *ghost_filter = NULL;
    vtkVisItPolyDataNormals *normals = NULL;
    vtkTransformFilter *xform_filter = NULL;
    vtkTransform *xform = NULL;
    vtkPolyData *pd = NULL;

    // break upstream vtk pipeline
    //in_ds->SetSource(NULL);
    if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        pd = (vtkPolyData *) in_ds;
    }
    else if (in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        gf = vtkGeometryFilter::New();
        gf->SetInputData(in_ds);
        ghost_filter = vtkDataSetRemoveGhostCells::New();
        ghost_filter->SetInputConnection(gf->GetOutputPort());
        ghost_filter->Update();
        pd = (vtkPolyData *) ghost_filter->GetOutput();
    }
    else if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        gf = vtkGeometryFilter::New();
        gf->SetInputData(in_ds);
        normals = vtkVisItPolyDataNormals::New();
        if (mapper->GetScalarVisibility() != 0 &&
            in_ds->GetPointData()->GetScalars() == NULL &&
            in_ds->GetCellData()->GetScalars() != NULL)
        {
            normals->SetNormalTypeToCell();
        }
        normals->SetInputConnection(gf->GetOutputPort());
        ghost_filter = vtkDataSetRemoveGhostCells::New();
        ghost_filter->SetInputConnection(normals->GetOutputPort());
        // Apply any inherent rectilinear grid transforms from the input.
        if (in_ds->GetFieldData()->GetArray("RectilinearGridTransform"))
        {
            vtkDoubleArray *matrix = (vtkDoubleArray*)in_ds->GetFieldData()->
                                          GetArray("RectilinearGridTransform");
            xform = vtkTransform::New();
            xform->SetMatrix(matrix->GetPointer(0));
            xform_filter = vtkTransformFilter::New();
            xform_filter->SetInputConnection(ghost_filter->GetOutputPort());
            xform_filter->SetTransform(xform);
            xform_filter->Update();
            pd = (vtkPolyData *) xform_filter->GetOutput();
        }
        else
        {
            ghost_filter->Update();
            pd = (vtkPolyData *) ghost_filter->GetOutput();
        }
    }
    else
    {
        gf = vtkGeometryFilter::New();
        gf->SetInputData(in_ds);
        gf->Update();
        pd = gf->GetOutput();
    }
    mapper->SetInputData(pd);

    //
    // Create the output dataset that we will be creating an RGBA field for.
    //
    vtkPolyData *prepDS = vtkPolyData::New();
    if (mapper->GetScalarVisibility() == 0)
    {
        // The color and the opacity of the whole actor will be applied to
        // every triangle of this poly data.
        prepDS->CopyStructure(pd);
        int npts = prepDS->GetNumberOfPoints();
        vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
        colors->SetNumberOfComponents(4);
        colors->SetNumberOfTuples(npts);
        colors->SetName("Colors");
        double *color   = actor->GetProperty()->GetColor();
        double  opacity = actor->GetProperty()->GetOpacity();
        unsigned char rgba[4];
        rgba[0] = (unsigned char) (color[0] * 255.0);
        rgba[1] = (unsigned char) (color[1] * 255.0);
        rgba[2] = (unsigned char) (color[2] * 255.0);
        rgba[3] = (unsigned char) (opacity * 255.0);
        unsigned char *ptr = colors->GetPointer(0);
        for (int i = 0 ; i < npts ; ++i)
        {
            ptr[4*i]   = rgba[0];
            ptr[4*i+1] = rgba[1];
            ptr[4*i+2] = rgba[2];
            ptr[4*i+3] = rgba[3];
        }
        prepDS->GetPointData()->AddArray(colors);
        colors->Delete();
        if (pd->GetPointData()->GetNormals() != NULL)
        {
            prepDS->GetPointData()->SetNormals(
                pd->GetPointData()->GetNormals());
        }
    }
    else
    {
        if (pd->GetPointData()->GetScalars())
        {
            // Prepare our own buffer to store the colors in.
            prepDS->CopyStructure(pd);

            // Disable interpolate scalars
            int interpolateScalars = mapper->GetInterpolateScalarsBeforeMapping();
            if (interpolateScalars > 0)
                mapper->InterpolateScalarsBeforeMappingOff();

            // Now let the mapper create the buffer of unsigned chars that it
            // would have created if we were to let it do the actual mapping.
            // note: VTK no longer uses this api internally. and there is some
            // logic (based on OpenGL features) in vtk that would make this
            // incorrect with translucent geometry. in practice it doesn't occur.
            // but the divergence from what we are doing compared to what VTK is
            // doing is something to be aware of.
            double opacity = actor->GetProperty()->GetOpacity();
            vtkUnsignedCharArray *colors = mapper->MapScalars(opacity);

            colors->SetName("Colors");
            prepDS->GetPointData()->AddArray(colors);

            if (pd->GetPointData()->GetNormals())
                prepDS->GetPointData()->SetNormals(
                    pd->GetPointData()->GetNormals());

            // Restore interpolate scalars
            if(interpolateScalars > 0)
                mapper->InterpolateScalarsBeforeMappingOn();
        }
        else if (pd->GetCellData()->GetScalars())
        {
            //
            // This is a sad state -- we have decided that the poly data will
            // all have point data.  But we have cell data.  So we are going to
            // dummy up a dataset that has point data, but appears to have cell
            // data (by replicating a lot of points).
            //

            //
            // Start off by replicating the connectivity.  This will speculate
            // on a point list that we will build later.
            //
            prepDS->Allocate(pd);
            vtkIdType ncells = pd->GetNumberOfCells();
            vtkIdType  *cellPts = NULL;
            vtkIdType   myCellPts[100];
            vtkIdType   npts = 0;
            vector<vtkIdType> ptIds;
            vector<vtkIdType> cellIds;
            pd->BuildCells();
            vtkIdType count = 0;
            for (vtkIdType i = 0 ; i < ncells ; i++)
            {
                pd->GetCellPoints(i, npts, cellPts);
                if (cellPts == NULL || npts == 0)
                {
                    continue;
                }
                for (vtkIdType j = 0 ; j < npts ; j++)
                {
                    ptIds.push_back(cellPts[j]);
                    cellIds.push_back(i);
                    myCellPts[j] = count;
                    count++;
                }

                prepDS->InsertNextCell(pd->GetCellType(i), npts, myCellPts);
            }

            //
            // Create the point list.  This will have a lot of repeated points.
            //
            vtkPoints *in_pts = pd->GetPoints();
            vtkPoints *pts = vtkPoints::New(in_pts->GetDataType());
            pts->SetNumberOfPoints(count);
            for (vtkIdType i = 0 ; i < count ; i++)
            {
                double pt[3];
                in_pts->GetPoint(ptIds[i], pt);
                pts->SetPoint(i, pt);
            }
            prepDS->SetPoints(pts);
            pts->Delete();

            //
            // Now convert our cell data into point data that will appear as
            // cell data (through lots of replication of points).
            //
            vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
            colors->SetNumberOfComponents(4);
            colors->SetNumberOfTuples(count);
            colors->SetName("Colors");
            unsigned char *ptr = (unsigned char *) colors->GetVoidPointer(0);
            double opacity = actor->GetProperty()->GetOpacity();
            unsigned char *buff = 
              (unsigned char *) mapper->MapScalars(opacity)->GetVoidPointer(0);
            for (vtkIdType i = 0 ; i < count ; i++)
            {
                ptr[4*i]   = buff[4*cellIds[i]];
                ptr[4*i+1] = buff[4*cellIds[i]+1];
                ptr[4*i+2] = buff[4*cellIds[i]+2];
                ptr[4*i+3] = buff[4*cellIds[i]+3];
            }
            prepDS->GetPointData()->AddArray(colors);
            colors->Delete();
            vtkDataArray *cell_normals = pd->GetCellData()->GetNormals();
            if (cell_normals != NULL)
            {
                vtkDataArray *newNormals = cell_normals->NewInstance();
                newNormals->SetNumberOfComponents(3);
                newNormals->SetNumberOfTuples(count);
                newNormals->SetName("Normals");
                if(cell_normals->GetDataType() == VTK_FLOAT)
                {
                    const float *cn = (float *) cell_normals->GetVoidPointer(0);
                    float *newNormalPtr = (float*)newNormals->GetVoidPointer(0);
                    for (vtkIdType i = 0 ; i < count ; i++)
                    {
                        *newNormalPtr++ = cn[cellIds[i]*3];
                        *newNormalPtr++ = cn[cellIds[i]*3+1];
                        *newNormalPtr++ = cn[cellIds[i]*3+2];
                    }
                }
                else if(cell_normals->GetDataType() == VTK_DOUBLE)
                {
                    double *cn = (double *) cell_normals->GetVoidPointer(0);
                    double *nn = (double*)newNormals->GetVoidPointer(0);
                    for (vtkIdType i = 0 ; i < count ; i++)
                    {
                        *nn++ = cn[cellIds[i]*3];
                        *nn++ = cn[cellIds[i]*3+1];
                        *nn++ = cn[cellIds[i]*3+2];
                    }
                }
                else
                {
                    for (vtkIdType i = 0 ; i < count ; i++)
                        newNormals->SetTuple(i, cell_normals->GetTuple(cellIds[i]));
                }
                prepDS->GetPointData()->SetNormals(newNormals);
                newNormals->Delete();
            }
        }
    }

    //
    // Now that we are done preparing the dataset, let's store it off so we
    // can save the results.
    //
    preparedDataset[input][subinput] = prepDS;
    preparedDataset[input][subinput]->Register(NULL);

    //
    // Clean up memory.
    //
    prepDS->Delete();
    if (gf)
        gf->Delete();
    if (ghost_filter)
        ghost_filter->Delete();
    if (normals)
        normals->Delete();
    if (xform_filter)
        xform_filter->Delete();
    if (xform)
        xform->Delete();
}


// ****************************************************************************
//  Method: avtTransparencyActor::ScaleByVector
//
//  Purpose:
//      Scales the actor by a vector. 
//
//  Arguments:
//      vec    The vector to scale by.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 16, 2003 
//
// ****************************************************************************

void
avtTransparencyActor::ScaleByVector(const double vec[3])
{
    myActor->SetScale(vec[0], vec[1], vec[2]);
}


// ****************************************************************************
//  Method: avtTransparencyActor::SetSpecularProperties
//
//  Purpose:
//      Sets the specular properties.
//
//  Programmer: Hank Childs
//  Creation:   February 17, 2010
//
// ****************************************************************************

void
avtTransparencyActor::SetSpecularProperties(bool flag,double coeff,double power,
                                            const ColorAttribute &color)
{
    vtkProperty *prop = myActor->GetProperty();
    if(prop != NULL && prop->GetRepresentation() == VTK_SURFACE)
    {
        prop->SetSpecular(flag ? coeff : 0);
        prop->SetSpecularPower(power);
        int r = color.Red();
        int g = color.Green();
        int b = color.Blue();
        prop->SetSpecularColor(double(r)/255.,
                               double(g)/255.,
                               double(b)/255.);
    }
}


// ****************************************************************************
//  Method: avtTransparencyActor::TransparenciesExist
//
//  Purpose:
//    Returns true if this actor is active (appender has inputs).
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 3, 2003
//
//  Modifications:
//
//    Tom Fogal, Mon May 25 14:04:56 MDT 2009
//    Changed interface; this method checks the memo'd cache.
//
//    Burlen Loring, Sat Sep 12 08:55:19 PDT 2015
//    SyncProps here so that we only sync once per frame
//
// ****************************************************************************

bool
avtTransparencyActor::TransparenciesExist()
{
    if(!cachedTransparencies)
    {
        DetermineTransparencies();

        // since we are already doing communication here, and this only
        // occurs once per frame, this is as good a place as any to ensure
        // that the props are in sync TODO -- this is needed when some ranks
        // don't have data, there should be a way to pass down the props
        // and eliminate the communication.
        SyncProps();
    }

    return transparenciesExist;
}

// ****************************************************************************
//  Method: avtTransparencyActor::DetermineTransparencies
//
//  Purpose:
//    Returns true if this actor is active (appender has inputs).
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 3, 2003
//
//  Modifications:
//    Jeremy Meredith and Hank Childs, Thu Oct 21 15:27:44 PDT 2004
//    This method was not getting updated soon enough for calls to
//    it to return the correct value.  We took the key pieces
//    from PrepareDataset to get the right answer here, and I
//    added code to enforce that all processors have the same answer.
//
//    Tom Fogal, Mon May 25 14:06:37 MDT 2009
//    Renamed from `TransparenciesExist'.  Use member variable to store the
//    result, and note that the result is cached.
//
// ****************************************************************************

void
avtTransparencyActor::DetermineTransparencies()
{
#ifdef avtTransparencyActorDEBUG
    debug2 << "avtTransparencyActor::DetermineTransparencies" << endl;
#endif
    transparenciesExist = false;
    size_t numActors = datasets.size();
    for (size_t i = 0 ; i < numActors && !transparenciesExist; i++)
    {
        if (useActor[i] && visibility[i] == true)
        {
            size_t numParts = datasets[i].size();
            for (size_t j = 0 ; j < numParts && !transparenciesExist; j++)
            {
                vtkDataSet       *in_ds  = datasets[i][j];
                vtkActor         *actor  = actors[i][j];
                vtkDataSetMapper *mapper = mappers[i][j];
                if (in_ds && actor && mapper &&
                    actor->GetProperty()->GetOpacity() > 0. &&
                    actor->GetProperty()->GetOpacity() < 1.)
                {
                    transparenciesExist = true;
                }
            }
        }
    }

    // We need all processors to agree!
    transparenciesExist = UnifyMaximumValue(transparenciesExist);

    // Note that we've memo'ized this value.
    cachedTransparencies = true;
}


// ****************************************************************************
//  Method: avtTransparencyActor::TransparenciesMightExist
//
//  Purpose:
//    Returns true if there might be some transparency on some processor
//
//  Programmer: Mark C. Miller 
//  Creation:   January 20, 2005
//
//  Modifications:
//
//    Burlen Loring, Fri Oct 16 12:20:18 PDT 2015
//    Don't use pre increment on stl iterators, move function calls out of
//    for.
//
// ****************************************************************************

bool
avtTransparencyActor::TransparenciesMightExist() const
{
    std::map<int,double>::const_iterator it = inputsOpacities.begin();
    std::map<int,double>::const_iterator end = inputsOpacities.end();

    bool has_transparency = false;
    for (; it != end; ++it)
    {
        if ((it->second > 0.0) && (it->second < 1.0))
        {
            has_transparency = true;
            break;
        }
    }

    return has_transparency;
}

// ****************************************************************************
//  Method: avtTransparencyActor::SetIs2Dimensional
//
//  Purpose:
//      Sets whether or not the actor is 2D.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2004
//
// ****************************************************************************

void
avtTransparencyActor::SetIs2Dimensional(bool val)
{
    if (is2Dimensional != val)
    {
        //
        // If we have any inputs left over, remove them.  This is important,
        // because the 2D execution flow will bail out before removing them,
        // which can potentially leave old 3D datasets in the same window
        // as the 2D dataset.
        //
        if (appender != NULL)
        {
            appender->RemoveAllInputs();

            // Note: with lineouts in SR mode, it can sometimes hang
            // because one processor decides to re-execute despite having
            // no inputs.  Let's just force what we think should be
            // happening anyway, and force all processors to re-execute
            // so they all go through the distribute.  This is similar
            // to VisIt00005467.
            appender->Modified();
        }
    }

    is2Dimensional = val;
}

// ****************************************************************************
//  Method: avtTransparencyActor::ReplaceActorVisibility
//
//  Purpose:
//    Repaces the stored actor's visibility state for the given
//    index.   Used when a mapper changes visibility state of its actor.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 12, 2016
//
// ****************************************************************************

void
avtTransparencyActor::ReplaceActorVisibility(int ind, vector<int> &v)
{
    if (ind < 0 || (size_t)ind >= datasets.size())
    {
        EXCEPTION2(BadIndexException, ind, (int)datasets.size());
    }

    actorVis[ind] = v;

    inputModified = true;
}
