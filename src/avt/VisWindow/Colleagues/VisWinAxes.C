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
//                               VisWinAxes.C                                //
// ************************************************************************* //

#include <VisWinAxes.h>

#include <string>
#include <vector>
#include <snprintf.h>

#include <vtkVisItAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <DebugStream.h>


//
// Static Function Prototypes
//

static int   LabelExponent(double, double);
static int   Digits(double, double);


using   std::string;
using   std::vector;


// ****************************************************************************
//  Method: VisWinAxes constructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:34:01 PDT 2000
//    Initialized addedAxes.
//
//    Hank Childs, Wed Jul 12 09:12:02 PDT 2000
//    Initialized new data members for normalizing values and changing format.
//    Told axes that they should adjust the ticks instead of this module
//    doing it.
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//    Make the axis un-pickable. 
//
//    Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//    Initialize xTitle, yTitle. 
//
//    Hank Childs, Fri Sep 27 16:16:56 PDT 2002
//    Initialize more data members for the title.
//
//    Eric Brugger, Wed Nov  5 11:41:19 PST 2002
//    Added more user control over the axes and tick marks.
//
//    Eric Brugger, Fri Jan 24 09:06:18 PST 2003
//    Changed the way the font sizes for the axes labels are set.
//
//    Kathleen Bonnell, Tue Dec 16 11:47:25 PST 2003 
//    Intialize autlabelscaling, userPowX,  userPowY. 
//
//    Brad Whitlock, Thu Jul 28 08:47:01 PDT 2005
//    I initialized some label flags.
//
//    Brad Whitlock, Thu Mar 27 11:47:46 PDT 2008
//    Call SetUseSeparateColors(1) on the axes so the colors will come from
//    the title and label text properties.
//
// ****************************************************************************

VisWinAxes::VisWinAxes(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    xAxis = vtkVisItAxisActor2D::New();
    xAxis->SetTickVisibility(1);
    xAxis->SetLabelVisibility(1);
    xAxis->SetTitleVisibility(1);
    xAxis->SetFontFamilyToCourier();
    xAxis->SetLabelFontHeight(0.02);
    xAxis->SetTitleFontHeight(0.02);
    xAxis->SetShadow(0);
    xAxis->SetAdjustLabels(1);
    xAxis->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
    xAxis->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    xAxis->PickableOff();
    xAxis->SetUseSeparateColors(1);

    yAxis = vtkVisItAxisActor2D::New();
    yAxis->SetTickVisibility(1);
    yAxis->SetLabelVisibility(1);
    yAxis->SetTitleVisibility(1);
    yAxis->SetFontFamilyToCourier();
    yAxis->SetLabelFontHeight(0.02);
    yAxis->SetTitleFontHeight(0.02);
    yAxis->SetShadow(0);
    yAxis->SetAdjustLabels(1);
    yAxis->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
    yAxis->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    yAxis->PickableOff();
    yAxis->SetUseSeparateColors(1);

    lastXPow = 0;
    lastYPow = 0;

    lastXAxisDigits = 3;
    lastYAxisDigits = 3;

    addedAxes = false;

    SNPRINTF(xTitle, 8,  "X-Axis");
    SNPRINTF(yTitle, 8, "Y-Axis");
    unitsX[0] = '\0';
    unitsY[0] = '\0';
    powX = 0;
    powY = 0;
    SetTitle();

    autolabelScaling = true; 
    userPowX = 0;
    userPowY = 0;
    userXTitle = false;
    userXUnits = false;
    userYTitle = false;
    userYUnits = false;
}


// ****************************************************************************
//  Method: VisWinAxes destructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

VisWinAxes::~VisWinAxes()
{
    if (xAxis != NULL)
    {
        xAxis->Delete();
        xAxis = NULL;
    }
    if (yAxis != NULL)
    {
        yAxis->Delete();
        yAxis = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinAxes::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the axes.
//
//  Arguments:
//      fr       The red component (rgb) of the foreground.
//      fg       The green component (rgb) of the foreground.
//      fb       The blue component (rgb) of the foreground.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Mar 27 11:09:01 PDT 2008
//    Update the title and label text attributes.
//
// ****************************************************************************

void
VisWinAxes::SetForegroundColor(double fr, double fg, double fb)
{
    xAxis->GetProperty()->SetColor(fr, fg, fb);
    yAxis->GetProperty()->SetColor(fr, fg, fb);
    UpdateTitleTextAttributes(fr, fg, fb);
    UpdateLabelTextAttributes(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinAxes::Start2DMode
//
//  Purpose:
//      Adds the axes to the window.  The axes are added to the background
//      renderer.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:34:01 PDT 2000
//    Pushed logic of adding axes to the window into its own routine.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002  
//    Set the titles here, since CurveMode uses different titles. 
//
//    Kathleen Bonnell, Wed Mar 31 16:20:39 PST 2004 
//    Don't set title here, allow them to be over-ridden by user-set titles. 
//
// ****************************************************************************

void
VisWinAxes::Start2DMode(void)
{
    SetTitle();
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxes::Stop2DMode
//
//  Purpose:
//      Removes the axes from the window.  The axes are removed from the
//      background renderer.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:34:01 PDT 2000
//    Pushed logic of removing axes from the window into its own routine.
//
// ****************************************************************************

void
VisWinAxes::Stop2DMode(void)
{
    RemoveAxesFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxes::StartCurveMode
//
//  Purpose:
//      Adds the axes to the window.  The axes are added to the background
//      renderer.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:20:39 PST 2004 
//    Don't set x/ytitle here, allow them to be over-ridden by user-set titles. 
//
// ****************************************************************************

void
VisWinAxes::StartCurveMode(void)
{
    SetTitle();
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxes::StopCurveMode
//
//  Purpose:
//      Removes the axes from the window.  The axes are removed from the
//      background renderer.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002
//
// ****************************************************************************

void
VisWinAxes::StopCurveMode(void)
{
    RemoveAxesFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxes::SetViewport
//
//  Purpose:
//      Changes the xAxis and yAxis to be fit with the new viewport.
//
//  Arguments:
//      vl      The left viewport in normalized device coordinates.
//      vb      The bottom viewport in normalized device coordinates.
//      vr      The right viewport in normalized device coordinates.
//      vt      The top viewport in normalized device coordinates.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

void
VisWinAxes::SetViewport(double vl, double vb, double vr, double vt)
{
    xAxis->GetPoint1Coordinate()->SetValue(vl, vb);
    xAxis->GetPoint2Coordinate()->SetValue(vr, vb);

    //
    // Make coordinates for y-axis backwards so the labels will appear on
    // the left side and out of the viewport.
    //
    yAxis->GetPoint1Coordinate()->SetValue(vl, vt);
    yAxis->GetPoint2Coordinate()->SetValue(vl, vb);
}


// ****************************************************************************
//  Method: VisWinAxes::AddAxesToWindow
//
//  Purpose:
//      Adds the axes to the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Jul  6 14:09:00 PDT 2001
//    Added axes to foreground instead of background.
//
// ****************************************************************************

void
VisWinAxes::AddAxesToWindow(void)
{
    if (addedAxes)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    foreground->AddActor2D(xAxis);
    foreground->AddActor2D(yAxis);

    addedAxes = true;
}


// ****************************************************************************
//  Method: VisWinAxes::RemoveAxesFromWindow
//
//  Purpose:
//      Removes the axes from the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Jul  6 14:09:00 PDT 2001
//    Removed axes from foreground instead of background.
//
// ****************************************************************************

void
VisWinAxes::RemoveAxesFromWindow(void)
{
    if (! addedAxes)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    foreground->RemoveActor2D(xAxis);
    foreground->RemoveActor2D(yAxis);

    addedAxes = false;
}


// ****************************************************************************
//  Method: VisWinAxes::ShouldAddAxes
//
//  Purpose:
//      Hides from routines that would like to add axes the logic about what
//      state the VisWindow must be in.  It should only be added if we are
//      in 2D mode and there are plots.
//
//  Returns:    true if the axes should be added to the vis window, false
//              otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Allowed for curve mdoe.
//
//    Mark Blair, Mon Sep 25 11:41:09 PDT 2006
//    No axes if axis annotations have already been disabled in the vis window.
//
//    Jeremy Meredith, Tue Apr 22 14:31:47 EDT 2008
//    Removed axis annotation disabling -- it was only added for a single
//    plot, and the functionality has been accomodated in a new window
//    modality supporting the correct style annotations.
//
// ****************************************************************************

bool
VisWinAxes::ShouldAddAxes(void)
{
    return ((mediator.GetMode() == WINMODE_2D || 
             mediator.GetMode() == WINMODE_CURVE) && 
             mediator.HasPlots());
}


// ****************************************************************************
//  Method: VisWinAxes::HasPlots
//
//  Purpose:
//      Receives the message from the vis window that it has plots.  This means
//      adding the axes to the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinAxes::HasPlots(void)
{
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxes::NoPlots
//
//  Purpose:
//      Receives the message from the vis window that it has no plots.  This
//      means that we should remove the axes from the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinAxes::NoPlots(void)
{
    RemoveAxesFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxes::UpdateView
//
//  Purpose:
//      Updates the axes so that they will reflect the current view.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jul 11 13:22:37 PDT 2000
//    Re-wrote function.
//
//    Kathleen Bonnell, Fri Jul  6 14:09:00 PDT 2001
//    Added functionality to set up gridlines. 
//
//    Kathleen Bonnell, Fri Tue Mar 12 11:31:32 PST 2002 
//    Ensure gridlines have correct length. 
//
//    Kathleen Bonnell,  Wed May  8 14:06:50 PDT 2002  
//    Scale y direction for winmode Curve.
//
//    Kathleen Bonnell,  Fri May 17 09:50:12 PDT 2002 
//    Correct gridline length. 
//
//    Eric Brugger, Fri Feb 28 11:18:28 PST 2003
//    Modified the routine to change the major tick mark labels to
//    scientific notation using vtkVisItAxisActor2D::SetMajorTickLabelScale
//    instead of modifying the range passed to vtkVisItAxisActor2D::SetRange.
//
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003  
//    Scaling of labels can take place for 2D views well as Curve views. 
//    
//    Eric Brugger, Mon Nov 24 15:55:23 PST 2003
//    I removed the code to adjust the range using the scale factor since
//    the values returned from GetRange already accounted for it.  I also
//    added code to set the axis orientation angle to handle degenerate
//    viewports.
//
//    Kathleen Bonnell, Thu Apr 29 16:54:44 PDT 2004
//    Initialize min_x, etc, to avoid UMR's. 
//
//    Kathleen Bonnell, Thu Mar 22 19:24:21 PDT 2007 
//    Send the log-scaling mode to the axis if Curve mode.
//
//    Kathleen Bonnell, Thu Mar 29 10:30:41 PDT 2007
//    Call AdjustLabelFormatForLogScale.
//
//    Kathleen Bonnell, Wed May  9 11:01:47 PDT 2007 
//    Account for 2D log scaling.
//
//    Kathleen Bonnell,Tue May 15 08:52:02 PDT 2007
//    Remove early termination if not 2D or Curve, was causing regression 
//    failures.
//
// ****************************************************************************

void
VisWinAxes::UpdateView(void)
{
    double  min_x = 0., max_x = 0., min_y = 0., max_y = 0.;
    GetRange(min_x, max_x, min_y, max_y);

    //
    // If the range or values are too big or too small, put them in scientific
    // notation.
    //
    AdjustValues(min_x, max_x, min_y, max_y);
    AdjustRange(min_x, max_x, min_y, max_y);

    //
    // We put the y-axis in reverse so that its labels would appear on the
    // correct side of the viewport.  Must propogate kludge by sending
    // range in backwards.
    //
    yAxis->SetRange(max_y, min_y);
    yAxis->SetUseOrientationAngle(1);
    yAxis->SetOrientationAngle(-1.5707963);
    if (powY != 0)
        yAxis->SetMajorTickLabelScale(1./pow(10., powY));
    else
        yAxis->SetMajorTickLabelScale(1.);
    xAxis->SetRange(min_x, max_x);
    xAxis->SetUseOrientationAngle(1);
    xAxis->SetOrientationAngle(0.);
    if (powX != 0)
        xAxis->SetMajorTickLabelScale(1./pow(10., powX));
    else
        xAxis->SetMajorTickLabelScale(1.);

    //
    // Set up Gridlines lengths. 
    // 
    vtkRenderer *fg = mediator.GetForeground();
    int *x = xAxis->GetPoint2Coordinate()->GetComputedViewportValue(fg);
    int *y = yAxis->GetPoint1Coordinate()->GetComputedViewportValue(fg);
    xAxis->SetGridlineXLength(0.);
    xAxis->SetGridlineYLength(abs(x[1] - y[1]));
    yAxis->SetGridlineXLength(abs(x[0] - y[0]));
    yAxis->SetGridlineYLength(0.);

    bool scaleMode[2] = {false, false};

    VisWindow *vw = mediator;
    if (vw->GetWindowMode() == WINMODE_CURVE)
    {
        const avtViewCurve viewCurve = vw->GetViewCurve();
        scaleMode[0] = viewCurve.domainScale == LOG;     
        scaleMode[1] = viewCurve.rangeScale == LOG;     
    }
    else if (vw->GetWindowMode() == WINMODE_2D)
    {
        const avtView2D view2D = vw->GetView2D();
        scaleMode[0] = view2D.xScale == LOG;     
        scaleMode[1] = view2D.yScale == LOG;     
    }
    xAxis->SetLogScale((int)scaleMode[0]);
    yAxis->SetLogScale((int)scaleMode[1]);
    if (scaleMode[0] || scaleMode[1])
    {
        AdjustLabelFormatForLogScale(min_x, max_x, min_y, max_y, scaleMode);
    }
}


// ****************************************************************************
//  Method: VisWinAxes::UpdatePlotList
//
//  Purpose:
//      Decides what the units are for the X and Y directions.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Oct 15 21:44:19 PDT 2003
//    Add labels.
//
//    Kathleen Bonnell, Tue Mar 23 08:57:31 PST 2004 
//    Allow labels from DataAtts to be set for Curve window, too.
//
//    Brad Whitlock, Thu Jul 28 08:48:35 PDT 2005
//    Added code that allows the user to override the titles and units.
//
// ****************************************************************************

void
VisWinAxes::UpdatePlotList(vector<avtActor_p> &list)
{
    int nActors = list.size();
    string x, y;
    string lx, ly;
    for (int i = 0 ; i < nActors ; i++)
    {
        avtDataAttributes &atts = 
                             list[i]->GetBehavior()->GetInfo().GetAttributes();

        // Last one in is the winner.
        if (atts.GetXUnits() != "")
        {
            x = atts.GetXUnits();
        }
        if (atts.GetYUnits() != "")
        {
            y = atts.GetYUnits();
        }
        if ((atts.GetXLabel() != "") && (atts.GetXLabel() != "X-Axis"))
        {
            lx = atts.GetXLabel();
        }
        if ((atts.GetYLabel() != "") && (atts.GetYLabel() != "Y-Axis"))
        {
            ly = atts.GetYLabel();
        }
    }

    if(!userXUnits)
        SNPRINTF(unitsX, 256, x.c_str());
    if(!userYUnits)
        SNPRINTF(unitsY, 256, y.c_str());
    if(!userXTitle)
    {
        if (lx != "")
            SNPRINTF(xTitle, 256, lx.c_str());
        else
            strcpy(xTitle, "X-Axis");
    }
    if(!userYTitle)
    {
        if (ly != "")
            SNPRINTF(yTitle, 256, ly.c_str());
        else
            strcpy(yTitle, "Y-Axis");
    }

    SetTitle();
}


// ****************************************************************************
//  Method: VisWinAxes::GetRange
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
//    Hank Childs, Fri Jun  9 14:43:27 PDT 2006
//    Add missing cases for the switch statement.
//
//    Kathleen Bonnell, Tue Nov 20 15:12:57 PST 2007 
//    Removed debug message for default case.
// 
// ****************************************************************************

void
VisWinAxes::GetRange(double &min_x, double &max_x, double &min_y, double &max_y)
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
      default:
        break;
    }
}


// ****************************************************************************
//  Method: VisWinAxes::AdjustValues
//
//  Purpose:
//      If the range or values is too big or too small, put them in scientific
//      notation and changes the labels.
//
//  Arguments:
//      min_x    The minimum x value.  This may be put into scientific form.
//      max_x    The maximum x value.  This may be put into scientific form.
//      min_y    The minimum y value.  This may be put into scientific form.
//      max_y    The maximum y value.  This may be put into scientific form.
//
//  Note:       This code is partially stolen from old MeshTV code,
//              /meshtvx/toolkit/plotgrid.c, axlab[x|y].
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Use members to set titles, as they change depending on winmode.
//
//    Hank Childs, Fri Sep 27 17:46:30 PDT 2002
//    Do the actual setting of the title in a separate routine.
//
//    Eric Brugger, Fri Feb 28 11:18:28 PST 2003
//    Modified the routine to leave the input values unchanged.
//
//    Kathleen Bonnell, Tue Dec 16 11:47:25 PST 2003 
//    Utilize user-set exponents if autolabelscaling is off. 
//
// ****************************************************************************

void
VisWinAxes::AdjustValues(double min_x, double max_x, double min_y, double max_y)
{
    int curPowX;
    int curPowY;
    if (autolabelScaling) 
    {
        curPowX = LabelExponent(min_x, max_x);
        curPowY = LabelExponent(min_y, max_y);
    }
    else 
    {
        curPowX = userPowX;
        curPowY = userPowY;
    }
 
    if (curPowX != powX)
    {
        powX = curPowX;
        SetTitle();
    }

    if (curPowY != powY)
    {
        powY = curPowY;
        SetTitle();
    }
}


// ****************************************************************************
//  Method: VisWinAxes::AdjustRange
//
//  Purpose:
//      If the range is small, adjust the precision of the values displayed.
//
//  Arguments:
//      min_x   The minimum x value.
//      max_x   The maximum x value.
//      min_y   The minimum y value.
//      max_y   The maximum y value.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Eric Brugger, Fri Feb 28 11:18:28 PST 2003
//    Modified the routine to correct the range values using powX and powY
//    since they are not being corrected before calling this routine.
//
// ****************************************************************************

void
VisWinAxes::AdjustRange(double min_x, double max_x, double min_y, double max_y)
{
    if (powX != 0)
    {
        min_x /= pow(10., powX);
        max_x /= pow(10., powX);
    }
    int xAxisDigits = Digits(min_x, max_x);
    if (xAxisDigits != lastXAxisDigits)
    {
        char  format[16];
        SNPRINTF(format, 16, "%%.%df", xAxisDigits);
        xAxis->SetLabelFormat(format);
        lastXAxisDigits = xAxisDigits;
    }

    if (powY != 0)
    {
        min_y /= pow(10., powY);
        max_y /= pow(10., powY);
    }
    int yAxisDigits = Digits(min_y, max_y);
    if (yAxisDigits != lastYAxisDigits)
    {
        char  format[16];
        SNPRINTF(format, 16, "%%.%df", yAxisDigits);
        yAxis->SetLabelFormat(format);
        lastYAxisDigits = yAxisDigits;
    }
}


// ****************************************************************************
//  Function: Digits
//
//  Purpose:
//      Determines the appropriate number of digits for a given range.
//
//  Arguments:
//      min    The minimum value in the range.
//      max    The maximum value in the range.
//
//  Returns:   The appropriate number of digits.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//    Casted to get rid of warning.
//
//    Akira Haddox, Wed Jul 16 16:48:20 PDT 2003
//    Added special case for when range is zero.
//
//    Mark C. Miller, Wed Jul 27 08:39:38 PDT 2005
//    Moved test for non-positive range to *before* call to log10
//
// ****************************************************************************

int
Digits(double min, double max)
{
    double  range = max - min;
    double  pow10 = -5.0;
    if (range > 0)
        pow10 = log10(range);

    int    ipow10  = (int)floor(pow10);

    int    digitsPastDecimal = -ipow10;

    if (digitsPastDecimal < 0)
    {
        //
        // The range is more than 10, we don't need to worry about decimals.
        //
        digitsPastDecimal = 0;
    }
    else
    {
        //
        // We want one more than the range since there is more than one
        // tick per decade.
        //
        digitsPastDecimal++;

        //
        // Anything more than 5 is just noise.  (and probably 5 is noise with
        // floating point is the part before the decimal is big).
        //
        if (digitsPastDecimal > 5)
        {
            digitsPastDecimal = 5;
        }
    }
 
    return digitsPastDecimal;
}


// ****************************************************************************
//  Function: LabelExponent
//
//  Purpose:
//      Determines the proper exponent for the min and max values.
//
//  Arguments:
//      min     The minimum value along a certain axis.
//      max     The maximum value along a certain axis.
//
//  Note:       This code is mostly stolen from old MeshTV code,
//              /meshtvx/toolkit/plotgrid.c, axlab_format.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//    Casted to get rid of compiler warning.
//
//    Jeremy Meredith, Fri Aug 14 09:42:43 EDT 2009
//    Fix case where range==0.
//
// ****************************************************************************

int
LabelExponent(double min, double max)
{
    //
    // Determine power of 10 to scale axis labels to.
    //
    double range = (fabs(min) > fabs(max) ? fabs(min) : fabs(max));

    double pow10 = -5.0;
    if (range > 0)
        pow10 = log10(range);

    //
    // Cutoffs for using scientific notation.
    //
    static double  eformat_cut_min = -1.5;
    static double  eformat_cut_max =  3.0;
    static double  cut_min = pow(10., eformat_cut_min);
    static double  cut_max = pow(10., eformat_cut_max);
    double ipow10;
    if (range < cut_min || range > cut_max)
    {
        //
        // We are going to use scientific notation and round the exponents to
        // the nearest multiple of three.
        //
        ipow10 = (floor(floor(pow10)/3.))*3;
    }
    else
    {
        ipow10 = 0.;
    }

    return (int)ipow10;
}    


// ****************************************************************************
//  Method: VisWinAxes::SetXTickVisibility
//
//  Purpose:
//    Sets the visibility of x-axis ticks. 
//
//  Arguments:
//    xVis       The visibility of the x-axis ticks. 
//    xLabelsVis The visibility of the x-axis labels. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001. 
//
// ****************************************************************************

void
VisWinAxes::SetXTickVisibility(int xVis, int xLabelsVis)
{
    xAxis->SetMinorTicksVisible(xVis);

    // Major ticks dependent upon visibility of labels.
    xAxis->SetTickVisibility(xVis || xLabelsVis);
} 


// ****************************************************************************
//  Method: VisWinAxes::SetYTickVisibility
//
//  Purpose:
//      Sets the visibility of y-axis ticks. 
//
//  Arguments:
//    yVis       The visibility of the y-axis ticks. 
//    yLabelsVis The visibility of the y-axis labels. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001. 
//
// ****************************************************************************

void
VisWinAxes::SetYTickVisibility(int yVis, int yLabelsVis)
{
    yAxis->SetMinorTicksVisible(yVis);
    // Major ticks dependent upon visibility of labels.
    yAxis->SetTickVisibility(yVis || yLabelsVis);
} 


// ****************************************************************************
//  Method: VisWinAxes::SetLabelsVisibility
//
//  Purpose:
//      Sets the visibility of x-axis and y-axis labels. 
//
//  Arguments:
//      x-vis     The visibility of the x-axis labels. 
//      y-vis     The visibility of the y-axis labels. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001. 
//
//  Modifications:
//    Eric Brugger, Wed Nov  5 11:41:19 PST 2002
//    Removed the control of the title visibility out of this method into
//    its own method.
//
// ****************************************************************************

void
VisWinAxes::SetLabelsVisibility(int xVis, int yVis)
{
    xAxis->SetLabelVisibility(xVis);
    yAxis->SetLabelVisibility(yVis);
} 


// ****************************************************************************
//  Method: VisWinAxes::SetTitleVisibility
//
//  Purpose:
//      Sets the visibility of x-axis and y-axis titles. 
//
//  Arguments:
//      x-vis     The visibility of the x-axis titles. 
//      y-vis     The visibility of the y-axis titles. 
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2002
//
// ****************************************************************************

void
VisWinAxes::SetTitleVisibility(int xVis, int yVis)
{
    xAxis->SetTitleVisibility(xVis);
    yAxis->SetTitleVisibility(yVis);
} 


// ****************************************************************************
//  Method: VisWinAxes::SetVisibility
//
//  Purpose:
//      Sets the visibility of this colleague. 
//
//  Arguments:
//      vis     The visibility of this colleague.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001. 
//
// ****************************************************************************

void
VisWinAxes::SetVisibility(int vis)
{
    xAxis->SetVisibility(vis);
    yAxis->SetVisibility(vis);
} 
       
    
// ****************************************************************************
//  Method: VisWinAxes::SetTickLocation
//
//  Purpose:
//      Sets the location of the ticks. 
//
//  Arguments:
//      loc     The location of the ticks.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 27, 2001. 
//
// ****************************************************************************

void
VisWinAxes::SetTickLocation(int loc)
{
    xAxis->SetTickLocation(loc);
    yAxis->SetTickLocation(loc);
} 


// ****************************************************************************
//  Method: VisWinAxes::SetXGridVisibility
//
//  Purpose:
//      Sets the visibility of x-axis gridlines. 
//
//  Arguments:
//      vis     The visibility of the gridlines. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 6, 2001. 
//
// ****************************************************************************

void
VisWinAxes::SetXGridVisibility(int vis)
{
    xAxis->SetDrawGridlines(vis);
} 
       
       
// ****************************************************************************
//  Method: VisWinAxes::SetYGridVisibility
//
//  Purpose:
//      Sets the visibility of y-axis gridlines. 
//
//  Arguments:
//      vis     The visibility of the gridlines. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 6, 2001. 
//
// ****************************************************************************

void
VisWinAxes::SetYGridVisibility(int vis)
{
    yAxis->SetDrawGridlines(vis);
} 
       

// ****************************************************************************
//  Method: VisWinAxes::SetAutoSetTicks
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
//  Creation:   November 5, 2002
//
// ****************************************************************************

void
VisWinAxes::SetAutoSetTicks(int autoSetTicks)
{
    xAxis->SetAdjustLabels(autoSetTicks);
    yAxis->SetAdjustLabels(autoSetTicks);
}


// ****************************************************************************
//  Method: VisWinAxes::SetMajorTickMinimum
//
//  Purpose:
//      Sets the minimum values for the major tick marks.
//
//  Arguments:
//      xMajorMinimum The minimum value for the x major tick marks.
//      yMajorMinimum The minimum value for the y major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 5, 2002
//
// ****************************************************************************

void
VisWinAxes::SetMajorTickMinimum(double xMajorMinimum, double yMajorMinimum)
{
    xAxis->SetMajorTickMinimum(xMajorMinimum);
    yAxis->SetMajorTickMinimum(yMajorMinimum);
}


// ****************************************************************************
//  Method: VisWinAxes::SetMajorTickMaximum
//
//  Purpose:
//      Sets the maximum values for the major tick marks.
//
//  Arguments:
//      xMajorMaximum The maximum value for the x major tick marks.
//      yMajorMaximum The maximum value for the y major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 5, 2002
//
// ****************************************************************************

void
VisWinAxes::SetMajorTickMaximum(double xMajorMaximum, double yMajorMaximum)
{
    xAxis->SetMajorTickMaximum(xMajorMaximum);
    yAxis->SetMajorTickMaximum(yMajorMaximum);
}


// ****************************************************************************
//  Method: VisWinAxes::SetMajorTickSpacing
//
//  Purpose:
//      Sets the spacing for the major tick marks.
//
//  Arguments:
//      xMajorSpacing The spacing for the x major tick marks.
//      yMajorSpacing The spacing for the y major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 5, 2002
//
// ****************************************************************************

void
VisWinAxes::SetMajorTickSpacing(double xMajorSpacing, double yMajorSpacing)
{
    xAxis->SetMajorTickSpacing(xMajorSpacing);
    yAxis->SetMajorTickSpacing(yMajorSpacing);
}


// ****************************************************************************
//  Method: VisWinAxes::SetMinorTickSpacing
//
//  Purpose:
//      Sets the spacing for the minor tick marks.
//
//  Arguments:
//      xMinorSpacing The spacing for the x minor tick marks.
//      yMinorSpacing The spacing for the y minor tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   November 5, 2002
//
// ****************************************************************************

void
VisWinAxes::SetMinorTickSpacing(double xMinorSpacing, double yMinorSpacing)
{
    xAxis->SetMinorTickSpacing(xMinorSpacing);
    yAxis->SetMinorTickSpacing(yMinorSpacing);
}


// ****************************************************************************
//  Method: VisWinAxes::SetXLabelFontHeight
//
//  Purpose:
//      Sets the label font height for the x axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Eric Brugger
//  Creation:   January 24, 2003
//
// ****************************************************************************

void
VisWinAxes::SetXLabelFontHeight(double height)
{
    xAxis->SetLabelFontHeight(height);
}


// ****************************************************************************
//  Method: VisWinAxes::SetYLabelFontHeight
//
//  Purpose:
//      Sets the label font height for the y axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Eric Brugger
//  Creation:   January 24, 2003
//
// ****************************************************************************

void
VisWinAxes::SetYLabelFontHeight(double height)
{
    yAxis->SetLabelFontHeight(height);
}


// ****************************************************************************
//  Method: VisWinAxes::SetXTitleFontHeight
//
//  Purpose:
//      Sets the title font height for the x axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Eric Brugger
//  Creation:   January 24, 2003
//
// ****************************************************************************

void
VisWinAxes::SetXTitleFontHeight(double height)
{
    xAxis->SetTitleFontHeight(height);
}


// ****************************************************************************
//  Method: VisWinAxes::SetYTitleFontHeight
//
//  Purpose:
//      Sets the title font height for the y axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Eric Brugger
//  Creation:   January 24, 2003
//
// ****************************************************************************

void
VisWinAxes::SetYTitleFontHeight(double height)
{
    yAxis->SetTitleFontHeight(height);
}


// ****************************************************************************
//  Method: VisWinAxes::SetLineWidth
//
//  Purpose:
//      Sets the line width for the axes.
//
//  Arguments:
//      width   The line width.
//
//  Programmer: Eric Brugger
//  Creation:   June 25, 2003
//
// ****************************************************************************

void
VisWinAxes::SetLineWidth(int width)
{
    xAxis->GetProperty()->SetLineWidth(width);
    yAxis->GetProperty()->SetLineWidth(width);
}


// ****************************************************************************
//  Method: VisWinAxes::SetTitle
//
//  Purpose:
//      Sets the title, including exponent and units.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2002
//
//  Modifications:
//    Eric Brugger, Wed Nov  5 11:41:19 PST 2002
//    Added the font scale factor.
//
//    Eric Brugger, Fri Jan 24 09:06:18 PST 2003
//    Removed the setting of the font size since it no longer depends on
//    the title.
//
//    Eric Brugger, Fri Feb 28 11:18:28 PST 2003
//    Changed the printing of the y axis label to look nicer when displaying
//    scientific notation and units.
//
//    Kathleen Bonnell, Tue Dec 16 11:47:25 PST 2003 
//    Use '10e' insted of just 'e' when diplaying scientific notation. 
//
//    Kathleen Bonnell, Tue Jul 20 11:44:36 PDT 2004 
//    Use 'x10^' insted of '10e' when diplaying scientific notation. 
//
// ****************************************************************************

void
VisWinAxes::SetTitle(void)
{
    char buffer[1024];
    if (powX == 0)
    {
        if (unitsX[0] == '\0')
            SNPRINTF(buffer, 1024, "%s", xTitle);
        else
            SNPRINTF(buffer, 1024, "%s (%s)", xTitle, unitsX);
    }
    else
    {
        if (unitsX[0] == '\0')
            SNPRINTF(buffer, 1024, "%s (x10^%d)", xTitle, powX);
        else
            SNPRINTF(buffer, 1024, "%s (x10^%d %s)", xTitle, powX, unitsX);
    }
    xAxis->SetTitle(buffer);

    if (powY == 0)
    {
        if (unitsY[0] == '\0')
            SNPRINTF(buffer, 1024, "%s", yTitle);
        else
            SNPRINTF(buffer, 1024, "%s\n (%s)", yTitle, unitsY);
    }
    else
    {
        if (unitsY[0] == '\0')
            SNPRINTF(buffer, 1024, "%s\n (x10^%d)", yTitle, powY);
        else
            SNPRINTF(buffer, 1024, " %s\n(x10^%d %s)", yTitle, powY, unitsY);
    }
    yAxis->SetTitle(buffer);
}


// ****************************************************************************
//  Method: VisWinAxes::SetLabelScaling
//
//  Purpose:
//      Sets autolabelscaling and the exponents to be used with each axis. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2003
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxes::SetLabelScaling(bool autoscale, int upowX, int upowY)
{
    autolabelScaling = autoscale;
    userPowX = upowX;
    userPowY = upowY;
} 

// ****************************************************************************
// Method: VisWinAxes::SetTitle
//
// Purpose: 
//   Sets the title for an axis, overriding any title that comes from the plot.
//
// Arguments:
//   title   : The new title.
//   userSet : Whether the title is user-set.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 10:58:41 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes::SetXTitle(const string &title, bool userSet)
{
    if(userSet)
        SNPRINTF(xTitle, 256, "%s", title.c_str());
    userXTitle = userSet;
}

void
VisWinAxes::SetYTitle(const string &title, bool userSet)
{
    if(userSet)
        SNPRINTF(yTitle, 256, "%s", title.c_str());
    userYTitle = userSet;
}

// ****************************************************************************
// Method: VisWinAxes::SetUnits
//
// Purpose: 
//   Sets the units for an axis, overriding any units that comes from the plot.
//
// Arguments:
//   units   : The new units.
//   userSet : Whether the title is user-set.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 10:58:41 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes::SetXUnits(const string &units, bool userSet)
{
    if(userSet)
        SNPRINTF(unitsX, 256, "%s", units.c_str());
    userXUnits = userSet;
}

void
VisWinAxes::SetYUnits(const string &units, bool userSet)
{
    if(userSet)
        SNPRINTF(unitsY, 256, "%s", units.c_str());
    userYUnits = userSet;
}

// ****************************************************************************
// Method: VisWinAxes::AdjustLabelFormatForLogScale
//
// Purpose: 
//   Performs some of the same adjustments as AdjustLabels and AdjustRange,
//   without the side-effect of setting ivars.  Does, however change
//   the axis label formats.
//
// Arguments:
//   min_x     The minimum x value. (already log-scaled)
//   max_x     The maximum x value. (already log-scaled)
//   min_y     The minimum y value. (already log-scaled)
//   max_y     The maximum y value. (already log-scaled)
//
// Programmer: Kathleen Bonnell 
// Creation:   March 29, 2007 
//
// Modifications:
//   Kathleen Bonnell, Wed Apr  4 17:04:54 PDT 2007
//   Modified to handle situations where scientific notation is required.
// 
//   Kathleen Bonnell, Thu Apr  5 15:03:40 PDT 2007 
//   Fix setting of y-axis info, also changed to use new LogLabelFormat so 
//   that the switching between Log-Linear the labels have the right format. 
//
//    Kathleen Bonnell, Wed May  9 11:01:47 PDT 2007 
//    Account for 2D log scaling, added bool args.
//
// ****************************************************************************

void
VisWinAxes::AdjustLabelFormatForLogScale(
    double min_x, double max_x, double min_y, double max_y, bool sm[2])
{
    VisWindow *vw = mediator;
    if (vw->GetWindowMode() != WINMODE_CURVE &&
        vw->GetWindowMode() != WINMODE_2D)
        return;

    //
    // The labels will be non-log-scaled, so must convert min & max here
    // to get the correct range that will be used.
    //
    if (sm[0])
    {    
        double minx = pow(10., min_x);
        double maxx = pow(10., max_x);
        int xAxisDigits = Digits(minx, maxx);
        char  format[16];

        int ipow_minx = (int) ((floor(floor(min_x)/3.))*3);
        int ipow_maxx = (int) ((floor(floor(max_x)/3.))*3);

        int ipow_min = ipow_minx < ipow_maxx ? ipow_minx : ipow_maxx;
        int ipow_max = ipow_minx > ipow_maxx ? ipow_minx : ipow_maxx;

        if (ipow_min < -4 || ipow_max > 4)
            SNPRINTF(format, 16, "%%.%de", xAxisDigits);
        else 
            SNPRINTF(format, 16, "%%.%df", xAxisDigits);
        xAxis->SetLogLabelFormat(format);
    }
    if (sm[1])
    {    
        double miny = pow(10., min_y);
        double maxy = pow(10., max_y);
        int yAxisDigits = Digits(miny, maxy);
        char  format[16];

        int ipow_miny = (int) ((floor(floor(min_y)/3.))*3);
        int ipow_maxy = (int) ((floor(floor(max_y)/3.))*3);
        int ipow_min = ipow_miny < ipow_maxy ? ipow_miny : ipow_maxy;
        int ipow_max = ipow_miny > ipow_maxy ? ipow_miny : ipow_maxy;

        if (ipow_min < -4 || ipow_max > 4)
            SNPRINTF(format, 16, "%%.%de", yAxisDigits);
        else 
            SNPRINTF(format, 16, "%%.%df", yAxisDigits);
        yAxis->SetLogLabelFormat(format);
    }
}

// ****************************************************************************
// Method: VisWinAxes::SetTitleTextAttributes
//
// Purpose: 
//   Sets the title properties for all axes.
//
// Arguments:
//   x : The text properties for the X axis title.
//   y : The text properties for the Y axis title.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 26 14:17:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes::SetTitleTextAttributes(
    const VisWinTextAttributes &x, 
    const VisWinTextAttributes &y)
{
    titleTextAttributes[0] = x;
    titleTextAttributes[1] = y;

    double rgb[3];
    mediator.GetForegroundColor(rgb);
    UpdateTitleTextAttributes(rgb[0], rgb[1], rgb[2]);
}

// ****************************************************************************
// Method: VisWinAxes::SetLabelTextAttributes
//
// Purpose: 
//   Sets the label properties for all axes.
//
// Arguments:
//   xAxis : The text properties for the X axis labels.
//   yAxis : The text properties for the Y axis labels.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 26 14:17:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes::SetLabelTextAttributes(
    const VisWinTextAttributes &x, 
    const VisWinTextAttributes &y)
{
    labelTextAttributes[0] = x;
    labelTextAttributes[1] = y;

    double rgb[3];
    mediator.GetForegroundColor(rgb);
    UpdateLabelTextAttributes(rgb[0], rgb[1], rgb[2]);
}

// ****************************************************************************
// Method: VisWinAxes::UpdateTitleTextAttributes
//
// Purpose: 
//   Sets the title text properties into the axes.
//
// Arguments:
//   fr, fg, fb : Red, green, blue color components [0.,1].
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 26 14:18:45 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes::UpdateTitleTextAttributes(double fr, double fg, double fb)
{
    vtkVisItAxisActor2D *axes[2];
    axes[0] = xAxis;
    axes[1] = yAxis;

    for(int i = 0; i < 2; ++i)
    {
        // Set the colors
        if(titleTextAttributes[i].useForegroundColor)
            axes[i]->GetTitleTextProperty()->SetColor(fr, fg, fb);
        else
        {
            axes[i]->GetTitleTextProperty()->SetColor(
                titleTextAttributes[i].color[0],
                titleTextAttributes[i].color[1],
                titleTextAttributes[i].color[2]);
        }

        axes[i]->GetTitleTextProperty()->SetFontFamily((int)titleTextAttributes[i].font);
        axes[i]->GetTitleTextProperty()->SetBold(titleTextAttributes[i].bold?1:0);
        axes[i]->GetTitleTextProperty()->SetItalic(titleTextAttributes[i].italic?1:0);

        // Pass the opacity in the line offset.
        axes[i]->GetTitleTextProperty()->SetLineOffset(titleTextAttributes[i].color[3]);
    }
}

// ****************************************************************************
// Method: VisWinAxes::UpdateLabelTextAttributes
//
// Purpose: 
//   Sets the label text properties into the axes.
//
// Arguments:
//   fr, fg, fb : Red, green, blue color components [0.,1].
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 26 14:18:45 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes::UpdateLabelTextAttributes(double fr, double fg, double fb)
{
    vtkVisItAxisActor2D *axes[2];
    axes[0] = xAxis;
    axes[1] = yAxis;

    for(int i = 0; i < 2; ++i)
    {
        // Set the colors
        if(labelTextAttributes[i].useForegroundColor)
            axes[i]->GetLabelTextProperty()->SetColor(fr, fg, fb);
        else
        {
            axes[i]->GetLabelTextProperty()->SetColor(
                labelTextAttributes[i].color[0],
                labelTextAttributes[i].color[1],
                labelTextAttributes[i].color[2]);
        }

        axes[i]->GetLabelTextProperty()->SetFontFamily((int)labelTextAttributes[i].font);
        axes[i]->GetLabelTextProperty()->SetBold(labelTextAttributes[i].bold?1:0);
        axes[i]->GetLabelTextProperty()->SetItalic(labelTextAttributes[i].italic?1:0);

        // Pass the opacity in the line offset.
        axes[i]->GetLabelTextProperty()->SetLineOffset(labelTextAttributes[i].color[3]);
    }
}
