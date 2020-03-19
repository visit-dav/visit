// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtXRayFilter.C                             //
// ************************************************************************* //

#include <avtXRayFilter.h>

#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif 

#include <algorithm>

#include <vtkAppendPolyData.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <avtAccessor.h>
#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>

#include <Utility.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>

#include <vector>

using std::vector;
#define avtXRayFilter_GetCellPointsTypeMacro(ptype, n) \
{ \
    ptype *pts = static_cast<ptype *>(points->GetVoidPointer(0)); \
    for (int i = 0; i < 3; ++i) \
    { \
        p0[i]=pts[ids[0]*3+i]; \
        p1[i]=pts[ids[1]*3+i]; \
        p2[i]=pts[ids[2]*3+i]; \
        p3[i]=pts[ids[3]*3+i]; \
        if (n > 4) \
            p4[i]=pts[ids[4]*3+i]; \
        if (n > 5) \
            p5[i]=pts[ids[5]*3+i]; \
        if (n > 6) \
            p6[i]=pts[ids[6]*3+i]; \
        if (n > 7) \
            p7[i]=pts[ids[7]*3+i]; \
    } \
}

#define avtXRayFilter_GetCellPointsMacro(n) \
{ \
    if (points->GetDataType() == VTK_FLOAT) \
    { \
        avtXRayFilter_GetCellPointsTypeMacro(float, n); \
    } \
    else  if (points->GetDataType() == VTK_DOUBLE) \
    { \
        avtXRayFilter_GetCellPointsTypeMacro(double, n); \
    } \
    else \
    { \
        points->GetPoint(ids[0], p0); \
        points->GetPoint(ids[1], p1); \
        points->GetPoint(ids[2], p2); \
        points->GetPoint(ids[3], p3); \
        if (n > 4) \
            points->GetPoint(ids[4], p4); \
        if (n > 5) \
            points->GetPoint(ids[5], p5); \
        if (n > 6) \
            points->GetPoint(ids[6], p6); \
        if (n > 7) \
            points->GetPoint(ids[7], p7); \
    } \
}

static int IntersectLineWithRevolvedSegment(const double *line_pt,
                                            const double *, const double *, 
                                            const double *, double *);


inline double Dot(const double v1[3], const double v2[3])
{
    return (v1[0] * v2[0]) + (v1[1] * v2[1]) +  (v1[2] * v2[2]);
}

inline void Cross(double result[3], const double v1[3], const double v2[3])
{
    result[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    result[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    result[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

// ------------------------------------------------------------------------- //
#ifdef PARALLEL
// ------------------------------------------------------------------------- //
// these helpers are only needed for MPI case
// ------------------------------------------------------------------------- //
// ****************************************************************************
//  Method: VisIt_XRay_MPI_Alltoallv + VisIt_XRay_MPI_Gatherv
//
//  Purpose:
//    Wrappers for Alltoallv and Gatherv that we can use safely in 
//    avtXRayFilter's templated double vs float calling scenarios.
//
//  Programmer: Cyrus Harrison + Matt Larsen
//  Creation:   Tue Apr 16 16:24:28 PDT 2019
//
//  Modifications:
//
// ****************************************************************************

//-----------------------------
// Templated MPI_Alltoallv 
//-----------------------------
// method def
template <typename T>
int VisIt_XRay_MPI_Alltoallv(T *sendbuf, int *sendcounts,
                             int *sdispls, T *recvbuf,
                             int *recvcounts, int *rdispls,
                             MPI_Comm comm);

// case specifically for floats
template<>
int VisIt_XRay_MPI_Alltoallv<float>(float *sendbuf, int *sendcounts,
                                    int *sdispls, float *recvbuf,
                                    int *recvcounts, int *rdispls,
                                    MPI_Comm comm)
{
    return MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_FLOAT,
                         recvbuf, recvcounts, rdispls, MPI_FLOAT,
                         comm);
}

// case specifically for double
template<>
int VisIt_XRay_MPI_Alltoallv<double>(double *sendbuf, int *sendcounts,
                                     int *sdispls, double *recvbuf,
                                     int *recvcounts, int *rdispls,
                                     MPI_Comm comm)
{
    return MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_DOUBLE,
                         recvbuf, recvcounts, rdispls, MPI_DOUBLE,
                         comm);
}

//-----------------------------
// Templated MPI_Gatherv
//-----------------------------
template <typename T>
int VisIt_XRay_MPI_Gatherv(T *sendbuf, int sendcnt,
                           T *recvbuf, int *recvcnts, int *displs,
                           int root,
                           MPI_Comm comm);

// case specifically for floats
template<>
int VisIt_XRay_MPI_Gatherv<float>(float *sendbuf, int sendcnt,
                                  float *recvbuf, int *recvcnts, int *displs,
                                  int root,
                                  MPI_Comm comm)
{
    return MPI_Gatherv(sendbuf, sendcnt, MPI_FLOAT,
                       recvbuf, recvcnts, displs, MPI_FLOAT,
                       root, comm);
}

// case specifically for doubles
template<>
int VisIt_XRay_MPI_Gatherv<double>(double *sendbuf, int sendcnt,
                                   double *recvbuf, int *recvcnts, int *displs,
                                   int root,
                                   MPI_Comm comm)
{
    return MPI_Gatherv(sendbuf, sendcnt, MPI_DOUBLE,
                       recvbuf, recvcnts, displs, MPI_DOUBLE,
                       root, comm);
}

// ------------------------------------------------------------------------- //
// endif -- these helpers are only needed for MPI case
// ------------------------------------------------------------------------- //
#endif
// ------------------------------------------------------------------------- //

// ****************************************************************************
//  Method: IntersectLineWithTri
//
//  Purpose:
//    Intersect a line with a triangle, returning the intersection point.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Thu Nov 29 16:44:27 PST 2012
//    I changed the value of eps so that the test that rejects triangles that
//    are parallel to the line works better with small triangles.
//
// ****************************************************************************

static bool IntersectLineWithTri(const double v0[3], const double v1[3],
    const double v2[3], const double origin[3],
    const double direction[3], double& t)
{
    static const double eps = 1e-12;

    //
    // Reject rays that are parallel to Q, and rays that intersect the
    // plane of Q either on the left of the line V00V01 or on the right
    // of the line V00V10.
    //

    double E_01[3], E_02[3];
    E_01[0] = v1[0] - v0[0];
    E_01[1] = v1[1] - v0[1];
    E_01[2] = v1[2] - v0[2];
    E_02[0] = v2[0] - v0[0];
    E_02[1] = v2[1] - v0[1];
    E_02[2] = v2[2] - v0[2];
    double P[3];
    Cross(P, direction, E_02);
    double det = Dot(E_01, P);
    if (fabs(det) < eps) return false;
    double inv_det = 1.0 / det;
    double T[3];
    T[0] = origin[0] - v0[0];
    T[1] = origin[1] - v0[1];
    T[2] = origin[2] - v0[2];
    double alpha = Dot(T, P) * inv_det;
    if (alpha < 0.0) return false;
    double Q[3];
    Cross(Q, T, E_01);
    double beta = Dot(direction, Q) * inv_det;
    if (beta < 0.0) return false; 

    if ((alpha + beta) > 1.0)
    {
        //
        // Rejects rays that intersect the plane of Q either on the
        // left of the line V11V10 or on the right of the line V11V01.
        //

        double E_20[3], E_21[3];
        E_20[0] = v0[0] - v2[0];
        E_20[1] = v0[1] - v2[1];
        E_20[2] = v0[2] - v2[2];
        E_21[0] = v1[0] - v2[0];
        E_21[1] = v1[1] - v2[1];
        E_21[2] = v1[2] - v2[2];
        double P_prime[3];
        Cross(P_prime, direction, E_21);
        double det_prime = Dot(E_20, P_prime);
        if (fabs(det_prime) < eps) return false;
        double inv_det_prime = double(1.0) / det_prime;
        double T_prime[3];
        T_prime[0] = origin[0] - v2[0];
        T_prime[1] = origin[1] - v2[1];
        T_prime[2] = origin[2] - v2[2];
        double alpha_prime = Dot(T_prime, P_prime) * inv_det_prime;
        if (alpha_prime < double(0.0)) return false;
    }

    //
    // Compute the ray parameter of the intersection point, and
    // reject the ray if it does not hit Q.
    //

    t = Dot(E_02, Q) * inv_det;
    if (t < 0.0) return false; 

    return true;
}


// ****************************************************************************
//  Method: IntersectLineWithQuad
//
//  Purpose:
//    Intersect a line with a quad, returning the intersection point.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Thu Nov 29 16:44:27 PST 2012
//    I replaced the code with two calls to the intersect with triangle
//    function, since the intersect with quad code had problems with lines
//    being nearly planar with non-planar quads.
//
// ****************************************************************************

static bool IntersectLineWithQuad(const double v_00[3], const double v_10[3],
    const double v_11[3], const double v_01[3], const double origin[3],
    const double direction[3], double& t)
{
    if (IntersectLineWithTri(v_00, v_10, v_01, origin, direction, t))
        return true;
    else if (IntersectLineWithTri(v_10, v_01, v_11, origin, direction, t))
        return true;
    else
        return false;
}


// ****************************************************************************
//  Method: avtXRayFilter constructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Mon Dec  6 12:37:40 PST 2010
//    I modified the view information stored internally to correspond more
//    closely to an avtView3D structure instead of having it match the
//    parameters to SetImageProperty.
//
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Eric Brugger, Mon Dec  3 13:41:10 PST 2012
//    I added the ability to output the cells intersected by a specified
//    ray to a vtk file.
//
//    Gunther H. Weber, Wed Jan 23 15:23:14 PST 2013
//    Add support for specifying background intensity.
//
//    Eric Brugger, Thu Jan 15 11:02:10 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Wed May 27 10:10:28 PDT 2015
//    I modified the filter to also output the path length field.
//
//    Eric Brugger, Thu Jun  4 15:58:10 PDT 2015
//    I added an option to enable outputting the ray bounds to a vtk file.
//
// ****************************************************************************

avtXRayFilter::avtXRayFilter()
{
    lines  = NULL;

    normal[0] = 0;
    normal[1] = 0;
    normal[2] = 1;
    focus[0] = 0;
    focus[1] = 0;
    focus[2] = 0;
    viewUp[0] = 0;
    viewUp[1] = 1;
    viewUp[2] = 0;
    viewAngle = 30;
    parallelScale = 0.5;
    nearPlane = -0.5;
    farPlane = 0.5;
    imagePan[0] = 0;
    imagePan[1] = 0;
    imageZoom = 1;
    perspective = false;
    imageSize[0] = 200;
    imageSize[1] = 200;

    divideEmisByAbsorb = false;

    numPixels = imageSize[0] * imageSize[1];
    numPixelsPerIteration = 4000;

    backgroundIntensity = 0.0;
    backgroundIntensities = NULL;
    nBackgroundIntensities = 0;
    intensityBins = NULL;
    pathBins = NULL;
    numBins = 1;

    debugRay = -1;
    outputRayBounds = false;
}


// ****************************************************************************
//  Method: avtXRayFilter destructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Thu Jan 15 11:02:10 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Wed May 27 10:10:28 PDT 2015
//    I modified the filter to also output the path length field.
//
// ****************************************************************************

avtXRayFilter::~avtXRayFilter()
{
    if (lines != NULL)
        delete [] lines;
    if (backgroundIntensities != NULL)
        delete [] backgroundIntensities;
    if (intensityBins != NULL)
        delete [] intensityBins;
    if (pathBins != NULL)
        delete [] pathBins;
}


// ****************************************************************************
//  Method:  avtXRayFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Set up the atttributes and validity for the output of the filter.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Aug 27 11:15:48 PDT 2010
//    I removed the requirement that a 2d spatial mesh must be an RZ mesh,
//    and had it assume that it was.
//  
// ****************************************************************************

void
avtXRayFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();

    if (inAtts.GetSpatialDimension() == 2)
        outAtts.SetSpatialDimension(3);

    outAtts.SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Method: avtXRayFilter::SetImageProperties
//
//  Purpose:
//    Set the x ray image properties.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Mon Dec  6 12:37:40 PST 2010
//    I modified the view information stored internally to correspond more
//    closely to an avtView3D structure instead of having it match the
//    parameters to SetImageProperty.
//
//    Kathleen Biagas, Wed Oct 17 14:08:40 PDT 2012
//    Added 'up' argument.  Allow it to be NULL, to preserve old behavior
//    of avtXRayImageQuery, which did not have an 'up_vector' argument.
//
//    Eric Brugger, Wed Nov 19 15:48:18 PST 2014
//    I modified the arguments so that they map one for one with the actual
//    image properties stored in the class.
//
// ****************************************************************************

void
avtXRayFilter::SetImageProperties(double *_normal, double *_focus, 
    double *_viewUp, double _viewAngle, double _parallelScale,
    double _nearPlane, double _farPlane, double *_imagePan,
    double _imageZoom, bool _perspective, int *_imageSize)
{
    normal[0]     = _normal[0];
    normal[1]     = _normal[1];
    normal[2]     = _normal[2];
    focus[0]      = _focus[0];
    focus[1]      = _focus[1];
    focus[2]      = _focus[2];
    viewUp[0]     = _viewUp[0];
    viewUp[1]     = _viewUp[1];
    viewUp[2]     = _viewUp[2];
    viewAngle     = _viewAngle;
    parallelScale = _parallelScale;
    nearPlane     = _nearPlane;
    farPlane      = _farPlane;
    imagePan[0]   = _imagePan[0];
    imagePan[1]   = _imagePan[1];
    imageZoom     = _imageZoom;
    perspective   = _perspective;
    imageSize[0]  = _imageSize[0];
    imageSize[1]  = _imageSize[1];

    numPixels = imageSize[0] * imageSize[1];
}


// ****************************************************************************
//  Method: avtXRayFilter::SetDivideEmisByAbsorb
//
//  Purpose:
//    Set the flag that controls if the emissivity divided by the absorbtivity
//    is used in place of the emissivity.
//
//  Programmer: Eric Brugger
//  Creation:   December 28, 2010
//
// ****************************************************************************

void
avtXRayFilter::SetDivideEmisByAbsorb(bool flag)
{
    divideEmisByAbsorb = flag;
}

// ****************************************************************************
//  Method: avtXRayFilter::SetBackgroundIntensity
//
//  Purpose:
//    Set the background intensity entering the volume
//
//  Programmer: Gunther H. Weber
//  Creation:   January 23, 2013
//
// ****************************************************************************

void
avtXRayFilter::SetBackgroundIntensity(double intensity)
{
    backgroundIntensity = intensity;
}

// ****************************************************************************
//  Method: avtXRayFilter::SetBackgroundIntensities
//
//  Purpose:
//    Set the background intensities entering the volume, one per bin.
//
//  Programmer: Eric Brugger
//  Creation:   January 15, 2015
//
// ****************************************************************************

void
avtXRayFilter::SetBackgroundIntensities(double *intensities, int nIntensities)
{
    if (backgroundIntensities != NULL)
        delete [] backgroundIntensities;

    backgroundIntensities = new double[nIntensities];
    memcpy(backgroundIntensities, intensities, nIntensities*sizeof(double));
    nBackgroundIntensities = nIntensities;
}

// ****************************************************************************
//  Method: avtXRayFilter::SetDebugRay
//
//  Purpose:
//    Set the id for the debug ray.
//
//  Programmer: Eric Brugger
//  Creation:   May 21, 2015
//
// ****************************************************************************

void
avtXRayFilter::SetDebugRay(int ray)
{
    debugRay = ray;
}

// ****************************************************************************
//  Method: avtXRayFilter::SetOutputRayBounds
//
//  Purpose:
//    Set the output ray bounds flag.
//
//  Programmer: Eric Brugger
//  Creation:   June 4, 2015
//
// ****************************************************************************

void
avtXRayFilter::SetOutputRayBounds(bool flag)
{
    outputRayBounds = flag;
}

// ****************************************************************************
//  Method: avtXRayFilter::Execute
//
//  Purpose:
//    Processes all the domains.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Jul 16 15:42:20 PDT 2010
//    I modified the filter to handle the case where some of the processors
//    didn't have any data sets when executing in parallel.
//
//    Eric Brugger, Fri Aug 27 11:15:48 PDT 2010
//    I removed the requirement that a 2d spatial mesh must be an RZ mesh,
//    and had it assume that it was.
//
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Eric Brugger, Fri Mar 18 14:13:56 PDT 2011
//    I corrected a bug where the filter would crash when running in parallel
//    and the number of pixels processed in a pass was divisible by the
//    number of processors.
//
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Moved some code to ImageStripExecute, call a templatized version of the
//    method for double-precision support.
//
//    Eric Brugger, Wed Jul 18 13:05:27 PDT 2012
//    I corrected a bug where the filter would crash when running in parallel
//    for some combinations of processor count and image size.
//
//    Eric Brugger, Mon Dec  3 13:41:10 PST 2012
//    I added the ability to output the cells intersected by a specified
//    ray to a vtk file.
//
//    Eric Brugger, Wed May 27 10:10:28 PDT 2015
//    I modified the filter to also output the path length field.
//
// ****************************************************************************

void
avtXRayFilter::Execute(void)
{
    //
    // Process the pixels in multiple iterations.
    //
    actualPixelsPerIteration = (numPixelsPerIteration / imageSize[0]) *
        imageSize[0];

    pixelsForFirstPass = actualPixelsPerIteration;
    pixelsForLastPass = ((numPixels % actualPixelsPerIteration) == 0) ?
        actualPixelsPerIteration : numPixels % actualPixelsPerIteration;

    pixelsForFirstPassFirstProc = pixelsForFirstPass / PAR_Size();
    while (PAR_Size() > 1 && (pixelsForFirstPassFirstProc + 1) * (PAR_Size() - 1) < pixelsForFirstPass)
        pixelsForFirstPassFirstProc++;
    pixelsForFirstPassLastProc =
        ((pixelsForFirstPass % pixelsForFirstPassFirstProc) == 0) ?
        pixelsForFirstPassFirstProc :
        pixelsForFirstPass - (pixelsForFirstPassFirstProc * (PAR_Size() - 1));
    pixelsForLastPassFirstProc = pixelsForLastPass / PAR_Size();
    while (PAR_Size() > 1 && (pixelsForLastPassFirstProc + 1) * (PAR_Size() - 1) < pixelsForLastPass)
        pixelsForLastPassFirstProc++;
    pixelsForLastPassLastProc =
        ((pixelsForLastPass % pixelsForLastPassFirstProc) == 0) ?
        pixelsForLastPassFirstProc :
        pixelsForLastPass - (pixelsForLastPassFirstProc * (PAR_Size() - 1));

    numPasses = numPixels / actualPixelsPerIteration;
    if (numPixels % actualPixelsPerIteration != 0)
        numPasses++;

    iFragment = 0;
    nImageFragments = numPasses;
    imageFragmentSizes = new int[nImageFragments];
    intensityFragments = new vtkDataArray *[nImageFragments];
    pathLengthFragments = new vtkDataArray *[nImageFragments];

    //
    // Get the input data tree to obtain the data sets.
    //
    avtDataTree_p tree = GetInputDataTree();

    //
    // Get the data sets.
    //
    vtkDataSet **dataSets = tree->GetAllLeaves(totalNodes);

    //
    // Check that the data sets are valid.
    //
    CheckDataSets(totalNodes, dataSets);

    lineOffset = 0;
    for (iPass = 0; iPass < numPasses; iPass++)
    {
        int pixelsForThisPass = (iPass == numPasses - 1) ?
            pixelsForLastPass : pixelsForFirstPass;
        int pixelsForThisPassFirstProc = (iPass == numPasses - 1) ?
            pixelsForLastPassFirstProc : pixelsForFirstPassFirstProc;

        linesForThisPass = pixelsForThisPass;
        linesForThisPassFirstProc = pixelsForThisPassFirstProc;

        int pixelsForThisProc;
        if (PAR_Rank() < PAR_Size() - 1)
        {
            if (iPass < numPasses - 1)
                pixelsForThisProc = pixelsForFirstPassFirstProc;
            else
                pixelsForThisProc = pixelsForLastPassFirstProc;
        }
        else
        {
            if (iPass < numPasses - 1)
                pixelsForThisProc = pixelsForFirstPassLastProc;
            else
                pixelsForThisProc = pixelsForLastPassLastProc;
        }

        imageFragmentSizes[iPass] = pixelsForThisProc;

        if (cellDataType == VTK_FLOAT)
            ImageStripExecute<float>(totalNodes, dataSets);
        else if (cellDataType == VTK_DOUBLE)
            ImageStripExecute<double>(totalNodes, dataSets);
        else
            {
                // TODO: else error
            }

        int extraMsg = 100;
        int totalProg = numPasses * extraMsg;
        UpdateProgress(extraMsg*iPass, totalProg);

        lineOffset += pixelsForThisPass;
    }

    //
    // Collect all the fragments on the root processor.
    //
    int t1;
    if (PAR_Size() > 1)
    {
        //
        // Collect the images.
        //
        t1 = visitTimer->StartTimer();
        vtkDataArray *intensity = NULL;
        vtkDataArray *pathLength = NULL;
        if (cellDataType == VTK_DOUBLE)
        {
            CollectFragments<double>(0, nImageFragments, imageFragmentSizes,
                                     intensityFragments, intensity);
            CollectFragments<double>(0, nImageFragments, imageFragmentSizes,
                                     pathLengthFragments, pathLength);
        }
        else
        {
            CollectFragments<float>(0, nImageFragments, imageFragmentSizes,
                                    intensityFragments, intensity);
            CollectFragments<float>(0, nImageFragments, imageFragmentSizes,
                                    pathLengthFragments, pathLength);
        }
        visitTimer->StopTimer(t1, "avtXRayImageQuery::CollectFragments");

        //
        // Swap out the current fragments and replace them with the
        // unified ones.
        //
        for (int i = 0; i < nImageFragments; i++)
        {
            intensityFragments[i]->Delete();
            pathLengthFragments[i]->Delete();
        }
        if (PAR_Rank() == 0)
            nImageFragments = 1;
        else
            nImageFragments = 0;
        imageFragmentSizes[0] = numPixels;
        intensityFragments[0] = intensity;
        pathLengthFragments[0] = pathLength;
    }

    //
    // Merge all the fragments together.
    //
    if (PAR_Rank() == 0)
    {
        vtkDataSet **pdarray = new vtkDataSet*[2*numBins];
        int        *indarray = new int[2*numBins];

        //
        // Add the intensities.
        //
        for (int iBin = 0; iBin < numBins; iBin++)
        {
            vtkDataArray *intensityArray;
            MergeFragments(iBin, intensityFragments, intensityArray);
            intensityArray->SetName("Intensity");
            vtkDataSet *outDataSet = vtkPolyData::New();
            outDataSet->GetPointData()->AddArray(intensityArray);
            outDataSet->GetPointData()->CopyFieldOn("Intensity");

            pdarray[iBin] = outDataSet;
            indarray[iBin] = iBin;
        }

        //
        // Add the path lengths.
        //
        for (int iBin = 0; iBin < numBins; iBin++)
        {
            vtkDataArray *pathArray;
            MergeFragments(iBin, pathLengthFragments, pathArray);
            pathArray->SetName("PathLength");
            vtkDataSet *outDataSet = vtkPolyData::New();
            outDataSet->GetPointData()->AddArray(pathArray);
            outDataSet->GetPointData()->CopyFieldOn("PathLength");

            pdarray[numBins+iBin] = outDataSet;
            indarray[numBins+iBin] = numBins+iBin;
        }

        //
        // Create an avtDataTree to return the results in.
        //
        avtDataTree_p newtree = new avtDataTree(2*numBins, pdarray, indarray);
        SetOutputDataTree(newtree);
        for (int iBin = 0; iBin < 2*numBins; iBin++)
        {
            pdarray[iBin]->Delete();
        }

        delete [] pdarray;
        delete [] indarray;
    }
    else
    {
        avtDataTree_p newtree = new avtDataTree(NULL, -1);
        SetOutputDataTree(newtree);
    }

    //
    // Clean up temporary arrays.
    //
    for (int i = 0; i < nImageFragments; i++)
    {
        intensityFragments[i]->Delete();
        pathLengthFragments[i]->Delete();
    }
    delete [] imageFragmentSizes;
    delete [] intensityFragments;
    delete [] pathLengthFragments;
    intensityFragments = NULL;
    pathLengthFragments = NULL;

    // Free the memory from the GetAllLeaves function call.
    delete [] dataSets;
}
   

// ****************************************************************************
//  Method:  avtXRayFilter::ImageStripExecute
//
//  Purpose:
//    Processes a strip of the image.
//
//  Programmer: Eric Brugger
//  Creation:   December 28, 2010
//
//  Modifications:
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Templatized this method, moved a bit of code out of Execute to here,
//    for double-precision support.
//
//    Eric Brugger, Mon Dec  3 13:41:10 PST 2012
//    I added the ability to output the cells intersected by a specified
//    ray to a vtk file.
//
// ****************************************************************************

template <typename T>
void
avtXRayFilter::ImageStripExecute(int nDataSets, vtkDataSet **dataSets)
{
    //
    // Calculate the lines for this image strip.
    //
    CalculateLines();

    //
    // Intersect the data sets with the lines.
    //
    int *nLinesPerDataset = new int[nDataSets];
    vector<double> *dists = new vector<double>[nDataSets];
    vector<int> *lineIds = new vector<int>[nDataSets];
    T ***cellData = new T**[nDataSets];

    int t1 = visitTimer->StartTimer();
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        for (currentNode = 0; currentNode < nDataSets; currentNode++)
            CylindricalExecute(dataSets[currentNode],
                nLinesPerDataset[currentNode], dists[currentNode],
                lineIds[currentNode], cellData[currentNode]);
        visitTimer->StopTimer(t1, "avtXRayFilter::CylindricalExecute");
    }
    else
    {
        for (currentNode = 0; currentNode < nDataSets; currentNode++)
            CartesianExecute(dataSets[currentNode],
                nLinesPerDataset[currentNode], dists[currentNode],
                lineIds[currentNode], cellData[currentNode]);
        visitTimer->StopTimer(t1, "avtXRayFilter::CartesianExecute");
    }

    //
    // Redistribute the line segments to processors that own them.
    //
    t1 = visitTimer->StartTimer();

    int nComponentsPerCellArray = 0;
    if (nDataSets > 0)
    {
        nComponentsPerCellArray = dataSets[0]->GetCellData()->
            GetArray(absVarName.c_str())->GetNumberOfComponents();
    }
  
    int nPts;
    int *outLineIds;
    double *outDists;
    T **outCellData;
    RedistributeLines(nDataSets, nLinesPerDataset, dists, lineIds,
        nComponentsPerCellArray, cellData, nPts, outLineIds, outDists,
        outCellData);

    int pixelOffset = (iPass == (numPasses - 1)) ?
        PAR_Rank() * pixelsForLastPassFirstProc :
        PAR_Rank() * pixelsForFirstPassFirstProc;

    IntegrateLines(pixelOffset, nPts, outLineIds, outDists,
        outCellData[0], outCellData[1]);

    visitTimer->StopTimer(t1, "avtXRayFilter::RedistributeLines");

    //
    // Clean up temporary arrays.
    //
    delete [] nLinesPerDataset;
    delete [] dists;
    delete [] lineIds;
    for (int i = 0; i < nDataSets; i++)
    {
        T **cellDataI = cellData[i];
        for (int j = 0; j < 2; j++)
        {
            T *vals = cellDataI[j];
            delete [] vals;
        }
        delete [] cellData[i];
    }
    delete [] cellData;

    delete [] outDists;
    delete [] outLineIds;
    delete [] outCellData[0];
    delete [] outCellData[1];
    delete [] outCellData;
}


// ****************************************************************************
//  Method: avtXRayFilter::PreExecute
//
//  Purpose:
//    This is called before all of the domains are executed.  This defines
//    the lines that will be intersected with the cells.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Aug 13 10:16:33 PDT 2010
//    I corrected a bug where the lines for the pixels were not defined
//    properly for most cases.
//
//    Eric Brugger, Fri Aug 27 11:15:48 PDT 2010
//    I removed the requirement that a 2d spatial mesh must be an RZ mesh,
//    and had it assume that it was.
//
//    Eric Brugger, Mon Dec  6 12:37:40 PST 2010
//    I modified the view information stored internally to correspond more
//    closely to an avtView3D structure instead of having it match the
//    parameters to SetImageProperty.
//
//    Eric Brugger, Tue Dec 21 15:54:04 PST 2010
//    I moved the code that defines the lines into the method CalculateLines.
//
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
// ****************************************************************************

void
avtXRayFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() != AVT_RZ)
        {
            avtCallback::IssueWarning("Encountered a 2D mesh that was not an "
                "RZ mesh, assuming it is an RZ mesh.");
        }
    }
}


// ****************************************************************************
//  Method: avtXRayFilter::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
// ****************************************************************************

void
avtXRayFilter::PostExecute(void)
{
    avtDatasetToDatasetFilter::PostExecute();
}


// ****************************************************************************
//  Method: avtXRayFilter::CartesianExecute
//
//  Purpose:
//    Finds line intersections in cartesian space.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Mon Jul 12 13:37:46 PDT 2010
//    I added more timing calls to get finer grained timing information.
//  
//    Eric Brugger, Fri Jul 16 15:42:20 PDT 2010
//    I modified the filter to handle the case where some of the processors
//    didn't have any data sets when executing in parallel.
//
//    Eric Brugger, Thu Jul 29 14:21:50 PDT 2010
//    I put in optimizations for tets, pyramids, wedges, and hexes in
//    unstructured grids.
//
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Templatized this method, for double-precision support.  Added macros for
//    retrieving cell points based on data type. (Fast paths for float/double).
//
//    Eric Brugger, Fri May 11 16:33:04 PDT 2012
//    I added logic to handle rectilinear meshes.
//
//    Eric Brugger, Mon Dec  3 13:41:10 PST 2012
//    I added the ability to output the cells intersected by a specified
//    ray to a vtk file.
//
//    Gunther H. Weber, Wed Jan 23 15:18:27 PST 2013
//    Add skipping ghost cells for rectilinear and structured grids.
//
// ****************************************************************************

template <typename T>
void
avtXRayFilter::CartesianExecute(vtkDataSet *ds, int &nLinesPerDataset,
    vector<double> &dist, vector<int> &line_id, T **&cellData)
{
    int  i, j;

    //
    // Create an interval tree for the data set to find the intersections
    // of the lines with the data set.
    //
    int t1 = visitTimer->StartTimer();
    int nCells = ds->GetNumberOfCells();
    if (nCells == 0)
    {
        nLinesPerDataset = 0;

        int nCellArrays = ds->GetCellData()->GetNumberOfArrays();
        cellData = new T*[nCellArrays];
        for (int i = 0; i < nCellArrays; i++)
            cellData[i] = new T[0];

        return;
    }
    int dims   = 3;
    avtIntervalTree tree(nCells, dims);
    double bounds[6];
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    bool hasGhost = (ghosts != NULL);
    for (i = 0 ; i < nCells ; i++)
    {
        ds->GetCellBounds(i, bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);
    visitTimer->StopTimer(t1, "avtXRayFilter::CreateIntervalTree");

    t1 = visitTimer->StartTimer();
    //
    // Loop over the lines.
    //
    vector<int> cells_matched;
    double p0[3]={0., 0., 0.};
    double p1[3]={0., 0., 0.};
    double p2[3]={0., 0., 0.};
    double p3[3]={0., 0., 0.};
    double p4[3]={0., 0., 0.};
    double p5[3]={0., 0., 0.};
    double p6[3]={0., 0., 0.};
    double p7[3]={0., 0., 0.};

    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
        vtkDataArray *xCoords = rgrid->GetXCoordinates();
        vtkDataArray *yCoords = rgrid->GetYCoordinates();
        vtkDataArray *zCoords = rgrid->GetZCoordinates();

        int ndims[3];
        rgrid->GetDimensions(ndims);

        int zdims[3];
        zdims[0] = ndims[0] - 1;
        zdims[1] = ndims[1] - 1;
        zdims[2] = ndims[2] - 1;

        int nx = zdims[0];
        int ny = zdims[1];
        int nxy = nx * ny;

        for (i = 0 ; i < linesForThisPass ; i++)
        {
            double pt1[3];
            pt1[0] = lines[6*i];
            pt1[1] = lines[6*i+2];
            pt1[2] = lines[6*i+4];
            double pt2[3];
            pt2[0] = lines[6*i+1];
            pt2[1] = lines[6*i+3];
            pt2[2] = lines[6*i+5];
            double dir[3];
            dir[0] = pt2[0] - pt1[0];
            dir[1] = pt2[1] - pt1[1];
            dir[2] = pt2[2] - pt1[2];

            vector<int> list;
            tree.GetElementsList(pt1, dir, list);
            int nCells = (int)list.size();
            if (nCells == 0)
                continue;  // No intersection

            double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                     (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                     (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));

            for (j = 0 ; j < nCells ; j++)
            {
                //
                // Determine the index into the look up table.
                //
                int iCell = list[j];
                if (hasGhost && ghosts->GetTuple1(iCell) != 0.)
                    continue;

                int iZ = iCell / nxy;
                int iXY = iCell % nxy;
                int iY = iXY / nx;
                int iX = iXY % nx;

                if (xCoords->GetDataType() == VTK_FLOAT)
                {
                    float *xpts = static_cast<float*>(xCoords->GetVoidPointer(0)); \
                    float *ypts = static_cast<float*>(yCoords->GetVoidPointer(0)); \
                    float *zpts = static_cast<float*>(zCoords->GetVoidPointer(0)); \
                    p0[0] = xpts[iX];
                    p0[1] = ypts[iY];
                    p0[2] = zpts[iZ];
                    p1[0] = xpts[iX+1];
                    p1[1] = ypts[iY];
                    p1[2] = zpts[iZ];
                    p2[0] = xpts[iX+1];
                    p2[1] = ypts[iY+1];
                    p2[2] = zpts[iZ];
                    p3[0] = xpts[iX];
                    p3[1] = ypts[iY+1];
                    p3[2] = zpts[iZ];
                    p4[0] = xpts[iX];
                    p4[1] = ypts[iY];
                    p4[2] = zpts[iZ+1];
                    p5[0] = xpts[iX+1];
                    p5[1] = ypts[iY];
                    p5[2] = zpts[iZ+1];
                    p6[0] = xpts[iX+1];
                    p6[1] = ypts[iY+1];
                    p6[2] = zpts[iZ+1];
                    p7[0] = xpts[iX];
                    p7[1] = ypts[iY+1];
                    p7[2] = zpts[iZ+1];
                }
                else if (xCoords->GetDataType() == VTK_DOUBLE)
                {
                    double *xpts = static_cast<double*>(xCoords->GetVoidPointer(0)); \
                    double *ypts = static_cast<double*>(yCoords->GetVoidPointer(0)); \
                    double *zpts = static_cast<double*>(zCoords->GetVoidPointer(0)); \
                    p0[0] = xpts[iX];
                    p0[1] = ypts[iY];
                    p0[2] = zpts[iZ];
                    p1[0] = xpts[iX+1];
                    p1[1] = ypts[iY];
                    p1[2] = zpts[iZ];
                    p2[0] = xpts[iX+1];
                    p2[1] = ypts[iY+1];
                    p2[2] = zpts[iZ];
                    p3[0] = xpts[iX];
                    p3[1] = ypts[iY+1];
                    p3[2] = zpts[iZ];
                    p4[0] = xpts[iX];
                    p4[1] = ypts[iY];
                    p4[2] = zpts[iZ+1];
                    p5[0] = xpts[iX+1];
                    p5[1] = ypts[iY];
                    p5[2] = zpts[iZ+1];
                    p6[0] = xpts[iX+1];
                    p6[1] = ypts[iY+1];
                    p6[2] = zpts[iZ+1];
                    p7[0] = xpts[iX];
                    p7[1] = ypts[iY+1];
                    p7[2] = zpts[iZ+1];
                }
                else
                {
                    p0[0] = xCoords->GetTuple1(iX);
                    p0[1] = yCoords->GetTuple1(iY);
                    p0[2] = zCoords->GetTuple1(iZ);
                    p1[0] = xCoords->GetTuple1(iX+1);
                    p1[1] = yCoords->GetTuple1(iY);
                    p1[2] = zCoords->GetTuple1(iZ);
                    p2[0] = xCoords->GetTuple1(iX+1);
                    p2[1] = yCoords->GetTuple1(iY+1);
                    p2[2] = zCoords->GetTuple1(iZ);
                    p3[0] = xCoords->GetTuple1(iX);
                    p3[1] = yCoords->GetTuple1(iY+1);
                    p3[2] = zCoords->GetTuple1(iZ);
                    p4[0] = xCoords->GetTuple1(iX);
                    p4[1] = yCoords->GetTuple1(iY);
                    p4[2] = zCoords->GetTuple1(iZ+1);
                    p5[0] = xCoords->GetTuple1(iX+1);
                    p5[1] = yCoords->GetTuple1(iY);
                    p5[2] = zCoords->GetTuple1(iZ+1);
                    p6[0] = xCoords->GetTuple1(iX+1);
                    p6[1] = yCoords->GetTuple1(iY+1);
                    p6[2] = zCoords->GetTuple1(iZ+1);
                    p7[0] = xCoords->GetTuple1(iX);
                    p7[1] = yCoords->GetTuple1(iY+1);
                    p7[2] = zCoords->GetTuple1(iZ+1);
                }

                double t;
                int nInter = 0;
                double inter[6];

                if (IntersectLineWithQuad(p0, p1, p2, p3, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p4, p7, p6, p5, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p0, p4, p5, p1, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p1, p5, p6, p2, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p2, p6, p7, p3, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p0, p3, p7, p4, pt1, dir, t))
                    inter[nInter++] = t;

                if (nInter == 2)
                {
                    cells_matched.push_back(iCell);
                    dist.push_back(inter[0]*lineLength);
                    dist.push_back(inter[1]*lineLength);
                    line_id.push_back(i);
                }
            }
        }
    }
    else if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) ds;
        vtkPoints *points = sgrid->GetPoints();

        int ndims[3];
        sgrid->GetDimensions(ndims);

        int zdims[3];
        zdims[0] = ndims[0] - 1;
        zdims[1] = ndims[1] - 1;
        zdims[2] = ndims[2] - 1;

        int nx = ndims[0];
        int ny = ndims[1];
        int nxy = nx * ny;

        int nx2 = zdims[0];
        int ny2 = zdims[1];
        int nxy2 = nx2 * ny2;

        for (i = 0 ; i < linesForThisPass ; i++)
        {
            double pt1[3];
            pt1[0] = lines[6*i];
            pt1[1] = lines[6*i+2];
            pt1[2] = lines[6*i+4];
            double pt2[3];
            pt2[0] = lines[6*i+1];
            pt2[1] = lines[6*i+3];
            pt2[2] = lines[6*i+5];
            double dir[3];
            dir[0] = pt2[0] - pt1[0];
            dir[1] = pt2[1] - pt1[1];
            dir[2] = pt2[2] - pt1[2];

            vector<int> list;
            tree.GetElementsList(pt1, dir, list);
            int nCells = (int)list.size();
            if (nCells == 0)
                continue;  // No intersection

            double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                     (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                     (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));

            for (j = 0 ; j < nCells ; j++)
            {
                //
                // Determine the index into the look up table.
                //
                int iCell = list[j];
                if (hasGhost && ghosts->GetTuple1(iCell) != 0.)
                    continue;

                int iZ = iCell / nxy2;
                int iXY = iCell % nxy2;
                int iY = iXY / nx2;
                int iX = iXY % nx2;
                int idx = iX+ iY*nx + iZ*nxy;

                int ids[8];
                ids[0] = idx;
                ids[1] = idx + 1;
                ids[2] = idx + 1 + nx;
                ids[3] = idx + nx;
                idx += nxy;
                ids[4] = idx;
                ids[5] = idx + 1;
                ids[6] = idx + 1 + nx;
                ids[7] = idx + nx;

                avtXRayFilter_GetCellPointsMacro(8); 

                double t;
                int nInter = 0;
                double inter[6];

                if (IntersectLineWithQuad(p0, p1, p2, p3, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p4, p7, p6, p5, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p0, p4, p5, p1, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p1, p5, p6, p2, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p2, p6, p7, p3, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p0, p3, p7, p4, pt1, dir, t))
                    inter[nInter++] = t;

                if (nInter == 2)
                {
                    cells_matched.push_back(iCell);
                    dist.push_back(inter[0]*lineLength);
                    dist.push_back(inter[1]*lineLength);
                    line_id.push_back(i);
                }
            }
        }
    }
    else if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) ds;
        vtkPoints *points = ugrid->GetPoints();

        vtkUnsignedCharArray *cellTypes = ugrid->GetCellTypesArray();
        vtkIdTypeArray *cellLocations = ugrid->GetCellLocationsArray();
        vtkCellArray *cells = ugrid->GetCells();

        vtkIdType *nl = cells->GetPointer();
        unsigned char *ct = cellTypes->GetPointer(0);
        vtkIdType *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < linesForThisPass ; i++)
        {
            double pt1[3];
            pt1[0] = lines[6*i];
            pt1[1] = lines[6*i+2];
            pt1[2] = lines[6*i+4];
            double pt2[3];
            pt2[0] = lines[6*i+1];
            pt2[1] = lines[6*i+3];
            pt2[2] = lines[6*i+5];
            double dir[3];
            dir[0] = pt2[0] - pt1[0];
            dir[1] = pt2[1] - pt1[1];
            dir[2] = pt2[2] - pt1[2];

            vector<int> list;
            tree.GetElementsList(pt1, dir, list);
            int nCells = (int)list.size();
            if (nCells == 0)
                continue;  // No intersection

            double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                     (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                     (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));

            for (j = 0 ; j < nCells ; j++)
            {
                //
                // Determine the index into the look up table.
                //
                int iCell = list[j];
                if (hasGhost && ghosts->GetTuple1(iCell) != 0.)
                    continue;

                double t;
                int nInter = 0;
                double inter[100];

                vtkIdType *ids = &(nl[cl[iCell]+1]);
                if (ct[iCell] == VTK_HEXAHEDRON)
                {
                    avtXRayFilter_GetCellPointsMacro(8); 

                    if (IntersectLineWithQuad(p0, p1, p2, p3, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p4, p7, p6, p5, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p0, p4, p5, p1, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p1, p5, p6, p2, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p2, p6, p7, p3, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p0, p3, p7, p4, pt1, dir, t))
                        inter[nInter++] = t;
                }
                else if (ct[iCell] == VTK_WEDGE)
                {
                    avtXRayFilter_GetCellPointsMacro(6); 

                    if (IntersectLineWithTri(p0, p1, p2, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p3, p5, p4, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p0, p3, p4, p1, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p1, p4, p5, p2, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithQuad(p2, p5, p3, p0, pt1, dir, t))
                        inter[nInter++] = t;
                }
                else if (ct[iCell] == VTK_PYRAMID)
                {
                    avtXRayFilter_GetCellPointsMacro(5); 

                    if (IntersectLineWithQuad(p0, p1, p2, p3, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p0, p4, p1, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p1, p4, p2, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p2, p4, p3, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p3, p4, p0, pt1, dir, t))
                        inter[nInter++] = t;
                }
                else if (ct[iCell] == VTK_TETRA)
                {
                    avtXRayFilter_GetCellPointsMacro(4); 

                    if (IntersectLineWithTri(p0, p1, p2, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p0, p3, p1, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p2, p3, p0, pt1, dir, t))
                        inter[nInter++] = t;
                    if (IntersectLineWithTri(p1, p3, p2, pt1, dir, t))
                        inter[nInter++] = t;
                }
                else
                {
                    vtkCell *cell = ds->GetCell(iCell);

                    if (cell->GetCellDimension() == 3)
                    {
                        int nFaces = cell->GetNumberOfFaces();
                        for (int k = 0 ; k < nFaces ; k++)
                        {
                            vtkCell *face = cell->GetFace(k);
                            double x[3];
                            double pcoords[3];
                            double t;
                            int subId;
                            if (face->IntersectWithLine(pt1, pt2, 1e-10, t,
                                                        x, pcoords, subId))
                                inter[nInter++] = t;
                        }
                    }
                    else if (cell->GetCellDimension() == 2)
                    {
                        int nEdges = cell->GetNumberOfEdges();
                        for (int k = 0 ; k < nEdges ; k++)
                        {
                            vtkCell *edge = cell->GetEdge(k);
                            double x[3];
                            double pcoords[3];
                            double t;
                            int subId;
                            if (edge->IntersectWithLine(pt1, pt2, 1e-10, t,
                                                        x, pcoords, subId))
                                inter[nInter++] = t;
                        }
                    }

                    // See if we have any near duplicates.
                    if (nInter > 2)
                    {
                        for (int ii = 0 ; ii < nInter-1 ; ii++)
                        {
                            for (int jj = ii+1 ; jj < nInter ; jj++)
                            {
                                if (fabs(inter[ii]-inter[jj]) < 1e-10)
                                {
                                    inter[ii] = inter[nInter-1];
                                    nInter--;
                                }
                            }
                        }
                    }
                }

                if (nInter == 2)
                {
                    cells_matched.push_back(iCell);
                    dist.push_back(inter[0]*lineLength);
                    dist.push_back(inter[1]*lineLength);
                    line_id.push_back(i);
                }
            }
        }
    }
    else
    {
        for (i = 0 ; i < linesForThisPass ; i++)
        {
            //
            // Determine which cells intersect the line.
            //
            double pt1[3];
            pt1[0] = lines[6*i];
            pt1[1] = lines[6*i+2];
            pt1[2] = lines[6*i+4];
            double pt2[3];
            pt2[0] = lines[6*i+1];
            pt2[1] = lines[6*i+3];
            pt2[2] = lines[6*i+5];
            double dir[3];
            dir[0] = pt2[0] - pt1[0];
            dir[1] = pt2[1] - pt1[1];
            dir[2] = pt2[2] - pt1[2];

            vector<int> list;
            tree.GetElementsList(pt1, dir, list);
            int nCells = (int)list.size();
            if (nCells == 0)
                continue;  // No intersection

            double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                     (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                     (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));
            for (j = 0 ; j < nCells ; j++)
            {
                int id = list[j];
                if (hasGhost && ghosts->GetTuple1(id) != 0.)
                    continue;
                vtkCell *cell = ds->GetCell(id);

                int nInter = 0;
                double inter[100];
                if (cell->GetCellDimension() == 3)
                {
                    int nFaces = cell->GetNumberOfFaces();
                    for (int k = 0 ; k < nFaces ; k++)
                    {
                        vtkCell *face = cell->GetFace(k);
                        double x[3];
                        double pcoords[3];
                        double t;
                        int subId;
                        if (face->IntersectWithLine(pt1, pt2, 1e-10, t, x, pcoords, 
                                                    subId))
                            inter[nInter++] = t;
                    }
                }
                else if (cell->GetCellDimension() == 2)
                {
                    int nEdges = cell->GetNumberOfEdges();
                    for (int k = 0 ; k < nEdges ; k++)
                    {
                        vtkCell *edge = cell->GetEdge(k);
                        double x[3];
                        double pcoords[3];
                        double t;
                        int subId;
                        if (edge->IntersectWithLine(pt1, pt2, 1e-10, t, x, pcoords, 
                                                    subId))
                            inter[nInter++] = t;
                    }
                }
                // See if we have any near duplicates.
                if (nInter > 2)
                {
                    for (int ii = 0 ; ii < nInter-1 ; ii++)
                    {
                        for (int jj = ii+1 ; jj < nInter ; jj++)
                        {
                            if (fabs(inter[ii]-inter[jj]) < 1e-10)
                            {
                                inter[ii] = inter[nInter-1];
                                nInter--;
                            }
                        }
                    }
                }
                if (nInter == 2)
                {
                    cells_matched.push_back(id);
                    dist.push_back(inter[0]*lineLength);
                    dist.push_back(inter[1]*lineLength);
                    line_id.push_back(i);
                }
                else
                {
                    // So this is technically an error state.  We have
                    // intersected the shape an odd number of times, which
                    // should mean that we are inside the shape.  We constructed
                    // our lines so that is not possible.  In reality, this occurs
                    // because of floating point precision issues.  In addition,
                    // every time it occurs, it is because we have a *very*
                    // small cell.  The queries that use this filter need to
                    // call "CleanPolyData" on it anyway, so cells this small
                    // will be "cleaned out".  So, rather than throwing an 
                    // exception, we can just continue.
                    continue;
                }
            }
        }
    }
    visitTimer->StopTimer(t1, "avtXRayFilter::LoopOverLines");

    nLinesPerDataset = (int)cells_matched.size();

    t1 = visitTimer->StartTimer();
    //
    // Copy the cell data.
    //
    vtkDataArray *dataArrays[2];
    vtkDataArray *da1=ds->GetCellData()->GetArray(absVarName.c_str());
    vtkDataArray *da2=ds->GetCellData()->GetArray(emisVarName.c_str());
    dataArrays[0] = da1;
    dataArrays[1] = da2;
    cellData = new T*[2];
    
    for (int i = 0; i < 2; i++)
    {
        vtkDataArray *da=dataArrays[i];
        int nComponents = da->GetNumberOfComponents();

        T *outVals = new T[cells_matched.size()*nComponents];
        if (da->GetDataType() == VTK_FLOAT)
        {
            float *inVals = vtkFloatArray::SafeDownCast(da)->GetPointer(0);
            int ndx = 0;
            for (size_t j = 0; j < cells_matched.size(); j++)
                for (int k = 0; k < nComponents; k++)
                    outVals[ndx++] = (T)inVals[cells_matched[j]*nComponents+k];
        }
        else // if (da->GetDataType() == VTK_DOUBLE)
        {
            double *inVals = vtkDoubleArray::SafeDownCast(da)->GetPointer(0);
            int ndx = 0;
            for (size_t j = 0; j < cells_matched.size(); j++)
                for (int k = 0; k < nComponents; k++)
                    outVals[ndx++] = (T)inVals[cells_matched[j]*nComponents+k];
        }
        cellData[i] = outVals;
    }
    visitTimer->StopTimer(t1, "avtXRayFilter::CopyCellData");

    //
    // Dump the ray cell intersections into a vtk file.
    //
    DumpRayHexIntersections(PAR_Rank(), currentNode, cells_matched,
                            line_id, ds, dataArrays);
}




// ****************************************************************************
//  Method: avtXRayFilter::CylindricalExecute
//
//  Purpose:
//    Finds line intersections in cylindrical space.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Wed Aug 18 14:58:47 PDT 2010
//    I corrected a bug copying the cell data.
//  
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Templatized this method, for double-precision support.
//
// ****************************************************************************

template <typename T>
void
avtXRayFilter::CylindricalExecute(vtkDataSet *ds, int &nLinesPerDataset,
    vector<double> &dist, vector<int> &line_id, T **&cellData)
{
    int  i, j;

    //
    // Create an interval tree for the data set to find the intersections
    // of the lines with the data set.
    //
    int nCells = ds->GetNumberOfCells();
    int dims   = 2;
    avtIntervalTree tree(nCells, dims);
    double bounds[6];
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    bool hasGhost = (ghosts != NULL);
    for (i = 0 ; i < nCells ; i++)
    {
        ds->GetCellBounds(i, bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);

    //
    // Loop over the lines.
    //
    vector<int> cells_matched;
    for (i = 0 ; i < linesForThisPass ; i++)
    {

        //
        // Determine which cells intersect the line.
        //
        double pt1[3];
        pt1[0] = lines[6*i];
        pt1[1] = lines[6*i+2];
        pt1[2] = lines[6*i+4];
        double pt2[3];
        pt2[0] = lines[6*i+1];
        pt2[1] = lines[6*i+3];
        pt2[2] = lines[6*i+5];
        double dir[3];
        dir[0] = pt2[0]-pt1[0];
        dir[1] = pt2[1]-pt1[1];
        dir[2] = pt2[2]-pt1[2];

        vector<int> list;
        tree.GetElementsFromAxiallySymmetricLineIntersection(pt1, dir, list);
        int nCells = (int)list.size();
        if (nCells == 0)
            continue;  // No intersection

        double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                 (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                 (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));

        for (j = 0 ; j < nCells ; j++)
        {
            int id = list[j];
            if (hasGhost && ghosts->GetTuple1(id) != 0.)
                continue;
            vtkCell *cell = ds->GetCell(id);
            vector<double> inter;
            int nEdges = cell->GetNumberOfEdges();
            for (int k = 0 ; k < nEdges ; k++)
            {
                vtkCell *edge = cell->GetEdge(k);
                int id1 = edge->GetPointId(0);
                double ePt1[3];
                ds->GetPoint(id1, ePt1);
                int id2 = edge->GetPointId(1);
                double ePt2[3];
                ds->GetPoint(id2, ePt2);
                double curInter[100];  // shouldn't really be more than 4.
                int numInter =
                    IntersectLineWithRevolvedSegment(pt1, dir, ePt1, ePt2,
                                                     curInter);
                for (int l = 0 ; l < numInter ; l++)
                    inter.push_back(curInter[l]);
            }

            if (inter.size() % 2 == 0)
            {
                if (inter.size() > 0)
                {
                    std::sort(inter.begin(), inter.end());
                    for (size_t l = 0 ; l < inter.size() / 2 ; l++)
                    {
                        cells_matched.push_back(id);
                        dist.push_back(inter[2*l]*lineLength);
                        dist.push_back(inter[2*l+1]*lineLength);
                        line_id.push_back(i);
                    }
                }
            }
            else
            {
                // So this is technically an error state.  We have
                // intersected the shape an odd number of times, which
                // should mean that we are inside the shape.  We constructed
                // our lines so that is not possible.  In reality, this occurs
                // because of floating point precision issues.  In addition,
                // every time it occurs, it is because we have a *very*
                // small cell.  The queries that use this filter need to
                // call "CleanPolyData" on it anyway, so cells this small
                // will be "cleaned out".  So, rather than throwing an 
                // exception, we can just continue.
                continue;
            }
        }
          
    }

    nLinesPerDataset = (int)cells_matched.size();

    //
    // Copy the cell data.
    //
    vtkDataArray *dataArrays[2];
    vtkDataArray *da1=ds->GetCellData()->GetArray(absVarName.c_str());
    vtkDataArray *da2=ds->GetCellData()->GetArray(emisVarName.c_str());
    dataArrays[0] = da1;
    dataArrays[1] = da2;
    cellData = new T*[2];
    for (int i = 0; i < 2; i++)
    {
        vtkDataArray *da=dataArrays[i];
        int nComponents = da->GetNumberOfComponents();
        T *outVals = new T[cells_matched.size()*nComponents];

        if (da->GetDataType() == VTK_FLOAT)
        {
            float *inVals = vtkFloatArray::SafeDownCast(da)->GetPointer(0);
            int ndx = 0;
            for (size_t j = 0; j < cells_matched.size(); j++)
                for (int k = 0; k < nComponents; k++)
                    outVals[ndx++] = (T)inVals[cells_matched[j]*nComponents+k];
        }
        else //  (da->GetDataType() == VTK_DOUBLE)
        {
            double *inVals = vtkDoubleArray::SafeDownCast(da)->GetPointer(0);
            int ndx = 0;
            for (size_t j = 0; j < cells_matched.size(); j++)
                for (int k = 0; k < nComponents; k++)
                    outVals[ndx++] = (T)inVals[cells_matched[j]*nComponents+k];
        }
        cellData[i] = outVals;
    }
}


#ifdef PARALLEL
static int
AssignToProc(int val, int linesPerProc)
{
    int proc = val / linesPerProc;
    if (proc > PAR_Size() - 1) proc = PAR_Size() - 1;
    return proc;
}
#endif


// ****************************************************************************
//  Method: avtXRayFilter::RedistributeLines
//
//  Purpose:
//    Redistribute the lines to the processors.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Jul 16 15:42:20 PDT 2010
//    I modified the filter to handle the case where some of the processors
//    didn't have any data sets when executing in parallel.
//
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Eric Brugger, Fri Mar 18 14:13:56 PDT 2011
//    I corrected a bug where the filter would crash when running in parallel
//    and the number of pixels processed in a pass was divisible by the
//    number of processors.
//
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Templatized this method, for double-precision support.
//
// ****************************************************************************
template <typename T>
void 
avtXRayFilter::RedistributeLines(int nLeaves, int *nLinesPerDataset,
    vector<double> *dists, vector<int> *line_ids,
    int nComponentsPerCellArray, T ***cellData,
    int &nPts, int *&outLineIds, double *&outDists, T **&outCellData)
{
#ifdef PARALLEL
    //
    // Assign lines to processors.
    //
    int nProcs = PAR_Size();
    int *sendCounts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        sendCounts[i] = 0;
    for (int i = 0; i < nLeaves; i++)
    {
        vector<int> inLineIds = line_ids[i];
        for (int j = 0; j < nLinesPerDataset[i]; j++)
            sendCounts[AssignToProc(inLineIds[j], linesForThisPassFirstProc)]++;
    }

    //
    // Determine the size of the send buffers.
    //
    int nLinesSend = 0;
    for (int i = 0; i < nLeaves; i++)
        nLinesSend += nLinesPerDataset[i];

    //
    // Create the send buffers.
    //
    int *sendLineIds = new int[nLinesSend];
    double *sendDists = new double[2*nLinesSend];
    T **sendCellData = new T*[2];
    for (int i = 0; i < 2; i++)
        sendCellData[i] = new T[nComponentsPerCellArray*nLinesSend];
    
    //
    // Fill the send buffers.
    //
    int *sendOffsets = new int[nProcs];
    sendOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        sendOffsets[i] = sendOffsets[i-1] + sendCounts[i-1];

    for (int i = 0; i < nLeaves; i++)
    {
        vector<int> inLineIds = line_ids[i];
        vector<double> inDists = dists[i];
        T **inCellData = cellData[i];
        for (int j = 0; j < nLinesPerDataset[i]; j++)
        {
            int iProc = AssignToProc(inLineIds[j], linesForThisPassFirstProc);
            int iOffset = sendOffsets[iProc];
            sendLineIds[iOffset] = inLineIds[j];
            sendDists[iOffset*2] = inDists[j*2];
            sendDists[iOffset*2+1] = inDists[j*2+1];
            for (int k = 0; k < 2; k++)
            {
                T *inVar = inCellData[k];
                T *sendVar = sendCellData[k];
                int nComps = nComponentsPerCellArray;
                for (int l = 0; l < nComps; l++)
                    sendVar[iOffset*nComps+l] = inVar[j*nComps+l];
            }
            sendOffsets[iProc]++;
        }
    }

    //
    // Calculate the receive counts.
    //
    int *recvCounts = new int[nProcs];
    MPI_Alltoall(sendCounts, 1, MPI_INT, recvCounts, 1, MPI_INT,
        VISIT_MPI_COMM);

    //
    // Determine the size of the receive buffers.
    //
    int nLinesRecv = 0;
    for (int i = 0; i < nProcs; i++)
        nLinesRecv += recvCounts[i];

    //
    // Determine the receiving nComponentsPerCellArray.
    //
    int nComponentsPerCellArrayRecv;
    MPI_Allreduce(&nComponentsPerCellArray, &nComponentsPerCellArrayRecv,
        1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);

    //
    // Create the output arrays.
    //
    nPts = nLinesRecv;
    outLineIds = new int[nLinesRecv];
    for (int i = 0; i < nLinesRecv; i++)
        outLineIds[i] = 0;

    outDists = new double[nLinesRecv*2];
    for (int i = 0; i < nLinesRecv*2; i++)
        outDists[i] = 0.;

    outCellData = new T*[2];
    outCellData[0] = new T[nLinesRecv*nComponentsPerCellArrayRecv];
    outCellData[1] = new T[nLinesRecv*nComponentsPerCellArrayRecv];
    for (int j = 0; j < 2; j++)
    {
        T *buf = outCellData[j];
        for (int i = 0; i < nLinesRecv*nComponentsPerCellArrayRecv; i++)
            buf[i] = 0.;
    }

    //
    // Calculate the send and receive offsets for the line ids.  The
    // previously calculated send offsets have been modified so they need
    // to be recalculated.
    //
    sendOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        sendOffsets[i] = sendOffsets[i-1] + sendCounts[i-1];

    int *recvOffsets = new int[nProcs];
    recvOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        recvOffsets[i] = recvOffsets[i-1] + recvCounts[i-1];

    //
    // Exchange the line ids.
    //
    MPI_Alltoallv(sendLineIds, sendCounts, sendOffsets, MPI_INT,
                  outLineIds, recvCounts, recvOffsets, MPI_INT,
                  VISIT_MPI_COMM);

    //
    // Calculate the send and receive offsets for the dists.  The distances
    // are twice the amount of data as the line ids.
    //
    for (int i = 0; i < nProcs; i++)
        sendCounts[i] *= 2;
    sendOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        sendOffsets[i] = sendOffsets[i-1] + sendCounts[i-1];

    for (int i = 0; i < nProcs; i++)
        recvCounts[i] *= 2;
    recvOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        recvOffsets[i] = recvOffsets[i-1] + recvCounts[i-1];

    //
    // Exchange the dists.
    //
    MPI_Alltoallv(sendDists, sendCounts, sendOffsets, MPI_DOUBLE,
                  outDists, recvCounts, recvOffsets, MPI_DOUBLE,
                  VISIT_MPI_COMM);

    //
    // Exchange the cell data.
    //
    for (int i = 0; i < nProcs; i++)
        sendCounts[i] /= 2;
    for (int i = 0; i < nProcs; i++)
        recvCounts[i] /= 2;
    for (int i = 0; i < 2; i++)
    {
        //
        // Calculate the send and receive offsets for the data.
        //
        for (int j = 0; j < nProcs; j++)
            sendCounts[j] *= nComponentsPerCellArrayRecv;
        sendOffsets[0] = 0;
        for (int j = 1; j < nProcs; j++)
            sendOffsets[j] = sendOffsets[j-1] + sendCounts[j-1];

        for (int j = 0; j < nProcs; j++)
            recvCounts[j] *= nComponentsPerCellArrayRecv;
        recvOffsets[0] = 0;
        for (int j = 1; j < nProcs; j++)
            recvOffsets[j] = recvOffsets[j-1] + recvCounts[j-1];

        //
        // Exchange the cell data.
        //
        
        //
        // Note: this helper is defined at the top of this file
        // it allows us to call the proper template double vs float 
        // variants of this function and avoid MPI type matching 
        // warnings.
        // 
        VisIt_XRay_MPI_Alltoallv(sendCellData[i], sendCounts, sendOffsets,
                                 outCellData[i], recvCounts, recvOffsets,
                                 VISIT_MPI_COMM);

        //
        // Restore the send and receive counts.
        //
        for (int j = 0; j < nProcs; j++)
            sendCounts[j] /= nComponentsPerCellArrayRecv;
        for (int j = 0; j < nProcs; j++)
            recvCounts[j] /= nComponentsPerCellArrayRecv;
    }

    //
    // Set numBins for use with the integration coding.
    //
    numBins = nComponentsPerCellArrayRecv;

    //
    // Clean up memory.
    //
    delete [] sendCounts;
    delete [] sendOffsets;

    delete [] recvCounts;
    delete [] recvOffsets;

    delete [] sendLineIds;
    delete [] sendDists;
    for (int i = 0; i < 2; i++)
        delete [] sendCellData[i];
    delete [] sendCellData;
#else
    int nLinesTotal = 0;
    for (int i = 0; i < nLeaves; i++)
        nLinesTotal += nLinesPerDataset[i];

    nPts = nLinesTotal;
    outLineIds = new int[nLinesTotal];

    outDists = new double[nLinesTotal*2];

    outCellData = new T*[2];
    outCellData[0] = new T[nLinesTotal*nComponentsPerCellArray];
    outCellData[1] = new T[nLinesTotal*nComponentsPerCellArray];

    int iLines = 0;
    int iPoints = 0;
    int *iCellStart = new int[2];
    for (int i = 0; i < 2; i++)
        iCellStart[i] = 0;
    for (int i = 0; i < nLeaves; i++)
    {
        vector<int> inLineIds = line_ids[i];
        vector<double> inDists = dists[i];
        for (int j = 0; j < nLinesPerDataset[i]; j++)
        {
            outLineIds[iLines++] = inLineIds[j];
            outDists[iPoints++] = inDists[j*2];
            outDists[iPoints++] = inDists[j*2+1];
        }

        T **inCellData = cellData[i];
        for (int j = 0; j < 2; j++)
        {
            int iCell = iCellStart[j];
            T *inVar = inCellData[j];
            T *outVar = outCellData[j];
            for (int k = 0; k < nLinesPerDataset[i]*nComponentsPerCellArray; k++)
                outVar[iCell++] = inVar[k];
            iCellStart[j] = iCell;
        }
    }

    //
    // Set numBins for use with the integration coding.
    //
    numBins = nComponentsPerCellArray;

    delete [] iCellStart;
#endif
}


// ****************************************************************************
//  Method: avtXRayFilter::CalculateLines
//
//  Purpose:
//    This defines the lines that will be intersected with the cells.
//
//  Programmer: Eric Brugger
//  Creation:   December 28, 2010
//
//  Modifications:
//    Eric Brugger, Thu Jun  4 15:58:10 PDT 2015
//    I added an option to enable outputting the ray bounds to a vtk file.
//
// ****************************************************************************

void
avtXRayFilter::CalculateLines(void)
{
    if (lines != NULL)
        delete [] lines;
    lines = new double[6*linesForThisPass];

    double viewSide[3];
    viewSide[0] = viewUp[1] * normal[2] - viewUp[2] * normal[1];
    viewSide[1] = -viewUp[0] * normal[2] + viewUp[2] * normal[0];
    viewSide[2] = viewUp[0] * normal[1] - viewUp[1] * normal[0];

    //
    // Calculate the width and height in the near plane, view plane and
    // far plane.
    //
    double nearHeight, viewHeight, farHeight;
    double nearWidth, viewWidth, farWidth;

    viewHeight = parallelScale;
    viewWidth  = (imageSize[1] / imageSize[0]) * viewHeight;
    if (perspective)
    {
        double viewDist = parallelScale / tan ((viewAngle * 3.1415926535) / 360.);
        double nearDist = viewDist + nearPlane;
        double farDist  = viewDist + farPlane;

        nearHeight = (nearDist * viewHeight) / viewDist;
        nearWidth  = (nearDist * viewWidth) / viewDist;
        farHeight  = (farDist * viewHeight) / viewDist;
        farWidth   = (farDist * viewWidth) / viewDist;
    }
    else
    {
        nearHeight = viewHeight;
        nearWidth  = viewWidth;
        farHeight  = viewHeight;
        farWidth   = viewWidth;
    }

    // Adjust for the image zoom.
    nearHeight = nearHeight / imageZoom;
    nearWidth  = nearWidth  / imageZoom;
    farHeight  = farHeight  / imageZoom;
    farWidth   = farWidth   / imageZoom;

    // Calculate the center of the image in the near and far planes.
    double nearOrigin[3], farOrigin[3];
    nearOrigin[0] = focus[0] + nearPlane * normal[0];
    nearOrigin[1] = focus[1] + nearPlane * normal[1];
    nearOrigin[2] = focus[2] + nearPlane * normal[2];
    farOrigin[0]  = focus[0] + farPlane  * normal[0];
    farOrigin[1]  = focus[1] + farPlane  * normal[1];
    farOrigin[2]  = focus[2] + farPlane  * normal[2];

    double nearDx, nearDy, farDx, farDy;
    nearDx = (2. * nearWidth)  / imageSize[0];
    nearDy = (2. * nearHeight) / imageSize[1];
    farDx = (2. * farWidth)   / imageSize[0];
    farDy = (2. * farHeight)  / imageSize[1];

    //
    // If this is the first processor and the first group of lines
    // then output the ray bounds if requested.
    //
    if (outputRayBounds && PAR_Rank() == 0 && lineOffset == 0)
    {
        double x[8], y[8], z[8];
        double y2 = - (2. * imagePan[1] * imageZoom + 1) * nearHeight +
                    nearDy / 2.;
        double y3 = - (2. * imagePan[1] * imageZoom + 1) * farHeight +
                    farDy / 2.;
        int ii = 0;
        for (int j = 0; j < 2; j++)
        {
            double x2 = - (2. * imagePan[0] * imageZoom + 1) * nearWidth +
                        nearDx / 2.;
            double x3 = - (2. * imagePan[0] * imageZoom + 1) * farWidth +
                        farDx / 2.;
            for (int i = 0; i < 2; i++)
            {
                x[ii] = nearOrigin[0] + x2 * viewSide[0] + y2 * viewUp[0];
                y[ii] = nearOrigin[1] + x2 * viewSide[1] + y2 * viewUp[1];
                z[ii] = nearOrigin[2] + x2 * viewSide[2] + y2 * viewUp[2];
                ii++;
                x[ii] = farOrigin[0]  + x3 * viewSide[0] + y3 * viewUp[0];
                y[ii] = farOrigin[1]  + x3 * viewSide[1] + y3 * viewUp[1];
                z[ii] = farOrigin[2]  + x3 * viewSide[2] + y3 * viewUp[2];
                ii++;
                x2 += (imageSize[0] - 1) * nearDx;
                x3 += (imageSize[0] - 1) * farDx;
            }
            y2 += (imageSize[1] - 1) * nearDy;
            y3 += (imageSize[1] - 1) * farDy;
        }
        FILE *f = fopen("ray_bounds.vtk", "w");
        fprintf(f, "# vtk DataFile Version 3.0\n");
        fprintf(f, "vtk output\n");
        fprintf(f, "ASCII\n");
        fprintf(f, "DATASET POLYDATA\n");
        fprintf(f, "POINTS 8 float\n");
        for (int i = 0; i < 8; i++)
            fprintf(f, "%g %g %g\n", x[i], y[i], z[i]);
        fprintf(f, "\n");
        fprintf(f, "POLYGONS 12 36\n");
            fprintf(f, "2 0 1\n");
        fprintf(f, "2 2 3\n");
        fprintf(f, "2 4 5\n");
        fprintf(f, "2 6 7\n");
        fprintf(f, "2 0 2\n");
        fprintf(f, "2 2 6\n");
        fprintf(f, "2 6 4\n");
        fprintf(f, "2 4 0\n");
        fprintf(f, "2 1 3\n");
        fprintf(f, "2 3 7\n");
        fprintf(f, "2 7 5\n");
        fprintf(f, "2 5 1\n");
        fclose(f);
    }

    int jstart = lineOffset / imageSize[0];
    int jend = jstart + (linesForThisPass / imageSize[0]);
    double y2 = - (2. * imagePan[1] * imageZoom + 1) * nearHeight +
                nearDy / 2. + jstart * nearDy;
    double y3 = - (2. * imagePan[1] * imageZoom + 1) * farHeight +
                farDy / 2.  + jstart * farDy;
    int ii = 0;
    for (int j = jstart; j < jend; j++)
    {
        double x2 = - (2. * imagePan[0] * imageZoom + 1) * nearWidth +
                    nearDx / 2.;
        double x3 = - (2. * imagePan[0] * imageZoom + 1) * farWidth +
                    farDx / 2.;
        for (int i = 0; i < imageSize[0]; i++)
        {
            lines[6*ii+0] = nearOrigin[0] + x2 * viewSide[0] + y2 * viewUp[0];
            lines[6*ii+1] = farOrigin[0]  + x3 * viewSide[0] + y3 * viewUp[0];
            lines[6*ii+2] = nearOrigin[1] + x2 * viewSide[1] + y2 * viewUp[1];
            lines[6*ii+3] = farOrigin[1]  + x3 * viewSide[1] + y3 * viewUp[1];
            lines[6*ii+4] = nearOrigin[2] + x2 * viewSide[2] + y2 * viewUp[2];
            lines[6*ii+5] = farOrigin[2]  + x3 * viewSide[2] + y3 * viewUp[2];

            x2 += nearDx;
            x3 += farDx;
            ii++;
        }
        y2 += nearDy;
        y3 += farDy;
    }
}


// ****************************************************************************
//  Method: avtXRayFilter::CheckDataSets
//
//  Purpose:
//    This routine performs some error checks on the input data sets.
//
//  Programmer: Eric Brugger
//  Creation:   December 28, 2010
//
// ****************************************************************************

void
avtXRayFilter::CheckDataSets(int nDataSets, vtkDataSet **dataSets)
{
    int numBins;
    cellDataType = -1;
    
    for (int i = 0; i < nDataSets; i++)
    {
        vtkDataArray *abs  = dataSets[i]->GetCellData()->GetArray(absVarName.c_str());
        vtkDataArray *emis = dataSets[i]->GetCellData()->GetArray(emisVarName.c_str());

        if (abs == NULL)
        {
            char msg[256];
            if (dataSets[i]->GetPointData()->GetArray(absVarName.c_str())
                != NULL)
            {
                snprintf(msg,256, "Failure: variable %s is node-centered, but "
                                  "it must be zone-centered for this query.",
                                  absVarName.c_str());
            }
            else
                snprintf(msg,256, "Variable %s not found.", absVarName.c_str());
            
            EXCEPTION1(VisItException, msg);
        }
        if (emis == NULL)
        {
            char msg[256];
            if (dataSets[i]->GetPointData()->GetArray(emisVarName.c_str())
                != NULL)
            {
                snprintf(msg,256, "Failure: variable %s is node-centered, but "
                                  "it must be zone-centered for this query.",
                                  emisVarName.c_str());
            }
            else
                snprintf(msg,256, "Variable %s not found.",
                                  emisVarName.c_str());
            EXCEPTION1(VisItException, msg);
        }

        if (abs->GetNumberOfComponents() != emis->GetNumberOfComponents())
        {
            EXCEPTION1(VisItException, "Number of bins for absorption and "
                                       "emission did not match.");
        }

        if (i == 0)
        {
            numBins = abs->GetNumberOfComponents();
        }
        else
        {
            if (numBins != abs->GetNumberOfComponents())
            {
                EXCEPTION1(VisItException, "Number of bins across chunks "
                                           "did not match.");
            }
        }

        if ((abs->GetDataType() != VTK_FLOAT &&
             abs->GetDataType() != VTK_DOUBLE) ||
            (emis->GetDataType() != VTK_FLOAT &&
             emis->GetDataType() != VTK_DOUBLE))
        {
            EXCEPTION1(VisItException, "The absorption and emission must "
                                       "be float data.");
        }
        if (abs->GetDataType() == VTK_DOUBLE ||
            emis->GetDataType() == VTK_DOUBLE) 
            cellDataType = VTK_DOUBLE;
        else
            cellDataType = VTK_FLOAT;
    }

    // Currently ony VTK_FLOAT and VTK_DOUBLE are supported, so this
    // works.  If all data types are allowed, this would not work.
    cellDataType = UnifyMaximumValue(cellDataType);
}


typedef struct
{
    int lineId;
    int ptId;
    double dist;
}  IdPoint;


// ****************************************************************************
//  Function: IdPointSorter
//
//  Purpose:
//    Comparison routine used to sort the line segments.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

static int
IdPointSorter(const void *arg1, const void *arg2)
{
    const IdPoint *r1 = (const IdPoint *) arg1;
    const IdPoint *r2 = (const IdPoint *) arg2;

    if (r1->lineId > r2->lineId)
        return 1;
    else if (r1->lineId < r2->lineId)
        return -1;

    if (r1->dist > r2->dist)
        return 1;
    else if (r1->dist < r2->dist)
        return -1;

    return 0;
}


// ****************************************************************************
//  Function: SortSegments
//
//  Purpose:
//    Sort the line segments by pixel id and distance.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

static int *
SortSegments(int nLines, int *lineId, double *dists)
{
    //
    // Sort the distances for each line segment.
    //
    for (int i = 0; i < nLines; i++)
    {
        if (dists[i*2] > dists[i*2+1])
        {
            double tmp = dists[i*2];
            dists[i*2] = dists[i*2+1];
            dists[i*2+1] = tmp;
        }
    }

    //
    // Sort the segments by line id and distance.
    //
    IdPoint *idPoints = new IdPoint[nLines];
    for (int i = 0 ; i < nLines; i++)
    {
        idPoints[i].lineId = lineId[i]; // See assumption above
        idPoints[i].ptId = i;
        idPoints[i].dist = dists[i*2];
    }
    qsort(idPoints, nLines, sizeof(IdPoint), IdPointSorter);

    //
    // Form the output array.
    //
    int *compositeOrder = new int[nLines];
    for (int i = 0; i < nLines; i++)
        compositeOrder[i] = idPoints[i].ptId;

    delete [] idPoints;

    return compositeOrder;
}


// ****************************************************************************
//  Method: avtXRayFilter::IntegrateLines
//
//  Purpose:
//    Integrate the line segments using the order specified by segmentOrder.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Kathleen Biagas, Thu Mar 29 07:48:13 PDT 2012
//    Templatized this method, for double-precision support.
//
//    Gunther H. Weber, Wed Jan 23 15:23:14 PST 2013
//    Add support for specifying background intensity.
//
//    Eric Brugger, Thu Jan 15 11:02:10 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Thu May 21 12:21:25 PDT 2015
//    I added support for debugging a ray.
//
//    Eric Brugger, Wed May 27 10:10:28 PDT 2015
//    I modified the filter to also output the path length field.
//
//    Eric Brugger, Wed Aug  2 09:35:37 PDT 2017
//    I modified the ray debugging to properly work in the case where
//    divideEmisByAbsorb was set.
//
// ****************************************************************************

template <typename T>
void
avtXRayFilter::IntegrateLines(int pixelOffset, int nPts, int *lineId,
    double *dist, T *absorbtivity, T *emissivity)
{
    //
    // Determine the order to do the compositing.
    //
    int *segmentOrder = SortSegments(nPts, lineId, dist);

    //
    // Set the background intensities used by the calculation. It uses
    // background intensities specified on a per bin basis and then fills
    // in with the single background.
    //
    double *background = new double[numBins];

    int nBackground =
        numBins < nBackgroundIntensities ? numBins : nBackgroundIntensities;
    for (int i = 0; i < nBackground; i++)
        background[i] = backgroundIntensities[i];
    for (int i = nBackground; i < numBins; i++)
        background[i] = backgroundIntensity;

    //
    // Do the integration.
    //
    if (intensityBins == NULL)
    {
        intensityBins = new double[numBins];
        pathBins = new double[numBins];
    }
    for (int i = 0 ; i < numBins ; i++)
    {
        intensityBins[i] = background[i];
        pathBins[i] = 0.;
    }

    int prevLineId = -1;
    if (cellDataType == VTK_FLOAT)
    {
        intensityFragments[iFragment] = vtkFloatArray::New();
        pathLengthFragments[iFragment] = vtkFloatArray::New();
    }
    else
    {
        intensityFragments[iFragment] = vtkDoubleArray::New();
        pathLengthFragments[iFragment] = vtkDoubleArray::New();
    }

    intensityFragments[iFragment]->SetNumberOfTuples(imageFragmentSizes[iFragment]*numBins);
    pathLengthFragments[iFragment]->SetNumberOfTuples(imageFragmentSizes[iFragment]*numBins);

    avtDirectAccessor<T> currentIntensityFragment(intensityFragments[iFragment]);
    avtDirectAccessor<T> currentPathFragment(pathLengthFragments[iFragment]);
    for (int i = 0; i < imageFragmentSizes[iFragment]; i++)
    {
        for (int j = 0; j < numBins; j++)
            currentIntensityFragment.SetTuple1(i*numBins+j, background[j]);
        for (int j = 0; j < numBins; j++)
            currentPathFragment.SetTuple1(i*numBins+j, 0.);
    }

    iFragment++;

    for (int i = 0; i < nPts; i++)
    {
        int iPt = segmentOrder[i];

        if (lineId[iPt] != prevLineId)
        {
            if (prevLineId != -1)
            {
                for (int j = 0; j < numBins; j++)
                    currentIntensityFragment.SetTuple1((prevLineId-pixelOffset)*numBins+j, intensityBins[j]);
                for (int j = 0; j < numBins; j++)
                    currentPathFragment.SetTuple1((prevLineId-pixelOffset)*numBins+j, pathBins[j]);
            }

            for (int j = 0; j < numBins; j++)
            {
                intensityBins[j] = background[j];
                pathBins[j] = 0.;
            }
            prevLineId = lineId[iPt];
        }

        double segLength = dist[iPt*2+1] - dist[iPt*2];
        T *a = &(absorbtivity[iPt*numBins]);
        T *e = &(emissivity[iPt*numBins]);

        if (divideEmisByAbsorb)
        {
            for (int j = 0 ; j < numBins ; j++)
            {
                double tmp = exp(-a[j] * segLength);
                intensityBins[j] = intensityBins[j] * tmp + (e[j] / a[j]) * (1.0 - tmp);
                pathBins[j] = pathBins[j] + a[j] * segLength;
            }
        }
        else
        {
            for (int j = 0 ; j < numBins ; j++)
            {
                double tmp = exp(-a[j] * segLength);
                intensityBins[j] = intensityBins[j] * tmp + e[j] * (1.0 - tmp);
                pathBins[j] = pathBins[j] + a[j] * segLength;
            }
        }
    }

    if (prevLineId != -1)
    {
        for (int j = 0; j < numBins; j++)
            currentIntensityFragment.SetTuple1((prevLineId-pixelOffset)*numBins+j, intensityBins[j]);
        for (int j = 0; j < numBins; j++)
            currentPathFragment.SetTuple1((prevLineId-pixelOffset)*numBins+j, pathBins[j]);
    }

    //
    // Make another pass if ray debugging is set. We only trace the first
    // bin.
    //

    if (debugRay != -1)
    {
        double intensityBinZero(0.);
        prevLineId = -1;

        FILE *f = NULL;
        for (int i = 0; i < nPts; i++)
        {
            int iPt = segmentOrder[i];

            if (lineId[iPt] != prevLineId)
            {
                if (lineOffset + lineId[iPt] == debugRay)
                {
                    char filename[80];
                    snprintf(filename, 80, "ray%d.csv", debugRay);
                    f = fopen(filename, "w");
                    fprintf(f, " dist1, dist2, tmp, segLength, a, e, intensityBinZero\n");
                    intensityBinZero = background[0];
                }
                if (lineOffset + prevLineId == debugRay)
                {
                    fclose(f);
                }
                prevLineId = lineId[iPt];
            }

            if (lineOffset + lineId[iPt] == debugRay)
            {
                double segLength = dist[iPt*2+1] - dist[iPt*2];
                T *a = &(absorbtivity[iPt*numBins]);
                T *e = &(emissivity[iPt*numBins]);

                // bin zero.
                double tmp = exp(-a[0] * segLength);
                if (divideEmisByAbsorb)
                    intensityBinZero = intensityBinZero * tmp + (e[0] / a[0]) * (1.0 - tmp);
                else
                    intensityBinZero = intensityBinZero * tmp + e[0] * (1.0 - tmp);
                fprintf(f, "%g, %g, %g, %g, %g, %g, %g\n",
                        dist[iPt*2], dist[iPt*2+1], tmp, segLength,
                        a[0], e[0], intensityBinZero);
            }
        }

        if (prevLineId != -1 && lineOffset + prevLineId == debugRay)
        {
            fclose(f);
        }
    }

    delete [] segmentOrder;
    delete [] background;
}


// ****************************************************************************
//  Method: avtXRayFilter::CollectFragments
//
//  Purpose:
//    Collect the fragments on the first processor.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:22:48 PST 2010
//    I modified the filter to return a set of images instead of a collection
//    of line segments representing the intersections of a collection of lines
//    with the cells in the dataset.
//
//    Eric Brugger, Fri Mar 18 14:13:56 PDT 2011
//    I modified the routine to only create the final image on processor
//    zero. This didn't seem to cause any problems, but was wasteful.
//
//    Eric Brugger, Wed May 27 10:10:28 PDT 2015
//    I modified the filter to also output the path length field.
//
// ****************************************************************************

template <typename T>
void
avtXRayFilter::CollectFragments(int root, int nFragments, int *fragmentSizes,
    vtkDataArray **fragments, vtkDataArray *&collectedFragments)
{
#ifdef PARALLEL
    int nProcs = PAR_Size();

    //
    // Set up the send information.
    //
    int sendCount = 0;
    for (int i = 0; i < nFragments; i++)
        sendCount += fragmentSizes[i]*numBins;

    T *sendBuf = new T[sendCount];
    for (int i = 0, ndx = 0; i < nFragments; i++)
    {
        avtDirectAccessor<T> currentFragment(fragments[i]);
        while (currentFragment.Iterating())
        {
            sendBuf[ndx] = currentFragment.GetTuple1();
            currentFragment++;
            ndx++;
        }
    }

    //
    // Set up the receive information.
    //
    int *recvCounts = new int[nProcs];
    int *displs = new int[nProcs];

    recvCounts[0] = ((nFragments - 1) * pixelsForFirstPassFirstProc +
        pixelsForLastPassFirstProc) * numBins;
    displs[0] = 0;
    for (int i = 1; i < nProcs-1; i++)
    {
        recvCounts[i] = ((nFragments - 1) * pixelsForFirstPassFirstProc +
            pixelsForLastPassFirstProc) * numBins;
        displs[i] = displs[i-1] + recvCounts[i-1];
    }
    recvCounts[nProcs-1] = ((nFragments - 1) * pixelsForFirstPassLastProc +
        pixelsForLastPassLastProc) * numBins;
    displs[nProcs-1] = displs[nProcs-1-1] + recvCounts[nProcs-1-1];

    T *recvBuf = NULL;
    if (PAR_Rank() == 0)
    {
        recvBuf = new T[numPixels * numBins];
    }

    //
    // Note: this helper is defined at the top of this file
    // it allows us to call the proper template double vs float 
    // variants of this function and avoid MPI type matching 
    // warnings.
    // 
    VisIt_XRay_MPI_Gatherv(sendBuf, sendCount,
                           recvBuf, recvCounts, displs,
                           root, VISIT_MPI_COMM);

    if (PAR_Rank() == 0)
    {
        //
        // Reorganize the receive buffer in the correct order.
        //
        if (cellDataType == VTK_FLOAT)
            collectedFragments = vtkFloatArray::New();
        else
            collectedFragments = vtkDoubleArray::New();
        collectedFragments->SetNumberOfTuples(numPixels * numBins);
        avtDirectAccessor<T> collectedFragmentsA(collectedFragments);
        for (int i = 0; i < nFragments-1; i++)
        {
            for (int j = 0; j < nProcs-1; j++)
            {
                for (int k = 0; k < pixelsForFirstPassFirstProc*numBins; k++)
                {
                    collectedFragmentsA.SetTuple1(recvBuf[displs[j]]);
                    collectedFragmentsA++;
                    displs[j]++;
                }
            }
            int j = nProcs - 1;
            for (int k = 0; k < pixelsForFirstPassLastProc*numBins; k++)
            {
                collectedFragmentsA.SetTuple1(recvBuf[displs[j]]);
                collectedFragmentsA++;
                displs[j]++;
            }
        }
        for (int j = 0; j < nProcs-1; j++)
        {
            for (int k = 0; k < pixelsForLastPassFirstProc*numBins; k++)
            {
                collectedFragmentsA.SetTuple1(recvBuf[displs[j]]);
                collectedFragmentsA++;
                displs[j]++;
            }
        }
        int j = nProcs - 1;
        for (int k = 0; k < pixelsForLastPassLastProc*numBins; k++)
        {
            collectedFragmentsA.SetTuple1(recvBuf[displs[j]]);
            collectedFragmentsA++;
            displs[j]++;
        }

        delete [] recvBuf;
    }
    else
    {
        collectedFragments = NULL ;
    }
#else
    collectedFragments = NULL;
#endif
}


// ****************************************************************************
//  Method: avtIntersectionTests::IntersectLineWithRevolvedSegment
//
//  Purpose:
//    Takes a segment that is in cylindrical coordinates and revolves it
//    into three-dimensional Cartesian space and finds the intersections
//    with a line.  The number of intersections can be 1, 2, or 4.
//
//  Arguments:
//    line_pt    A point on the line (Cartesian)
//    line_dir   The direction of the line (Cartesian)
//    seg_1      One endpoint of the segment (Cylindrical)
//    seg_2      The other endpoint of the segment (Cylindrical)
//    inter      The intersections found.  Output value.  They are 
//                 represented distances along line_dir from line_pt.
//
//  Returns:       The number of intersections
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Sep  7 16:29:51 PDT 2010
//    I had the routine return instead of throwing an exception if the
//    segment had a negative R value.  Throwing an exception caused a
//    crash in parallel.
//    
//    Matt Larsen, Thurs May 3rd 09:00:01 PDT 2018
//    I fixed two issues. One, if the coordinates of a mesh were nearly
//    vertical or horizontal, floating point error resulted in misses
//    when there was absolutely a hit. In the worst cases, this resulted in
//    answers being off by over 66%. Two, there was never a check for a 0
//    discriminant when solving a quadratic. This resulted in two intersections
//    when there was only ever one, and the logic around the caller would assume
//    something whet terribly wrong and ignore the cell.
//
//    Matt Larsen, Mon May 7th, 15:33:01 PDT 2018
//    Altering previous fix to work via a tolerance
//  
// ****************************************************************************

int
IntersectLineWithRevolvedSegment(const double *line_pt,
                                 const double *line_dir, const double *seg_p1, 
                                 const double *seg_p2, double *inter)
{
    //
    // Exit if the segment crosses the axis line.
    //
    if (seg_p1[1] < 0. || seg_p2[1] < 0.)
    {
        return 0;
    }
    //
    // Note that in the logic below, we are using the Z-component
    // of the line to compare with the X-component of the cell,
    // since the cell's X-component is actually 'Z' in RZ-space.
    //

    //
    // We have to check for lines that are near vertical. Slopes
    // greater than 1B can lead to floating point errors that lead
    // to missed cell intersections, no matter how large the cell.
    //
    bool near_vertical = false;
    if(seg_p1[0] != seg_p2[0])
    {
        double slope = (seg_p1[1] - seg_p2[1]) / (seg_p1[0] - seg_p2[0]);
        if(slope > 1e10 || slope < -1e10)
        {
          near_vertical = true;
        }
    }

    if (seg_p1[0] == seg_p2[0] || near_vertical)
    {
        // Vertical line .. revolves to hollow disc.
        // Disc is at some constant Z (seg_p1[0]) and ranges between some
        // Rmin and Rmax.
        double Rmin = (seg_p1[1] < seg_p2[1] ? seg_p1[1] : seg_p2[1]);
        double Rmax = (seg_p1[1] > seg_p2[1] ? seg_p1[1] : seg_p2[1]);
        if (line_dir[2] == 0.)
        {
            if (seg_p1[0] != line_pt[2])
                return 0;
            
            // Solving for inequalities is tough.  In this case, we will
            // solve for equalities.  Solve for R = Rmax and R = Rmin.
            // At^2 + Bt + C = 0
            //  A = Dx^2 + Dy^2
            //  B = 2*Dx*Px + 2*Dy*Py
            //  C = Px^2 + Py^2 - R^2
            double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1];
            double B = 2*line_dir[0]*line_pt[0] + 2*line_dir[1]*line_pt[1];
            double C0 = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1];
            double C = C0 - Rmax*Rmax;
            double det = B*B - 4*A*C;
            int nInter = 0;
            if (det > 0)
            {
                double soln1 = (-B + sqrt(det)) / (2*A);
                double soln2 = (-B - sqrt(det)) / (2*A);
                inter[nInter++] = soln1;
                if(det != 0.f) inter[nInter++] = soln2;
            }
            C = C0 - Rmin*Rmin;
            det = B*B - 4*A*C;
            if (det > 0)
            {
                double soln1 = (-B + sqrt(det)) / (2*A);
                double soln2 = (-B - sqrt(det)) / (2*A);
                inter[nInter++] = soln1;
                if(det != 0.) inter[nInter++] = soln2;
            }
        }
        else
        {
            // Our line will go through the constant Z-plane that the segment
            // gets revolved into just once.  So calculate that plane and
            // determine if the line intersects the disc.
            double t = (seg_p1[0] - line_pt[2]) / line_dir[2];
            double x = line_pt[0] + t*line_dir[0];
            double y = line_pt[1] + t*line_dir[1];
            double Rsq = x*x + y*y;
            if (Rmin*Rmin < Rsq && Rsq < Rmax*Rmax)
            {
                inter[0] = t;
                return 1;
            }
        }
    }
    else if (seg_p1[1] == seg_p2[1])
    {
        // Horizonal line ... revolves to cylindrical shell.
        // Shell has constant radius (seg_p1[1]) and ranges between Z=seg_p1[0]
        // and Z=seg_p2[0].
        // Solve for t that has intersection.
        // ((Px + t*Dx)^2 + (Py + t*Dy)^2)^0.5 = R
        // ((Px + t*Dx)^2 + (Py + t*Dy)^2) = R^2
        // At^2 + Bt + C = 0
        // A = (Dx^2 + Dy^2)
        // B = (2*Dx*Px + 2*Dy*Py)
        // C = (Px^2 + Py^2 - R^2)
        double R = seg_p1[1];
        double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1];
        double B = 2*line_pt[0]*line_dir[0] + 2*line_pt[1]*line_dir[1];
        double C = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1] - R*R;
        double det = B*B - 4*A*C;
        if (det < 0)
            return 0;
        double soln1 = (-B + sqrt(det)) / (2*A);
        double soln2 = (-B - sqrt(det)) / (2*A);
        double Z1 = line_pt[2] + soln1*line_dir[2];
        double Z2 = line_pt[2] + soln2*line_dir[2];

        int nInter = 0;
        double Zmin = (seg_p1[0] < seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        double Zmax = (seg_p1[0] > seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        if (Zmin <= Z1 && Z1 <= Zmax)
        {
            inter[nInter] = soln1;
            nInter++;
        }
        if (Zmin <= Z2 && Z2 <= Zmax && det != 0.)
        {
            inter[nInter] = soln2;
            nInter++;
        }
        return nInter;
    }
    else
    {
        // We have a segment that is neither horizontal or vertical.  So the
        // revolution of this segment will result in a conic shell, truncated
        // in Z.  The conic shell will be oriented around the Z-axis.
        // The segment is along a line of form r = mz+b.  If we can calculate
        // m and b, then we can ask when a point on the line will coincide
        // with our line.  If they coincided, they would have the same r and
        // z values.  Since the line is in 3D, having the same r values
        // means having the same sqrt(x^2+y^2) values.
        // 
        // Then, for the line:
        // x = Px + t*Dx
        // y = Py + t*Dy
        // z = Pz + t*Dz
        // For the conic shell, we know:
        // sqrt(x^2+y^2) = m*z+b
        // Substituting for Z gives:
        // sqrt(x^2+y^2) = m*(Pz+tDz)+b
        // sqrt(x^2+y^2) = (m*Pz+b) + t*m*Dz
        // Introducing K for m*Pz+b (to simplify algebra)
        // sqrt(x^2+y^2) = K + t*m*Dz
        // Substituting for x and y and squaring gives:
        // (Px^2 + Py^2) + (2*Px*Dx + 2*Py*Dy)t + (Dx^2 + Dy^2)t^2 
        //    = K^2 + (2*K*m*Dz)*t + m^2*Dz^2*t^2
        // Combining like terms gives:
        //  At^2 + Bt + C = 0
        //  A = Dx^2 + Dy^2 - m^2*Dz^2
        //  B = 2*Px*Dx + 2*Py*Dy - 2*K*m*Dz
        //  C = Px^2 + Py^2 - K^2
        // And then we can solve for t to find the intersections.
        // At the end, we will restrict the answer to be between the
        // valid range for the segment.
        double m = (seg_p1[1] - seg_p2[1]) / (seg_p1[0] - seg_p2[0]);
        double b = seg_p1[1] - m*seg_p1[0];
        double K = m*line_pt[2]+b;
        double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1]
                 - m*m*line_dir[2]*line_dir[2];
        double B = 2*line_pt[0]*line_dir[0] + 2*line_pt[1]*line_dir[1]
                 - 2*K*m*line_dir[2];
        double C = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1] - K*K;
        double det = B*B - 4*A*C;
        if (det < 0)
            return 0;
        double soln1 = (-B + sqrt(det)) / (2. * A);
        double soln2 = (-B - sqrt(det)) / (2. * A);

        double Zmin = (seg_p1[0] < seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        double Zmax = (seg_p1[0] > seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        
        int nInter = 0;
        
        double Z1 = line_pt[2] + soln1*line_dir[2];
        double Z2 = line_pt[2] + soln2*line_dir[2];

        if (Zmin <= Z1 && Z1 <= Zmax)
        {
            inter[nInter] = soln1;
            nInter++;
        }
        // We have to check to see if the discrim in
        // 0, since both solutions would be identicle
        if (Zmin <= Z2 && Z2 <= Zmax && det != 0.)
        {
            inter[nInter] = soln2;
            nInter++;
        }

        return nInter;
    }

    return 0;
}

template <class Accessor>
void
MergeFragments_Impl(int iBin, int numBins, int nFragments, int *fragmentSizes,
    vtkDataArray **fragments, vtkDataArray *&outputArray)
{
    Accessor ibuf(outputArray);
    ibuf.InitTraversal();
    for (int i = 0; i < nFragments; ++i)
    {
        Accessor currentFragment(fragments[i]);
        for (int j = 0; j < fragmentSizes[i]; ++j)
        {
            ibuf.SetTuple1(currentFragment.GetTuple1(j*numBins+iBin));
            ibuf++;
        }
    }
}

void
avtXRayFilter::MergeFragments(int iBin, vtkDataArray **fragments,
    vtkDataArray *&outputArray)
{
    if (cellDataType == VTK_FLOAT)
    {
        outputArray = vtkFloatArray::New();
        outputArray->SetNumberOfTuples(imageSize[0]*imageSize[1]);
        MergeFragments_Impl<avtDirectAccessor<float> >(iBin, numBins, 
            nImageFragments, imageFragmentSizes, fragments, outputArray);
    }
    else // if (cellDataType == VTK_DOUBLE)
    {
        outputArray = vtkDoubleArray::New();
        outputArray->SetNumberOfTuples(imageSize[0]*imageSize[1]);
        MergeFragments_Impl<avtDirectAccessor<double> >(iBin, numBins,
            nImageFragments, imageFragmentSizes, fragments, outputArray);
    }
}


// ****************************************************************************
//  Method: avtXRayFilter::DumpRayHexIntersections
//
//  Purpose:
//    Dump the ray intersections into a vtk file.
//
//  Programmer: Eric Brugger
//  Creation:   December 3, 2012
//
//  Notes:
//    Assumes that the data set is an unstructured grid that consists of
//    only hexes.
//
//  Modifications:
//    Eric Brugger, Thu May 21 12:21:25 PDT 2015
//    I enhanced the routine to support structured as well as unstructured
//    grids.
//
// ****************************************************************************

void
avtXRayFilter::DumpRayHexIntersections(int iProc, int iDataset,
    vector<int> &cells_matched, vector<int> &line_id,
    vtkDataSet *ds, vtkDataArray **dataArrays)
{
    //
    // We can only handle structured and unstructured grids. Return if
    // this isn't the case.
    //
    if (ds->GetDataObjectType() != VTK_STRUCTURED_GRID &&
        ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
        return;

    //
    // Determine the id within the current strip of the line. Return if
    // the line is outside of the current strip.
    //
    int strip_id = debugRay - (iPass * pixelsForFirstPass);

    if (strip_id < 0 || strip_id >= linesForThisPass)
        return;

    //
    // Determine the number of cells where the line ids match. Return if
    // if no cells intersect the line of interest.
    //
    int nCells = 0;
    for (size_t i = 0; i < line_id.size(); i++)
        if (line_id[i] == strip_id)
            nCells++;
    
    if (nCells <= 0)
        return;

    //
    // Write the vtk file.
    //
    char filename[80];
    snprintf(filename, 80, "ray%d_proc%02d_ds%02d.vtk",
             debugRay, iProc, iDataset);
    FILE *f = fopen(filename, "w");
    fprintf(f, "# vtk DataFile Version 3.0\n");
    fprintf(f, "vtk output\n");
    fprintf(f, "ASCII\n");
    fprintf(f, "DATASET UNSTRUCTURED_GRID\n");
    fprintf(f, "POINTS %d float\n", nCells * 8);

    if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) ds;
        vtkPoints *points = sgrid->GetPoints();

        int ndims[3];
        sgrid->GetDimensions(ndims);

        int zdims[3];
        zdims[0] = ndims[0] - 1;
        zdims[1] = ndims[1] - 1;
        zdims[2] = ndims[2] - 1;

        int nx = ndims[0];
        int ny = ndims[1];
        int nxy = nx * ny;

        int nx2 = zdims[0];
        int ny2 = zdims[1];
        int nxy2 = nx2 * ny2;

        double p0[3], p1[3], p2[3], p3[3], p4[3], p5[3], p6[3], p7[3];
        for (size_t i = 0; i < cells_matched.size(); i++)
        {
            if (line_id[i] == strip_id)
            {
                int iCell = cells_matched[i];

                int iZ = iCell / nxy2;
                int iXY = iCell % nxy2;
                int iY = iXY / nx2;
                int iX = iXY % nx2;
                int idx = iX+ iY*nx + iZ*nxy;

                int ids[8];
                ids[0] = idx;
                ids[1] = idx + 1;
                ids[2] = idx + 1 + nx;
                ids[3] = idx + nx;
                idx += nxy;
                ids[4] = idx;
                ids[5] = idx + 1;
                ids[6] = idx + 1 + nx;
                ids[7] = idx + nx;

                avtXRayFilter_GetCellPointsMacro(8);
                fprintf(f, "%g %g %g\n", p0[0], p0[1], p0[2]);
                fprintf(f, "%g %g %g\n", p1[0], p1[1], p1[2]);
                fprintf(f, "%g %g %g\n", p2[0], p2[1], p2[2]);
                fprintf(f, "%g %g %g\n", p3[0], p3[1], p3[2]);
                fprintf(f, "%g %g %g\n", p4[0], p4[1], p4[2]);
                fprintf(f, "%g %g %g\n", p5[0], p5[1], p5[2]);
                fprintf(f, "%g %g %g\n", p6[0], p6[1], p6[2]);
                fprintf(f, "%g %g %g\n", p7[0], p7[1], p7[2]);
            }
        }
    }
    else if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) ds;
        vtkPoints *points = ugrid->GetPoints();

        vtkIdTypeArray *cellLocations = ugrid->GetCellLocationsArray();
        vtkCellArray *cells = ugrid->GetCells();

        vtkIdType *nl = cells->GetPointer();
        vtkIdType *cl = cellLocations->GetPointer(0);

        double p0[3], p1[3], p2[3], p3[3], p4[3], p5[3], p6[3], p7[3];
        for (size_t i = 0; i < cells_matched.size(); i++)
        {
            if (line_id[i] == strip_id)
            {
                int iCell = cells_matched[i];
                vtkIdType *ids = &(nl[cl[iCell]+1]);
                avtXRayFilter_GetCellPointsMacro(8);
                fprintf(f, "%g %g %g\n", p0[0], p0[1], p0[2]);
                fprintf(f, "%g %g %g\n", p1[0], p1[1], p1[2]);
                fprintf(f, "%g %g %g\n", p2[0], p2[1], p2[2]);
                fprintf(f, "%g %g %g\n", p3[0], p3[1], p3[2]);
                fprintf(f, "%g %g %g\n", p4[0], p4[1], p4[2]);
                fprintf(f, "%g %g %g\n", p5[0], p5[1], p5[2]);
                fprintf(f, "%g %g %g\n", p6[0], p6[1], p6[2]);
                fprintf(f, "%g %g %g\n", p7[0], p7[1], p7[2]);
            }
        }
    }
    fprintf(f, "\n");
    fprintf(f, "CELLS %d %d\n", nCells, nCells*9);
    int j = 0;
    for (size_t i = 0; i < cells_matched.size(); i++)
        if (line_id[i] == strip_id)
        {
            fprintf(f, "8 %d %d %d %d %d %d %d %d\n",
                    j*8, j*8+1, j*8+2, j*8+3, j*8+4, j*8+5, j*8+6, j*8+7);
            j++;
        }
    fprintf(f, "\n");
    fprintf(f, "CELL_TYPES %d\n", nCells);
    for (int i = 0; i < nCells; i++)
        fprintf(f, "12\n");
    fprintf(f, "\n");
    fprintf(f, "CELL_DATA %d\n", nCells);

    //
    // Output the first component of the opacities.
    //
    fprintf(f, "SCALARS opacity float\n");
    fprintf(f, "LOOKUP_TABLE default\n");

    vtkDataArray *da = dataArrays[0];
    int nComponents = da->GetNumberOfComponents();
    if (da->GetDataType() == VTK_FLOAT)
    {
        float *vals = vtkFloatArray::SafeDownCast(da)->GetPointer(0);
        for (size_t i = 0; i < cells_matched.size(); i++)
            if (line_id[i] == strip_id)
            {
                fprintf(f, "%g\n", vals[cells_matched[i]*nComponents+0]);
            }
    }
    else // if (da->GetDataType() == VTK_DOUBLE)
    {
        double *vals = vtkDoubleArray::SafeDownCast(da)->GetPointer(0);
        for (size_t i = 0; i < cells_matched.size(); i++)
            if (line_id[i] == strip_id)
            {
                fprintf(f, "%g\n", vals[cells_matched[i]*nComponents+0]);
            }
    }

    //
    // Output the first component of the emissivities.
    //
    fprintf(f, "SCALARS emissivity float\n");
    fprintf(f, "LOOKUP_TABLE default\n");

    da = dataArrays[1];
    nComponents = da->GetNumberOfComponents();
    if (da->GetDataType() == VTK_FLOAT)
    {
        float *vals = vtkFloatArray::SafeDownCast(da)->GetPointer(0);
        for (size_t i = 0; i < cells_matched.size(); i++)
            if (line_id[i] == strip_id)
            {
                fprintf(f, "%g\n", vals[cells_matched[i]*nComponents+0]);
            }
    }
    else // if (da->GetDataType() == VTK_DOUBLE)
    {
        double *vals = vtkDoubleArray::SafeDownCast(da)->GetPointer(0);
        for (size_t i = 0; i < cells_matched.size(); i++)
            if (line_id[i] == strip_id)
            {
                fprintf(f, "%g\n", vals[cells_matched[i]*nComponents+0]);
            }
    }

    fclose(f);
}
