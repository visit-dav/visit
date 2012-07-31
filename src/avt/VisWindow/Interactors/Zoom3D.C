/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
    ctrlOrShiftPushed = false;
    shouldSpin = false;    
}


// ****************************************************************************
//  Method: Zoom3D::OnTimer
//
//  Purpose:
//      Throw out the timer events so we can do zooms using only mouse
//      movements.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 2001
//
//  Modifications:
//    Eric Brugger, Fri Apr 12 14:11:55 PDT 2002
//    I modified the routine to handle the zooming in here rather than in
//    superclass.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002 
//    Retreive LastPos from RenderWindowInteractor as it is no longer a member
//    of the parent class. 
//    
//    Kathleen Bonnell, Wed Jun  8 10:01:52 PDT 2011
//    Use current EventPostion instead of last.
//
// ****************************************************************************

void
Zoom3D::OnTimer(void)
{
    if (!rubberBandMode)
    {
        vtkRenderWindowInteractor *rwi = Interactor;

        int Pos[2];
        rwi->GetEventPosition(Pos);

        bool matchedUpState = true;
        switch (State)
        {
        case VTKIS_PAN:
          PanImage3D(Pos[0], Pos[1]);
          
          rwi->CreateTimer(VTKI_TIMER_UPDATE);
          break;
          
        case VTKIS_ZOOM:
          ZoomImage3D(Pos[0], Pos[1]);
          
          rwi->CreateTimer(VTKI_TIMER_UPDATE);
          break;
          
        default:
          matchedUpState = false;
          break;
        }
        
        if (!matchedUpState && shouldSpin)
        {
            VisWindow *vw = proxy;
            if(!vw->GetSpinModeSuspended())
            {
                if (vw->GetSpinMode())
                {
                  OldX = spinOldX;
                  OldY = spinOldY;
                  RotateAboutFocus3D(spinNewX, spinNewY, false);
                  IssueViewCallback(true);
                  rwi->CreateTimer(VTKI_TIMER_UPDATE);
                }
                else
                {
                    DisableSpinMode();
                }
            }
            else if(vw->GetSpinMode())
            {
              // Don't mess with the camera, just create another timer so
              // we keep getting into this method until spin mode is no
              // longer suspended.
              rwi->CreateTimer(VTKI_TIMER_UPDATE);
            }
        }
    }
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
    DisableSpinMode();

    //
    // If ctrl or shift is pushed, pan, otherwise zoom.  Save which one we
    // did so we can issue the proper "End.." statement when the button is
    // released.
    //
    if (Interactor->GetControlKey() || Interactor->GetShiftKey())
    {
        StartBoundingBox();
        StartPan();
        ctrlOrShiftPushed = true;
    }
    else
    {
        int x, y;
        Interactor->GetEventPosition(x, y);
        StartZoom();
        StartRubberBand(x, y);
        ctrlOrShiftPushed = false;
    }
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
    //
    // We must issue the proper end state for either pan or rotate depending
    // on whether the shift or ctrl button was pushed.
    //
    if (ctrlOrShiftPushed)
    {
        EndBoundingBox();
        EndPan();
    }
    else
    {
        EndRubberBand();
        ZoomCamera();
        EndZoom();
    }

    EnableSpinMode();

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
    if (ctrlOrShiftPushed)
    {
        EndBoundingBox();
        EndPan();
    }
    else
    {
        EndRubberBand();
        EndZoom();
    }
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
//      standard zooming.
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
//    Eric Brugger, Thu May 26 12:29:20 PDT 2011
//    Remove an unnecessary render call.
//
//    Kathleen Bonnell, Wed Jun  8 14:11:39 PDT 2011
//    Add to (instead of subtract from ) imagePan when doing zoom.
//
//    Marc Durant, Mon Dec 19 14:40:00 MST 2011
//    Cancelling zoom action if mouse movement was too small.
//
// ****************************************************************************

void
Zoom3D::ZoomCamera(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (!SufficientDistanceMoved())
    {
        //
        // This is a point, line, or very, very small rectangle
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
//    Eric Brugger, Thu May 26 12:29:20 PDT 2011
//    Remove an unnecessary render call.
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

        OldX = x;
        OldY = y;

        vw->SetView3D(newView3D);
    }
}


// ****************************************************************************
//  Method: Zoom3D::OnMouseWheelForward()
//
//  Purpose:
//    Handles the mouse wheel turned backward.
//
//  Arguments:
//
//  Programmer: Gunther H. Weber
//  Creation:   August 07, 2007
//
//  Modifications:
//    Gunther H. Weber, Fri Sep 28 13:48:04 PDT 2007
//    Added missing StartZoom() / EndZoom()
//
//    Brad Whitlock, Fri Mar  2 14:19:45 PST 2012
//    I added a call to issue the view callback.
//
// ****************************************************************************

void
Zoom3D::OnMouseWheelForward()
{
    StartZoom();
    ZoomImage3D(MotionFactor * 0.2 * this->MouseWheelMotionFactor);
    EndZoom();
    IssueViewCallback(true);
}


// ****************************************************************************
//  Method: Zoom3D::OnMouseWheelBackward()
//
//  Purpose:
//    Handles the mouse wheel turned forward.  
//
//  Arguments:
//
//  Programmer: Gunther H. Weber
//  Creation:   August 07, 2007
//
//  Modifications:
//    Gunther H. Weber, Fri Sep 28 13:48:04 PDT 2007
//    Added missing StartZoom() / EndZoom()
//
//    Brad Whitlock, Fri Mar  2 14:19:45 PST 2012
//    I added a call to issue the view callback.
//
// ****************************************************************************

void
Zoom3D::OnMouseWheelBackward()
{
    StartZoom();
    ZoomImage3D(MotionFactor * -0.2 * this->MouseWheelMotionFactor);
    EndZoom();
    IssueViewCallback(true);
}

// ****************************************************************************
//  Method: Zoom3D::EnableSpinMode
//
//  Purpose:
//      Enables spin mode.  This will determine if spin mode is appropriate,
//      and make the correct calls to start it, if so.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
//  Modifications:
//
// ****************************************************************************

void
Zoom3D::EnableSpinMode(void)
{
    VisWindow *vw = proxy;
    if (vw->GetSpinMode())
    {
        shouldSpin = true;

        //
        // VTK will not be happy unless we enter one of its pre-defined modes.
        // Timer seems as appropriate as any (there idea of spin is much
        // different than ours).  Also, set up the first timer so our spinning
        // can get started.
        //
        StartTimer();
        vtkRenderWindowInteractor *rwi = Interactor;
        rwi->CreateTimer(VTKI_TIMER_UPDATE);
    }
}

// ****************************************************************************
//  Method: Zoom3D::DisableSpinMode
//
//  Purpose:
//      Disables spin mode if it is currently in action.  This may be called
//      at any time, even if spin mode is not currently on or even enabled.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

void
Zoom3D::DisableSpinMode(void)
{
    if (shouldSpin)
    {
        EndTimer();
        shouldSpin = false;
    }
}
