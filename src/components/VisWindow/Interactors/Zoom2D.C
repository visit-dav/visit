// ************************************************************************* //
//                                  Zoom2D.C                                 //
// ************************************************************************* //

#include <Zoom2D.h>

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkDashedXorGridMapper2D.h>
#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: Zoom2D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000 
//
//  Modifications:
//    Akira Haddox, Thu Jul  3 13:58:11 PDT 2003
//    Added initialization of guideLine data.
//
// ****************************************************************************

Zoom2D::Zoom2D(VisWindowInteractorProxy &v) : ZoomInteractor(v)
{
    guideLines       = vtkPolyData::New();

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(2);
    guideLines->SetPoints(pts);
    pts->Delete();

    vtkCellArray *lines  = vtkCellArray::New();
    vtkIdType  ids[2] = { 0, 1 };
    lines->InsertNextCell(2, ids);
    guideLines->SetLines(lines);
    lines->Delete();

    guideLinesMapper = vtkDashedXorGridMapper2D::New();
    guideLinesMapper->SetInput(guideLines);
    guideLinesMapper->SetDots(2, 3);
    
    guideLinesActor  = vtkActor2D::New();
    guideLinesActor->SetMapper(guideLinesMapper);
    guideLinesActor->GetProperty()->SetColor(0., 0., 0.);
}


// ****************************************************************************
//  Method: Zoom2D destructor
//
//  Programmer: Akira Haddox
//  Creation:   July 3, 2003
//
// ****************************************************************************

Zoom2D::~Zoom2D()
{
    guideLines->Delete();
    guideLinesMapper->Delete();
    guideLinesActor->Delete();
}

// ****************************************************************************
//  Method: Zoom2D::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Zoom2D, this means
//      a rubber band zoom mode.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 13:11:47 PST 2002
//    Update for the VTK magic to make the zoom work.
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom2D::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    StartZoom();
    StartRubberBand(x, y);
}


// ****************************************************************************
//  Method: Zoom2D::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed up.  For Zoom2D, this means
//      a rubber band zoom mode.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 17:09:25 PDT 2000
//    Removed arguments to EndRubberBand.
//
//    Hank Childs, Tue Feb 19 13:11:47 PST 2002
//    Update for the VTK magic to make the zoom work.
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom2D::EndLeftButtonAction()
{
    EndRubberBand();
    ZoomCamera();
    EndZoom();
}


// ****************************************************************************
//  Method: Zoom2D::AbortLeftButtonAction
//
//  Purpose:
//      Handles an abort action for the left button.  This means giving up on
//      the rubber band.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom2D::AbortLeftButtonAction()
{
    EndRubberBand();
    EndZoom();
}


// ****************************************************************************
//  Method: Zoom2D::StartMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed down.  For Zoom2D, this 
//      means standard zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom2D::StartMiddleButtonAction()
{
    StartZoom();
}


// ****************************************************************************
//  Method: Zoom2D::EndMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed up.  For Zoom2D, this means
//      standard panning.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonUp.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom2D::EndMiddleButtonAction()
{
    EndZoom();
}


// ****************************************************************************
//  Method: Zoom2D::StartRubberBand
//
//  Purpose:
//      Calls parent's ZoomInteractor::StartRubberBand, and if control key is
//      down, calls DrawAllGuideLines.
//
//  Arguments:
//      x       The x-coordinate of the pointer in display coordinates when
//              the button was pressed.
//      y       The y-coordinate of the pointer in display coordinates when
//              the button was pressed.
//
//  Programmer: Akira Haddox 
//  Creation:   July 3, 2003
//
// ****************************************************************************

void
Zoom2D::StartRubberBand(int x, int y)
{
    // Must call this first: It sets some variables that we use.
    ZoomInteractor::StartRubberBand(x, y);
    if (controlKeyDown)
    {
        vtkRenderer *ren = proxy.GetBackground();
        ren->AddActor2D(guideLinesActor);
       
        lastGuideX = x;
        lastGuideY = y;
        DrawAllGuideLines(x, y, x, y);
    }
}


// ****************************************************************************
//  Method: Zoom2D::EndRubberBand
//
//  Purpose:
//      Removes the guideLines actor if it's in place, then call's parent's
//      ZoomInteractor::EndRubberBand.
//
//  Programmer: Akira Haddox
//  Creation:   July 3, 2003
// ****************************************************************************

void
Zoom2D::EndRubberBand()
{
    vtkRenderer *ren = proxy.GetBackground();
    ren->RemoveActor2D(guideLinesActor);

    ZoomInteractor::EndRubberBand();
}


// ****************************************************************************
//  Method: Zoom2D::UpdateRubberBand
//
//  Purpose:
//      Calls parent's ZoomInteractor::UpdateRubberBand to draw the actual
//      rubber band, and if control key is down, makes the call to draw
//      the guidelines.
//
//  Arguments:
//      aX      The x-coordinate of the anchor in display coordinates.
//      aY      The y-coordinate of the anchor in display coordinates.
//      lX      The x-coordinate of the last corner in display coordinates.
//      lY      The y-coordinate of the last corner in display coordinates.
//      nX      The x-coordinate of the new corner in display coordinates.
//      nY      The y-coordinate of the new corner in display coordinates.
//
//  Programmer: Akira Haddox 
//  Creation:   July 3, 2003
//
// ****************************************************************************

void
Zoom2D::UpdateRubberBand(int aX, int aY, int lX, int lY, int nX, int nY)
{
    if (nX == lX && nY == lY)
    {
        //
        // No update neccessary (from Zoom2D or ZoomInteractor)
        //
        return;
    }
    
    ZoomInteractor::UpdateRubberBand(aX, aY, lX, lY, nX, nY);

    // We have to keep track of the last x and y positions in this class,
    // since lX and lY and not always the last values used in a previous
    // call. The rubberBand doesn't need it exactly, but we do.
    if (controlKeyDown)
    {
        UpdateGuideLines(aX, aY, lastGuideX, lastGuideY, nX, nY);
        lastGuideX = nX;
        lastGuideY = nY;
    }
}


// ****************************************************************************
//  Function: GetGuideSegment
//
//  Purpose:
//      Determines what pixels should be drawn based on the anchor location,
//      the location of the old pixel and the location of the new pixel.
//      It is identical to GetSegement used in ZoomInteractor for rubberBands
//      except that the one pixel adjustment is in the oppossite direstion, and
//      for the other point.
//
//  Note:       This is meant for extending or retracting a segment.  If
//              both dimensions vary for the new pixel, this should not be 
//              used.
//
//  Arguments:
//      a       The location of the anchor in one dimension in display coords.
//      l       The location of the last coord in one dim in display coords.
//      n       The location of the new coord in one dim in display coords.
//      outl    Where the last coord portion of the segment should be placed.
//      newl    Where the new coord portion of the segment should be placed.
//
//  Programmer: Akira Haddox 
//  Creation:   July 3, 2003
//
// ****************************************************************************

static inline void
GetGuideSegment(int a, int l, int n, int &outl, int &newl)
{
    outl = l;
    newl = n;
    if (abs(l-a) > abs(n-a))
    {
        //
        // We are moving towards the anchor.
        //
        int offset = (l < a ? 1 : -1);
        outl += offset;
    }
    else
    {
        //
        // We are moving away from the anchor.
        //
        int offset = (n < a ? 1 : -1);
        newl += offset;
    }
}


// ****************************************************************************
//  Method: Zoom2D::UpdateGuideLines
//
//  Purpose:
//      Updates the guide lines to new positions, with minimized flashing. 
//      This method only updates the guide lines, assuming that guide
//      lines already exist on the screen (to be erased if needed).
//      Uses same naming conventions as described in DrawGuideLines.
//
//  Arguments:
//      aX      The x-coordinate of the anchor in display coordinates.
//      aY      The y-coordinate of the anchor in display coordinates.
//      lX      The x-coordinate of the last corner in display coordinates.
//      lY      The y-coordinate of the last corner in display coordinates.
//      nX      The x-coordinate of the new corner in display coordinates.
//      nY      The y-coordinate of the new corner in display coordinates.
//
//  Programmer: Akira Haddox 
//  Creation:   July 3, 2003
//
// ****************************************************************************

void
Zoom2D::UpdateGuideLines(int aX, int aY, int lX, int lY, int nX, int nY)
{
    // Certain lines need to be completely erased and redrawn. Those we
    // draw using DrawGuideLines with this array. Lines which need to be
    // 'adjusted' (like how the rubberBand is adjusted in ZoomInteractor)
    // are done manually in this function.
    bool refresh[8];
    int i;
    for (i = 0; i < 8; ++i)
    {
        refresh[i] = false;
    }
    
    // If any lines will be or are down on a border, or we're moving over
    // a line, just erase and redraw. It's more work than it's worth
    // to make it work, and it doesn't happen often enough for the flashing
    // to be noticable.
    if (((nX-aX)*(lX-aX) <= 0) || ((nY-aY)*(lY-aY) <= 0))
    {
        for (i = 0; i < 8; ++i)
            refresh[i] = true;
    }
    else
    {
        bool cornerPartialRefresh = true;
        // If both coordinates have changed, the following refreshes: 
        // refresh: corner [h/v], shareX[h], shareY[v]
        if (nX != lX && nY != lY)
        {
            refresh[2] = refresh[3] = refresh[4] = refresh[7] = true;
            cornerPartialRefresh = false;
        }
        
        // If the x coordinate has changed, the following refreshes:
        // refresh: corner[v], shareY[v]
        // partials: shareY[h], corner[h]
        if(nX != lX)
        {
            refresh[3] = refresh[7] = true;
            
            // Partially refresh shareY[h]
            guideLinesMapper->SetHorizontalBias(true);
            int fromX, toX;
            GetGuideSegment(aX, lX, nX, fromX, toX);
        
            DrawGuideLine(fromX, aY, toX, aY);

            // Partially refresh corner[h]
            if (cornerPartialRefresh)
                DrawGuideLine(fromX, nY, toX, nY);
        }
       
        // If the y coordinate has changed, the following refreshes:
        // refresh: corner[h], shareX[h]
        // partials: shareX[v], corner[v]
        if(nY != lY)
        {
            refresh[2] = refresh[4] = true;
            
            // Partially refresh shareX[v]
            guideLinesMapper->SetHorizontalBias(false);
            int fromY, toY;
            GetGuideSegment(aY, lY, nY, fromY, toY);
            DrawGuideLine(aX, fromY, aX, toY);
            
            //Partially refresh corner[v]
            if (cornerPartialRefresh)
                DrawGuideLine(nX, fromY, nX, toY);
        }
    }
   
    // Completely refreshed lines
    // Draw the new lines
    DrawGuideLines(aX, aY, nX, nY, refresh);
    // Erase the old lines
    DrawGuideLines(aX, aY, lX, lY, refresh);
}


// ****************************************************************************
//  Method: Zoom2D::DrawAllGuideLines
//
//  Purpose:
//      Draw all the guide lines without considering what may already be
//      on the screen. Useful for initial drawing, and for erasing all
//      lines on the screen.
//
//  Arguments:
//      aX:     Anchor's x display coordinate
//      aY:     Anchor's y display coordinate
//      nX:     Oppossite corner's x display coordinate
//      nY:     Oppossite corner's y display coordinate
//
//  Programmer: Akira Haddox 
//  Creation:   July 3, 2003
//
// ****************************************************************************

void
Zoom2D::DrawAllGuideLines(int aX, int aY, int nX, int nY)
{
    const bool drawAll[8] = { true, true, true, true, true, true, true, true };
    DrawGuideLines(aX, aY, nX, nY, drawAll);
}


// ****************************************************************************
//  Method: Zoom2D::DrawGuideLines
//
//  Purpose:
//      Draws the guidelines requested. The four points are mentioned
//      as anchor (aX, aY), corner (nX, nY), shareX (aX, nY), shareY (nX, aY).
//      This function will not draw duplicate lines if they overlap.
//      All lines are drawn from the boundary to the box.
//
//  Arguments:
//      aX:     Anchor's x display coordinate
//      aY:     Anchor's y display coordinate
//      nX:     Opposite corner's x display coordinate
//      nY:     Opposite corner's y display coordinate
//      which:  Which lines to draw. Stored in pairs of the order of:
//              anchor, corner, shareX, shareY. Even values are horizontal
//              line requests, odd values are vertical requests.
//
//              which[0]: anchor horizontal     which[1]: anchor vertical
//              which[2]: corner horizontal     which[3]: corner vertical
//              which[4]: shareX horizontal     which[5]: shareX vertical
//              which[6]: shareY horizontal     which[7]: shareY vertical
//
//  Programmer: Akira Haddox 
//  Creation:   July 3, 2003
//
// ****************************************************************************

void
Zoom2D::DrawGuideLines(int aX, int aY, int nX, int nY, const bool which[8])
{
    int xMin = (int)(canvasDeviceMinX);
    int yMin = (int)(canvasDeviceMinY);
    int xMax = (int)(canvasDeviceMaxX);
    int yMax = (int)(canvasDeviceMaxY);

    // Existance tests for certain lines
    bool shareXHorizontal = (aY != nY) && which[4];
    bool shareXVertical = (aX != nX) && which[5];
    bool shareYHorizontal = (aY != nY) && which[6];
    bool shareYVertical = (aX != nX) && which[7];
    
    // If the anchor is to the left of the corner
    if (aX < nX)
    {
        // The anchor horizontal line goes to the left
        if (which[0])
            DrawGuideLine(xMin, aY, aX, aY);
        // The corner horizontal line goes to the right
        if (which[2])
            DrawGuideLine(xMax, nY, nX, nY);
        // The shareX horizontal line goes to the left
        if (shareXHorizontal)
            DrawGuideLine(xMin, nY, aX, nY);
        // The shareY horizontal line goes to the right
        if (shareYHorizontal)
            DrawGuideLine(xMax, aY, nX, aY);
    }
    else
    {
        // The anchor horizontal line goes to the right
        if (which[0])
            DrawGuideLine(xMax, aY, aX, aY);
        // The corner horizontal line goes to the left
        if (which[2])
            DrawGuideLine(xMin, nY, nX, nY);
        // The shareX horizontal line goes to the right
        if (shareXHorizontal)
            DrawGuideLine(xMax, nY, aX, nY);
        // The shareY horizontal line goes to the left
        if (shareYHorizontal)
            DrawGuideLine(xMin, aY, nX, aY);
    }

    // If the anchor is below the corner
    if (aY < nY)
    {
        // The anchor vertical line goes down
        if (which[1])
            DrawGuideLine(aX, yMin, aX, aY);
        // The corner vertical line goes up
        if (which[3])
            DrawGuideLine(nX, yMax, nX, nY);
        // The shareX vertical line goes up
        if (shareXVertical)
            DrawGuideLine(aX, yMax, aX, nY);
        // The shareY vertical line goes down
        if (shareYVertical)
            DrawGuideLine(nX, yMin, nX, aY);
    }
    else
    {
        // The anchor vertical line goes up
        if (which[1])
            DrawGuideLine(aX, yMax, aX, aY);
        // The corner vertical line goes down
        if (which[3])
            DrawGuideLine(nX, yMin, nX, nY);
        // The shareX vertical line goes down
        if (shareXVertical)
            DrawGuideLine(aX, yMin, aX, nY);
        // The shareY vertical line goes up
        if (shareYVertical)
            DrawGuideLine(nX, yMax, nX, aY);
    }
}

// ****************************************************************************
//  Method: Zoom2D::DrawGuideLine
//
//  Purpose:
//      Draws a guide line.
//
//  Arguments:
//      x1      The x-coordinate of the first endpoint in display coordinates.
//      y1      The y-coordinate of the first endpoint in display coordinates.
//      x2      The x-coordinate of the second endpoint in display coordinates.
//      y2      The y-coordinate of the second endpoint in display coordinates.
//
//  Programmer: Akira Haddox
//  Creation:   July 3, 2003
//
// ****************************************************************************

void
Zoom2D::DrawGuideLine(int x1, int y1, int x2, int y2)
{
    vtkViewport *ren = proxy.GetBackground();

    vtkPoints *pts = guideLines->GetPoints();
    pts->SetPoint(0, (double) x1, (double) y1, 0.);   
    pts->SetPoint(1, (double) x2, (double) y2, 0.);   
    guideLinesMapper->RenderOverlay(ren, guideLinesActor);
}


// ****************************************************************************
//  Method: Zoom2D::ZoomCamera
//
//  Purpose:
//      Zooms the camera to be between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Eric Brugger
//  Creation:    March 26, 2002
//
//  Modifications:
//    Akira Haddox, Thu Jul  3 14:06:42 PDT 2003
//    Changed so no zooming occurs if zooming area is a line.
//    Also clamped zooming to the boundries (necessary since we round
//    outwards in our zooming, to always be able to reach the boundry).
//
//    Kathleen Bonnell, Mon Jul  7 15:59:11 PDT 2003 
//    Copy newView2D from current View2D to preserve fields which are
//    not being overwritten here. 
// 
//    Eric Brugger, Thu Oct  9 17:03:59 PDT 2003
//    Modified to handle full frame properly.
//
// ****************************************************************************

void
Zoom2D::ZoomCamera(void)
{
    if (anchorX == lastX || anchorY == lastY)
    {
        //
        // This is a point, not a rectangle.
        //
        return;
    }

    //
    // Figure out the lower left and upper right hand corners in
    // display space.
    //
    float leftX   = (float) (anchorX < lastX ? anchorX : lastX);
    float rightX  = (float) (anchorX > lastX ? anchorX : lastX);
    float bottomY = (float) (anchorY < lastY ? anchorY : lastY);
    float topY    = (float) (anchorY > lastY ? anchorY : lastY);
    float dummyZ  = 0.;

    //
    // Convert them to world coordinates.
    //
    vtkRenderer *canvas = proxy.GetCanvas();

    canvas->DisplayToNormalizedDisplay(leftX, topY);
    canvas->NormalizedDisplayToViewport(leftX, topY);
    canvas->ViewportToNormalizedViewport(leftX, topY);

    // Clamp the boundaries.
    if (leftX < 0.0)
        leftX = 0.0;
    if (topY > 1.0)
        topY = 1.0;

    canvas->NormalizedViewportToView(leftX, topY, dummyZ);
    canvas->ViewToWorld(leftX, topY, dummyZ);

    canvas->DisplayToNormalizedDisplay(rightX, bottomY);
    canvas->NormalizedDisplayToViewport(rightX, bottomY);
    canvas->ViewportToNormalizedViewport(rightX, bottomY);

    // Clamp the boundaries.
    if (rightX > 1.0)
        rightX = 1.0;
    if (bottomY < 0.0)
        bottomY = 0.0;

    canvas->NormalizedViewportToView(rightX, bottomY, dummyZ);
    canvas->ViewToWorld(rightX, bottomY, dummyZ);

    //
    // Set the new view window.
    //
    VisWindow *vw = proxy;

    double s = 1.;

    avtView2D newView2D = vw->GetView2D();

    //
    // Handle full frame mode.
    //
    if (newView2D.fullFrame)
    {
        int       size[2];

        vtkRenderWindowInteractor *rwi = Interactor;
        rwi->GetSize(size);

        s = newView2D.GetScaleFactor(size);
    }

    newView2D.window[0] = leftX;
    newView2D.window[1] = rightX;
    newView2D.window[2] = bottomY / s;
    newView2D.window[3] = topY / s;

    vw->SetView2D(newView2D);

    //
    // It looks like we need to explicitly re-render.
    //
    proxy.Render();
}


// ****************************************************************************
//  Method: Zoom2D::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   April 12, 2002
//
// ****************************************************************************

void
Zoom2D::ZoomCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);
        double zoomFactor = pow((double)1.1, dyf);

        //
        // Calculate the new window.
        //
        VisWindow *vw = proxy;

        avtView2D newView2D = vw->GetView2D();

        double dX = ((1. / zoomFactor) - 1.) *
                    ((newView2D.window[1] - newView2D.window[0]) / 2.);
        double dY = ((1. / zoomFactor) - 1.) *
                    ((newView2D.window[3] - newView2D.window[2]) / 2.);

        newView2D.window[0] -= dX;
        newView2D.window[1] += dX;
        newView2D.window[2] -= dY;
        newView2D.window[3] += dY;

        vw->SetView2D(newView2D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
