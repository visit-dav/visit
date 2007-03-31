// ************************************************************************* //
//                                  ZoomCurve.C                              //
// ************************************************************************* //

#include <ZoomCurve.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: ZoomCurve constructor
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
// ****************************************************************************

ZoomCurve::ZoomCurve(VisWindowInteractorProxy &v) : ZoomInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: ZoomCurve::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For ZoomCurve, this means
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
ZoomCurve::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    StartZoom();
    StartRubberBand(x, y);
}


// ****************************************************************************
//  Method: ZoomCurve::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed up.  For ZoomCurve, this means
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
//    Eric Brugger, Fri Nov 21 08:00:11 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
ZoomCurve::EndLeftButtonAction()
{
    EndRubberBand();
    ZoomCamera();
    EndZoom();
    IssueViewCallback();
}


// ****************************************************************************
//  Method: ZoomCurve::AbortLeftButtonAction
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
ZoomCurve::AbortLeftButtonAction()
{
    EndRubberBand();
    EndZoom();
}


// ****************************************************************************
//  Method: ZoomCurve::StartMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed down.  For ZoomCurve, this 
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
ZoomCurve::StartMiddleButtonAction()
{
    StartZoom();
}


// ****************************************************************************
//  Method: ZoomCurve::EndMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed up.  For ZoomCurve, this means
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
//    Eric Brugger, Fri Nov 21 08:00:11 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
ZoomCurve::EndMiddleButtonAction()
{
    EndZoom();
    IssueViewCallback();
}


// ****************************************************************************
//  Method: ZoomCurve::ZoomCamera
//
//  Purpose:
//      Zooms the camera to be between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Eric Brugger
//  Creation:    March 26, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:59:50 PDT 2003
//    I changed the routine to match the new definition of avtViewCurve.
//
//    Eric Brugger, Wed Oct 15 16:29:52 PDT 2003
//    I modified the routine to handle the fact that curve views are
//    full frame.
//
// ****************************************************************************

void
ZoomCurve::ZoomCamera(void)
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

    avtViewCurve newViewCurve=vw->GetViewCurve();

    int       size[2];

    vtkRenderWindowInteractor *rwi = Interactor;
    rwi->GetSize(size);

    double s = newViewCurve.GetScaleFactor(size);

    newViewCurve.domain[0] = leftX;
    newViewCurve.domain[1] = rightX;
    newViewCurve.range[0]  = bottomY / s;
    newViewCurve.range[1]  = topY / s;

    vw->SetViewCurve(newViewCurve);

    //
    // It looks like we need to explicitly re-render.
    //
    proxy.Render();
}


// ****************************************************************************
//  Method: ZoomCurve::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   April 12, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:59:50 PDT 2003
//    I changed the routine to match the new definition of avtViewCurve.
//
// ****************************************************************************

void
ZoomCurve::ZoomCamera(const int x, const int y)
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

        avtViewCurve newViewCurve = vw->GetViewCurve();

        double dX = ((1. / zoomFactor) - 1.) *
                    ((newViewCurve.domain[1] - newViewCurve.domain[0]) / 2.);
        double dY = ((1. / zoomFactor) - 1.) *
                    ((newViewCurve.range[1] - newViewCurve.range[0]) / 2.);

        newViewCurve.domain[0] -= dX;
        newViewCurve.domain[1] += dX;
        newViewCurve.range[0]  -= dY;
        newViewCurve.range[1]  += dY;

        vw->SetViewCurve(newViewCurve);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
