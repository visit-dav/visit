/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#define INIT_POINT(p, a, b, c) (p)[0] = a; (p)[1] = b; (p)[2] = c;

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
// ****************************************************************************

avtStreamlineFilter::avtStreamlineFilter()
{
    pointDensity1 = 1;
    pointDensity2 = 1;
    pointDensity3 = 1;
    coloringMethod = STREAMLINE_COLOR_SPEED;
    displayMethod = STREAMLINE_DISPLAY_LINES;

    //
    // Initialize source values.
    //
    sourceType = STREAMLINE_SOURCE_POINT;
    INIT_POINT(pointSource, 0., 0., 0.);
    INIT_POINT(lineStart, 0., 0., 0.);
    INIT_POINT(lineEnd, 1., 0., 0.);
    INIT_POINT(planeOrigin, 0., 0., 0.);
    INIT_POINT(planeNormal, 0., 0., 1.);
    INIT_POINT(planeUpAxis, 0., 1., 0.);
    planeRadius = 1.4142136;
    INIT_POINT(sphereOrigin, 0., 0., 0.);
    sphereRadius = 1.;
    INIT_POINT(boxExtents, 0., 1., 0.);
    INIT_POINT(boxExtents+3, 1., 0., 1.);
    useWholeBox = false;
    intersectObj = NULL;
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
    if (intersectObj)
        intersectObj->Delete();
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
// ****************************************************************************

avtIntegralCurve *
avtStreamlineFilter::CreateIntegralCurve(void)
{
    avtStateRecorderIntegralCurve *rv = new avtStateRecorderIntegralCurve();
    if (intersectObj)
        rv->SetIntersectionObject(intersectObj);

    if (coloringVariable != "")
        rv->scalars.push_back(coloringVariable);
    if (opacityVariable != "")
        rv->scalars.push_back(opacityVariable);

    return rv;
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
// ****************************************************************************

avtIntegralCurve *
avtStreamlineFilter::CreateIntegralCurve(const avtIVPSolver* model,
                                         const double& t_start,
                                         const avtVector &p_start, int ID) 
{
    avtStateRecorderIntegralCurve *rv = new avtStateRecorderIntegralCurve(
                                           model, t_start, p_start, ID);
   
    avtStateRecorderIntegralCurve::ScalarValueType scalarVal =  
                                         avtStateRecorderIntegralCurve::NONE;
    if (coloringMethod == STREAMLINE_COLOR_SPEED)
        scalarVal = avtStateRecorderIntegralCurve::SPEED;
    else if (coloringMethod == STREAMLINE_COLOR_VORTICITY)
        scalarVal = avtStateRecorderIntegralCurve::VORTICITY;
    else if (coloringMethod == STREAMLINE_COLOR_VARIABLE)
        scalarVal = avtStateRecorderIntegralCurve::SCALAR_VARIABLE;

    if (displayMethod == STREAMLINE_DISPLAY_RIBBONS)
        scalarVal = (avtStateRecorderIntegralCurve::ScalarValueType)
                        (scalarVal | avtStateRecorderIntegralCurve::VORTICITY);

    rv->SetScalarValueType(scalarVal);

    if (coloringVariable != "")
        rv->scalars.push_back(coloringVariable);
    if (opacityVariable != "")
        rv->scalars.push_back(opacityVariable);

    if (intersectObj)
        rv->SetIntersectionObject(intersectObj);

    return rv;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetIntersectionObject
//
// Purpose: 
//   Sets the intersection object.
//
// Arguments:
//   obj : Intersection object.
//
// Programmer: Dave Pugmire
// Creation:   11 August 2009
//
// Modifications:
//
// ****************************************************************************

void
avtStreamlineFilter::SetIntersectionObject(vtkObject *obj)
{
    if (obj)
    {
        intersectObj = obj;
        intersectObj->Register(NULL);
    }
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
// Method: avtStreamlineFilter::SetSourceType
//
// Purpose: 
//   Sets the type of source to be used in the streamline process.
//
// Arguments:
//   t : The new streamline source type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:04 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetSourceType(int t)
{
    sourceType = t;
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
// ****************************************************************************

void
avtStreamlineFilter::SetPointSource(double pt[3])
{
    pointSource[0] = pt[0];
    pointSource[1] = pt[1];
    pointSource[2] = pt[2];
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
// ****************************************************************************

void
avtStreamlineFilter::SetLineSource(double pt[3], double pt2[3])
{
    for(int i = 0; i < 3; ++i)
    {
        lineStart[i] = pt[i];
        lineEnd[i] = pt2[i];
    }
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
// ****************************************************************************

void
avtStreamlineFilter::SetPlaneSource(double O[3], double N[3], double U[3], 
                                    double R)
{
    for(int i = 0; i < 3; ++i)
    {
        planeOrigin[i] = O[i];
        planeNormal[i] = N[i];
        planeUpAxis[i] = U[i];
    }
    planeRadius = R;
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
// ****************************************************************************

void
avtStreamlineFilter::SetSphereSource(double O[3], double R)
{
    sphereOrigin[0] = O[0];
    sphereOrigin[1] = O[1];
    sphereOrigin[2] = O[2];
    sphereRadius = R;
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
// ****************************************************************************

void
avtStreamlineFilter::SetBoxSource(double E[6])
{
    for(int i = 0; i < 6; ++i)
        boxExtents[i] = E[i];
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
// ****************************************************************************

void
avtStreamlineFilter::SetPointListSource(const std::vector<double> &ptList)
{
    pointList = ptList;
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
// ****************************************************************************

std::string
avtStreamlineFilter::SeedInfoString() const
{
    char buff[256];
    if (sourceType == STREAMLINE_SOURCE_POINT)
        sprintf(buff, "Point [%g %g %g]", 
                pointSource[0], pointSource[1], pointSource[2]);
    else if (sourceType == STREAMLINE_SOURCE_LINE)
        sprintf(buff, "Line [%g %g %g] [%g %g %g] D: %d", 
                lineStart[0], lineStart[1], lineStart[2],
                lineEnd[0], lineEnd[1], lineEnd[2], pointDensity1);
    else if (sourceType == STREAMLINE_SOURCE_PLANE)
        sprintf(buff, "Plane O[%g %g %g] N[%g %g %g] R: %g D: %d %d",
                planeOrigin[0], planeOrigin[1], planeOrigin[2],
                planeNormal[0], planeNormal[1], planeNormal[2],
                planeRadius, pointDensity1, pointDensity2);
    else if (sourceType == STREAMLINE_SOURCE_SPHERE)
        sprintf(buff, "Sphere [%g %g %g] %g D: %d %d",
                sphereOrigin[0],sphereOrigin[1],sphereOrigin[2],
                sphereRadius, pointDensity1, pointDensity2);
    else if (sourceType == STREAMLINE_SOURCE_BOX)
        sprintf(buff, "Box [%g %g] [%g %g] [%g %g] D: %d %d %d",
                boxExtents[0], boxExtents[1],
                boxExtents[2], boxExtents[3],
                boxExtents[4], boxExtents[5],
                pointDensity1, pointDensity2, pointDensity3);
    else if (sourceType == STREAMLINE_SOURCE_CIRCLE)
        sprintf(buff, "Cirlce O[%g %g %g] N[%g %g %g] R: %g D: %d %d",
                planeOrigin[0], planeOrigin[1], planeOrigin[2],
                planeNormal[0], planeNormal[1], planeNormal[2],
                planeRadius, pointDensity1, pointDensity2);    
    else if (sourceType == STREAMLINE_SOURCE_POINT_LIST)
        strcpy(buff, "Point list [points not printed]");
    else
        sprintf(buff, "%s", "UNKNOWN");
    
    string str = buff;
    return str;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPointDensity
//
// Purpose: 
//   Sets the point density used for streamlines. The meaning depends on the
//   source type.
//
// Arguments:
//   den : The new point density.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:03:12 PST 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPointDensity(int den)
{
    pointDensity1 = (den > 0) ? den : 1;
    pointDensity2 = pointDensity1;
    pointDensity3 = pointDensity1;
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
// ****************************************************************************

void
avtStreamlineFilter::PostExecute(void)
{
    avtPICSFilter::PostExecute();

    if (coloringMethod == STREAMLINE_COLOR_VORTICITY ||
        coloringMethod == STREAMLINE_COLOR_SPEED ||
        coloringMethod == STREAMLINE_COLOR_VARIABLE)
    {
        double range[2];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetDataExtents(ds, range, "colorVar");

        avtExtents *e;
        e = GetOutput()->GetInfo().GetAttributes()
                                            .GetCumulativeTrueDataExtents();
        e->Merge(range);
        e = GetOutput()->GetInfo().GetAttributes()
                                           .GetCumulativeCurrentDataExtents();
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
// ****************************************************************************

std::vector<avtVector>
avtStreamlineFilter::GetInitialLocations(void)
{
    std::vector<avtVector> seedPts;

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
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromPoint(std::vector<avtVector> &pts)
{
    double z0 = (dataSpatialDimension > 2) ? pointSource[2] : 0.0;
    avtVector pt(pointSource[0], pointSource[1], z0);
    pts.push_back(pt);
}



void
avtStreamlineFilter::GenerateSeedPointsFromLine(std::vector<avtVector> &pts)
{
    vtkLineSource* line = vtkLineSource::New();
    double z0 = (dataSpatialDimension > 2) ? lineStart[2] : 0.;
    double z1 = (dataSpatialDimension > 2) ? lineEnd[2] : 0.;
    line->SetPoint1(lineStart[0], lineStart[1], z0);
    line->SetPoint2(lineEnd[0], lineEnd[1], z1);
    line->SetResolution(pointDensity1);
    line->Update();

    for (int i = 0; i< line->GetOutput()->GetNumberOfPoints(); i++)
    {
        double *pt = line->GetOutput()->GetPoint(i);
        avtVector p(pt[0], pt[1], pt[2]);
        pts.push_back(p);
    }
    line->Delete();
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
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromPlane(std::vector<avtVector> &pts)
{
    vtkPlaneSource* plane = vtkPlaneSource::New();
    plane->SetXResolution(pointDensity1);
    plane->SetYResolution(pointDensity2);
    avtVector O(planeOrigin), U(planeUpAxis), N(planeNormal);
    
    U.normalize();
    N.normalize();
    if(dataSpatialDimension <= 2)
        N = avtVector(0.,0.,1.);
    // Determine the right vector.
    avtVector R(U % N);
    R.normalize();
    plane->SetOrigin(O.x, O.y, O.z);
    avtVector P1(U * (2./1.414214) * planeRadius + O);
    avtVector P2(R * (2./1.414214) * planeRadius + O);
    plane->SetPoint2(P1.x, P1.y, P1.z);
    plane->SetPoint1(P2.x, P2.y, P2.z);
    plane->SetNormal(N.x, N.y, N.z);
    plane->SetCenter(O.x, O.y, O.z);
    plane->SetResolution(pointDensity1, pointDensity2);
    plane->Update();

    for (int i = 0; i< plane->GetOutput()->GetNumberOfPoints(); i++)
    {
        double *pt = plane->GetOutput()->GetPoint(i);
        avtVector p(pt[0], pt[1], pt[2]);
        pts.push_back(p);
    }
    plane->Delete();
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
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromCircle(std::vector<avtVector> &pts)
{
    avtVector O(planeOrigin), U(planeUpAxis), N(planeNormal);
    
    U.normalize();
    N.normalize();
    if(dataSpatialDimension <= 2)
        N = avtVector(0.,0.,1.);
        
    // Determine the right vector.
    avtVector R(U % N);
    R.normalize();

    for (int i = 0; i<pointDensity1; i++)
    {
        double t = (6.28318531*i) / pointDensity1;

        avtVector p = planeRadius * (cos(t) * U + sin(t) * R) + O;
        pts.push_back(p);
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
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromSphere(std::vector<avtVector> &pts)
{
    vtkSphereSource* sphere = vtkSphereSource::New();
    sphere->SetCenter(sphereOrigin[0], sphereOrigin[1], sphereOrigin[2]);
    sphere->SetRadius(sphereRadius);
    sphere->SetLatLongTessellation(1);
    double t = double(30 - pointDensity1) / 29.;
    double angle = t * 3. + (1. - t) * 30.;
    sphere->SetPhiResolution(int(angle));
    sphere->SetThetaResolution(int(angle));

    sphere->Update();
    for (int i = 0; i < sphere->GetOutput()->GetNumberOfPoints(); i++)
    {
        double *pt = sphere->GetOutput()->GetPoint(i);
        avtVector p(pt[0], pt[1], pt[2]);
        pts.push_back(p);
    }
    sphere->Delete();
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
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromBox(std::vector<avtVector> &pts)
{
    int npts = (pointDensity1+1)*(pointDensity2+1);

    int nZvals = 1;
    if(dataSpatialDimension > 2)
    {
        npts *= (pointDensity3+1);
        nZvals = (pointDensity3+1);
    }

    //Whole domain, ask intervalTree.
    if (useWholeBox)
        intervalTree->GetExtents( boxExtents );

    float dX = boxExtents[1] - boxExtents[0];
    float dY = boxExtents[3] - boxExtents[2];
    float dZ = boxExtents[5] - boxExtents[4];

    // If using whole box, shrink the extents inward by 0.5%
    const float shrink = 0.005;
    if (useWholeBox)
    {
        if (dX > 0.0)
        {
            boxExtents[0] += (shrink*dX);
            boxExtents[1] -= (shrink*dX);
            dX = boxExtents[1] - boxExtents[0];
        }
        if ( dY > 0.0 )
        {
            boxExtents[2] += (shrink*dY);
            boxExtents[3] -= (shrink*dY);
            dY = boxExtents[3] - boxExtents[2];
        }
        if ( dZ > 0.0 )
        {
            boxExtents[4] += (shrink*dZ);
            boxExtents[5] -= (shrink*dZ);
            dZ = boxExtents[5] - boxExtents[4];
        }
    }

    int index = 0;
    for(int k = 0; k < nZvals; ++k)
    {
        float Z = 0.;
        if(dataSpatialDimension > 2)
            Z = (float(k) / float(pointDensity3)) * dZ + boxExtents[4];
        for(int j = 0; j < pointDensity2+1; ++j)
        {
            float Y = (float(j) / float(pointDensity2)) * dY +boxExtents[2];
            for(int i = 0; i < pointDensity1+1; ++i)
            {
                float X = (float(i) / float(pointDensity1)) * dX 
                    + boxExtents[0];
                avtVector p(X,Y,Z);
                pts.push_back(p);
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
// ****************************************************************************

void
avtStreamlineFilter::GenerateSeedPointsFromPointList(std::vector<avtVector> &pts)
{
    if ((pointList.size() % 3) != 0)
    {
        EXCEPTION1(VisItException, "The seed points for the streamline "
                   "are incorrectly specified.  The number of values must be a "
                   "multiple of 3 (X, Y, Z).");
    }
    int npts = pointList.size() / 3;
    for (int i = 0 ; i < npts ; i++)
    {
        avtVector p(pointList[3*i], pointList[3*i+1], pointList[3*i+2]);
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


