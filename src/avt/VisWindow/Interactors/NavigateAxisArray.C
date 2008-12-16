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
//                            NavigateAxisArray.C                            //
// ************************************************************************* //

#include <NavigateAxisArray.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: NavigateAxisArray constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:58:11 EST 2008
//    Added support for toggling horizontal snap-to-grid.
//
//    Eric Brugger, Tue Dec  9 15:56:38 PST 2008
//    Added an axis orientation, which interchanges the horizontal and
//    vertical zooming.
//
// ****************************************************************************

NavigateAxisArray::NavigateAxisArray(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    shiftKeyDown = controlKeyDown = false;
    VisWindow *win = v;
    shouldSnap = win->GetInteractorAtts()->GetAxisArraySnap();
    axisOrientation = Vertical;
}


// ****************************************************************************
//  Method: NavigateAxisArray::OnTimer
//
//  Purpose:
//    Handles the timer event.  For NavigateAxisArray, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned or zoomed.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:58:11 EST 2008
//    Added support for toggling horizontal snap-to-grid.
//
// ****************************************************************************

void
NavigateAxisArray::OnTimer(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    int LastPos[2];
    rwi->GetLastEventPosition(LastPos);

    VisWindow *win = proxy;
    shouldSnap = win->GetInteractorAtts()->GetAxisArraySnap();

    switch (State)
    {
      case VTKIS_PAN:
        PanCamera(LastPos[0], LastPos[1], shouldSnap);

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
//  Method: NavigateAxisArray::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For NavigateAxisArray, this means
//     panning.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::StartLeftButtonAction()
{
    StartPan();
}


// ****************************************************************************
//  Method: NavigateAxisArray::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For NavigateAxisArray, this means
//    panning.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::EndLeftButtonAction()
{
    EndPan();

    IssueViewCallback();
}


// ****************************************************************************
//  Method: NavigateAxisArray::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For NavigateAxisArray, this 
//    means zooming.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::StartMiddleButtonAction()
{
    shiftKeyDown = Interactor->GetShiftKey();
    controlKeyDown = Interactor->GetControlKey();

    StartDolly();
}


// ****************************************************************************
//  Method: NavigateAxisArray::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For NavigateAxisArray, this means
//    zooming.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::EndMiddleButtonAction()
{
    EndDolly();

    IssueViewCallback();
}

// ****************************************************************************
//  Method: NavigateAxisArray::OnMouseWheelForward()
//
//  Purpose:
//    Handles the mouse wheel turned backward.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Tue Dec  9 15:56:38 PST 2008
//    Added an axis orientation, which interchanges the horizontal and
//    vertical zooming.
//
// ****************************************************************************

void
NavigateAxisArray::OnMouseWheelForward()
{
    StartZoom();
    if (axisOrientation == Vertical)
        ZoomHorizontalFixed(-0.25 * this->MouseWheelMotionFactor);
    else
        ZoomVerticalFixed(-0.25 * this->MouseWheelMotionFactor);
    EndZoom();
}

// ****************************************************************************
//  Method: NavigateAxisArray::OnMouseWheelBackward()
//
//  Purpose:
//    Handles the mouse wheel turned forward.  
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Tue Dec  9 15:56:38 PST 2008
//    Added an axis orientation, which interchanges the horizontal and
//    vertical zooming.
//
// ****************************************************************************

void
NavigateAxisArray::OnMouseWheelBackward()
{
    StartZoom();
    if (axisOrientation == Vertical)
        ZoomHorizontalFixed(0.25 * this->MouseWheelMotionFactor);
    else
        ZoomVerticalFixed(0.25 * this->MouseWheelMotionFactor);
    EndZoom();
}

// ****************************************************************************
//  Method: NavigateAxisArray::SetAxisOrientation
//
//  Purpose:
//    Set the axis orientation.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
NavigateAxisArray::SetAxisOrientation(const AxisOrientation orientation)
{
    axisOrientation = orientation;
}

// ****************************************************************************
//  Method: NavigateAxisArray::PanCamera
//
//  Purpose:
//    Handle panning the camera.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
NavigateAxisArray::PanCamera(const int x, const int y, bool snap_horiz)
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

        avtViewAxisArray newView = vw->GetViewAxisArray();
        
        double xscale = (newView.domain[1] - newView.domain[0]) /
            ((newView.viewport[1] - newView.viewport[0]) * (double)(size[0]));
        double yscale = (newView.range[1] - newView.range[0]) /
            ((newView.viewport[3] - newView.viewport[2]) * (double)(size[0]));

        pan[0] = (double)(x - OldX) * xscale;
        pan[1] = (double)(y - OldY) * yscale;

        newView.domain[0] -= pan[0];
        newView.domain[1] -= pan[0];
        newView.range[0]  -= pan[1];
        newView.range[1]  -= pan[1];

        // perform a horizontal snap
        int newX = x;
        if (snap_horiz)
        {
            double dx0 = newView.domain[0] - double(int(newView.domain[0]));
            double dx1 = newView.domain[1] - double(int(newView.domain[1]));
            double dx0check = dx0 / (newView.domain[1] - newView.domain[0]);
            double dx1check = dx1 / (newView.domain[1] - newView.domain[0]);

            const double threshold = 0.025;
            if (fabs(dx0check) < threshold)
            {
                newView.domain[0] -= dx0;
                newView.domain[1] -= dx0;
                newX += int(.5 + dx0 / xscale);
            }
            else if (fabs(dx1check) < threshold)
            {
                newView.domain[0] -= dx1;
                newView.domain[1] -= dx1;
                newX += int(.5 + dx1 / xscale);
            }
        }

        if (newX == OldX && OldY == y)
        {
            // do nothing
        }
        else
        {
            vw->SetViewAxisArray(newView);

            OldX = newX;
            OldY = y;
            rwi->Render();
        }
    }
}

// ****************************************************************************
//  Method: NavigateAxisArray::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Tue Dec  9 15:56:38 PST 2008
//    Added an axis orientation, which interchanges the horizontal and
//    vertical zooming.
//
// ****************************************************************************

void
NavigateAxisArray::ZoomCamera(const int x, const int y)
{
    if (OldY != y || OldX != x)
    {
        vtkRenderWindowInteractor *rwi = Interactor;

        //
        // Calculate the zoom factor.
        //
        double dyf = MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);
        double dxf = MotionFactor * (double)(x - OldX) /
                         (double)(Center[0]);

	//
	// Perform zoom
	//
        if (shiftKeyDown || controlKeyDown)
        {
            if (axisOrientation == Vertical)
                ZoomHorizontal(dyf);
            else
                ZoomVertical(dyf);
        }
        else
        {
            if (axisOrientation == Vertical)
                ZoomVertical(dyf);
            else
                ZoomHorizontal(dyf);
        }
        
        //
        // Experimenting with horizontal and vertical zooming
        // based on hor/vert mouse motion..
        //
        //         if (fabs(dxf) < fabs(dyf))
        //         {
        //             ZoomVertical(dyf);
        //         }
        //         else
        //         {
        //             ZoomHorizontal(dxf);
        //         }

        // Redraw
        rwi->Render();

	//
	// Update old mouse position
	//
        OldX = x;
        OldY = y;
    }
}

// ****************************************************************************
//  Method: NavigateAxisArray::ZoomHorizontal
//
//  Purpose:
//    Zoom the camera horizontally by a continuous factor.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Tue Dec  9 15:56:38 PST 2008
//    I corrected an error in one of the calculations, where the range
//    was used instead of the domain.
//
// ****************************************************************************

void
NavigateAxisArray::ZoomHorizontal(double f)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    //
    // Calculate the zoom factor.
    //
    double zoomFactor = pow((double)1.1, f);

    //
    // Calculate the new parallel scale.
    //
    VisWindow *vw = proxy;

    avtViewAxisArray newViewAxisArray = vw->GetViewAxisArray();

    double xDist = newViewAxisArray.domain[1] - newViewAxisArray.domain[0];
    double dX = ((1. / zoomFactor) - 1.) * (xDist / 2.);

    newViewAxisArray.domain[0] -= dX;
    newViewAxisArray.domain[1] += dX;

    vw->SetViewAxisArray(newViewAxisArray);
}

// ****************************************************************************
//  Method: NavigateAxisArray::ZoomHorizontalFixed
//
//  Purpose:
//    Zoom the camera horizontally by a fixed amount.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::ZoomHorizontalFixed(double f)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    VisWindow *vw = proxy;

    avtViewAxisArray newViewAxisArray = vw->GetViewAxisArray();

    newViewAxisArray.domain[0] -= f;
    newViewAxisArray.domain[1] += f;
    if (newViewAxisArray.domain[0] >= newViewAxisArray.domain[1])
    {
        newViewAxisArray.domain[0] += f;
        newViewAxisArray.domain[1] -= f;
    }

    vw->SetViewAxisArray(newViewAxisArray);
}

// ****************************************************************************
//  Method: NavigateAxisArray::ZoomVertical
//
//  Purpose:
//    Zoom the camera vertically by a continuous factor.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::ZoomVertical(double f)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    //
    // Calculate the zoom factor.
    //
    double zoomFactor = pow((double)1.1, f);

    //
    // Calculate the new parallel scale.
    //
    VisWindow *vw = proxy;

    avtViewAxisArray newViewAxisArray = vw->GetViewAxisArray();

    double yDist = newViewAxisArray.range[1] - newViewAxisArray.range[0];
    double dY = ((1. / zoomFactor) - 1.) * (yDist / 2.);

    newViewAxisArray.range[0]  -= dY;
    newViewAxisArray.range[1]  += dY;

    vw->SetViewAxisArray(newViewAxisArray);
}

// ****************************************************************************
//  Method: NavigateAxisArray::ZoomVerticalFixed
//
//  Purpose:
//    Zoom the camera vertically by a fixed amount.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//
// ****************************************************************************

void
NavigateAxisArray::ZoomVerticalFixed(double f)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    VisWindow *vw = proxy;

    avtViewAxisArray newViewAxisArray = vw->GetViewAxisArray();

    newViewAxisArray.range[0] -= f;
    newViewAxisArray.range[1] += f;
    if (newViewAxisArray.range[0] >= newViewAxisArray.domain[1])
    {
        newViewAxisArray.range[0] += f;
        newViewAxisArray.range[1] -= f;
    }

    vw->SetViewAxisArray(newViewAxisArray);
}
