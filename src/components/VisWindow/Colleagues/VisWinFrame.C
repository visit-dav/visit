// ************************************************************************* //
//                                VisWinFrame.C                              //
// ************************************************************************* //

#include <VisWinFrame.h>

#include <vtkVisItAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>


// ****************************************************************************
//  Method: VisWinFrame constructor
//
//  Arguments:
//      p      A proxy that allows more access to the VisWindow for this
//             colleague.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  1 16:08:20 PDT 2000
//    Push assigning border's coordinates out to SetViewport.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    Set AdjustTicks mode for top & right borders. 
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//    Make the frame un-pickable.
//
//    Eric Brugger, Tue Nov  6 12:44:51 PST 2002
//    Changed some method calls to match changes in vtk.
//
// ****************************************************************************

VisWinFrame::VisWinFrame(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    bottomBorder = vtkVisItAxisActor2D::New();
    bottomBorder->GetPoint1Coordinate()
                    ->SetCoordinateSystemToNormalizedViewport();
    bottomBorder->GetPoint2Coordinate()
                    ->SetCoordinateSystemToNormalizedViewport();
    bottomBorder->SetTickVisibility(0);
    bottomBorder->SetLabelVisibility(0);
    bottomBorder->PickableOff();

    topBorder    = vtkVisItAxisActor2D::New();
    topBorder->GetPoint1Coordinate()
                    ->SetCoordinateSystemToNormalizedViewport();
    topBorder->GetPoint2Coordinate()
                    ->SetCoordinateSystemToNormalizedViewport();
    topBorder->SetTickVisibility(0);
    topBorder->SetLabelVisibility(0);
    topBorder->PickableOff();

    leftBorder   = vtkVisItAxisActor2D::New();
    leftBorder->GetPoint1Coordinate()
                    ->SetCoordinateSystemToNormalizedViewport();
    leftBorder->GetPoint2Coordinate()
                    ->SetCoordinateSystemToNormalizedViewport();
    leftBorder->SetTickVisibility(0);
    leftBorder->SetLabelVisibility(0);
    leftBorder->PickableOff();

    rightBorder  = vtkVisItAxisActor2D::New();
    rightBorder->GetPoint1Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport();
    rightBorder->GetPoint2Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport();
    rightBorder->SetTickVisibility(0);
    rightBorder->SetLabelVisibility(0);
    rightBorder->PickableOff();

    addedFrame = false;

    // only concerned about ticks along top & right borders, because
    // VisWinAxes takes care of bottom and left.
    topBorder->SetAdjustLabels(1);
    rightBorder->SetAdjustLabels(1);
}


// ****************************************************************************
//  Method: VisWinFrame destructor
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
// ****************************************************************************

VisWinFrame::~VisWinFrame()
{
    if (leftBorder != NULL)
    {
        leftBorder->Delete();
        leftBorder = NULL;
    }
    if (rightBorder != NULL)
    {
        rightBorder->Delete();
        rightBorder = NULL;
    }
    if (topBorder != NULL)
    {
        topBorder->Delete();
        topBorder = NULL;
    }
    if (bottomBorder != NULL)
    {
        bottomBorder->Delete();
        bottomBorder = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinFrame::SetForegroundColor
//
//  Purpose:
//      Sets the Foreground color for the renderers and axis actors.
//
//  Arguments:
//      fr      The red component (rgb) of the foreground.
//      fg      The green component (rgb) of the foreground.
//      fb      The blue component (rgb) of the foreground.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
// ****************************************************************************

void
VisWinFrame::SetForegroundColor(float fr, float fg, float fb)
{
    leftBorder->GetProperty()->SetColor(fr, fg, fb);
    rightBorder->GetProperty()->SetColor(fr, fg, fb);
    bottomBorder->GetProperty()->SetColor(fr, fg, fb);
    topBorder->GetProperty()->SetColor(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinFrame::SetViewport
//
//  Purpose:
//      Notifies the Colleague of the new viewport specifications.  Modifies
//      location of frame to match.
//
//  Arguments:
//      vl      The left viewport in normalized device coordinates.
//      vb      The bottom viewport in normalized device coordinates.
//      vr      The right viewport in normalized device coordinates.
//      vt      The top viewport in normalized device coordinates.
//
//  Programmer: Hank Childs
//  Creation:   July 25, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Jun 19 15:05:21 PDT 2001
//    Changed order of coordinates for topBorder, so that ticks
//    would line up with xAxis of VisWinAxes.
//
//    Kathleen Bonnell, Wed Apr 23 13:15:03 PDT 2003  
//    Explicitly tell the border actors that they have been modified. 
//
//    Kathleen Bonnell, Thu May 15 09:35:45 PDT 2003   
//    Removed *->Modified() calls.  Better handled at vtk level. 
//
// ****************************************************************************

void
VisWinFrame::SetViewport(float vl, float vb, float vr, float vt)
{
    bottomBorder->GetPoint1Coordinate()->SetValue(vl, vb);
    bottomBorder->GetPoint2Coordinate()->SetValue(vr, vb);

    leftBorder->GetPoint1Coordinate()->SetValue(vl, vb);
    leftBorder->GetPoint2Coordinate()->SetValue(vl, vt);

    topBorder->GetPoint1Coordinate()->SetValue(vr, vt);
    topBorder->GetPoint2Coordinate()->SetValue(vl, vt);

    rightBorder->GetPoint1Coordinate()->SetValue(vr, vb);
    rightBorder->GetPoint2Coordinate()->SetValue(vr, vt);
}


// ****************************************************************************
//  Method: VisWinFrame::Start2DMode
//
//  Purpose:
//      Puts this module in 2D mode.  This means adding the frame around to the
//      foreground.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
// ****************************************************************************

void
VisWinFrame::Start2DMode(void)
{
    if (ShouldAddFrame())
    {
        AddFrameToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinFrame::Stop2DMode
//
//  Purpose:
//      Takes the frame module out of 2D mode.  This means removing the
//      frame from the foreground.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
// ****************************************************************************

void
VisWinFrame::Stop2DMode(void)
{
    RemoveFrameFromWindow();
}


// ****************************************************************************
//  Method: VisWinFrame::HasPlots
//
//  Purpose:
//      Recognizes that the vis window now has plots.  Add the frame to the
//      background if it "should" (meaning it is also in 2D mode).
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
// ****************************************************************************

void
VisWinFrame::HasPlots(void)
{
    if (ShouldAddFrame())
    {
        AddFrameToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinFrame::NoPlots
//
//  Purpose:
//      Recognizes that the vis window now has no plots.  Remove the frame
//      from the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
// ****************************************************************************

void
VisWinFrame::NoPlots(void)
{
    RemoveFrameFromWindow();
}


// ****************************************************************************
//  Method: VisWinFrame::AddFrameToWindow
//
//  Purpose:
//      Adds the frame to vis window, provided it hasn't been added already.
//
//  Programmer: Hank Chlids
//  Creation:   July 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jul 25 13:37:22 PDT 2000
//    Put frame on foreground, rather than canvas.
//
// ****************************************************************************

void
VisWinFrame::AddFrameToWindow(void)
{
    if (addedFrame)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    foreground->AddActor2D(bottomBorder);
    foreground->AddActor2D(topBorder);
    foreground->AddActor2D(leftBorder);
    foreground->AddActor2D(rightBorder);

    addedFrame = true;
}


// ****************************************************************************
//  Method: VisWinFrame::RemoveFrameFromWindow
//
//  Purpose:
//      Removes the frame from the vis window, provided it has already been
//      added.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jul 25 13:37:22 PDT 2000
//    Put frame on foreground, rather than canvas.
//
// ****************************************************************************

void
VisWinFrame::RemoveFrameFromWindow(void)
{
    if (! addedFrame)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    foreground->RemoveActor2D(leftBorder);
    foreground->RemoveActor2D(rightBorder);
    foreground->RemoveActor2D(bottomBorder);
    foreground->RemoveActor2D(topBorder);
   
    addedFrame = false;
}


// ****************************************************************************
//  Method: VisWinFrame::ShouldAddFrame
//
//  Purpose:
//      Determines if the module should add the frame to the vis window.  This
//      should only be done if the vis window is in 2D mode and if it has 
//      plots.
//
//  Returns:     true if the frame should be added, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    July 7, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Allow the frame to be used for curve mode.
//
// ****************************************************************************

bool
VisWinFrame::ShouldAddFrame(void)
{
    return ((mediator.GetMode() == WINMODE_2D  ||
             mediator.GetMode() == WINMODE_CURVE)  &&
             mediator.HasPlots());
}


// ****************************************************************************
//  Method: VisWinFrame::SetVisibility
//
//  Purpose:
//      Sets the visibility of the frame. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2001 
//
// ****************************************************************************

void
VisWinFrame::SetVisibility(int vis)
{
    bottomBorder->SetVisibility(vis);
    rightBorder->SetVisibility(vis);
    topBorder->SetVisibility(vis);
    leftBorder->SetVisibility(vis);
}


// ****************************************************************************
//  Method: VisWinFrame::SetTopRightTickVisibility
//
//  Purpose:
//      Sets the visibility of the frame. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2001 
//
// ****************************************************************************

void
VisWinFrame::SetTopRightTickVisibility(int vis)
{
    if (vis)
    {
        topBorder->SetTickVisibility(1);
        rightBorder->SetTickVisibility(1);
    }
    else 
    {
        topBorder->SetTickVisibility(0);
        rightBorder->SetTickVisibility(0);
    }
}


// ****************************************************************************
//  Method: VisWinFrame::SetTickLocation
//
//  Purpose:
//      Sets the visibility of the frame. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2001 
//
// ****************************************************************************

void
VisWinFrame::SetTickLocation(int loc)
{
    rightBorder->SetTickLocation(loc);
    topBorder->SetTickLocation(loc);
}


// ****************************************************************************
//  Method: VisWinFrame::SetAutoSetTicks
//
//  Purpose:
//      Sets the flag which specifies if the ticks should be automatically
//      selected or user specified.
//
//  Arguments:
//      autoSetTicks The flag indicating if the ticks should be selected
//                   automatically.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2002
//
// ****************************************************************************
 
void
VisWinFrame::SetAutoSetTicks(int autoSetTicks)
{
    topBorder->SetAdjustLabels(autoSetTicks);
    rightBorder->SetAdjustLabels(autoSetTicks);
}


// ****************************************************************************
//  Method: VisWinFrame::SetMajorTickMinimum
//
//  Purpose:
//      Sets the minimum values for the major tick marks.
//
//  Arguments:
//      xMajorMinimum The minimum value for the x major tick marks.
//      yMajorMinimum The minimum value for the y major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2002
//
// ****************************************************************************
 
void
VisWinFrame::SetMajorTickMinimum(double xMajorMinimum, double yMajorMinimum)
{
    topBorder->SetMajorTickMinimum(xMajorMinimum);
    rightBorder->SetMajorTickMinimum(yMajorMinimum);
}


// ****************************************************************************
//  Method: VisWinFrame::SetMajorTickMaximum
//
//  Purpose:
//      Sets the maximum values for the major tick marks.
//
//  Arguments:
//      xMajorMaximum The maximum value for the x major tick marks.
//      yMajorMaximum The maximum value for the y major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2002
//
// ****************************************************************************
 
void
VisWinFrame::SetMajorTickMaximum(double xMajorMaximum, double yMajorMaximum)
{
    topBorder->SetMajorTickMaximum(xMajorMaximum);
    rightBorder->SetMajorTickMaximum(yMajorMaximum);
}


// ****************************************************************************
//  Method: VisWinFrame::SetMajorTickSpacing
//
//  Purpose:
//      Sets the spacing for the major tick marks.
//
//  Arguments:
//      xMajorSpacing The spacing for the x major tick marks.
//      yMajorSpacing The spacing for the y major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2002
//
// ****************************************************************************
 
void
VisWinFrame::SetMajorTickSpacing(double xMajorSpacing, double yMajorSpacing)
{
    topBorder->SetMajorTickSpacing(xMajorSpacing);
    rightBorder->SetMajorTickSpacing(yMajorSpacing);
}
 
 
// ****************************************************************************
//  Method: VisWinFrame::SetMinorTickSpacing
//
//  Purpose:
//      Sets the spacing for the minor tick marks.
//
//  Arguments:
//      xMinorSpacing The spacing for the x minor tick marks.
//      yMinorSpacing The spacing for the y minor tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2002
//
// ****************************************************************************

void
VisWinFrame::SetMinorTickSpacing(double xMinorSpacing, double yMinorSpacing)
{
    topBorder->SetMinorTickSpacing(xMinorSpacing);
    rightBorder->SetMinorTickSpacing(yMinorSpacing);
}


// ****************************************************************************
//  Method: VisWinFrame::SetLineWidth
//
//  Purpose:
//      Sets the line width for the frame.
//
//  Arguments:
//      width   The line width.
//
//  Programmer: Eric Brugger
//  Creation:   June 25, 2003
//
// ****************************************************************************
 
void
VisWinFrame::SetLineWidth(int width)
{
    leftBorder->GetProperty()->SetLineWidth(width);
    rightBorder->GetProperty()->SetLineWidth(width);
    topBorder->GetProperty()->SetLineWidth(width);
    bottomBorder->GetProperty()->SetLineWidth(width);
}


// ****************************************************************************
//  Method: VisWinFrame::UpdateView
//
//  Purpose:
//      Updates the frame so that it will reflect the current view.
//      Mainly to keep tick marks for top/right borders coordinated
//      with bottom/left as setin VisWinAxes.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jul 11 13:22:37 PDT 2000
//    Re-wrote function.
//
//    Eric Brugger, Mon Nov 24 15:55:23 PST 2003
//    I added code to set the axis orientation angle to handle degenerate
//    viewports.
//
//    Kathleen Bonnell, Thu Apr 29 16:54:44 PDT 2004
//    Initialize min_x, etc, to avoid UMR's. 
//
// ****************************************************************************

void
VisWinFrame::UpdateView(void)
{
    float  min_x = 0., max_x = 0., min_y = 0., max_y = 0.;
    GetRange(min_x, max_x, min_y, max_y);

    //
    // We put the topBorder in reverse so that its ticks would appear on the
    // correct side of the viewport.  Must propogate kludge by sending
    // range in backwards.
    //
    leftBorder->SetRange(max_y, min_y);
    leftBorder->SetUseOrientationAngle(1);
    leftBorder->SetOrientationAngle(-1.5707963);

    rightBorder->SetRange(min_y, max_y);
    rightBorder->SetUseOrientationAngle(1);
    rightBorder->SetOrientationAngle(1.5707963);

    bottomBorder->SetRange(min_x, max_x);
    bottomBorder->SetUseOrientationAngle(1);
    bottomBorder->SetOrientationAngle(0.);

    topBorder->SetRange(max_x, min_x);
    topBorder->SetUseOrientationAngle(1);
    topBorder->SetOrientationAngle(3.1415926);
}




// ****************************************************************************
//  Method: VisWinFrame::GetRange
//
//  Purpose:
//      Gets the range of the viewport.
//
//  Arguments:
//      min_x        Set to be the minimum x value.
//      max_x        Set to be the maximum x value.
//      min_y        Set to be the minimum y value.
//      max_y        Set to be the maximum y value.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Eric Brugger, Mon Nov 24 15:55:23 PST 2003
//    I rewrote the routine to get the range from the curve or 2d view from
//    the vis window.
//
// ****************************************************************************

void
VisWinFrame::GetRange(float &min_x, float &max_x, float &min_y, float &max_y)
{
    VisWindow *vw = mediator;

    switch (vw->GetWindowMode())
    {
      case WINMODE_2D:
        {
        const avtView2D view2D = vw->GetView2D();
        min_x = view2D.window[0];
        max_x = view2D.window[1];
        min_y = view2D.window[2];
        max_y = view2D.window[3];
        }
        break;
      case WINMODE_CURVE:
        {
        const avtViewCurve viewCurve = vw->GetViewCurve();
        min_x = viewCurve.domain[0];
        max_x = viewCurve.domain[1];
        min_y = viewCurve.range[0];
        max_y = viewCurve.range[1];
        }
        break;
    }
}


