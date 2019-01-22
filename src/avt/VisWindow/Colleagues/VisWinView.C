/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                               VisWinView.C                                //
// ************************************************************************* //

#include <float.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <VisWindow.h>
#include <VisWinView.h>


// ****************************************************************************
//  Method: VisWinView constructor
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//    Hank Childs, Wed Dec 27 14:32:17 PST 2000
//    Added viewUp for view information.
//
//    Kathleen Bonnell, Mon Jan  8 11:56:50 PST 2001 
//    Made use of newly defined method 'SetToDefault'. 
//
//    Eric Brugger, Mon Mar 12 14:52:31 PST 2001
//    I removed the perspective projection from this class since it was
//    in the viewInfo where it really belonged.
//
// ****************************************************************************

VisWinView::VisWinView(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    viewInfo.SetToDefault();
}


// ****************************************************************************
//  Method: VisWinView::GetViewInfo
//
//  Purpose:
//    Gets the view info of the camera.
//
//  Returns:    The view info for the vis window.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//    Hank Childs, Wed Dec 27 14:32:17 PST 2000
//    Added viewUp for view information.
//
//    Kathleen Bonnell, Thu Jan  4 15:45:05 PST 2001 
//    Removed code that set the data members of viewInfo, as this is now 
//    contained in the avtViewInfo method 'SetViewFromCamera'. 
//
// ****************************************************************************

const avtViewInfo &
VisWinView::GetViewInfo(void)
{
    viewInfo.SetViewFromCamera(mediator.GetCanvas()->GetActiveCamera());
    return viewInfo;
}


// ****************************************************************************
//  Method: VisWinView::SetViewInfo
//
//  Purpose:
//    Sets the view info to reflect the argument and updates the internal
//    information.
//
//  Arguments:
//    vI        The new view information.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//    Hank Childs, Wed Dec 27 14:32:17 PST 2000
//    Added viewUp and near/far for view information.
//
//    Kathleen Bonnell, Thu Jan  4 15:45:05 PST 2001 
//    Made this method use avtViewInfo's assignment operator. 
//
//    Hank Childs, Fri Jan  5 17:55:42 PST 2001
//    Pushed logic for antiquated method SetCameraToView into this routine.
//
//    Kathleen Bonnell, Thu Aug 29 09:49:36 PDT 2002
//    Added call to UpdateLightPositions. 
//
// ****************************************************************************

void
VisWinView::SetViewInfo(const avtViewInfo &vI)
{
    viewInfo = vI;
    viewInfo.SetCameraFromView(mediator.GetCanvas()->GetActiveCamera());

    //
    //  Changes to the camera may necessitate changes to light positions.
    //  Would have the vis window handle the call, but it should be done
    //  before the Render.
    //
    mediator.UpdateLightPositions();
    //
    // Do an explicit render to make our changes show up.
    //
    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinView::Start2DMode
//
//  Purpose:
//    Takes the camera out of perspective projection because it doesn't make
//    sense for 2D.
//
//  Programmer: Hank Childs
//  Creation:   November 10, 2000
//
//  Modifications:
//    Eric Brugger, Mon Mar 12 14:52:31 PST 2001
//    I modified the routine to use the viewInfo's orthographic mode instead
//    of the VisWinView's perspectiveProjection since it was removed.
//
// ****************************************************************************

void
VisWinView::Start2DMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(1);
    }
}


// ****************************************************************************
//  Method: VisWinView::Stop2DMode
//
//  Purpose:
//    Takes the camera back into perspective projection if it should be, but
//    we disabled it for 2D mode.
//
//  Programmer: Hank Childs
//  Creation:   November 10, 2000
//
//  Modifications:
//    Eric Brugger, Mon Mar 12 14:52:31 PST 2001
//    I modified the routine to use the viewInfo's orthographic mode instead
//    of the VisWinView's perspectiveProjection since it was removed.
//
// ****************************************************************************

void
VisWinView::Stop2DMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(0);
    }
}


// ****************************************************************************
//  Method: VisWinView::StartCurveMode
//
//  Purpose:
//    Takes the camera out of perspective projection because it doesn't make
//    sense for Curve.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002 
//
// ****************************************************************************

void
VisWinView::StartCurveMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(1);
    }
}


// ****************************************************************************
//  Method: VisWinView::StopCurveMode
//
//  Purpose:
//    Takes the camera back into perspective projection if it should be, but
//    we disabled it for Curve mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002 
//
// ****************************************************************************

void
VisWinView::StopCurveMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(0);
    }
}


// ****************************************************************************
//  Method: VisWinView::StartAxisArrayMode
//
//  Purpose:
//    Takes the camera out of perspective projection because it doesn't make
//    sense for AxisArray.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 29, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinView::StartAxisArrayMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(1);
    }
}


// ****************************************************************************
//  Method: VisWinView::StopAxisArrayMode
//
//  Purpose:
//    Takes the camera back into perspective projection if it should be, but
//    we disabled it for AxisArray mode.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 29, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinView::StopAxisArrayMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(0);
    }
}


// ****************************************************************************
//  Method: VisWinView::StartParallelAxesMode
//
//  Purpose:
//    Takes the camera out of perspective projection because it doesn't make
//    sense for ParallelAxes.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinView::StartParallelAxesMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(1);
    }
}


// ****************************************************************************
//  Method: VisWinView::StopParallelAxesMode
//
//  Purpose:
//    Takes the camera back into perspective projection if it should be, but
//    we disabled it for ParallelAxes mode.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinView::StopParallelAxesMode(void)
{
    vtkCamera *camera = mediator.GetCanvas()->GetActiveCamera();
    if (!viewInfo.orthographic)
    {
        camera->SetParallelProjection(0);
    }
}


