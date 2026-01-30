// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                Lineout2D.C                                //
// ************************************************************************* //

#include <vtkActor.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>
#include <Lineout2D.h>



// ****************************************************************************
//  Method: Lineout2D constructor
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004
//    Initialize doAlign.
//
//    Brad Whitlock, Fri Oct 14 16:25:57 PDT 2011
//    Create mapper via proxy.
//
//    Kathleen Biagas, Thu Jan 29, 2026
//    Replace 2D versions of mapper/actor with normal. Use vtkLineSource to
//    generate the polydata.
//
// ****************************************************************************

Lineout2D::Lineout2D(VisWindowInteractorProxy &vw) : VisitInteractor(vw)
{
    rubberBandMode = false;
    doAlign = false;
    lineSource = vtkLineSource::New();
    lineMapper = vtkPolyDataMapper::New();
    lineMapper->SetInputConnection(lineSource->GetOutputPort());
    lineActor = vtkActor::New();
    lineActor->GetProperty()->SetLineWidth(1.);
    lineActor->SetMapper(lineMapper);
}


// ****************************************************************************
//  Method: Lineout2D destructor
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************

Lineout2D::~Lineout2D()
{
    lineActor->Delete();
    lineMapper->Delete();
    lineSource->Delete();
}


// ****************************************************************************
//  Method: Lineout2D::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Lineout2D, this means
//      a rubber band mode.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//    Call GetEventPosition to determine x and y.
//
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004
//    Set doAlign.
//
// ****************************************************************************

void
Lineout2D::StartLeftButtonAction()
{
    //
    //  We need to set the state variable in vtkInteractorStyle,
    //  but currently there is no 'UserDefined' or such.  Using
    //  timer as it seems the most innocuous.
    //
    int x, y;
    Interactor->GetEventPosition(x, y);
    doAlign = Interactor->GetShiftKey();
    StartTimer();
    StartRubberBand(x, y);
}


// ****************************************************************************
//  Method: Lineout2D::SetCanvasViewport
//
//  Purpose:
//      Sets where the canvas is in display coordinates so that rubber bands
//      outside the canvas can be forced inside the canvas.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************

void
Lineout2D::SetCanvasViewport(void)
{
    vtkRenderer *ren = proxy.GetCanvas();

    canvasDeviceMinX = 0.;
    canvasDeviceMinY = 0.;
    ren->NormalizedViewportToViewport(canvasDeviceMinX, canvasDeviceMinY);
    ren->ViewportToNormalizedDisplay(canvasDeviceMinX, canvasDeviceMinY);
    ren->NormalizedDisplayToDisplay(canvasDeviceMinX, canvasDeviceMinY);

    canvasDeviceMaxX = 1.;
    canvasDeviceMaxY = 1.;
    ren->NormalizedViewportToViewport(canvasDeviceMaxX, canvasDeviceMaxY);
    ren->ViewportToNormalizedDisplay(canvasDeviceMaxX, canvasDeviceMaxY);
    ren->NormalizedDisplayToDisplay(canvasDeviceMaxX, canvasDeviceMaxY);
}

// ****************************************************************************
//  Method: Lineout2D::StartRubberBand
//
//  Purpose:
//      Tells the interactor to draw a rubber band and update it whenever the
//      mouse moves.
//
//  Arguments:
//      x       The x-coordinate of the pointer in display coordinates when
//              the button was pressed.
//      y       The y-coordinate of the pointer in display coordinates when
//              the button was pressed.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// Modifications:
//   Brad Whitlock, Fri Oct 14 16:25:13 PDT 2011
//   Set the actor's color.
//
//   Kathleen Biagas, Thu Jan 29, 2026
//   Replace 2D versions of mapper/actor with normal. Compute world
//   coordinate to set vtkLineSource end points. Add actor to Canvas.
//
// ****************************************************************************

void
Lineout2D::StartRubberBand(int x, int y)
{
    rubberBandMode = true;

    // Set the actor's color.
    double fg[3];
    proxy.GetForegroundColor(fg);
    lineActor->GetProperty()->SetColor(fg[0], fg[1], fg[2]);

    //
    // The anchor of the rubber band will be where the button press was.
    //
    anchorX = x;
    anchorY = y;

    //
    // Determine what to clamp the rubber band to.
    //
    SetCanvasViewport();

    //
    // If the user has clicked outside the viewport, force the back inside.
    //
    ForceCoordsToViewport(anchorX, anchorY);

    //
    // Must update bookkeeping so that OnMouseMove works correctly.
    //
    lastX   = anchorX;
    lastY   = anchorY;

    double pt1[4];
    double pt2[4];

    this->ComputeDisplayToWorld(anchorX, anchorY, 0.0, pt1);
    this->ComputeDisplayToWorld(lastX, lastY, 0.0, pt2);

    lineSource->SetPoint1(pt1[0], pt1[1], 0.000001);
    lineSource->SetPoint2(pt2[0], pt2[1], 0.000001);
    lineSource->SetResolution(1);
    lineSource->Update();
    proxy.GetCanvas()->AddActor(lineActor);
}


// ****************************************************************************
//  Method: Lineout2D::OnMouseMove
//
//  Purpose:
//      Receives a mouse movement and updates the rubber band.
//
//  Arguments:
//      ctrl          non-zero if the ctrl button was pushed.
//      shift         non-zero if the shift button was pushed.
//      x             The x-coordinate on the mouse pointer in display coords.
//      y             The y-coordinate on the mouse pointer in display coords.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modificationsy
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//    Call GetEventPosition to determine x and y.
//
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004
//    AlignToAxis if shift key is being pressed (doAlign).
//
// ****************************************************************************

void
Lineout2D::OnMouseMove()
{
    int x, y;
    if (rubberBandMode)
    {
        Interactor->GetEventPosition(x, y);
        if (doAlign)
        {
            AlignToAxis(x, y);
        }
        ForceCoordsToViewport(x, y);
        UpdateRubberBand(anchorX, anchorY, lastX, lastY, x, y);
        lastX = x;
        lastY = y;
    }
    else
    {
        //
        // We are not in rubber band mode, so we don't want to override
        // the interactor.  Call the base function.
        //
        VisitInteractor::OnMouseMove();
    }
}


// ****************************************************************************
//  Method: Lineout2D::EndRubberBand
//
//  Purpose:
//      Tells the interactor to remove the rubber band and no longer update it
//      whenever the mouse moves.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************

void
Lineout2D::EndRubberBand()
{
    rubberBandMode = false;
    proxy.GetCanvas()->RemoveActor(lineActor);
}


// ****************************************************************************
//  Method: Lineout2D::ForceCoordsToViewport
//
//  Purpose:
//      Takes coordinates and makes sure they are in the viewport.  If not,
//      it forces them inside the viewport.
//
//  Arguments:
//      x      X coordinate in display coordinates.
//      y      Y coordinate in display coordinates.
//
//  Note:    This routine assumes the canvasDevice[Min|Max][X|Y] has
//           already been set.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************

void
Lineout2D::ForceCoordsToViewport(int &x, int &y)
{
    if (x < canvasDeviceMinX)
    {
        x = (int)ceil(canvasDeviceMinX);
    }
    if (x > canvasDeviceMaxX)
    {
        x = (int)floor(canvasDeviceMaxX);
    }
    if (y < canvasDeviceMinY)
    {
        y = (int)ceil(canvasDeviceMinY);
    }
    if (y > canvasDeviceMaxY)
    {
        y = (int)floor(canvasDeviceMaxY);
    }
}



// ****************************************************************************
//  Method: Lineout2D::UpdateRubberBand
//
//  Purpose:
//      The real rubber band function.  This takes six arguments specifying
//      the anchor of the rubber band, what the last opposite corner was,
//      and what the new opposite corner is.  It implements the rubber band
//      through an XOR found in any text book.  To prevent blinking effects,
//      if extends the old rubber band lines instead of wiping them out and
//      redrawing.
//
//  Arguments:
//      aX      The x-coordinate of the anchor in display coordinates.
//      aY      The y-coordinate of the anchor in display coordinates.
//      lX      The x-coordinate of the last corner in display coordinates.
//      lY      The y-coordinate of the last corner in display coordinates.
//      nX      The x-coordinate of the new corner in display coordinates.
//      nY      The y-coordinate of the new corner in display coordinates.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Biagas, Thu Jan 29, 2026
//    Remove 2d actor/mapper. Use lineSource instead.
//    Compute world coordinate from lastX/Y to set second end point for
//    vtkLineSource.  Request a render.
//
// ****************************************************************************

void
Lineout2D::UpdateRubberBand(int aX, int aY, int lX, int lY,int nX,int nY)
{
    if (nX == lX && nY == lY)
    {
        //
        // No update necessary.
        //
        return;
    }

    //
    // Update the line source
    //
    double pt2[4];
    pt2[3] = 0;
    this->ComputeDisplayToWorld(lX, lY, 0.0, pt2);

    lineSource->SetPoint2(pt2[0], pt2[1], 0.000001);
    lineSource->Update();
    proxy.Render();
}


// ****************************************************************************
//  Method: Lineout2D::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed up.  For Lineout2D, this means
//      a rubber band zoom mode.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004
//    Reset doAlign.
//
// ****************************************************************************

void
Lineout2D::EndLeftButtonAction()
{
    EndRubberBand();
    Lineout();
    EndTimer();
    doAlign = false;
}


// ****************************************************************************
//  Method: Lineout2D::AbortLeftButtonAction
//
//  Purpose:
//      Handles an abort action for the left button.  This means giving up on
//      the rubber band.
//
//  Note:   Modified from Zoom2D.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
Lineout2D::AbortLeftButtonAction()
{
    EndRubberBand();
    EndTimer();
}


// ****************************************************************************
//  Method: Lineout2D::Lineout
//
//  Purpose:
//      Creates a line between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 16, 2002
//
// ****************************************************************************

void
Lineout2D::Lineout(void)
{
    if (anchorX == lastX && anchorY == lastY)
    {
        //
        // This is a point, not a line.
        //
        return;
    }
    proxy.Lineout(anchorX, anchorY, lastX, lastY);
}


// ****************************************************************************
//  Method: ZoomInteractor::OnTimer
//
//  Purpose:
//      Throw out timer events if we are in rubber Band mode.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 18, 2003
//
//  Modifications:
//
// ****************************************************************************

void
Lineout2D::OnTimer()
{
    if (!rubberBandMode)
    {
        vtkInteractorStyle::OnTimer();
    }
}


// ****************************************************************************
//  Method: ZoomInteractor::AlignToAxis
//
//  Purpose:
//     Modify either the x or y value to ensure the line between
//     (anchorX, anchorY) and (x, y) is axis-aligned.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 5, 2004
//
//  Modifications:
//
// ****************************************************************************

void
Lineout2D::AlignToAxis(int &x, int &y)
{
    float slope = (float)(y-anchorY) / (float)(x - anchorX);

    if (slope < -1. || slope > 1.)
    {
        x = anchorX;
    }
    else
    {
        y = anchorY;
    }
}

void
Lineout2D::ComputeDisplayToWorld(double x, double y, double z,
    double *worldPt)
{
    vtkRenderer *ren = proxy.GetCanvas();
    ren->SetDisplayPoint(x, y, z);
    ren->DisplayToWorld();
    ren->GetWorldPoint(worldPt);
    if(worldPt[3] != 0.)
    {
        worldPt[0] /= worldPt[3];
        worldPt[1] /= worldPt[3];
        worldPt[2] /= worldPt[3];
        worldPt[3] = 1.0;
    }
}
