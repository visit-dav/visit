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
//                                Lineout2D.C                                //
// ************************************************************************* //

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRubberBandMapper2D.h>
#include <vtkLineSource.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
 
#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>
#include <Lineout2D.h>



// ****************************************************************************
//  Method: Lineout2D constructor
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//  
//  Modifications:
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004
//    Initialize doAlign.
//
// ****************************************************************************

Lineout2D::Lineout2D(VisWindowInteractorProxy &vw) : VisitInteractor(vw)
{
    rubberBandMode = false;
    doAlign = false;
    //
    // Create the poly data that will map the rubber band onto the screen.
    //
 
    rubberBand       = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(2);
    rubberBand->SetPoints(pts);
    pts->Delete();
 
    vtkCellArray *lines  = vtkCellArray::New();
    vtkIdType  ids[2] = { 0, 1 };
    lines->InsertNextCell(2, ids);
    rubberBand->SetLines(lines);
    lines->Delete();
 
    rubberBandMapper = vtkRubberBandMapper2D::New();
    rubberBandMapper->SetInput(rubberBand);
 
    rubberBandActor  = vtkActor2D::New();
    rubberBandActor->SetMapper(rubberBandMapper);
    rubberBandActor->GetProperty()->SetColor(0., 0., 0.);
 
}

 
// ****************************************************************************
//  Method: Lineout2D destructor
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
Lineout2D::~Lineout2D()
{
    rubberBand->Delete();
    rubberBandMapper->Delete();
    rubberBandActor->Delete();
}


// ****************************************************************************
//  Method: Lineout2D::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Lineout2D, this means
//      a rubber band mode.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Removed arguments to match vtk's new interactor api. 
//    Call GetEventPosition to determine x and y.
//
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004
//    Set doAlign.
//
// ****************************************************************************

void
Lineout2D::StartLeftButtonAction()
{
    //
    //  We need to set the state variable in vtkInteractorStyle,
    //  but currently there is no 'UserDefined' or such.  Using
    //  timer as it seems the most innocuous.
    // 
    int x, y;
    Interactor->GetEventPosition(x, y);
    doAlign = Interactor->GetShiftKey();
    StartTimer();
    StartRubberBand(x, y);
}

 
// ****************************************************************************
//  Method: Lineout2D::SetCanvasViewport
//
//  Purpose:
//      Sets where the canvas is in display coordinates so that rubber bands
//      outside the canvas can be forced inside the canvas.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
void
Lineout2D::SetCanvasViewport(void)
{
    vtkRenderer *ren = proxy.GetCanvas();
 
    canvasDeviceMinX = 0.;
    canvasDeviceMinY = 0.;
    ren->NormalizedViewportToViewport(canvasDeviceMinX, canvasDeviceMinY);
    ren->ViewportToNormalizedDisplay(canvasDeviceMinX, canvasDeviceMinY);
    ren->NormalizedDisplayToDisplay(canvasDeviceMinX, canvasDeviceMinY);
 
    canvasDeviceMaxX = 1.;
    canvasDeviceMaxY = 1.;
    ren->NormalizedViewportToViewport(canvasDeviceMaxX, canvasDeviceMaxY);
    ren->ViewportToNormalizedDisplay(canvasDeviceMaxX, canvasDeviceMaxY);
    ren->NormalizedDisplayToDisplay(canvasDeviceMaxX, canvasDeviceMaxY);
}

// ****************************************************************************
//  Method: Lineout2D::StartRubberBand
//
//  Purpose:
//      Tells the interactor to draw a rubber band and update it whenever the
//      mouse moves.
//
//  Arguments:
//      x       The x-coordinate of the pointer in display coordinates when
//              the button was pressed.
//      y       The y-coordinate of the pointer in display coordinates when
//              the button was pressed.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
void
Lineout2D::StartRubberBand(int x, int y)
{
    rubberBandMode = true;
 
    //
    // Add the rubber band actors to the background.  We do this since the
    // background is in the same display coordinates that the rubber band will
    // be.  From an appearance standpoint it should be in the canvas, which
    // the routine ForceCoordsToViewport ensures.
    //
    vtkRenderer *ren = proxy.GetBackground();
    ren->AddActor2D(rubberBandActor);
 
    //
    // The anchor of the rubber band will be where the button press was.
    //
    anchorX = x;
    anchorY = y;
 
    //
    // Determine what to clamp the rubber band to.
    //
    SetCanvasViewport();

    //
    // If the user has clicked outside the viewport, force the back inside.
    //
    ForceCoordsToViewport(anchorX, anchorY);
 
    //
    // Must update bookkeeping so that OnMouseMove works correctly.
    //
    lastX   = anchorX;
    lastY   = anchorY;
}

 
// ****************************************************************************
//  Method: Lineout2D::OnMouseMove
//
//  Purpose:
//      Receives a mouse movement and updates the rubber band.
//
//  Arguments:
//      ctrl          non-zero if the ctrl button was pushed.
//      shift         non-zero if the shift button was pushed.
//      x             The x-coordinate on the mouse pointer in display coords.
//      y             The y-coordinate on the mouse pointer in display coords.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modificationsy
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Removed arguments to match vtk's new interactor api. 
//    Call GetEventPosition to determine x and y.
//
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004 
//    AlignToAxis if shift key is being pressed (doAlign). 
//    
// ****************************************************************************

void
Lineout2D::OnMouseMove()
{
    int x, y;
    if (rubberBandMode)
    {
        Interactor->GetEventPosition(x, y);
        if (doAlign)
        {
            AlignToAxis(x, y);
        }
        ForceCoordsToViewport(x, y);
        UpdateRubberBand(anchorX, anchorY, lastX, lastY, x, y);
        lastX = x;
        lastY = y;
    }
    else
    {
        //
        // We are not in rubber band mode, so we don't want to override
        // the interactor.  Call the base function.
        //
        VisitInteractor::OnMouseMove();
    }
}

 
// ****************************************************************************
//  Method: Lineout2D::EndRubberBand
//
//  Purpose:
//      Tells the interactor to remove the rubber band and no longer update it
//      whenever the mouse moves.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
void
Lineout2D::EndRubberBand()
{
    rubberBandMode = false;
 
    //
    // Remove the rubber band actors from the renderer.
    // We should use the background as the renderer because our rubber band
    // is in display coordinates.
    //
    vtkRenderer *ren = proxy.GetBackground();
    ren->RemoveActor2D(rubberBandActor);
}


// ****************************************************************************
//  Method: Lineout2D::ForceCoordsToViewport
//
//  Purpose:
//      Takes coordinates and makes sure they are in the viewport.  If not,
//      it forces them inside the viewport.
//
//  Arguments:
//      x      X coordinate in display coordinates.
//      y      Y coordinate in display coordinates.
//
//  Note:    This routine assumes the canvasDevice[Min|Max][X|Y] has
//           already been set.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
void
Lineout2D::ForceCoordsToViewport(int &x, int &y)
{
    if (x < canvasDeviceMinX)
    {
        x = (int)ceil(canvasDeviceMinX);
    }
    if (x > canvasDeviceMaxX)
    {
        x = (int)floor(canvasDeviceMaxX);
    }
    if (y < canvasDeviceMinY)
    {
        y = (int)ceil(canvasDeviceMinY);
    }
    if (y > canvasDeviceMaxY)
    {
        y = (int)floor(canvasDeviceMaxY);
    }
}


// ****************************************************************************
//  Method: Lineout2D::UpdateRubberBand
//
//  Purpose:
//      The real rubber band function.  This takes six arguments specifying
//      the anchor of the rubber band, what the last opposite corner was,
//      and what the new opposite corner is.  It implements the rubber band
//      through an XOR found in any text book.  To prevent blinking effects,
//      if extends the old rubber band lines instead of wiping them out and
//      redrawing.
//
//  Arguments:
//      aX      The x-coordinate of the anchor in display coordinates.
//      aY      The y-coordinate of the anchor in display coordinates.
//      lX      The x-coordinate of the last corner in display coordinates.
//      lY      The y-coordinate of the last corner in display coordinates.
//      nX      The x-coordinate of the new corner in display coordinates.
//      nY      The y-coordinate of the new corner in display coordinates.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
void
Lineout2D::UpdateRubberBand(int aX, int aY, int lX, int lY,int nX,int nY)
{
    if (nX == lX && nY == lY)
    {
        //
        // No update necessary.
        //
        return;
    }
    //
    // Add the new line and erase the old one.
    //
    DrawRubberBandLine(aX, aY, nX, nY);
    DrawRubberBandLine(aX, aY, lX, lY);
}

 
// ****************************************************************************
//  Method: Lineout2D::DrawRubberBandLine
//
//  Purpose:
//      Draws a rubber band line.
//
//  Arguments:
//      x1      The x-coordinate of the first endpoint in display coordinates.
//      y1      The y-coordinate of the first endpoint in display coordinates.
//      x2      The x-coordinate of the second endpoint in display coordinates.
//      y2      The y-coordinate of the second endpoint in display coordinates.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
// ****************************************************************************
 
void
Lineout2D::DrawRubberBandLine(int x1, int y1, int x2, int y2)
{
    //
    // The rubber band is drawn to the background since it is also in display
    // coordinates.
    //
    vtkViewport *ren = proxy.GetBackground();
    vtkPoints *pts = rubberBand->GetPoints();
    pts->SetPoint(0, (double) x1, (double) y1, 0.);
    pts->SetPoint(1, (double) x2, (double) y2, 0.);
    rubberBandMapper->RenderOverlay(ren, rubberBandActor);
}


// ****************************************************************************
//  Method: Lineout2D::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed up.  For Lineout2D, this means
//      a rubber band zoom mode.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Removed arguments to match vtk's new interactor api. 
//
//    Kathleen Bonnell, Thu Feb  5 10:20:38 PST 2004 
//    Reset doAlign.
//
// ****************************************************************************

void
Lineout2D::EndLeftButtonAction()
{
    EndRubberBand();
    Lineout();
    EndTimer();
    doAlign = false;
}


// ****************************************************************************
//  Method: Lineout2D::AbortLeftButtonAction
//
//  Purpose:
//      Handles an abort action for the left button.  This means giving up on
//      the rubber band.
//
//  Note:   Modified from Zoom2D.
// 
//  Programmer: Kathleen Bonnell
//  Creation:   April 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Removed arguments to match vtk's new interactor api. 
//
// ****************************************************************************

void
Lineout2D::AbortLeftButtonAction()
{
    EndRubberBand();
    EndTimer();
}


// ****************************************************************************
//  Method: Lineout2D::Lineout
//
//  Purpose:
//      Creates a line between the display coordinates specified by
//      start and end X and Y.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 16, 2002 
//
// ****************************************************************************

void
Lineout2D::Lineout(void)
{
    if (anchorX == lastX && anchorY == lastY)
    {
        //
        // This is a point, not a line.
        //
        return;
    }
    proxy.Lineout(anchorX, anchorY, lastX, lastY);
}


// ****************************************************************************
//  Method: ZoomInteractor::OnTimer
//
//  Purpose:
//      Throw out timer events if we are in rubber Band mode. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2003
//
//  Modifications:
//    
// ****************************************************************************

void
Lineout2D::OnTimer()
{
    if (!rubberBandMode)
    {
        vtkInteractorStyle::OnTimer();
    }
}    


// ****************************************************************************
//  Method: ZoomInteractor::AlignToAxis
//
//  Purpose:
//     Modify either the x or y value to ensure the line between 
//     (anchorX, anchorY) and (x, y) is axis-aligned. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 5, 2004
//
//  Modifications:
//    
// ****************************************************************************

void
Lineout2D::AlignToAxis(int &x, int &y)
{
    float slope = (float)(y-anchorY) / (float)(x - anchorX);

    if (slope < -1. || slope > 1.)
    {
        x = anchorX; 
    }
    else 
    {
        y = anchorY; 
    }
}
