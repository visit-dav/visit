// ************************************************************************* //
//                                  Zoom2D.C                                 //
// ************************************************************************* //

#include <Zoom2D.h>

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
// ****************************************************************************

Zoom2D::Zoom2D(VisWindowInteractorProxy &v) : ZoomInteractor(v)
{
    ;
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
//  Method: Zoom2D::ZoomCamera
//
//  Purpose:
//      Zooms the camera to be between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Eric Brugger
//  Creation:    March 26, 2002
//
// ****************************************************************************

void
Zoom2D::ZoomCamera(void)
{
    if (anchorX == lastX && anchorY == lastY)
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
    canvas->NormalizedViewportToView(leftX, topY, dummyZ);
    canvas->ViewToWorld(leftX, topY, dummyZ);

    canvas->DisplayToNormalizedDisplay(rightX, bottomY);
    canvas->NormalizedDisplayToViewport(rightX, bottomY);
    canvas->ViewportToNormalizedViewport(rightX, bottomY);
    canvas->NormalizedViewportToView(rightX, bottomY, dummyZ);
    canvas->ViewToWorld(rightX, bottomY, dummyZ);

    //
    // Set the new view window.
    //
    VisWindow *vw = proxy;

    const avtView2D &oldView2D=vw->GetView2D();

    avtView2D newView2D;

    newView2D.viewport[0] = oldView2D.viewport[0];
    newView2D.viewport[1] = oldView2D.viewport[1];
    newView2D.viewport[2] = oldView2D.viewport[2];
    newView2D.viewport[3] = oldView2D.viewport[3];
    newView2D.window[0] = leftX;
    newView2D.window[1] = rightX;
    newView2D.window[2] = bottomY;
    newView2D.window[3] = topY;

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
