/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              NavigateCurve.C                              //
// ************************************************************************* //

#include <NavigateCurve.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: NavigateCurve constructor
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

NavigateCurve::NavigateCurve(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: NavigateCurve::OnTimer
//
//  Purpose:
//    Handles the timer event.  For NavigateCurve, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned or zoomed.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
NavigateCurve::OnTimer(void)
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
//  Method: NavigateCurve::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For NavigateCurve, this means
//    panning.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
NavigateCurve::StartLeftButtonAction()
{
    StartPan();
}


// ****************************************************************************
//  Method: NavigateCurve::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For NavigateCurve, this means
//    panning.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
//  Modifications:
//    Eric Brugger, Thu Nov 20 15:22:59 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
NavigateCurve::EndLeftButtonAction()
{
    EndPan();

    IssueViewCallback();
}


// ****************************************************************************
//  Method: NavigateCurve::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For NavigateCurve, this 
//    means zooming.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
NavigateCurve::StartMiddleButtonAction()
{
    StartDolly();
}


// ****************************************************************************
//  Method: NavigateCurve::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For NavigateCurve, this means
//    zooming.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
//  Modifications:
//    Eric Brugger, Thu Nov 20 15:22:59 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
NavigateCurve::EndMiddleButtonAction()
{
    EndDolly();

    IssueViewCallback();
}


// ****************************************************************************
//  Method: NavigateCurve::PanCamera
//
//  Purpose:
//    Handle panning the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
NavigateCurve::PanCamera(const int x, const int y)
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

        double    pan[2];

        avtViewCurve newViewCurve = vw->GetViewCurve();

        pan[0] = (double)(x - OldX) /
                 ((newViewCurve.viewport[1] - newViewCurve.viewport[0]) *
                  (double)(size[0])) *
                 (newViewCurve.domain[1] - newViewCurve.domain[0]);
        pan[1] = (double)(y - OldY) /
                 ((newViewCurve.viewport[3] - newViewCurve.viewport[2]) *
                  (double)(size[1])) *
                 (newViewCurve.range[1] - newViewCurve.range[0]);

        newViewCurve.domain[0] -= pan[0];
        newViewCurve.domain[1] -= pan[0];
        newViewCurve.range[0]  -= pan[1];
        newViewCurve.range[1]  -= pan[1];

        vw->SetViewCurve(newViewCurve);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: NavigateCurve::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
NavigateCurve::ZoomCamera(const int x, const int y)
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

