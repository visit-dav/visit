/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <avtStreamlineFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkLineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRungeKutta4.h>
#include <vtkSphereSource.h>
#include <vtkVisItStreamLine.h>
#include <vtkTubeFilter.h>

#include <vtkRibbonFilter.h>

#include <math.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtVector.h>

#include <DebugStream.h>

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
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Initialize streamline direction option.
//
// ****************************************************************************

avtStreamlineFilter::avtStreamlineFilter()
{
    stepLength = 1.;
    maxTime = 100.;
    showStart = true;
    radius = 0.125;
    pointDensity = 1;
    coloringMethod = STREAMLINE_COLOR_SPEED;
    displayMethod = STREAMLINE_DISPLAY_LINES;
    streamlineDirection = VTK_INTEGRATE_FORWARD;

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

    // Set all of the filters to 0.
    streamline = 0;
    tubes = 0;
    ribbons = 0;
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
// ****************************************************************************

avtStreamlineFilter::~avtStreamlineFilter()
{
    if(streamline != NULL)
    {
        streamline->Delete();
    }
    if(tubes != NULL)
    {
        tubes->Delete();
    }
    if(ribbons != NULL)
    {
        ribbons->Delete();
    }
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetStepLength
//
// Purpose: 
//   Sets the filter's integration step length.
//
// Arguments:
//   len : The new step length.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:56:47 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Jan 4 08:58:59 PDT 2005
//   Removed code to set the step length for the filter.
//
// ****************************************************************************

void
avtStreamlineFilter::SetStepLength(double len)
{
    stepLength = len;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetMaxTime
//
// Purpose: 
//   Sets the maximum time used to integrate (this is not clock time).
//
// Arguments:
//   t : The new time.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:57:25 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Jan 4 08:58:59 PDT 2005
//   Removed code to set the max time for the filter.
//   
// ****************************************************************************

void
avtStreamlineFilter::SetMaxTime(double t)
{
    maxTime = t;
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
// ****************************************************************************

void
avtStreamlineFilter::SetColoringMethod(int m)
{
    coloringMethod = m;
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
avtStreamlineFilter::SetPlaneSource(double O[3], double N[3], double U[3], double R)
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
// Method: avtStreamlineFilter::SetShowStart
//
// Purpose: 
//   Indicates whether or not to show the stream starting points.
//
// Arguments:
//   val : Indicates whether or not to show the stream starting points.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:02:04 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetShowStart(bool val)
{
    showStart = val;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetRadius
//
// Purpose: 
//   Sets the radius used for tubes and ribbons.
//
// Arguments:
//   rad : The tube radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:02:45 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 14:28:35 PST 2004
//   Renamed the method and made it set both the tube radius and the ribbon
//   width.
//
//   Brad Whitlock, Tue Jan 4 09:11:38 PDT 2005
//   Removed the code to set the tube and ribbon width since it now happens
//   later in the ExecuteData method.
//
// ****************************************************************************

void
avtStreamlineFilter::SetRadius(double rad)
{
    radius = rad;
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
// ****************************************************************************

void
avtStreamlineFilter::SetPointDensity(int den)
{
    pointDensity = (den > 0) ? den : 1;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetStreamlineDirection
//
// Purpose: 
//   Sets the streamline integration direction
//
// Arguments:
//   dir : The new direction
//
// Programmer: Dave Pugmire
// Creation:   Thu Nov 15 12:09:08 EST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetStreamlineDirection(int dir)
{
    streamlineDirection = dir;
}

// ****************************************************************************
// Method: avtStreamlineFilter::ReleaseData
//
// Purpose: 
//   Tells the streamline and tube filters to release their data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:03:48 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 14:29:30 PST 2004
//   I added the ribbons filter.
//
//   Brad Whitlock, Tue Jan 4 09:00:38 PDT 2005
//   Added tests around the methods to set the input/output since the filters
//   are no longer created in the constructor.
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
// ****************************************************************************

void
avtStreamlineFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    if(streamline != 0)
    {
        streamline->SetInput(NULL);
        vtkPolyData *p = vtkPolyData::New();
        streamline->SetOutput(p);
        p->Delete();
    }
    if(tubes != 0)
    {
        tubes->SetInput(NULL);
        vtkPolyData *p = vtkPolyData::New();
        tubes->SetOutput(p);
        p->Delete();
    }
    if(ribbons != 0)
    {
        ribbons->SetInput(NULL);
        vtkPolyData *p = vtkPolyData::New();
        ribbons->SetOutput(p);
        p->Delete();
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 12:43:16 PDT 2004
//    I added code to convert the vorticity array to vorticity magnitude so
//    we can color by that too. I also added the ribbon filter.
//
//    Brad Whitlock, Mon Jan 3 13:25:51 PST 2005
//    Added code to create a polydata consisting of small line segments based
//    on the streamline source if the streamline filter did not produce any
//    streamlines. I also added code to restrict the Z coordinate of the
//    streamline sources if the input data is 2D.
//
//    Brad Whitlock, Tue Jan 4 09:03:49 PDT 2005
//    I moved all of the code to create the filters to this method so
//    the output from the last domain is not used for all domains.
//
//    Hank Childs, Fri Feb 23 09:22:20 PST 2007
//    Fix memory leaks.
//
//    Hank Childs, Sat Mar  3 11:16:26 PST 2007
//    Add support for getting the extents of the color variable later.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add support for streamline direction option.
//
// ****************************************************************************

vtkDataSet *
avtStreamlineFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkPolyData        *outPD = NULL;
    vtkPolyData        *ballPD = NULL;
    vtkLineSource      *line = NULL;
    vtkPlaneSource     *plane = NULL;
    vtkSphereSource    *sphere = NULL;

    //
    // Create and initialize the streamline filter.
    //
    if(streamline != 0)
        streamline->Delete();
    streamline = vtkVisItStreamLine::New();
    streamline->SetIntegrationDirection(streamlineDirection);
    streamline->SetIntegrationStepLength(stepLength);
    streamline->SetStepLength(stepLength);
    streamline->SetSavePointInterval(stepLength);
    streamline->SetMaximumPropagationTime(maxTime);
    streamline->SetTerminalSpeed(0.01);
    vtkRungeKutta4 *integrator = vtkRungeKutta4::New();
    streamline->SetIntegrator(integrator);
    integrator->Delete();

    bool showTube = displayMethod == STREAMLINE_DISPLAY_TUBES;
    int spatialDim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    //
    // Create a source for the filter's streamline points.
    //
    if(sourceType == STREAMLINE_SOURCE_POINT)
    {
        double z0 = (spatialDim > 2) ? pointSource[2] : 0.;
        streamline->SetStartPosition(pointSource[0], pointSource[1], z0);
    }
    else if(sourceType == STREAMLINE_SOURCE_LINE)
    {
        line = vtkLineSource::New();
        double z0 = (spatialDim > 2) ? lineStart[2] : 0.;
        double z1 = (spatialDim > 2) ? lineEnd[2] : 0.;
        line->SetPoint1(lineStart[0], lineStart[1], z0);
        line->SetPoint2(lineEnd[0], lineEnd[1], z1);
        line->SetResolution(pointDensity);
        if(showTube && showStart)
            ballPD = line->GetOutput();

        streamline->SetSource(line->GetOutput());
    }
    else if(sourceType == STREAMLINE_SOURCE_PLANE)
    {
        plane = vtkPlaneSource::New();
        plane->SetXResolution(pointDensity);
        plane->SetYResolution(pointDensity);
        avtVector O(planeOrigin);
        avtVector U(planeUpAxis);
        avtVector N(planeNormal);
        U.normalize();
        N.normalize();
        if(spatialDim <= 2)
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

        // Zero out the Z coordinate if the input dataset is only 2D.
        vtkPolyData *planePD = plane->GetOutput();
        if(spatialDim <= 2)
            SetZToZero(planePD);

        if(showTube && showStart)
            ballPD = planePD;

        streamline->SetSource(planePD);
    }
    else if(sourceType == STREAMLINE_SOURCE_SPHERE)
    {
        sphere = vtkSphereSource::New();
        sphere->SetCenter(sphereOrigin[0], sphereOrigin[1], sphereOrigin[2]);
        sphere->SetRadius(sphereRadius);
        sphere->SetLatLongTessellation(1);
        double t = double(30 - pointDensity) / 29.;
        double angle = t * 3. + (1. - t) * 30.;
        sphere->SetPhiResolution(int(angle));
        sphere->SetThetaResolution(int(angle));

        // Zero out the Z coordinate if the input dataset is only 2D.
        vtkPolyData *spherePD = sphere->GetOutput();
        if(spatialDim <= 2)
            SetZToZero(spherePD);

        if(showTube && showStart)
            ballPD = spherePD;

        streamline->SetSource(spherePD);
    }
    else if(sourceType == STREAMLINE_SOURCE_BOX)
    {
        //
        // Create polydata that contains the points that we want to streamline.
        //
        ballPD = vtkPolyData::New();
        vtkPoints *pts = vtkPoints::New();
        ballPD->SetPoints(pts);

        int npts = (pointDensity+1)*(pointDensity+1);
        int nZvals = 1;
        if(spatialDim > 2)
        {
            npts *= (pointDensity+1);
            nZvals = (pointDensity+1);
        }
        pts->SetNumberOfPoints(npts);

        float dX = boxExtents[1] - boxExtents[0];
        float dY = boxExtents[3] - boxExtents[2];
        float dZ = boxExtents[5] - boxExtents[4];
        int index = 0;
        for(int k = 0; k < nZvals; ++k)
        {
            float Z = 0.;
            if(spatialDim > 2)
                Z = (float(k) / float(pointDensity)) * dZ + boxExtents[4];
            for(int j = 0; j < pointDensity+1; ++j)
            {
                float Y = (float(j) / float(pointDensity)) * dY + boxExtents[2];
                for(int i = 0; i < pointDensity+1; ++i)
                {
                    float X = (float(i) / float(pointDensity)) * dX + boxExtents[0];
                    pts->SetPoint(index++, X, Y, Z);
                }
            }
        }
        pts->Delete();

        streamline->SetSource(ballPD);
    }

    bool doRibbons = displayMethod == STREAMLINE_DISPLAY_RIBBONS;
    if(coloringMethod == STREAMLINE_COLOR_SOLID)
    {
        // No variable coloring.
        streamline->SetSpeedScalars(1);  // hack
        streamline->SetVorticity(doRibbons?1:0);
    }
    else if(coloringMethod == STREAMLINE_COLOR_SPEED)
    {
        // Color by velocity magnitude.
        streamline->SetSpeedScalars(1);
        streamline->SetVorticity(doRibbons?1:0);
    }
    else
    {
        // Color by vorticity magnitude.
        streamline->SetSpeedScalars(0);
        streamline->SetVorticity(1);
    }

    // Set the input to the streamline filter and execute it.
    streamline->SetInput(inDS);
    streamline->Update();
    vtkPolyData *streams = streamline->GetOutput();
    if (coloringMethod == STREAMLINE_COLOR_SPEED)
        streams->GetPointData()->GetScalars()->SetName("colorVar");

    if(doRibbons)
    {
        //
        // If we're going to display the streamlines as ribbons, add the
        // streams to the ribbon filter and get the output.
        //
        if(ribbons != 0)
            ribbons->Delete();
        ribbons = vtkRibbonFilter::New();
        ribbons->SetWidth(2. * radius);
        ribbons->SetInput(streams);
        ribbons->Update();
        streams = ribbons->GetOutput();
    }

    // If we're coloring by vorticity magnitude, convert the vorticity to
    // vorticity magnitude and put it in the Scalars array.
    vtkDataArray *vorticity = streams->GetPointData()->GetVectors();
    if(vorticity != 0)
    {
        if(coloringMethod == STREAMLINE_COLOR_VORTICITY)
        {
            debug4 << "Computing vorticity magnitude." << endl;
            int n = vorticity->GetNumberOfTuples();
            vtkFloatArray *vortMag = vtkFloatArray::New();
            vortMag->SetName("colorVar");
            vortMag->SetNumberOfComponents(1);
            vortMag->SetNumberOfTuples(n);
            float *vm = (float *)vortMag->GetVoidPointer(0);
            for(int i = 0; i < n; ++i)
            {
                const double *val = vorticity->GetTuple3(i);
                *vm++ = (float)sqrt(val[0]*val[0] + val[1]*val[1] + val[2]*val[2]);
            }
            // If there is a scalar array, remove it.
            vtkDataArray *oldScalars = streams->GetPointData()->GetScalars();
            if(oldScalars != 0)
                streams->GetPointData()->RemoveArray(oldScalars->GetName());
            // Remove the vorticity array.
            streams->GetPointData()->RemoveArray(vorticity->GetName());

            // Make vorticity magnitude be the active scalar field.
            streams->GetPointData()->SetVectors(0);
            streams->GetPointData()->SetScalars(vortMag);
        }
        else
        {
            // Remove the vorticity array.
            streams->GetPointData()->RemoveArray(vorticity->GetName());
            streams->GetPointData()->SetVectors(0);
            debug4 << "Removed vorticity since we didn't need it." << endl;
        }
    }

    if(!doRibbons && showTube)
    {
        //
        // Create and initialize the tube filter.
        //
        if(tubes != 0)
            tubes->Delete();
        tubes = vtkTubeFilter::New();
        tubes->SetRadius(radius);
        tubes->SetNumberOfSides(8);
        tubes->SetRadiusFactor(2.);
        tubes->SetCapping(1);
        tubes->ReleaseDataFlagOn();

        if(showStart)
        {
            // Execute the tube filter.
            tubes->SetInput(streams);
            tubes->Update();
            vtkPolyData *tubeData = tubes->GetOutput();

            // Create an append filter that we'll use to add the start balls
            // to the tube poly data.
            vtkAppendPolyData *append = vtkAppendPolyData::New();
            append->AddInput(tubeData);
            vtkPolyData       **balls = NULL;
            int                 nballs = 0;

            // Add spheres to the start of the streamlines so we know where
            // they start.
            if(sourceType == STREAMLINE_SOURCE_POINT)
            {
                double pt[] = {pointSource[0], pointSource[1], 0.};
                if(spatialDim > 2) pt[2] = pointSource[2];
                vtkDataArray *arr = tubeData->GetPointData()->GetScalars();
                float val = arr->GetTuple1(0);
                balls = new vtkPolyData *[1];
                nballs = 1;
                balls[0] = AddStartSphere(tubeData, val, pt);
                append->AddInput(balls[0]);
            }
            else if(ballPD != NULL)
            {
                vtkCellArray *lines = streams->GetLines();
                if(lines != NULL && lines->GetNumberOfCells() > 0)
                {
                    lines->SetTraversalLocation(0);
                    nballs = ballPD->GetNumberOfPoints();
                    balls = new vtkPolyData*[nballs];
                    for(vtkIdType i = 0; i < nballs; ++i)
                    {
                        // Figure out the data value used to color the ball.
                        vtkIdType *pts = NULL, npts;
                        lines->GetNextCell(npts, pts);
                        if(pts != NULL)
                        {
                            vtkDataArray *arr = streams->GetPointData()->GetScalars();
                            double val = arr->GetTuple1(pts[0]);
                            balls[i] = AddStartSphere(tubeData, val, ballPD->GetPoint(i));
                        }
                        else
                        {
                            // There was no line for the point so add a ball 
                            // colored with zero speed.
                            balls[i] = AddStartSphere(tubeData, 0., ballPD->GetPoint(i));
                        }

                        // Add the ball to the append filter.
                        append->AddInput(balls[i]);
                    }
                }
            }

            append->Update();
            outPD = append->GetOutput();
            // Stash the polydata in the streamline filter since we keep it around for
            // the life of the filter. This keeps it from disappearing when we delete
            // the append filter.
            streamline->SetOutput(outPD);

            //
            // Delete the ball polydata.
            //
            append->Delete();
            if(nballs > 0)
            {
                for(int i = 0; i < nballs; ++i)
                    balls[i]->Delete();
                delete [] balls;
            }
        }
        else
        {
            tubes->SetInput(streams);
            tubes->Update();
            outPD = tubes->GetOutput();
        }
    }
    else
    {
        outPD = streams;
    }

    //
    // Delete any sources that we created.
    //
    if(line != NULL)
        line->Delete();
    if(plane != NULL)
        plane->Delete();
    if(sphere != NULL)
        sphere->Delete();

    return outPD;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetZToZero
//
// Purpose: 
//   Zero out the Z coordinates.
//
// Arguments:
//   pd : An input polydata dataset.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 3 10:42:42 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetZToZero(vtkPolyData *pd) const
{
    vtkPoints *pts = pd->GetPoints();
    if(pts != 0)
    {
        if (pts->GetDataType() == VTK_FLOAT)
        {
            float *p = (float*)pts->GetVoidPointer(0);
            for(int i = 0; i < pts->GetNumberOfPoints(); ++i)
            {
                p[3*i+2] = 0.f;
            }
        }
        if (pts->GetDataType() == VTK_DOUBLE)
        {
            double *p = (double*)pts->GetVoidPointer(0);
            for(int i = 0; i < pts->GetNumberOfPoints(); ++i)
            {
                p[3*i+2] = 0.;
            }
        }
    }
}

// ****************************************************************************
// Method: avtStreamlineFilter::AddStartSphere
//
// Purpose: 
//   Adds a sphere representing the start of a streamline to the input polydata.
//
// Arguments:
//   tubeData : The data array that we're copying.
//   val      : The data value that we're using.
//   pt       : The location of the sphere.
//
// Returns:    The polydata that represents the start sphere.
//
// Programmer: Brad Whitlock, Hank Childs
// Creation:   Mon Oct 7 14:34:59 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002    
//   Use NewInstance instead of MakeObject, in order to match vtk's new api. 
//
//   Brad Whitlock, Wed Dec 22 14:55:46 PST 2004
//   Changed tubeRadius to radius.
//
//   Brad Whitlock, Mon Jan 3 10:50:36 PDT 2005
//   I made the sphere be 2D if we're not in 3D.
//
// ****************************************************************************

vtkPolyData *
avtStreamlineFilter::AddStartSphere(vtkPolyData *tubeData, float val, double pt[3])
{
    // Create the sphere polydata.
    vtkSphereSource *sphere = vtkSphereSource::New();
    sphere->SetCenter(pt[0], pt[1], pt[2]);
    sphere->SetRadius(radius * 2.);
    sphere->SetLatLongTessellation(1);
    sphere->SetPhiResolution(8);
    sphere->SetThetaResolution(8);
    vtkPolyData *sphereData = sphere->GetOutput();
    sphereData->Update();

    // Copy the tube's starting scalar variable to the sphere polydata.
    vtkDataArray *arr = tubeData->GetPointData()->GetScalars();
    vtkDataArray *arr2 = arr->NewInstance();
    int npts = sphereData->GetNumberOfPoints();
    arr2->SetNumberOfTuples(npts);
    for (int i = 0; i < npts; ++i)
    {
        arr2->SetTuple1(i, val);
    }

    // If we're not 3D, make the sphere be 2D.
    if(GetInput()->GetInfo().GetAttributes().GetSpatialDimension() <= 2)
        SetZToZero(sphereData);

    sphereData->GetPointData()->SetScalars(arr2);
    arr2->Delete();
    sphereData->Register(NULL);
    sphere->Delete();

    return sphereData;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::PreExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Hank Childs
//  Creation:   March 3, 2007
//
// ****************************************************************************

void
avtStreamlineFilter::PreExecute(void)
{
    avtStreamer::PreExecute();

    // If we have a box source and we are using the whole box, then plug
    // the current spatial extents into the box extents.
    if (sourceType == STREAMLINE_SOURCE_BOX && useWholeBox)
    {
        avtDataset_p input = GetTypedInput();
        avtDatasetExaminer::GetSpatialExtents(input, boxExtents);
    }
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
// ****************************************************************************

void
avtStreamlineFilter::PostExecute(void)
{
    avtStreamer::PostExecute();

    if (coloringMethod == STREAMLINE_COLOR_VORTICITY ||
        coloringMethod == STREAMLINE_COLOR_SPEED)
    {
        double range[2];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetDataExtents(ds, range, "colorVar");

        avtExtents *e;
        e = GetOutput()->GetInfo().GetAttributes().GetCumulativeTrueDataExtents();
        e->Merge(range);
        e = GetOutput()->GetInfo().GetAttributes().GetCumulativeCurrentDataExtents();
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
// ****************************************************************************

void
avtStreamlineFilter::UpdateDataObjectInfo(void)
{
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


