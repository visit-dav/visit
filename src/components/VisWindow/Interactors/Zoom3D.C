// ************************************************************************* //
//                                   Zoom3D.C                                //
// ************************************************************************* //

#include <Zoom3D.h>
#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: Zoom3D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
// ****************************************************************************

Zoom3D::Zoom3D(VisWindowInteractorProxy &v) : ZoomInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: Zoom3D::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Zoom3D, this means
//      a rubber band zoom mode.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 21 08:35:12 PST 2001
//    Update for VTK magic to make zoom work.
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments in order to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom3D::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    StartZoom();
    StartRubberBand(x, y);
}


// ****************************************************************************
//  Method: Zoom3D::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed up.  For Zoom3D, this means
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
//    Hank Childs, Fri Dec 21 08:35:12 PST 2001
//    Update for VTK magic to make zoom work.
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonUp.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments in order to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom3D::EndLeftButtonAction()
{
    EndRubberBand();
    ZoomCamera();
    EndZoom();
}


// ****************************************************************************
//  Method: Zoom3D::AbortLeftButtonAction
//
//  Purpose:
//      Handles an abort action for the left button.  This means giving up on
//      the rubber band.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2002
//
// ****************************************************************************

void
Zoom3D::AbortLeftButtonAction()
{
    EndRubberBand();
    EndZoom();
}


// ****************************************************************************
//  Method: Zoom3D::StartMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed down.  For Zoom3D, this 
//      means standard zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddlettonDown.
//
//    Hank Childs, Tue Mar 19 14:31:55 PST 2002
//    Pushed code to start bounding box into base class.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments in order to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom3D::StartMiddleButtonAction()
{
    StartBoundingBox();

    StartZoom();
}


// ****************************************************************************
//  Method: Zoom3D::EndMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed up.  For Zoom3D, this means
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
//    Hank Childs, Tue Mar 19 14:31:55 PST 2002
//    Pushed code to end bounding box into base class.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//    Removed arguments in order to comply with vtk's new interactor interface.
//
// ****************************************************************************

void
Zoom3D::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();
}


// ****************************************************************************
//  Method: Zoom3D::ZoomCamera
//
//  Purpose:
//      Zooms the camera to be between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Eric Brugger
//  Creation:    March 26, 2002
//
//  Modifications:
//    Eric Brugger, Fri Apr 12 12:37:26 PDT 2002
//    Correct a bug in the calculation of the pan factor.
//
// ****************************************************************************

void
Zoom3D::ZoomCamera(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (anchorX == lastX && anchorY == lastY)
    {
        //
        // This is a point, not a rectangle.
        //
        return;
    }

    //
    // Determine the size of the window.
    //
    int       size[2];
    int       width, height;

    rwi->GetSize(size);
    width  = size[0];
    height = size[1];

    //
    // Get the current view information.
    //
    VisWindow *vw = proxy;

    const avtView3D &oldView3D = vw->GetView3D();

    double    VPN[3];
    double    VUP[3];

    VPN[0] = oldView3D.normal[0];
    VPN[1] = oldView3D.normal[1];
    VPN[2] = oldView3D.normal[2];
    VUP[0] = oldView3D.viewUp[0];
    VUP[1] = oldView3D.viewUp[1];
    VUP[2] = oldView3D.viewUp[2];

    //
    // Calculate the coordinate transformation from the world coordinate
    // space to the screen coordinate space.
    //
    double    VUPCrossVPN[3];
    double    mat[3][3], matTranspose[3][3];

    VectorCross(VUP, VPN, VUPCrossVPN);
    MatrixSet(mat, VUPCrossVPN, VUP, VPN);
    MatrixTranspose(mat, matTranspose);

    //
    // Calculate the x and y pan distances in the view plane.
    //
    double    pan[3];

    pan[0] = (((double)(anchorX + lastX - width)) / (2.0 * (double)width)) *
             2.0 * oldView3D.parallelScale;
    pan[1] = (((double)(anchorY + lastY - height)) / (2.0 * (double)height)) *
             2.0 * oldView3D.parallelScale;
    pan[2] = 0.;

    //
    // Transform the screen space pan factors to world space pan factors.
    //
    double    pan2[3];

    VectorMatrixMult(pan, matTranspose, pan2);

    //
    // Calculate the zoom factor.
    //
    double    zoomFactor;

    zoomFactor = fabs((double)(anchorY - lastY)) / (double) height;

    //
    // Set the new origin and parallel scale.
    //
    avtView3D newView3D = vw->GetView3D();

    newView3D.focus[0] += pan2[0];
    newView3D.focus[1] += pan2[1];
    newView3D.focus[2] += pan2[2];

    newView3D.parallelScale = newView3D.parallelScale * zoomFactor;

    vw->SetView3D(newView3D);

    //
    // It looks like we need to explicitly re-render.
    //
    proxy.Render();
}


// ****************************************************************************
//  Method: Zoom3D::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   April 12, 2002
//
// ****************************************************************************

void
Zoom3D::ZoomCamera(const int x, const int y)
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

        avtView3D newView3D = vw->GetView3D();

        newView3D.parallelScale = newView3D.parallelScale / zoomFactor;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
