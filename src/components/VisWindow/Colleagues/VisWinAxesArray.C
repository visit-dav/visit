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
//                             VisWinAxesArray.C                             //
// ************************************************************************* //

#include <VisWinAxesArray.h>

#include <string>
#include <vector>
#include <snprintf.h>

#include <vtkVisItAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <DebugStream.h>
#include <avtExtents.h>


//
// Static Function Prototypes
//

static int   LabelExponent(double, double);
static int   Digits(double, double);


using   std::string;
using   std::vector;


// ****************************************************************************
//  Method: VisWinAxesArray constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

VisWinAxesArray::VisWinAxesArray(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    addedAxes = false;
    axisVisibility = true;
    autolabelScaling = true;
    labelVisibility=1;
    titleVisibility=1;
    labelFontHeight=0.02;
    titleFontHeight=0.02;
    gridVisibility=0;
    tickLocation=2;
    majorTickMinimum=0;
    majorTickMaximum=1;
    majorTickSpacing=0.2;
    minorTickSpacing=0.02;
    autoSetTicks=1;
    lineWidth=1;
    fg=fr=fb=0;

    // These should really get set to something reasonable before we're
    // asked to draw, but just in case...
    vl = 0.2;
    vb = 0.2;
    vr = .8;
    vt = .8;
}


// ****************************************************************************
//  Method: VisWinAxesArray destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

VisWinAxesArray::~VisWinAxesArray()
{
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->Delete();
    }
    axes.clear();
}


// ****************************************************************************
//  Method: VisWinAxesArray::StartCurveMode
//
//  Purpose:
//      Adds the axes to the window.  The axes are added to the background
//      renderer.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxesArray::StartAxisArrayMode(void)
{
    //SetTitle();
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::StopCurveMode
//
//  Purpose:
//      Removes the axes from the window.  The axes are removed from the
//      background renderer.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::StopAxisArrayMode(void)
{
    RemoveAxesFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetViewport
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
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetViewport(double vl_, double vb_, double vr_, double vt_)
{
    vl = vl_;
    vb = vb_;
    vr = vr_;
    vt = vt_;

    UpdateView();
}


// ****************************************************************************
//  Method: VisWinAxesArray::AddAxesToWindow
//
//  Purpose:
//      Adds the axes to the vis window.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxesArray::AddAxesToWindow(void)
{
    if (addedAxes)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        foreground->AddActor2D(axes[i].axis);
    }

    addedAxes = true;
}


// ****************************************************************************
//  Method: VisWinAxesArray::RemoveAxesFromWindow
//
//  Purpose:
//      Removes the axes from the vis window.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxesArray::RemoveAxesFromWindow(void)
{
    if (! addedAxes)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        foreground->RemoveActor2D(axes[i].axis);
    }

    addedAxes = false;
}


// ****************************************************************************
//  Method: VisWinAxesArray::ShouldAddAxes
//
//  Purpose:
//      Hides from routines that would like to add axes the logic about what
//      state the VisWindow must be in.  It should only be added if we are
//      in 2D mode and there are plots.
//
//  Returns:    true if the axes should be added to the vis window, false
//              otherwise.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

bool
VisWinAxesArray::ShouldAddAxes(void)
{
    return (mediator.GetMode() == WINMODE_AXISARRAY &&
            mediator.HasPlots());
}


// ****************************************************************************
//  Method: VisWinAxesArray::HasPlots
//
//  Purpose:
//      Receives the message from the vis window that it has plots.  This means
//      adding the axes to the vis window.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::HasPlots(void)
{
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::NoPlots
//
//  Purpose:
//      Receives the message from the vis window that it has no plots.  This
//      means that we should remove the axes from the vis window.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::NoPlots(void)
{
    RemoveAxesFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxesArray::UpdateView
//
//  Purpose:
//      Updates the axes so that they will reflect the current view.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxesArray::UpdateView(void)
{
    double  xmin = 0., xmax = 0., ymin = 0., ymax = 0.;
    GetRange(xmin, xmax, ymin, ymax);

    double vx = vr-vl;
    double vy = vt-vb;
    double bottom = vb;
    double top = vt;
    double rx = xmax - xmin;
    double ry = ymax - ymin;

    bool CLAMP_AXES_TO_LIMITS = true;
    if (CLAMP_AXES_TO_LIMITS)
    {
        if (ymin < 0)
        {
            bottom = vb + vy*(0-ymin)/ry;
            ymin = 0;
        }
        if (ymax > 1)
        {
            top = vt + vy*(1-ymax)/ry;
            ymax = 1;
        }
    }
        
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        double dx = vx / rx;
        double dy = vy / ry;

        double amin = axes[i].range[0];
        double amax = axes[i].range[1];
        double ra = amax - amin;

        double xpos = vl + (i-xmin)*dx;
        if (xpos < vl-0.001 || xpos > vr+0.001 || ymin > 1 || ymax < 0)
        {
            axes[i].axis->SetVisibility(false);
            continue;
        }
        axes[i].axis->SetVisibility(axisVisibility);
        axes[i].axis->SetRange(amin + ymin*ra,amin + ymax*ra);
        axes[i].axis->GetPoint1Coordinate()->SetValue(xpos, bottom);
        axes[i].axis->GetPoint2Coordinate()->SetValue(xpos, top);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::UpdatePlotList
//
//  Purpose:
//      Decides what the units are for the X and Y directions.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxesArray::UpdatePlotList(vector<avtActor_p> &list)
{
    int nActors = list.size();

    // Find the highest-valued axis index for any variable
    int naxes = 0;
    for (int i = 0 ; i < nActors ; i++)
    {
        avtDataAttributes &atts = 
            list[i]->GetBehavior()->GetInfo().GetAttributes();
        int nvars = atts.GetNumberOfVariables();
        for (int j = 0 ; j < nvars ; j++)
        {
            const char *var = atts.GetVariableName(j).c_str();
            int axis = atts.GetUseForAxis(var);
            if (axis == -1)
                continue;
            naxes = (axis+1) > naxes ? (axis+1) : naxes;
        }
    }

    // Create the axes
    SetNumberOfAxes(naxes);

    // Set the title/units/extents for the variables
    for (int i = 0 ; i < nActors ; i++)
    {
        avtDataAttributes &atts = 
                             list[i]->GetBehavior()->GetInfo().GetAttributes();
        int nvars = atts.GetNumberOfVariables();
        for (int j = 0 ; j < nvars ; j++)
        {
            const char *var = atts.GetVariableName(j).c_str();
            int axis = atts.GetUseForAxis(var);
            if (axis == -1)
                continue;
            avtExtents *ext = atts.GetCumulativeTrueDataExtents(var);
            if (!ext)
            {
                char str[100];
                sprintf(str, "Did not have valid extents for var '%s'", var);
                EXCEPTION1(ImproperUseException, str);
            }
            atts.GetCumulativeTrueDataExtents(var)->CopyTo(axes[axis].range);
            SNPRINTF(axes[axis].title,256, var);
            SNPRINTF(axes[axis].units,256, atts.GetVariableUnits(var).c_str());
        }
    }
    SetTitles();
}


// ****************************************************************************
//  Method: VisWinAxesArray::GetRange
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
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
// 
// ****************************************************************************

void
VisWinAxesArray::GetRange(double &min_x, double &max_x,
                          double &min_y, double &max_y)
{
    VisWindow *vw = mediator;

    switch (vw->GetWindowMode())
    {
      case WINMODE_AXISARRAY:
        {
        const avtViewAxisArray viewAxisArray = vw->GetViewAxisArray();
        min_x = viewAxisArray.domain[0];
        max_x = viewAxisArray.domain[1];
        min_y = viewAxisArray.range[0];
        max_y = viewAxisArray.range[1];
        }
        break;
      default:
        break;
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
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: taken from VisWinAxes
//
//  Modifications:
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
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: taken from VisWinAxes
//
//  Modifications:
//
// ****************************************************************************

int
LabelExponent(double min, double max)
{
    //
    // Determine power of 10 to scale axis labels to.
    //
    double range = (fabs(min) > fabs(max) ? fabs(min) : fabs(max));

    double pow10 = log10(range);

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
//  Method: VisWinAxesArray::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the axes.
//
//  Arguments:
//      fr       The red component (rgb) of the foreground.
//      fg       The green component (rgb) of the foreground.
//      fb       The blue component (rgb) of the foreground.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetForegroundColor(double fr_, double fg_, double fb_)
{
    fr = fr_;
    fg = fg_;
    fb = fb_;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->GetProperty()->SetColor(fr, fg, fb);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetVisibility
//
//  Purpose:
//      Sets the visibility of this colleague. 
//
//  Arguments:
//      vis     The visibility of this colleague.  
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetVisibility(int vis)
{
    axisVisibility = vis;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetVisibility(axisVisibility);
    }
} 
       
    
// ****************************************************************************
//  Method: VisWinAxesArray::SetLabelsVisibility
//
//  Purpose:
//      Sets the visibility of axis labels. 
//
//  Arguments:
//      vis     The visibility of the axis labels. 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Wed Nov  5 11:41:19 PST 2002
//    Removed the control of the title visibility out of this method into
//    its own method.
//
// ****************************************************************************

void
VisWinAxesArray::SetLabelVisibility(int vis)
{
    labelVisibility = vis;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetLabelVisibility(labelVisibility);
    }
} 


// ****************************************************************************
//  Method: VisWinAxesArray::SetTitleVisibility
//
//  Purpose:
//      Sets the visibility of axis titles. 
//
//  Arguments:
//      vis     The visibility of the axis titles. 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetTitleVisibility(int vis)
{
    titleVisibility = vis;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetTitleVisibility(titleVisibility);
    }
} 


// ****************************************************************************
//  Method: VisWinAxesArray::SetTickLocation
//
//  Purpose:
//      Sets the location of the ticks. 
//
//  Arguments:
//      loc     The location of the ticks.  
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetTickLocation(int loc)
{
    tickLocation = loc;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetTickLocation(tickLocation);
    }
} 


// ****************************************************************************
//  Method: VisWinAxesArray::SetGridVisibility
//
//  Purpose:
//      Sets the visibility of axis gridlines. 
//
//  Arguments:
//      vis     The visibility of the gridlines. 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetGridVisibility(int vis)
{
    gridVisibility = vis;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetDrawGridlines(gridVisibility);
    }
} 
       
       
// ****************************************************************************
//  Method: VisWinAxesArray::SetAutoSetTicks
//
//  Purpose:
//      Sets the flag which specifies if the ticks should be automatically
//      selected or user specified.
//
//  Arguments:
//      autoSetTicks The flag indicating if the ticks should be selected
//                   automatically.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetAutoSetTicks(int autoset)
{
    autoSetTicks = autoset;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetAdjustLabels(autoSetTicks);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetMajorTickMinimum
//
//  Purpose:
//      Sets the minimum values for the major tick marks.
//
//  Arguments:
//      MajorMinimum The minimum value for the major tick marks.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetMajorTickMinimum(double majorMinimum)
{
    majorTickMinimum = majorMinimum;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetMajorTickMinimum(majorTickMinimum);
    }
    
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetMajorTickMaximum
//
//  Purpose:
//      Sets the maximum values for the major tick marks.
//
//  Arguments:
//      MajorMaximum The maximum value for the major tick marks.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetMajorTickMaximum(double majorMaximum)
{
    majorTickMaximum = majorMaximum;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetMajorTickMaximum(majorTickMaximum);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetMajorTickSpacing
//
//  Purpose:
//      Sets the spacing for the major tick marks.
//
//  Arguments:
//      MajorSpacing  The spacing for the major tick marks.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetMajorTickSpacing(double majorSpacing)
{
    majorTickSpacing = majorSpacing;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetMajorTickSpacing(majorTickSpacing);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetMinorTickSpacing
//
//  Purpose:
//      Sets the spacing for the minor tick marks.
//
//  Arguments:
//      MinorSpacing The spacing for the minor tick marks.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetMinorTickSpacing(double minorSpacing)
{
    minorTickSpacing = minorSpacing;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetMinorTickSpacing(minorTickSpacing);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetLabelFontHeight
//
//  Purpose:
//      Sets the label font height for the axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetLabelFontHeight(double height)
{
    labelFontHeight = height;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetLabelFontHeight(labelFontHeight);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetTitleFontHeight
//
//  Purpose:
//      Sets the title font height for the axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetTitleFontHeight(double height)
{
    titleFontHeight = height;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->SetTitleFontHeight(titleFontHeight);
    }
}


// ****************************************************************************
//  Method: VisWinAxesArray::SetLineWidth
//
//  Purpose:
//      Sets the line width for the axes.
//
//  Arguments:
//      width   The line width.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetLineWidth(int width)
{
    lineWidth = width;
    int axisCount = axes.size();
    for (int i=0; i < axisCount; i++)
    {
        axes[i].axis->GetProperty()->SetLineWidth(lineWidth);
    }
}


// ****************************************************************************
//  Method:  VisWinAxesArray::SetNumberOfAxes
//
//  Purpose:
//    Creates/destroyes axis actors when the number of axes changes.
//
//  Arguments:
//    n          the desired number of new axes
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Mon Feb  4 12:34:49 EST 2008
//    Made the orientation angle for the title point upwards instead of
//    downwards -- this obviates the need for a positive vertical string
//    offset.
//
//    Jeremy Meredith, Mon Feb  4 17:09:59 EST 2008
//    I didn't like all the major tick value labels being on the right
//    side of the axes, so I added a new option to vtkVisItAxisActor2D
//    to allow the title to use the opposite orientation from the labels.
//    I'm now using it to make the rightmost axis have the labels on the
//    right and the others on the left.  This should probably be
//    configurable by the user.....
//
// ****************************************************************************

void
VisWinAxesArray::SetNumberOfAxes(int n)
{
    bool axesCurrentlyInWindow = addedAxes;
    if (axesCurrentlyInWindow)
    {
        RemoveAxesFromWindow();
    }

    if (n < axes.size())
    {
        for (int i=n; i<axes.size(); i++)
        {
            axes[i].axis->Delete();
        }
        axes.resize(n);
    }
    else if (n > axes.size())
    {
        for (int i=axes.size(); i<n; i++)
        {
            vtkVisItAxisActor2D *ax;
            ax = vtkVisItAxisActor2D::New();
            ax->SetTickVisibility(1);
            ax->SetFontFamilyToCourier();
            ax->SetShadow(0);
            ax->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
            ax->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
            ax->PickableOff();
            ax->SetEndStringVOffsetFactor(0);
            ax->SetEndStringHOffsetFactor(-0.5);
            ax->SetUseOrientationAngle(1);
            if (i < n-1)
            {
                ax->SetOrientationAngle(-1.5707963);
                ax->SetEndStringReverseOrientation(true);
            }
            else
            {
                ax->SetOrientationAngle(+1.5707963);
                ax->SetEndStringReverseOrientation(false);
            }
            ax->SetTitleAtEnd(1);

            AxisInfo a(ax, 0, 3, 0, 0);
            SNPRINTF(a.title, 8,  "Axis%02d", i);
            axes.push_back(a);

            // Update the properties of this new axis
            axes[i].axis->SetVisibility(axisVisibility);
            axes[i].axis->SetLabelVisibility(labelVisibility);
            axes[i].axis->SetTitleVisibility(titleVisibility);
            axes[i].axis->SetTickLocation(tickLocation);
            axes[i].axis->SetDrawGridlines(gridVisibility);
            axes[i].axis->SetAdjustLabels(autoSetTicks);
            axes[i].axis->SetMajorTickMinimum(majorTickMinimum);
            axes[i].axis->SetMajorTickMaximum(majorTickMaximum);
            axes[i].axis->SetMajorTickSpacing(majorTickSpacing);
            axes[i].axis->SetMinorTickSpacing(minorTickSpacing);
            axes[i].axis->SetLabelFontHeight(labelFontHeight);
            axes[i].axis->SetTitleFontHeight(titleFontHeight);
            axes[i].axis->GetProperty()->SetLineWidth(lineWidth);
            axes[i].axis->GetProperty()->SetColor(fr, fg, fb);
        }
    }

    if (axesCurrentlyInWindow)
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method:  VisWinAxesArray::SetTitles
//
//  Purpose:
//    Updated the titles/units of the given axes.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2008
//
// ****************************************************************************

void
VisWinAxesArray::SetTitles(void)
{
    for (int i=0; i<axes.size(); i++)
    {
        char buffer[1024];
        if (axes[i].pow == 0)
        {
            if (axes[i].units[0] == '\0')
                SNPRINTF(buffer, 1024, "%s",
                         axes[i].title);
            else
                SNPRINTF(buffer, 1024, "%s (%s)",
                         axes[i].title, axes[i].units);
        }
        else
        {
            if (axes[i].units[0] == '\0')
                SNPRINTF(buffer, 1024, "%s (x10^%d)",
                         axes[i].title, axes[i].pow);
            else
                SNPRINTF(buffer, 1024, "%s (x10^%d %s)",
                         axes[i].title, axes[i].pow, axes[i].units);
        }
        axes[i].axis->SetTitle(buffer);
    }
}
