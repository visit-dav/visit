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
//                                  ZoomAxisArray.C                              //
// ************************************************************************* //

#include <ZoomAxisArray.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// ****************************************************************************
//  Method: ZoomAxisArray constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
// ****************************************************************************

ZoomAxisArray::ZoomAxisArray(VisWindowInteractorProxy &v) : ZoomInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: ZoomAxisArray::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For ZoomAxisArray, this means
//      a rubber band zoom mode.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    StartZoom();
    StartRubberBand(x, y);
}


// ****************************************************************************
//  Method: ZoomAxisArray::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed up.  For ZoomAxisArray, this means
//      a rubber band zoom mode.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::EndLeftButtonAction()
{
    EndRubberBand();
    ZoomCamera();
    EndZoom();
    IssueViewCallback();
}


// ****************************************************************************
//  Method: ZoomAxisArray::AbortLeftButtonAction
//
//  Purpose:
//      Handles an abort action for the left button.  This means giving up on
//      the rubber band.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::AbortLeftButtonAction()
{
    EndRubberBand();
    EndZoom();
}


// ****************************************************************************
//  Method: ZoomAxisArray::StartMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed down.  For ZoomAxisArray, this 
//      means standard zooming.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::StartMiddleButtonAction()
{
    StartZoom();
}


// ****************************************************************************
//  Method: ZoomAxisArray::EndMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed up.  For ZoomAxisArray, this means
//      standard panning.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::EndMiddleButtonAction()
{
    EndZoom();
    IssueViewCallback();
}


// ****************************************************************************
//  Method: ZoomAxisArray::ZoomCamera
//
//  Purpose:
//      Zooms the camera to be between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::ZoomCamera(void)
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
    double leftX   = (double) (anchorX < lastX ? anchorX : lastX);
    double rightX  = (double) (anchorX > lastX ? anchorX : lastX);
    double bottomY = (double) (anchorY < lastY ? anchorY : lastY);
    double topY    = (double) (anchorY > lastY ? anchorY : lastY);
    double dummyZ  = 0.;

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

    avtViewAxisArray newViewAxisArray=vw->GetViewAxisArray();

    int       size[2];

    vtkRenderWindowInteractor *rwi = Interactor;
    rwi->GetSize(size);

    double s = newViewAxisArray.GetScaleFactor(size);

    if (!controlKeyDown) // zoom
    {
        newViewAxisArray.domain[0] = leftX;
        newViewAxisArray.domain[1] = rightX;
        newViewAxisArray.range[0]  = bottomY/s;
        newViewAxisArray.range[1]  = topY/s;
    }
    else // un-zoom 
    {
        float win1[4], win2[4], win3[4], win4[4];

        // window created by rubber band
        win1[0] = leftX;
        win1[1] = rightX;
        win1[2] = bottomY;
        win1[3] = topY;
        float win1_w = win1[1] - win1[0];
        float win1_h = win1[3] - win1[2];

        // the current window 
        win2[0] = newViewAxisArray.domain[0];
        win2[1] = newViewAxisArray.domain[1];
        win2[2] = newViewAxisArray.range[0];
        win2[3] = newViewAxisArray.range[1];
        float win2_w = win2[1] - win2[0];
        float win2_h = win2[3] - win2[2];

        float scaleX = win1_w / win2_w;
        float scaleY = win1_h / win2_h;

        if (scaleY < scaleX)
        {
            float midX = (win2[0] + win2[1]) / 2.;
            float halfw = (win2_h) * (win1_w / win1_h) / 2.;
            win3[0] = midX - halfw;
            win3[1] = midX + halfw;
            win3[2] = win2[2];
            win3[3] = win2[3];
        }
        else 
        {
            float midY = (win2[2] + win2[3]) /2.;
            float halfh = (win2_w) * (win1_h / win1_w) / 2.;
            win3[0] = win2[0];
            win3[1] = win2[1]; 
            win3[2] = midY - halfh;
            win3[3] = midY + halfh;
        }

        float win3_w = (win3[1] - win3[0]);
        float win3_h = (win3[3] - win3[2]);

        win4[0] = ((win1[0] - win2[0]) / win2_w) * win3_w + win3[0];
        win4[1] = ((win1[1] - win2[0]) / win2_w) * win3_w + win3[0];
        win4[2] = ((win1[2] - win2[2]) / win2_h) * win3_h + win3[2];
        win4[3] = ((win1[3] - win2[2]) / win2_h) * win3_h + win3[2];

        float win4_w = (win4[1] - win4[0]);
        float win4_h = (win4[3] - win4[2]);

        newViewAxisArray.domain[0] = (win3[0] - win4[0]) * win3_w / win4_w + win3[0];
        newViewAxisArray.domain[1] = (win3[1] - win4[0]) * win3_w / win4_w + win3[0];
        newViewAxisArray.range[0]  = (win3[2] - win4[2]) * win3_h / win4_h + win3[2];
        newViewAxisArray.range[1]  = (win3[3] - win4[2]) * win3_h / win4_h + win3[2];

        newViewAxisArray.range[0] /= s;
        newViewAxisArray.range[1] /= s;
    }

    vw->SetViewAxisArray(newViewAxisArray);

    //
    // It looks like we need to explicitly re-render.
    //
    proxy.Render();
}


// ****************************************************************************
//  Method: ZoomAxisArray::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ZoomAxisArray::ZoomCamera(const int x, const int y)
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

        avtViewAxisArray newViewAxisArray = vw->GetViewAxisArray();

        double dX = ((1. / zoomFactor) - 1.) *
                    ((newViewAxisArray.domain[1] - newViewAxisArray.domain[0]) / 2.);
        double dY = ((1. / zoomFactor) - 1.) *
                    ((newViewAxisArray.range[1] - newViewAxisArray.range[0]) / 2.);

        newViewAxisArray.domain[0] -= dX;
        newViewAxisArray.domain[1] += dX;
        newViewAxisArray.range[0]  -= dY;
        newViewAxisArray.range[1]  += dY;

        vw->SetViewAxisArray(newViewAxisArray);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
