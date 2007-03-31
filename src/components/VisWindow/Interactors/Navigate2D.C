// ************************************************************************* //
//                                Navigate2D.C                               //
// ************************************************************************* //

#include <Navigate2D.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: Navigate2D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

Navigate2D::Navigate2D(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: Navigate2D::OnTimer
//
//  Purpose:
//    Handles the timer event.  For Navigate2D, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned or zoomed.
//
//  Programmer: Eric Brugger
//  Creation:   October 10, 2003
//
// ****************************************************************************

void
Navigate2D::OnTimer(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    int LastPos[2];
    rwi->GetLastEventPosition(LastPos);

    switch (State)
    {
      case VTKIS_PAN:
        PanCamera(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_DOLLY:
        ZoomCamera(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      default:
        break;
    }
}


// ****************************************************************************
//  Method: Navigate2D::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For Navigate2D, this means
//     panning.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
Navigate2D::StartLeftButtonAction()
{
    StartPan();
}


// ****************************************************************************
//  Method: Navigate2D::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For Navigate2D, this means
//    panning.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonUp.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
//    Eric Brugger, Thu Nov 20 15:24:17 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Navigate2D::EndLeftButtonAction()
{
    EndPan();

    IssueViewCallback();
}


// ****************************************************************************
//  Method: Navigate2D::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For Navigate2D, this 
//    means zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.  Zoom no longer 
//    implemented by parent class, Dolly performs same function. 
//
// ****************************************************************************

void
Navigate2D::StartMiddleButtonAction()
{
    StartDolly();
}


// ****************************************************************************
//  Method: Navigate2D::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For Navigate2D, this means
//    zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonUp.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.  Zoom no longer 
//    implemented by parent class, Dolly performs same function. 
//
//    Eric Brugger, Thu Nov 20 15:24:17 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Navigate2D::EndMiddleButtonAction()
{
    EndDolly();

    IssueViewCallback();
}


// ****************************************************************************
//  Method: Navigate2D::PanCamera
//
//  Purpose:
//    Handle panning the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 10, 2003
//
// ****************************************************************************

void
Navigate2D::PanCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        //
        // Determine the size of the window.
        //
        int       size[2];

        rwi->GetSize(size);

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        double    viewport[4];
        double    pan[2];

        avtView2D newView2D = vw->GetView2D();

        newView2D.GetActualViewport(viewport, size[0], size[1]);
        
        pan[0] = (double)(x - OldX) /
                 ((viewport[1] - viewport[0]) * (double)(size[0])) *
                 (newView2D.window[1] - newView2D.window[0]);
        pan[1] = (double)(y - OldY) /
                 ((viewport[3] - viewport[2]) * (double)(size[1])) *
                 (newView2D.window[3] - newView2D.window[2]);

        newView2D.window[0] -= pan[0];
        newView2D.window[1] -= pan[0];
        newView2D.window[2] -= pan[1];
        newView2D.window[3] -= pan[1];

        vw->SetView2D(newView2D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: Navigate2D::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 10, 2003
//
// ****************************************************************************

void
Navigate2D::ZoomCamera(const int x, const int y)
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
        // Calculate the new parallel scale.
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

