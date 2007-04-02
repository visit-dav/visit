/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//    Eric Brugger, Fri Nov 21 08:03:45 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Zoom3D::EndLeftButtonAction()
{
    EndRubberBand();
    ZoomCamera();
    EndZoom();
    IssueViewCallback();
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
//    Eric Brugger, Fri Nov 21 08:03:45 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Zoom3D::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();

    IssueViewCallback();
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
//    Eric Brugger, Thu Jun 12 12:35:20 PDT 2003
//    I changed the zoom behavior so that it pans and zooms the image and
//    doesn't change the camera and focal point.
//
//    Akira Haddox, Thu Jul  3 13:54:59 PDT 2003
//    Changed check for not zooming to include line rubberbands.
//
//    Kathleen Bonnell, Wed Aug  4 07:59:41 PDT 2004 
//    Added logic for un-zoom. 
//
// ****************************************************************************

void
Zoom3D::ZoomCamera(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (anchorX == lastX || anchorY == lastY)
    {
        //
        // This is a line, not a rectangle.
        //
        return;
    }

    //
    // Determine the size of the window.
    //
    int       size[2];

    rwi->GetSize(size);

    //
    // Get the current view information.
    //
    VisWindow *vw = proxy;

    //
    // Set the new image pan and image zoom.
    //
    double    zoomFactor;
    double    pan[2];

    avtView3D newView3D = vw->GetView3D();

    if (!controlKeyDown) // zoom
    {
        pan[0] = (((double)(anchorX + lastX - size[0])) / (2.0 * (double)size[0]))
             / newView3D.imageZoom;
        pan[1] = (((double)(anchorY + lastY - size[1])) / (2.0 * (double)size[1]))
             / newView3D.imageZoom;
        zoomFactor = fabs((double)(anchorY - lastY)) / (double) size[1];

        newView3D.imagePan[0] -= pan[0];
        newView3D.imagePan[1] -= pan[1];
        newView3D.imageZoom = newView3D.imageZoom / zoomFactor;
    }
    else  // unzoom
    {
        zoomFactor = fabs((double)(anchorY - lastY)) / (double) size[1];
        newView3D.imageZoom = newView3D.imageZoom * zoomFactor;

        pan[0] = (((double)(anchorX + lastX - size[0])) / (2.0 * (double)size[0]))
             / newView3D.imageZoom;
        pan[1] = (((double)(anchorY + lastY - size[1])) / (2.0 * (double)size[1]))
             / newView3D.imageZoom;

        newView3D.imagePan[0] += pan[0];
        newView3D.imagePan[1] += pan[1];
    }

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
//  Modifications:
//    Eric Brugger, Fri Jun 13 09:06:00 PDT 2003
//    I changed the zoom behavior so that it zooms the image and doesn't
//    change the camera or focal point.
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

        newView3D.imageZoom = newView3D.imageZoom * zoomFactor;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
