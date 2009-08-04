/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                                Navigate3D.C                               //
// ************************************************************************* //

#include <Navigate3D.h>

#include <avtVector.h>
#include <avtMatrix.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>


// ****************************************************************************
//  Method: Navigate3D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Initialized shouldSpin.
//
// ****************************************************************************

Navigate3D::Navigate3D(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ctrlOrShiftPushed = false;
    shouldSpin = false;
}

// ****************************************************************************
//  Method: Navigate3D::OnTimer
//
//  Purpose:
//    Handles the timer event.  For Navigate3D, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned, zoomed or rotated.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Hank Childs, Wed May 29 14:15:51 PDT 2002
//    If spin mode was set to be false while we are mid-spin, then honor that
//    and stop spinning.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Retrieve the LastPosition from the renderWindowInteractor.  It is no
//    longer a member of the parent class. 
//
//    Brad Whitlock, Wed Sep 10 16:05:08 PST 2003
//    I added support for temporarily suspending spin mode.
//
//    Eric Brugger, Tue Dec 28 16:33:02 PST 2004
//    I moved RotateCamera, PanCamera and ZoomCamera to the VisitInteractor
//    class as RotateAboutFocus3D, PanImage3D and ZoomImage3D.
//
//    Hank Childs, Thu Dec 29 10:30:53 PST 2005
//    Issue a view callback when in spin mode.  ['4231]
//
// ****************************************************************************

void
Navigate3D::OnTimer(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    int LastPos[2];
    rwi->GetLastEventPosition(LastPos);

    bool matchedUpState = true;
    switch (State)
    {
      case VTKIS_ROTATE:
        RotateAboutFocus3D(LastPos[0], LastPos[1], false);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_PAN:
        PanImage3D(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_ZOOM:
        ZoomImage3D(LastPos[0], LastPos[1]);

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

// ****************************************************************************
//  Method: Navigate3D::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For Navigate3D, this means
//    panning if the ctrl or shift is pushed, rotating otherwise.  Also,
//    this should start bounding box mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for starting bounding box into base class.
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api, they are accessed
//    directly through the Interactor now.  
//
// ****************************************************************************

void
Navigate3D::StartLeftButtonAction()
{
    DisableSpinMode();

    StartBoundingBox();

    //
    // If ctrl or shift is pushed, pan, otherwise rotate.  Save which one we
    // did so we can issue the proper "End.." statement when the button is
    // released.
    //
    if (Interactor->GetControlKey()|| Interactor->GetShiftKey())
    {
        StartPan();
        ctrlOrShiftPushed = true;
    }
    else
    {
        StartRotate();
        ctrlOrShiftPushed = false;
    }
}

// ****************************************************************************
//  Method: Navigate3D::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For Navigate3D, this means
//    panning if the ctrl or shift button was held down while the left
//    button was pushed, a rotation otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonUp.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for end bounding box into base class.
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api. 
//
//    Eric Brugger, Thu Nov 20 15:24:48 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Navigate3D::EndLeftButtonAction()
{
    //
    // We must issue the proper end state for either pan or rotate depending
    // on whether the shift or ctrl button was pushed.
    //
    if (ctrlOrShiftPushed)
    {
        EndPan();
    }
    else
    {
        EndRotate();

        EnableSpinMode();
    }

    EndBoundingBox();

    IssueViewCallback();
}

// ****************************************************************************
//  Method: Navigate3D::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For Navigate3D, this 
//    means zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonDown.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for starting bounding box into base class.
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
Navigate3D::StartMiddleButtonAction()
{
    DisableSpinMode();

    StartBoundingBox();

    StartZoom();
}

// ****************************************************************************
//  Method: Navigate3D::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For Navigate3D, this means
//    ending a zoom.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonUp.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for ending bounding box into base class.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
//    Eric Brugger, Thu Nov 20 15:24:48 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Navigate3D::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();

    IssueViewCallback();
}

// ****************************************************************************
//  Method: Navigate3D::OnMouseWheelForward()
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
// ****************************************************************************

void
Navigate3D::OnMouseWheelForward()
{
    StartZoom();
    ZoomImage3D(MotionFactor * 0.2 * this->MouseWheelMotionFactor);
    EndZoom();
}


// ****************************************************************************
//  Method: Navigate3D::OnMouseWheelBackward()
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
// ****************************************************************************

void
Navigate3D::OnMouseWheelBackward()
{
    StartZoom();
    ZoomImage3D(MotionFactor * -0.2 * this->MouseWheelMotionFactor);
    EndZoom();
}

// ****************************************************************************
//  Method: Navigate3D::EnableSpinMode
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
Navigate3D::EnableSpinMode(void)
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
//  Method: Navigate3D::DisableSpinMode
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
Navigate3D::DisableSpinMode(void)
{
    if (shouldSpin)
    {
        EndTimer();
        shouldSpin = false;
    }
}
