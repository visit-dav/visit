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

    //
    // Create and initialize the streamline filter.
    //
    streamline = vtkVisItStreamLine::New();
    streamline->SetIntegrationDirection(VTK_INTEGRATE_FORWARD);
    streamline->SetIntegrationStepLength(stepLength);
    streamline->SetStepLength(stepLength);
    streamline->SetSavePointInterval(stepLength);
    streamline->SetMaximumPropagationTime(maxTime);
    streamline->SetSpeedScalars(1);
    streamline->SetVorticity(0);
    streamline->SetTerminalSpeed(0.01);

    vtkRungeKutta4 *integrator = vtkRungeKutta4::New();
    streamline->SetIntegrator(integrator);

    SetSourceType(sourceType);

    //
    // Create and initialize the tube filter.
    //
    tubes = vtkTubeFilter::New();
    tubes->SetRadius(radius);
    tubes->SetNumberOfSides(8);
    tubes->SetRadiusFactor(2.);
    tubes->SetCapping(1);
    tubes->ReleaseDataFlagOn();

    //
    // Create and initialize the ribbon filter.
    //
    ribbons = vtkRibbonFilter::New();
    ribbons->SetWidth(0.1);
}


// ****************************************************************************
//  Method: avtStreamlineFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 14:18:03 PST 2004
//    Added ribbons and integrator.
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
//   
// ****************************************************************************

void
avtStreamlineFilter::SetStepLength(double len)
{
    stepLength = len;
    streamline->SetIntegrationStepLength(stepLength);
    streamline->SetStepLength(stepLength);
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
//   
// ****************************************************************************

void
avtStreamlineFilter::SetMaxTime(double t)
{
    maxTime = t;
    streamline->SetMaximumPropagationTime(maxTime);
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
// ****************************************************************************

void
avtStreamlineFilter::SetRadius(double rad)
{
    radius = rad;
    tubes->SetRadius(radius);
    ribbons->SetWidth(2. * radius);
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
// ****************************************************************************

void
avtStreamlineFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    streamline->SetInput(NULL);
    streamline->SetOutput(NULL);
    tubes->SetInput(NULL);
    tubes->SetOutput(NULL);
    ribbons->SetInput(NULL);
    ribbons->SetOutput(NULL);
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
// ****************************************************************************

vtkDataSet *
avtStreamlineFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkPolyData        *outPD = NULL;
    vtkPolyData        *ballPD = NULL;
    vtkLineSource      *line = NULL;
    vtkPlaneSource     *plane = NULL;
    vtkSphereSource    *sphere = NULL;

    bool showTube = displayMethod == STREAMLINE_DISPLAY_TUBES;

    //
    // Create a source for the filter's streamline points.
    //
    if(sourceType == STREAMLINE_SOURCE_POINT)
    {
        streamline->SetStartPosition(pointSource[0],pointSource[1],pointSource[2]);
    }
    else if(sourceType == STREAMLINE_SOURCE_LINE)
    {
        line = vtkLineSource::New();
        line->SetPoint1(lineStart[0], lineStart[1], lineStart[2]);
        line->SetPoint2(lineEnd[0], lineEnd[1], lineEnd[2]);
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

        if(showTube && showStart)
            ballPD = plane->GetOutput();

        streamline->SetSource(plane->GetOutput());
    }
    else if(sourceType == STREAMLINE_SOURCE_SPHERE)
    {
        sphere = vtkSphereSource::New();
        sphere->SetCenter(sphereOrigin[0], sphereOrigin[1], sphereOrigin[2]);
        sphere->SetRadius(sphereRadius);
        sphere->SetLatLongTessellation(1);
        double t = double(20 - pointDensity) / 19.;
        double angle = t * 3. + (1. - t) * 30.;
        sphere->SetPhiResolution(int(angle));
        sphere->SetThetaResolution(int(angle));

        if(showTube && showStart)
            ballPD = sphere->GetOutput();

        streamline->SetSource(sphere->GetOutput());
    }
    else if(sourceType == STREAMLINE_SOURCE_BOX)
    {
        //
        // Create polydata that contains the points that we want to streamline.
        //
        ballPD = vtkPolyData::New();
        vtkPoints *pts = vtkPoints::New();
        ballPD->SetPoints(pts);
        pts->SetNumberOfPoints((pointDensity+1)*(pointDensity+1)*(pointDensity+1));
        float dX = boxExtents[1] - boxExtents[0];
        float dY = boxExtents[3] - boxExtents[2];
        float dZ = boxExtents[5] - boxExtents[4];
        int index = 0;
        for(int k = 0; k < pointDensity+1; ++k)
        {
            float Z = (float(k) / float(pointDensity)) * dZ + boxExtents[4];
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
        streamline->SetSpeedScalars(0);
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

    // If we're going to display the streamlines as ribbons, add the
    // streams to the ribbon filter and get the output.
    if(doRibbons)
    {
        ribbons->SetInput(streams);
        ribbons->Update();
        streams = ribbons->GetOutput();
    }

    // If we're coloring by vorticity magnitude, convert the vorticity to
    // vorticity magnitude and put it in the Scalars array.
    vtkDataArray *vorticity = streams->GetPointData()->GetVectors();
    if(coloringMethod == STREAMLINE_COLOR_VORTICITY && vorticity != 0)
    {
        int n = vorticity->GetNumberOfTuples();
        vtkFloatArray *vortMag = vtkFloatArray::New();
        vortMag->SetNumberOfComponents(1);
        vortMag->SetNumberOfTuples(n);
        float *vm = (float *)vortMag->GetVoidPointer(0);
        for(int i = 0; i < n; ++i)
        {
            const float *val = vorticity->GetTuple3(i);
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

    if(!doRibbons && showTube)
    {
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
                float pt[] = {pointSource[0], pointSource[1], pointSource[2]};
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
                            float val = arr->GetTuple1(pts[0]);
                            balls[i] = AddStartSphere(tubeData, val, ballPD->GetPoint(i));
                        }
                        else
                        {
                            // There was no line for the point so add a ball colored
                            // with zero speed.
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
// ****************************************************************************

vtkPolyData *
avtStreamlineFilter::AddStartSphere(vtkPolyData *tubeData, float val, float pt[3])
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
    for (int i = 0 ; i < npts ; i++)
        arr2->SetTuple1(i, val);

    sphereData->GetPointData()->SetScalars(arr2);
    arr2->Delete();
    sphereData->Register(NULL);
    sphere->Delete();

    return sphereData;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::RefashionDataObjectInfo
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
// ****************************************************************************

void
avtStreamlineFilter::RefashionDataObjectInfo(void)
{
    //IF YOU SEE FUNNY THINGS WITH EXTENTS, ETC, YOU CAN CHANGE THAT HERE.
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetAttributes().SetVariableDimension(1);
}
