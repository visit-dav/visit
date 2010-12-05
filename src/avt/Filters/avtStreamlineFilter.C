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

// ************************************************************************* //
//                              avtStreamlineFilter.C                        //
// ************************************************************************* //

/**
TODO:
Sort: Bias sorting to "my domain". SLs on this domain will be coming to you.

Consider the leaveDomains SLs and the balancing at the same time.

 **/

#include <avtStreamlineFilter.h>
#include <math.h>
#include <visitstream.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkLineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkPointSource.h>
#include <vtkVisItStreamLine.h>
#include <vtkGlyph3D.h>

#include <vtkVisItInterpolatedVelocityField.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtDataset.h>
#include <avtDataTree.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtIntegralCurve.h>
#include <avtIVPVTKField.h>
#include <avtIVPVTKTimeVaryingField.h>
#include <avtIVPDopri5.h>
#include <avtIVPAdamsBashforth.h>
#include <avtIVPM3DC1Integrator.h>
#include <avtIVPM3DC1Field.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtStateRecorderIntegralCurve.h>
#include <avtStreamlineIC.h>
#include <avtVector.h>

#include <DebugStream.h>
#include <MemStream.h>
#include <TimingsManager.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>
#include <VisItStreamUtil.h>
#include <snprintf.h>

#ifdef PARALLEL
#include <time.h> // needed for nanosleep
#include <mpi.h>
#endif

static float random01()
{
    return (float)rand()/(float)RAND_MAX;
}

static float random_11()
{
    return (random01()*2.0) - 1.0;
}


// ****************************************************************************
//  Method: avtStreamlineFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 12:42:30 PDT 2004
//    I added coloringMethod and support for ribbons.
//
//    Hank Childs, Sat Mar  3 09:52:01 PST 2007
//    Initialized useWholeBox.
//
//    Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//    Initialize streamline direction option.
//
//    Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//    Port to new streamline infrastructure
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Kathleen Bonnell, Wed Aug 27 15:13:07 PDT 2008
//   Initialize solver.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added termination by number of steps.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Sun Mar 22 11:30:40 CDT 2009
//   Initialize specifyPoint.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Initialize seedTimeStep0 and seedTime0.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Remove radius and showStart members.
//
//   Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//   Remove data members that are being put into avtPICSFilter.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//
//   Dave Pugmire, Fri Jun 11 15:12:04 EDT 2010
//   Remove seed densities.
//
//   Hank Childs, Sun Dec  5 10:43:57 PST 2010
//   Initialize data members for warnings.
//
// ****************************************************************************

avtStreamlineFilter::avtStreamlineFilter()
{
    coloringMethod = STREAMLINE_COLOR_SPEED;
    displayMethod = STREAMLINE_DISPLAY_LINES;
    referenceTypeForDisplay = 0;

    //
    // Initialize source values.
    //
    sourceType = STREAMLINE_SOURCE_POINT;
    sampleDensity[0] = sampleDensity[1] = sampleDensity[2] = 0;
    sampleDistance[0] = sampleDistance[1] = sampleDistance[2] = 0.0;
    numSamplePoints = 0;
    randomSamples = false;
    randomSeed = 0;
    fill = false;
    useBBox = false;

    storeVelocitiesForLighting = false;
    issueWarningForMaxStepsTermination = true;
    issueWarningForStiffness = true;
    issueWarningForCriticalPoints = true;
    criticalPointThreshold = 1e-3;
}


// ****************************************************************************
//  Method: avtStreamlineFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 14:18:03 PST 2004
//    Added ribbons.
//
//    Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//    Port to new streamline infrastructure
//
//    Hank Childs, Fri Aug 22 09:41:02 PDT 2008
//    Move deletion of solver to PostExecute.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//   Remove data members that are being put into avtPICSFilter.
//
// ****************************************************************************

avtStreamlineFilter::~avtStreamlineFilter()
{
}

// ****************************************************************************
// Method:  avtStreamlineFilter::GenerateAttributeFields() const
//
// Programmer:  Dave Pugmire
// Creation:    November  5, 2010
//
// ****************************************************************************

unsigned char
avtStreamlineFilter::GenerateAttributeFields() const
{

    // need at least these three attributes
    unsigned char attr = avtStateRecorderIntegralCurve::SAMPLE_POSITION;

    if (storeVelocitiesForLighting)
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;

    switch (referenceTypeForDisplay)
    {
      case 0:  // Distance
        attr |= avtStateRecorderIntegralCurve::SAMPLE_ARCLENGTH;
        break;
      case 1:  // Time
        attr |= avtStateRecorderIntegralCurve::SAMPLE_TIME;
        break;
      case 2:  // Steps
        break;
    }

    // color scalars
    switch( coloringMethod )
    {
      case STREAMLINE_COLOR_SPEED:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VELOCITY;
        break;
      case STREAMLINE_COLOR_TIME:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_TIME;
        break;
      case STREAMLINE_COLOR_VORTICITY:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_VORTICITY;
        break;
      case STREAMLINE_COLOR_ARCLENGTH:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_ARCLENGTH;
        break;
      case STREAMLINE_COLOR_VARIABLE:
        attr |= avtStateRecorderIntegralCurve::SAMPLE_SCALAR0;
        break;
    }

    // opacity scalar
    if( !opacityVariable.empty() )
        attr |= avtStateRecorderIntegralCurve::SAMPLE_SCALAR1;

    return attr;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CreateIntegralCurve
//
//  Purpose:
//      Each derived type of avtPICSFilter must know how to create an integral
//      curve.  The streamline filter creates an avtStateRecorderIntegralCurve.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//    Hank Childs, Mon Oct  4 14:53:13 PDT 2010
//    Create an avtStreamline (not an avtStateRecorderIntegralCurve) and
//    put the termination criteria into the signature.
//
//   Dave Pugmire, Fri Nov  5 15:38:33 EDT 2010
//   Set maxSteps and historyMask.
//
// ****************************************************************************

avtIntegralCurve *
avtStreamlineFilter::CreateIntegralCurve()
{
    avtStreamlineIC *ic = new avtStreamlineIC();
    ic->maxSteps = maxSteps;
    ic->historyMask = GenerateAttributeFields();
    return ic;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::SetTermination
//
//  Purpose:
//      Sets the termination criteria for a streamline.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2010
//
// ****************************************************************************

void
avtStreamlineFilter::SetTermination(int maxSteps_, bool doDistance_,
                            double maxDistance_, bool doTime_, double maxTime_)
{
    maxSteps = maxSteps_;
    doDistance = doDistance_;
    maxDistance = maxDistance_;
    doTime = doTime_;
    maxTime = maxTime_;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CreateIntegralCurve
//
//  Purpose:
//      Each derived type of avtPICSFilter must know how to create an integral
//      curve.  The streamline filter creates an avtStateRecorderIntegralCurve.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//    Hank Childs, Mon Oct  4 14:53:13 PDT 2010
//    Create an avtStreamline (not an avtStateRecorderIntegralCurve) and
//    put the termination criteria into the signature.
//
// ****************************************************************************

avtIntegralCurve *
avtStreamlineFilter::CreateIntegralCurve( const avtIVPSolver* model,
                                          const avtIntegralCurve::Direction dir,
                                          const double& t_start,
                                          const avtVector &p_start, long ID ) 
{
    unsigned char attr = GenerateAttributeFields();

    avtStateRecorderIntegralCurve *rv = 
        new avtStreamlineIC(maxSteps, doDistance, maxDistance, doTime, maxTime,
                            attr, model, dir, t_start, p_start, ID);

    return rv;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetColoringMethod
//
// Purpose: 
//   Set the coloring method to use, which determines which auxiliary arrays
//   (if any) are also generated.
//
// Arguments:
//   m : The coloring method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 12:41:08 PDT 2004
//
// Modifications:
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Added color by secondary variable.
//   
// ****************************************************************************

void
avtStreamlineFilter::SetColoringMethod(int m, const string &var)
{
    coloringMethod = m;
    coloringVariable = var;
}

// ****************************************************************************
//  Method:  avtStreamlineFilter::SetOpacityVariable
//
//  Purpose:
//    Set the opacity variable.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineFilter::SetOpacityVariable(const std::string &var)
{
    opacityVariable = var;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetDisplayMethod
//
// Purpose: 
//   Sets the streamline display method.
//
// Arguments:
//   d : The display method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 14:18:47 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetDisplayMethod(int d)
{
    displayMethod = d;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPointSource
//
// Purpose: 
//   Sets the streamline point source.
//
// Arguments:
//   pt : The location of the point.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:36 PDT 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPointSource(const double *p)
{
    sourceType = STREAMLINE_SOURCE_POINT;
    points[0].set(p);
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetLineSource
//
// Purpose: 
//   Sets the source line endpoints.
//
// Arguments:
//   pt1 : The first line endpoint.
//   pt2 : The second line endpoint.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:59 PDT 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtStreamlineFilter::SetLineSource(const double *p0, const double *p1,
                                   int den, bool rand, int seed, int numPts)
{
    sourceType = STREAMLINE_SOURCE_LINE;
    points[0].set(p0);
    points[1].set(p1);
    
    numSamplePoints = numPts;
    sampleDensity[0] = den;
    sampleDensity[1] = 0;
    sampleDensity[2] = 0;
    
    randomSamples = rand;
    randomSeed = seed;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPlaneSource
//
// Purpose: 
//   Sets the plane source information.
//
// Arguments:
//   O : The plane origin.
//   N : The plane normal.
//   U : The plane up axis.
//   R : The plane radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:59:47 PDT 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPlaneSource(double O[3], double N[3], double U[3],
                                    int den1, int den2, double dist1, double dist2,
                                    bool f, 
                                    bool rand, int seed, int numPts)
{
    sourceType = STREAMLINE_SOURCE_PLANE;
    points[0].set(O);
    vectors[0].set(N);
    vectors[1].set(U);
    
    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = 0;
    sampleDistance[0] = dist1;
    sampleDistance[1] = dist2;
    sampleDistance[2] = 0.0;
    numSamplePoints = numPts;

    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetCircleSource
//
// Purpose: 
//   Sets the plane source information.
//
// Arguments:
//   O : The plane origin.
//   N : The plane normal.
//   U : The plane up axis.
//   R : The plane radius.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jun 10 10:44:02 EDT 2010
//
// Modifications:
//
//   
// ****************************************************************************

void
avtStreamlineFilter::SetCircleSource(double O[3], double N[3], double U[3], double r,
                                     int den1, int den2,
                                     bool f, bool rand, int seed, int numPts)
{
    sourceType = STREAMLINE_SOURCE_CIRCLE;
    points[0].set(O);
    vectors[0].set(N);
    vectors[1].set(U);
    
    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = 0;
    sampleDistance[0] = r;
    sampleDistance[1] = 0.0;
    sampleDistance[2] = 0.0;
    numSamplePoints = numPts;

    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetSphereSource
//
// Purpose: 
//   Sets the sphere source information.
//
// Arguments:
//   O : The sphere origin.
//   R : The sphere radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:00:34 PST 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtStreamlineFilter::SetSphereSource(double O[3], double R,
                                     int den1, int den2, int den3,
                                     bool f, bool rand, int seed, int numPts)
{
    sourceType = STREAMLINE_SOURCE_SPHERE;
    points[0].set(O);
    sampleDistance[0] = R;
    sampleDistance[1] = 0.0;
    sampleDistance[2] = 0.0;
    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = den3;

    numSamplePoints = numPts;
    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetBoxSource
//
// Purpose: 
//   Sets the box source information.
//
// Arguments:
//   E : The box extents.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:01:11 PST 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtStreamlineFilter::SetBoxSource(double E[6], bool wholeBox,
                                  int den1, int den2, int den3,
                                  bool f, bool rand, int seed, int numPts)
{
    sourceType = STREAMLINE_SOURCE_BOX;
    points[0].set(E[0], E[2], E[4]);
    points[1].set(E[1], E[3], E[5]);

    sampleDensity[0] = den1;
    sampleDensity[1] = den2;
    sampleDensity[2] = den3;

    useBBox = wholeBox;
    numSamplePoints = numPts;
    randomSamples = rand;
    randomSeed = seed;
    fill = f;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPointListSource
//
// Purpose: 
//   Sets the streamline point list source.
//
// Arguments:
//   ptlist : A list of points
//
// Programmer: Hank Childs
// Creation:   May 3, 2009
//
// Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPointListSource(const std::vector<double> &ptList)
{
    sourceType = STREAMLINE_SOURCE_POINT_LIST;
    listOfPoints = ptList;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SeedInfoString
//
// Purpose: 
//   Get info string on seeds.
//
// Arguments:
//   
//
// Programmer: Dave Pugmire
// Creation:   Fri Apr  3 09:18:03 EDT 2009
//
// Modifications:
//   
//   Hank Childs, Sun May  3 12:42:38 CDT 2009
//   Add case for point lists.
//
//   Dave Pugmire (for Christoph Garth), Wed Jan 20 09:28:59 EST 2010
//   Add circle source.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources. 
//
// ****************************************************************************

std::string
avtStreamlineFilter::SeedInfoString() const
{
    char buff[256];
    if (sourceType == STREAMLINE_SOURCE_POINT)
        sprintf(buff, "Point [%g %g %g]", 
                points[0].x, points[0].y, points[0].z);
    else if (sourceType == STREAMLINE_SOURCE_LINE)
        sprintf(buff, "Line [%g %g %g] [%g %g %g] D: %d",
                points[0].x, points[0].y, points[0].z,
                points[1].x, points[1].y, points[1].z, sampleDensity[0]);
    else if (sourceType == STREAMLINE_SOURCE_PLANE)
        sprintf(buff, "Plane O[%g %g %g] N[%g %g %g] D: %d %d",
                points[0].x, points[0].y, points[0].z,
                vectors[0].x, vectors[0].y, vectors[0].z,
                sampleDensity[0], sampleDensity[1]);
    else if (sourceType == STREAMLINE_SOURCE_SPHERE)
        sprintf(buff, "Sphere [%g %g %g] %g D: %d %d",
                points[0].x, points[0].y, points[0].z, sampleDistance[0],
                sampleDensity[0], sampleDensity[1]);
    else if (sourceType == STREAMLINE_SOURCE_BOX)
        sprintf(buff, "Box [%g %g] [%g %g] [%g %g] D: %d %d %d",
                points[0].x, points[1].x,
                points[0].y, points[1].y,
                points[0].z, points[1].z,
                sampleDensity[0], sampleDensity[1], sampleDensity[2]);
    else if (sourceType == STREAMLINE_SOURCE_CIRCLE)
        sprintf(buff, "Cirlce O[%g %g %g] N[%g %g %g] R: %g D: %d %d",
                points[0].x, points[0].y, points[0].z,
                vectors[0].x, vectors[0].y, vectors[0].z,
                sampleDistance[0],
                sampleDensity[0], sampleDensity[1]);
    else if (sourceType == STREAMLINE_SOURCE_POINT_LIST)
        strcpy(buff, "Point list [points not printed]");
    else
        sprintf(buff, "%s", "UNKNOWN");
    
    string str = buff;
    return str;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::PostExecute
//
//  Purpose:
//      Gets the variable extents and sets them.
//
//  Programmer: Hank Childs
//  Creation:   March 3, 2007
//
//  Modifications:
//
//    Hank Childs, Fri Aug 22 09:40:21 PDT 2008
//    Move the deletion of the solver here.
//
//    Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//    Remove data members that are being put into avtPICSFilter.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
//    Hank Childs, Sun Aug 29 19:26:47 PDT 2010
//    Set the extents in more situations.
//
// ****************************************************************************

void
avtStreamlineFilter::PostExecute(void)
{
    avtPICSFilter::PostExecute();

    if (coloringMethod == STREAMLINE_COLOR_VORTICITY ||
        coloringMethod == STREAMLINE_COLOR_SPEED ||
        coloringMethod == STREAMLINE_COLOR_ARCLENGTH ||
        coloringMethod == STREAMLINE_COLOR_TIME ||
        coloringMethod == STREAMLINE_COLOR_ID ||
        coloringMethod == STREAMLINE_COLOR_VARIABLE)
    {
        double range[2];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetDataExtents(ds, range, "colorVar");

        avtExtents *e;
        e = GetOutput()->GetInfo().GetAttributes()
                                            .GetThisProcsOriginalDataExtents();
        e->Merge(range);
        e = GetOutput()->GetInfo().GetAttributes()
                                           .GetThisProcsActualDataExtents();
        e->Merge(range);
    }
}


// ****************************************************************************
//  Method: avtStreamlineFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Allows the filter to change its output's data object information, which
//      is a description of the data object.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
//    Brad Whitlock, Mon Jan 3 13:31:11 PST 2005
//    Set the flag that prevents normals from being generated if we're
//    displaying the streamlines as lines.
//
//    Hank Childs, Sat Mar  3 11:02:33 PST 2007
//    Make sure we have a valid active variable before setting its dimension.
//
//    Hank Childs, Sun Mar  9 07:47:05 PST 2008
//    Call the base class' method as well.
//
//    Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//    Call the new base class' (avtPICSFilter) method.
//
// ****************************************************************************

void
avtStreamlineFilter::UpdateDataObjectInfo(void)
{
    avtPICSFilter::UpdateDataObjectInfo();

    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    if(displayMethod == STREAMLINE_DISPLAY_LINES)
        GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    atts.SetTopologicalDimension(1);
    if (! atts.ValidVariable("colorVar"))
    {
        atts.AddVariable("colorVar");
        atts.SetActiveVariable("colorVar");
        atts.SetVariableDimension(1);
        atts.SetCentering(AVT_NODECENT);
    }
}

typedef struct
{
    avtVector pt;
    int domain, id;
} seedPtDomain;

static int comparePtDom(const void *a, const void *b)
{
    seedPtDomain *pdA = (seedPtDomain *)a, *pdB = (seedPtDomain *)b;
    
    if (pdA->domain < pdB->domain)
        return -1;
    else if (pdA->domain > pdB->domain)
        return 1;
    return 0;
}


static float
randMinus1_1()
{
    float r = 2.0 * ((float)rand() / (float)RAND_MAX);
    return (r-1.0);
}


// ****************************************************************************
//  Method: avtStreamlineFilter::GetInitialLocations
//
//  Purpose:
//      Get the seed points out of the attributes.
//
//  Programmer: Hank Childs (harvested from GetStreamlinesFromInitialSeeds by
//                           David Pugmire)
//  Creation:   June 5, 2008
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Nov 10 09:20:06 EST 2010
//   Handle 2D datasets better.
//
// ****************************************************************************

std::vector<avtVector>
avtStreamlineFilter::GetInitialLocations(void)
{
    std::vector<avtVector> seedPts;
    
    if (randomSamples)
        srand(randomSeed);

    // Add seed points based on the source.
    if(sourceType == STREAMLINE_SOURCE_POINT)
        GenerateSeedPointsFromPoint(seedPts);
    else if(sourceType == STREAMLINE_SOURCE_LINE)
        GenerateSeedPointsFromLine(seedPts);
    else if(sourceType == STREAMLINE_SOURCE_PLANE)
        GenerateSeedPointsFromPlane(seedPts);
    else if(sourceType == STREAMLINE_SOURCE_SPHERE)
        GenerateSeedPointsFromSphere(seedPts);
    else if(sourceType == STREAMLINE_SOURCE_BOX)
        GenerateSeedPointsFromBox(seedPts);
    else if(sourceType == STREAMLINE_SOURCE_CIRCLE)
        GenerateSeedPointsFromCircle(seedPts);
    else if(sourceType == STREAMLINE_SOURCE_POINT_LIST)
        GenerateSeedPointsFromPointList(seedPts);

    //Check for 2D input.
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        vector<avtVector>::iterator it;
        for (it = seedPts.begin(); it != seedPts.end(); it++)
            (*it)[2] = 0.0f;
    }

    return seedPts;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromPoint
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromPoint(std::vector<avtVector> &pts)
{
    pts.push_back(points[0]);
}


// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromLine
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromLine(std::vector<avtVector> &pts)
{
    avtVector v = points[1]-points[0];

    if (randomSamples)
    {
        for (int i = 0; i < numSamplePoints; i++)
        {
            avtVector p = points[0] + random01()*v;
            pts.push_back(p);
        }
    }
    else
    {
        double t = 0.0, dt;
        if (sampleDensity[0] == 1)
        {
            t = 0.5;
            dt = 0.5;
        }
        else
            dt = 1.0/(double)(sampleDensity[0]-1);
    
        for (int i = 0; i < sampleDensity[0]; i++)
        {
            avtVector p = points[0] + t*v;
            pts.push_back(p);
            t = t+dt;
        }
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromPlane
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Fix the centering.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromPlane(std::vector<avtVector> &pts)
{
    //Generate all points on a plane at the origin with Normal=Z.
    //Use the following matrix to xform them to the user specified plane.
    
    avtVector X0(1,0,0), Y0(0,1,0), Z0(0,0,1), C0(0,0,0);
    avtVector Y1=vectors[1], Z1=vectors[0], C1=points[0];

    avtVector X1 = Y1.cross(Z1);
    avtMatrix m = avtMatrix::CreateFrameToFrameConversion(X1, Y1, Z1, C1,
                                                          X0, Y0, Z0, C0);
    
    float x0 = -(sampleDistance[0]/2.0);
    float y0 = -(sampleDistance[0]/2.0);
    float x1 = (sampleDistance[0]/2.0);
    float y1 = (sampleDistance[0]/2.0);

    if (randomSamples)
    {
        float dX = x1-x0, dY = y1-y0;
        if (!fill)
        {
            // There are 4 sides. Create a vector that we will shuffle each time.
            vector<int> sides(4);
            for (int i = 0; i < 4; i++)
                sides[i] = i;

            avtVector p;
            for (int i = 0; i < numSamplePoints; i++)
            {
                random_shuffle(sides.begin(), sides.end());
                if (sides[0] == 0) //Bottom side.
                    p.set(x0 + random01()*dX, y0, 0.0f);
                else if (sides[0] == 1) //Top side.
                    p.set(x0 + random01()*dX, y1, 0.0f);
                else if (sides[0] == 2) //Right side.
                    p.set(x0, y0+random01()*dY, 0.0f);
                else //Left side.
                    p.set(x1, y0+random01()*dY, 0.0f);
                
                p = m*p;
                pts.push_back(p);
            }
        }
        else
        {
            for (int i = 0; i < numSamplePoints; i++)
            {
                avtVector p(x0 + random01()*dX,
                            y0 + random01()*dY,
                            0.0);
                p = m*p;
                pts.push_back(p);
            }
        }
    }
    else
    {
        float dX = (x1-x0)/(float)(sampleDensity[0]-1), dY = (y1-y0)/(float)(sampleDensity[1]-1);
        for (int x = 0; x < sampleDensity[0]; x++)
        {
            for (int y = 0; y < sampleDensity[1]; y++)
            {
                if (!fill &&
                    !((x == 0 || x == sampleDensity[0]-1) ||
                      (y == 0 || y == sampleDensity[1]-1)))
                {
                    continue;
                }
                
                avtVector p(x0+((float)x*dX), 
                            y0+((float)y*dY),
                            0.0);

                p = m*p;
                pts.push_back(p);
            }
        }
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromCircle
//
//  Purpose:
//      
//
//  Programmer: Christoph Garth
//  Creation:   January 20, 2010
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Add circle center for interior sampling.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromCircle(std::vector<avtVector> &pts)
{
    //Generate all points on a plane at the origin with Normal=Z.
    //Use the following matrix to xform them to the user specified plane.
    
    avtVector X0(1,0,0), Y0(0,1,0), Z0(0,0,1), C0(0,0,0);
    avtVector Y1=vectors[1], Z1=vectors[0], C1=points[0];
    avtVector X1 = Y1.cross(Z1);
    avtMatrix m = avtMatrix::CreateFrameToFrameConversion(X1, Y1, Z1, C1,
                                                          X0, Y0, Z0, C0);
    float R = sampleDistance[0];
    if (randomSamples)
    {
        if (fill)
        {
            int n = numSamplePoints;
            while (n)
            {
                //Randomly sample a unit square, check if pt in circle.
                float x = random_11(), y = random_11();
                if (x*x + y*y <= 1.0) //inside the circle!
                {
                    avtVector p = m * avtVector(x*R, y*R, 0.0);
                    pts.push_back(p);
                    n--;
                }
            }
        }
        else
        {
            float TWO_PI = M_PI*2.0f;
            for (int i = 0; i < numSamplePoints; i++)
            {
                float theta = random01() * TWO_PI;
                avtVector p(cos(theta)*R, sin(theta)*R, 0.0);
                p = m*p;
                pts.push_back(p);
            }
        }
    }
    else
    {
        float TWO_PI = M_PI*2.0f;
        if (fill)
        {
            float dTheta = TWO_PI / (float)sampleDensity[0];
            float dR = R/(float)sampleDensity[1];

            float theta = 0.0;                
            for (int i = 0; i < sampleDensity[0]; i++)
            {
                float r = dR;
                for (int j = 0; j < sampleDensity[1]; j++)
                {
                    avtVector p(cos(theta)*r, sin(theta)*r, 0.0);
                    p = m*p;
                    pts.push_back(p);
                    r += dR;
                }
                theta += dTheta;
                pts.push_back(points[0]);
            }
        }
        else
        {
            for (int i = 0; i < sampleDensity[0]; i++)
            {
                float t = (TWO_PI*i) / (float)sampleDensity[0];
                avtVector p(cos(t)*R, sin(t)*R, 0.0);
                p = m*p;
                pts.push_back(p);
            }
        }
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromSphere
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Jun 23 16:44:36 EDT 2010
//   Bug fix for random sampling on a sphere. Implment uniform interior sampling.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromSphere(std::vector<avtVector> &pts)
{
    float R = sampleDistance[0];
    if (randomSamples)
    {
        if (fill)
        {
            int n = numSamplePoints;
            while (n)
            {
                //Randomly sample a unit cube, check if pt in sphere.
                float x = random_11(), y = random_11(), z = random_11();
                if (x*x + y*y  + z*z <= 1.0) //inside the sphere!
                {
                    avtVector p = avtVector(x*R, y*R, z*R) + points[0];
                    pts.push_back(p);
                    n--;
                }
            }
        }
        else
        {
            float TWO_PI = M_PI*2.0f;
            for (int i = 0; i < numSamplePoints; i++)
            {
                float theta = random01()*TWO_PI;
                float u = random_11();
                float x = sqrt(1.0-(u*u));
                avtVector p(cos(theta)*x, sin(theta)*x, u);
                p.normalize();
                p *= R;
                pts.push_back(p+points[0]);
            }
        }
    }
    else
    {
        vtkSphereSource* sphere = vtkSphereSource::New();
        sphere->SetCenter(points[0].x, points[0].y, points[0].z);
        sphere->SetRadius(R);
        sphere->SetLatLongTessellation(1);
        double t = double(30 - sampleDensity[0]) / 29.;
        double angle = t * 3. + (1. - t) * 30.;
        sphere->SetPhiResolution(int(angle));

        t = double(30 - sampleDensity[1]) / 29.;
        angle = t * 3. + (1. - t) * 30.;
        sphere->SetThetaResolution(int(angle));

        if (fill)
        {
            float dR = R/(float)sampleDensity[2];
            float r = dR;
            for (int i = 0; i < sampleDensity[2]; i++)
            {
                sphere->SetRadius(r);
                sphere->Update();

                for (int j = 0; j < sphere->GetOutput()->GetNumberOfPoints(); j++)
                {
                    double *pt = sphere->GetOutput()->GetPoint(j);
                    avtVector p(pt[0], pt[1], pt[2]);
                    pts.push_back(p);
                }
                r = r+dR;
            }
            //Add center, R=0 sample.
            pts.push_back(points[0]);

        }
        else //LAT-LONG
        {
            sphere->Update();
            for (int i = 0; i < sphere->GetOutput()->GetNumberOfPoints(); i++)
            {
                double *pt = sphere->GetOutput()->GetPoint(i);
                avtVector p(pt[0], pt[1], pt[2]);
                pts.push_back(p);
            }
        }
        
        sphere->Delete();
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromBox
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Dave Pugmire, Wed Nov 10 09:20:32 EST 2010
//   If box sampling is 1, use the mid value.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromBox(std::vector<avtVector> &pts)
{
    if (useBBox)
    {
        double bbox[6];
        intervalTree->GetExtents(bbox);
        points[0].set(bbox[0], bbox[2], bbox[4]);
        points[1].set(bbox[1], bbox[3], bbox[5]);
    }

    avtVector diff = points[1]-points[0];

    if (randomSamples)
    {
        if (fill)
        {
            for (int i = 0; i < numSamplePoints; i++)
            {
                avtVector p(points[0].x + (diff.x * random01()),
                            points[0].y + (diff.y * random01()),
                            points[0].z + (diff.z * random01()));
                pts.push_back(p);
            }
        }
        else
        {
            // There are 6 faces. Create a vector that we will shuffle each time.
            vector<int> faces(6);
            for (int i = 0; i < 6; i++)
                faces[i] = i;
            
            avtVector p;
            for (int i = 0; i < numSamplePoints; i++)
            {
                random_shuffle(faces.begin(), faces.end());
                if (faces[0] == 0) //X=0 face.
                    p.set(points[0].x,
                          points[0].y + (diff.y * random01()),
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 1) //X=1 face.
                    p.set(points[1].x,
                          points[0].y + (diff.y * random01()),
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 2) //Y=0 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[0].y,
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 3) //Y=1 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[1].y,
                          points[0].z + (diff.z * random01()));
                else if (faces[0] == 4) //Z=0 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[0].y + (diff.y * random01()),
                          points[0].z);
                else if (faces[0] == 5) //Z=1 face.
                    p.set(points[0].x + (diff.x * random01()),
                          points[0].y + (diff.y * random01()),
                          points[1].z);
                pts.push_back(p);
            }
        }
    }
    else
    {
        // If sample density is 1, sample at the mid point.
        diff.x /= (sampleDensity[0] == 1 ? 2.0 : (sampleDensity[0]-1));
        diff.y /= (sampleDensity[1] == 1 ? 2.0 : (sampleDensity[1]-1));
        diff.z /= (sampleDensity[2] == 1 ? 2.0 : (sampleDensity[2]-1));

        if (sampleDensity[0] == 1)
            points[0].x += diff.x;
        if (sampleDensity[1] == 1)
            points[0].y += diff.y;
        if (sampleDensity[2] == 1)
            points[0].z += diff.z;

        if (fill)
        {
            for (int i = 0; i < sampleDensity[0]; i++)
                for (int j = 0; j < sampleDensity[1]; j++)
                    for (int k = 0; k < sampleDensity[2]; k++)
                    {
                        avtVector p(points[0].x + i*diff.x,
                                    points[0].y + j*diff.y,
                                    points[0].z + k*diff.z);
                        pts.push_back(p);
                    }
        }
        else
        {
            for (int i = 0; i < sampleDensity[0]; i++)
                for (int j = 0; j < sampleDensity[1]; j++)
                    for (int k = 0; k < sampleDensity[2]; k++)
                    {
                        if ((i == 0 || i == sampleDensity[0]-1) ||
                            (j == 0 || j == sampleDensity[1]-1) ||
                            (k == 0 || k == sampleDensity[2]-1))
                        {
                            avtVector p(points[0].x + i*diff.x,
                                        points[0].y + j*diff.y,
                                        points[0].z + k*diff.z);
                            pts.push_back(p);
                        }
                    }
        }
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::GenerateSeedPointsFromPointList
//
//  Purpose:
//      
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromPointList(std::vector<avtVector> &pts)
{
    if ((listOfPoints.size() % 3) != 0)
    {
        EXCEPTION1(VisItException, "The seed points for the streamline "
                   "are incorrectly specified.  The number of values must be a "
                   "multiple of 3 (X, Y, Z).");
    }
    int npts = listOfPoints.size() / 3;
    for (int i = 0 ; i < npts ; i++)
    {
        avtVector p(listOfPoints[3*i], listOfPoints[3*i+1], listOfPoints[3*i+2]);
        pts.push_back(p);
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::ModifyContract
//
//  Purpose:
//      Tell the contract we need ghost data.
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2008
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 14:09:03 PDT 2008
//    Remove "colorVar" and replace it with the gradient variable.  This is 
//    a trick because the streamline plot requested "colorVar", which is the
//    variable it wants to color by.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.  
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Sat Jun  5 19:01:55 CDT 2010
//   Strip out the pieces that belong in PICS.
//
//   Hank Childs, Sun Nov 28 05:37:44 PST 2010
//   Always add necessary secondary variables, regardless of whether there
//   is "colorVar" in the contract.
//
// ****************************************************************************

avtContract_p
avtStreamlineFilter::ModifyContract(avtContract_p in_contract0)
{
    avtContract_p in_contract = avtPICSFilter::ModifyContract(in_contract0);

    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    avtDataRequest_p out_dr = NULL;

    if (strcmp(in_dr->GetVariable(), "colorVar") == 0 ||
        opacityVariable != "")
    {
        // The avtStreamlinePlot requested "colorVar", so remove that from the
        // contract now.
        out_dr = new avtDataRequest(in_dr,in_dr->GetOriginalVariable());
    }
    else
        out_dr = new avtDataRequest(in_dr);

    if (coloringMethod == STREAMLINE_COLOR_VARIABLE)
        out_dr->AddSecondaryVariable(coloringVariable.c_str());

    if (opacityVariable != "")
        out_dr->AddSecondaryVariable(opacityVariable.c_str());

    avtContract_p out_contract;
    if ( *out_dr )
        out_contract = new avtContract(in_contract, out_dr);
    else
        out_contract = new avtContract(in_contract);

    return out_contract;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::GetFieldForDomain
//
//  Purpose:
//      Calls avtPICSFilter::GetFieldForDomain and enables scalar 
//      variables according to coloringVariable and opacityVariable.
//
//  Programmer: Christoph Garth
//  Creation:   July 14, 2010
//
// ****************************************************************************

avtIVPField* 
avtStreamlineFilter::GetFieldForDomain(const DomainType& dom, vtkDataSet* ds)
{
    avtIVPField* field = avtPICSFilter::GetFieldForDomain( dom, ds );

    if( coloringMethod == STREAMLINE_COLOR_VARIABLE && 
        !coloringVariable.empty() )
        field->SetScalarVariable( 0, coloringVariable );

    if( !opacityVariable.empty() )
        field->SetScalarVariable( 1, opacityVariable );

    return field;
}
