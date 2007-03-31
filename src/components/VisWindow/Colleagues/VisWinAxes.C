// ************************************************************************* //
//                               VisWinAxes.C                                //
// ************************************************************************* //

#include <VisWinAxes.h>

#include <string>
#include <vector>

#include <vtkHankAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>


//
// Static Function Prototypes
//

static int   LabelExponent(float, float);
static int   Digits(float, float);


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
// ****************************************************************************

VisWinAxes::VisWinAxes(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    xAxis = vtkHankAxisActor2D::New();
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

    yAxis = vtkHankAxisActor2D::New();
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

    lastXPow = 0;
    lastYPow = 0;

    lastXAxisDigits = 3;
    lastYAxisDigits = 3;

    addedAxes = false;

    sprintf(xTitle, "X-Axis");
    sprintf(yTitle, "Y-Axis");
    unitsX[0] = '\0';
    unitsY[0] = '\0';
    powX = 0;
    powY = 0;
    SetTitle();
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
// ****************************************************************************

void
VisWinAxes::SetForegroundColor(float fr, float fg, float fb)
{
    xAxis->GetProperty()->SetColor(fr, fg, fb);
    yAxis->GetProperty()->SetColor(fr, fg, fb);
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
// ****************************************************************************

void
VisWinAxes::Start2DMode(void)
{
    sprintf(xTitle, "X-Axis");
    sprintf(yTitle, "Y-Axis");
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
// ****************************************************************************

void
VisWinAxes::StartCurveMode(void)
{
    sprintf(xTitle, "Distance");
    sprintf(yTitle, "Value");
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
VisWinAxes::SetViewport(float vl, float vb, float vr, float vt)
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
//    scientific notation using vtkHankAxisActor2D::SetMajorTickLabelScale
//    instead of modifying the range passed to vtkHankAxisActor2D::SetRange.
//
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003  
//    Scaling of labels can take place for 2D views well as Curve views. 
//    
// ****************************************************************************

void
VisWinAxes::UpdateView(void)
{
    float  min_x, max_x, min_y, max_y;
    GetRange(min_x, max_x, min_y, max_y);

    double scale;
    int type;
    mediator.GetScaleFactorAndType(scale, type);

    //
    // Ensure that the labels reflect any axis-scaling that may have
    // taken place. If scale <= 0, then no scaling took place. 
    //
    if (scale > 0.)
    {
        if (type == 0)      // x_axis
        {
            max_x /= scale;
            min_x /= scale;
        }
        else                // y_axis
        {
            max_y /= scale;
            min_y /= scale;
        }
    }

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
    if (powY != 0)
        yAxis->SetMajorTickLabelScale(1./pow(10., powY));
    else
        yAxis->SetMajorTickLabelScale(1.);
    xAxis->SetRange(min_x, max_x);
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
// ****************************************************************************

void
VisWinAxes::UpdatePlotList(vector<avtActor_p> &list)
{
    int nActors = list.size();
    string x, y;
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
    }

    sprintf(unitsX, x.c_str());
    sprintf(unitsY, y.c_str());
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
// ****************************************************************************

void
VisWinAxes::GetRange(float &min_x, float &max_x, float &min_y, float &max_y)
{
    //
    // Note that even though the axes are on the background, we are getting
    // the canvas here, since we are interested in having the axes reflect
    // the view of the canvas.
    //
    vtkRenderer *canvas = mediator.GetCanvas();

    //
    // Figure out what the minimum and maximum values in the view are by
    // querying the renderer.
    //
    float  origin_x = 0.;
    float  origin_y = 0.;
    float  origin_z = 0.;
    canvas->NormalizedViewportToView(origin_x, origin_y, origin_z);
    canvas->ViewToWorld(origin_x, origin_y, origin_z);

    float right_x = 1.;
    float right_y = 0.;
    float right_z = 0.;
    canvas->NormalizedViewportToView(right_x, right_y, right_z);
    canvas->ViewToWorld(right_x, right_y, right_z);

    float top_x = 0.;
    float top_y = 1.;
    float top_z = 0.;
    canvas->NormalizedViewportToView(top_x, top_y, top_z);
    canvas->ViewToWorld(top_x, top_y, top_z);

    //
    // Even though the points are labeled as "right" and "top", they may
    // actually be "left" and "below", so be careful when assigning the
    // min and max.
    //
    min_x = (origin_x < right_x ? origin_x : right_x);
    max_x = (origin_x > right_x ? origin_x : right_x);
    min_y = (origin_y < top_y ? origin_y : top_y);
    max_y = (origin_y > top_y ? origin_y : top_y);
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
// ****************************************************************************

void
VisWinAxes::AdjustValues(float min_x, float max_x, float min_y, float max_y)
{
    int curPowX = LabelExponent(min_x, max_x);
    if (curPowX != powX)
    {
        powX = curPowX;
        SetTitle();
    }

    int curPowY = LabelExponent(min_y, max_y);
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
VisWinAxes::AdjustRange(float min_x, float max_x, float min_y, float max_y)
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
        sprintf(format, "%%.%df", xAxisDigits);
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
        sprintf(format, "%%.%df", yAxisDigits);
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
// ****************************************************************************

int
Digits(float min, float max)
{
    float  range = max - min;
    float  pow10   = log10(range);
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
// ****************************************************************************

int
LabelExponent(float min, float max)
{
    //
    // Determine power of 10 to scale axis labels to.
    //
    float range = (fabs(min) > fabs(max) ? fabs(min) : fabs(max));

    float pow10 = log10(range);

    //
    // Cutoffs for using scientific notation.
    //
    static float  eformat_cut_min = -1.5;
    static float  eformat_cut_max =  3.0;
    static float  cut_min = pow(10.f, eformat_cut_min);
    static float  cut_max = pow(10.f, eformat_cut_max);
    float ipow10;
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
// ****************************************************************************

void
VisWinAxes::SetTitle(void)
{
    char buffer[1024];
    if (powX == 0)
    {
        if (unitsX[0] == '\0')
            sprintf(buffer, "%s", xTitle);
        else
            sprintf(buffer, "%s (%s)", xTitle, unitsX);
    }
    else
    {
        if (unitsX[0] == '\0')
            sprintf(buffer, "%s (e%d)", xTitle, powX);
        else
            sprintf(buffer, "%s (e%d %s)", xTitle, powX, unitsX);
    }
    xAxis->SetTitle(buffer);

    if (powY == 0)
    {
        if (unitsY[0] == '\0')
            sprintf(buffer, "%s", yTitle);
        else
            sprintf(buffer, "%s\n (%s)", yTitle, unitsY);
    }
    else
    {
        if (unitsY[0] == '\0')
            sprintf(buffer, "%s\n (e%d)", yTitle, powY);
        else
            sprintf(buffer, " %s\n(e%d %s)", yTitle, powY, unitsY);
    }
    yAxis->SetTitle(buffer);
}
 
