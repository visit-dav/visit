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
//                           VisWinAxesParallel.C                            //
// ************************************************************************* //

#include <VisWinAxesParallel.h>

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
//  Function: Digits
//
//  Purpose:
//      Determines the appropriate number of digits for a given range.  This
//      was taken from VisWinAxes.
//
//  Arguments:
//      min    The minimum value in the range.
//      max    The maximum value in the range.
//
//  Returns:   The appropriate number of digits.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
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
//      Determines the proper exponent for the min and max values.  This was
//      taken from VisWinAxes, which was mostly stolen from old MeshTV code,
//      /meshtvx/toolkit/plotgrid.c, axlab_format.
//
//  Arguments:
//      min     The minimum value along a certain axis.
//      max     The maximum value along a certain axis.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
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
//  Method: VisWinAxesParallel constructor
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
// ****************************************************************************

VisWinAxesParallel::VisWinAxesParallel(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    addedAxes = false;
    axisVisibility = true;
    autolabelScaling = true;
    labelVisibility=1;
    titleVisibility=1;
    labelFontHeight=0.02;
    titleFontHeight=0.02;
    tickVisibility = true;
    tickLabelVisibility = true;
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
//  Method: VisWinAxesParallel destructor
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

VisWinAxesParallel::~VisWinAxesParallel()
{
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->Delete();
        if (i > 0)
        {
            axes[i].axisCap1->Delete();
            axes[i].axisCap2->Delete();
        }
    }
    axes.clear();
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the axes.
//
//  Arguments:
//      fr       The red component (rgb) of the foreground.
//      fg       The green component (rgb) of the foreground.
//      fb       The blue component (rgb) of the foreground.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
// ****************************************************************************

void
VisWinAxesParallel::SetForegroundColor(double fr_, double fg_, double fb_)
{
    fr = fr_;
    fg = fg_;
    fb = fb_;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->GetProperty()->SetColor(fr, fg, fb);
        if (i > 0)
        {
            axes[i].axisCap1->GetProperty()->SetColor(fr, fg, fb);
            axes[i].axisCap2->GetProperty()->SetColor(fr, fg, fb);
        }
    }
    UpdateTitleTextAttributes(fr, fg, fb);
    UpdateLabelTextAttributes(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinAxesParallel::UpdateView
//
//  Purpose:
//      Updates the axes so that they will reflect the current view.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
//    Eric Brugger, Tue Mar  3 09:47:53 PST 2009
//    I modified the axes so that the number of ticks displayed on an
//    individual curve would increase as the number of axes decreased.
//
// ****************************************************************************

void
VisWinAxesParallel::UpdateView(void)
{
    double  xmin = 0., xmax = 0., ymin = 0., ymax = 0.;
    GetRange(ymin, ymax, xmin, xmax);
    ymin -= (ymax - ymin) / 10000.;
    ymax += (ymax - ymin) / 10000.;
    double vxmin = vb, vxmax = vt, vymin = vl, vymax = vr;

    bool titleChange = false;

    if (axes.size() > 0)
    {
        titleChange = AdjustValues(ymin, ymax);
        AdjustRange(ymin, ymax);

        axes[0].axis->SetVisibility(axisVisibility);
        axes[0].axis->SetLabelVisibility(labelVisibility);
        axes[0].axis->SetTitleVisibility(titleVisibility);
        axes[0].axis->SetRange(ymin, ymax);

        axes[0].axis->GetPoint1Coordinate()->SetValue(vymin, vxmin);
        axes[0].axis->GetPoint2Coordinate()->SetValue(vymax, vxmin);
        if (axisPow != 0)
            axes[0].axis->SetMajorTickLabelScale(1./pow(10., axisPow));
        else
            axes[0].axis->SetMajorTickLabelScale(1.);
    }

    for (int i=1; i < axes.size(); i++)
    {
        double dx = (vxmax - vxmin) / (xmax - xmin);

        double xpos = vxmin + (axes[i].xpos-xmin)*dx;
        if (xpos < vxmin-0.001 || xpos > vxmax+0.001)
        {
            axes[i].axis->SetVisibility(false);
            axes[i].axisCap1->SetVisibility(false);
            axes[i].axisCap2->SetVisibility(false);
            continue;
        }
        axes[i].axis->SetVisibility(axisVisibility);
        axes[i].axis->SetTitleVisibility(titleVisibility);
        axes[i].axis->SetRange(ymin, ymax);
        axes[i].axis->GetPoint1Coordinate()->SetValue(vymin, xpos);
        axes[i].axis->GetPoint2Coordinate()->SetValue(vymax, xpos);
        if (axisPow != 0)
            axes[i].axis->SetMajorTickLabelScale(1./pow(10., axisPow));
        else
            axes[i].axis->SetMajorTickLabelScale(1.);

        double nTicks = floor(30./(axes.size()-1.)-1.);
        double tickSize = (axes.size() - 1.) / 60.;

        double xpos1 = vxmin + (axes[i].xpos - (nTicks * tickSize) - xmin) * dx;
        double xpos2 = vxmin + (axes[i].xpos + (nTicks * tickSize) - xmin) * dx;

        axes[i].axisCap1->SetTitle(axes[i].title);
        axes[i].axisCap1->SetVisibility(axisVisibility);
        axes[i].axisCap1->SetTitleVisibility(titleVisibility);
        axes[i].axisCap1->SetRange(-nTicks, nTicks);
        axes[i].axisCap1->GetPoint1Coordinate()->SetValue(vymin, xpos1);
        axes[i].axisCap1->GetPoint2Coordinate()->SetValue(vymin, xpos2);

        axes[i].axisCap2->SetVisibility(axisVisibility);
        axes[i].axisCap2->SetRange(-nTicks, nTicks);
        axes[i].axisCap2->GetPoint1Coordinate()->SetValue(vymax, xpos1);
        axes[i].axisCap2->GetPoint2Coordinate()->SetValue(vymax, xpos2);
    }

    if (titleChange)
    {
        SetTitles();
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetViewport
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
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetViewport(double vl_, double vb_, double vr_, double vt_)
{
    vl = vl_;
    vb = vb_;
    vr = vr_;
    vt = vt_;

    UpdateView();
}


// ****************************************************************************
//  Method: VisWinAxesParallel::UpdatePlotList
//
//  Purpose:
//      Decides what the units are for the X and Y directions.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::UpdatePlotList(vector<avtActor_p> &list)
{
    int nActors = list.size();

    //
    // Find the highest-valued axis index for any plot
    //
    int naxes = -1;
    int actorIndex = -1;
    for (int i = 0 ; i < nActors ; i++)
    {
        avtDataAttributes &atts = 
            list[i]->GetBehavior()->GetInfo().GetAttributes();
        std::vector<std::string> labels;
        atts.GetLabels(labels);
        if ((int)labels.size() > naxes)
        {
            actorIndex = i;
            naxes = labels.size();
        }
    }

    SetNumberOfAxes(naxes+1);

    if (actorIndex >= 0)
    {
        avtDataAttributes &atts = 
            list[actorIndex]->GetBehavior()->GetInfo().GetAttributes();
        std::vector<std::string> labels;
        atts.GetLabels(labels);

        double extents[6];
        if (atts.GetTrueSpatialExtents()->HasExtents())
            atts.GetTrueSpatialExtents()->CopyTo(extents);
        else if (atts.GetCumulativeTrueSpatialExtents()->HasExtents())
            atts.GetCumulativeTrueSpatialExtents()->CopyTo(extents);
        else
            EXCEPTION1(ImproperUseException,
                       "Did not have valid Spatial extents");
      
        axes[0].xpos = 0;
        axes[0].range[0] = extents[0];
        axes[0].range[1] = extents[1];
        for (int i = 0; i < naxes; i++)
        {
            axes[i+1].xpos = i + 0.5;
            axes[i+1].range[0] = extents[0];
            axes[i+1].range[1] = extents[1];
            SNPRINTF(axes[i+1].title, 256, labels[i].c_str());
        }
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::HasPlots
//
//  Purpose:
//      Receives the message from the vis window that it has plots.  This means
//      adding the axes to the vis window.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::HasPlots(void)
{
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::NoPlots
//
//  Purpose:
//      Receives the message from the vis window that it has no plots.  This
//      means that we should remove the axes from the vis window.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::NoPlots(void)
{
    RemoveAxesFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxesParallel::StartAxesParallelMode
//
//  Purpose:
//      Adds the axes to the window.  The axes are added to the background
//      renderer.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::StartAxesParallelMode(void)
{
    if (ShouldAddAxes())
    {
        AddAxesToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::StopAxesParallelMode
//
//  Purpose:
//      Removes the axes from the window.  The axes are removed from the
//      background renderer.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::StopAxesParallelMode(void)
{
    RemoveAxesFromWindow();
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
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::SetTitles(void)
{
    if (axes.size() > 0)
    {
        char buffer[1024];
        if (axisPow == 0)
        {
            if (axes[0].units[0] == '\0')
                SNPRINTF(buffer, 1024, "%s",
                         axes[0].title);
            else
                SNPRINTF(buffer, 1024, "%s (%s)",
                         axes[0].title, axes[0].units);
        }
        else
        {
            if (axes[0].units[0] == '\0')
                SNPRINTF(buffer, 1024, "%s (x10^%d)",
                         axes[0].title, axisPow);
            else
                SNPRINTF(buffer, 1024, "%s (x10^%d %s)",
                         axes[0].title, axisPow, axes[0].units);
        }
        axes[0].axis->SetTitle(buffer);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetLabelsVisibility
//
//  Purpose:
//      Sets the visibility of axis labels. 
//
//  Arguments:
//      vis     The visibility of the axis labels. 
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
// ****************************************************************************

void
VisWinAxesParallel::SetLabelVisibility(int vis)
{
    labelVisibility = vis;
    if (axes.size() > 0)
    {
        axes[0].axis->SetLabelVisibility(labelVisibility);
    }
} 


// ****************************************************************************
//  Method: VisWinAxesParallel::SetTitleVisibility
//
//  Purpose:
//      Sets the visibility of axis titles. 
//
//  Arguments:
//      vis     The visibility of the axis titles. 
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
// ****************************************************************************

void
VisWinAxesParallel::SetTitleVisibility(int vis)
{
    titleVisibility = vis;
    if (axes.size() > 0)
    {
        axes[0].axis->SetTitleVisibility(titleVisibility);
    }
    for (int i=1; i < axes.size(); i++)
    {
        axes[i].axisCap1->SetTitleVisibility(titleVisibility);
    }
} 


// ****************************************************************************
//  Method: VisWinAxesParallel::SetVisibility
//
//  Purpose:
//      Sets the visibility of this colleague. 
//
//  Arguments:
//      vis     The visibility of this colleague.  
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetVisibility(int vis)
{
    axisVisibility = vis;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetVisibility(axisVisibility);
        if (i > 0)
        {
            axes[i].axisCap1->SetVisibility(axisVisibility);
            axes[i].axisCap2->SetVisibility(axisVisibility);
        }
    }
} 
       
    
// ****************************************************************************
//  Method: VisWinAxesParallel::SetTickVisibility
//
//  Purpose:
//      Sets the visibility of the ticks.
//
//  Arguments:
//      loc     The visibility of the ticks.
//
//  Programmer: Eric Brugger
//  Creation:   January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::SetTickVisibility(bool vis, bool labelvis)
{
    tickVisibility = vis;
    tickLabelVisibility = labelvis;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetMinorTicksVisible(tickVisibility);
        axes[i].axis->SetTickVisibility(tickVisibility || tickLabelVisibility);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetAutoSetTicks
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
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetAutoSetTicks(int autoset)
{
    autoSetTicks = autoset;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetAdjustLabels(autoSetTicks);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetMajorTickMinimum
//
//  Purpose:
//      Sets the minimum values for the major tick marks.
//
//  Arguments:
//      majorMinimum The minimum value for the major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetMajorTickMinimum(double majorMinimum)
{
    majorTickMinimum = majorMinimum;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetMajorTickMinimum(majorTickMinimum);
    }
    
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetMajorTickMaximum
//
//  Purpose:
//      Sets the maximum values for the major tick marks.
//
//  Arguments:
//      majorMaximum The maximum value for the major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetMajorTickMaximum(double majorMaximum)
{
    majorTickMaximum = majorMaximum;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetMajorTickMaximum(majorTickMaximum);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetMajorTickSpacing
//
//  Purpose:
//      Sets the spacing for the major tick marks.
//
//  Arguments:
//      majorSpacing  The spacing for the major tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetMajorTickSpacing(double majorSpacing)
{
    majorTickSpacing = majorSpacing;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetMajorTickSpacing(majorTickSpacing);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetMinorTickSpacing
//
//  Purpose:
//      Sets the spacing for the minor tick marks.
//
//  Arguments:
//      minorSpacing The spacing for the minor tick marks.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetMinorTickSpacing(double minorSpacing)
{
    minorTickSpacing = minorSpacing;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->SetMinorTickSpacing(minorTickSpacing);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetLabelFontHeight
//
//  Purpose:
//      Sets the label font height for the axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
// ****************************************************************************

void
VisWinAxesParallel::SetLabelFontHeight(double height)
{
    labelFontHeight = height;
    if (axes.size() > 0)
    {
        axes[0].axis->SetLabelFontHeight(labelFontHeight);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetTitleFontHeight
//
//  Purpose:
//      Sets the title font height for the axis.
//
//  Arguments:
//      height  The font height.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
// ****************************************************************************

void
VisWinAxesParallel::SetTitleFontHeight(double height)
{
    titleFontHeight = height;
    if (axes.size() > 0)
    {
        axes[0].axis->SetTitleFontHeight(titleFontHeight);
    }
    for (int i=1; i < axes.size(); i++)
    {
        axes[i].axisCap1->SetTitleFontHeight(titleFontHeight);
    }
}


// ****************************************************************************
//  Method: VisWinAxesParallel::SetLineWidth
//
//  Purpose:
//      Sets the line width for the axes.
//
//  Arguments:
//      width   The line width.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::SetLineWidth(int width)
{
    lineWidth = width;
    for (int i=0; i < axes.size(); i++)
    {
        axes[i].axis->GetProperty()->SetLineWidth(lineWidth);
        if (i > 0)
        {
            axes[i].axisCap1->GetProperty()->SetLineWidth(lineWidth);
            axes[i].axisCap2->GetProperty()->SetLineWidth(lineWidth);
        }
    }
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::SetLabelScaling
//
//  Purpose:
//    Sets the attributes used to determine scaling for axis labels
//
//  Arguments:
//    autoscale  true if we want to determine the scale automatically
//    upow       the user-set pow to use if autoscale is false
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::SetLabelScaling(bool autoscale, int upow)
{
    autolabelScaling = autoscale;
    userPow = upow;
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::SetTitleTextAttributes
//
//  Purpose:
//    Update text style for title
//
//  Arguments:
//    att        the new text attributes
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::SetTitleTextAttributes(const VisWinTextAttributes &att)
{
    titleTextAttributes = att;

    double rgb[3];
    mediator.GetForegroundColor(rgb);
    UpdateTitleTextAttributes(rgb[0], rgb[1], rgb[2]);
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::SetLabelTextAttributes
//
//  Purpose:
//    Update text style for labels
//
//  Arguments:
//    att        the new text attributes
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::SetLabelTextAttributes(const VisWinTextAttributes &att)
{
    labelTextAttributes = att;

    double rgb[3];
    mediator.GetForegroundColor(rgb);
    UpdateLabelTextAttributes(rgb[0], rgb[1], rgb[2]);
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::SetNumberOfAxes
//
//  Purpose:
//    Creates/destroyes axis actors when the number of axes changes.
//
//  Arguments:
//    n          the desired number of new axes
//
//  Programmer:  Eric Brugger
//  Creation:    December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 10:54:08 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
//    Eric Brugger, Tue Mar  3 09:47:53 PST 2009
//    I modified the axes so that the number of ticks displayed on an
//    individual curve would increase as the number of axes decreased.
//
// ****************************************************************************

void
VisWinAxesParallel::SetNumberOfAxes(int n)
{
    bool axesCurrentlyInWindow = addedAxes;
    if (axesCurrentlyInWindow)
    {
        RemoveAxesFromWindow();
    }

    if (n < axes.size())
    {
        for (int i=1; i<n; i++)
        {
            axes[i].axisCap1->SetMajorTickMinimum(-floor(30./(n-1.)-1.));
            axes[i].axisCap1->SetMajorTickMaximum(floor(30./(n-1.)-1.));
            axes[i].axisCap2->SetMajorTickMinimum(-floor(30./(n-1.)-1.));
            axes[i].axisCap2->SetMajorTickMaximum(floor(30./(n-1.)-1.));
        }
        for (int i=n; i<axes.size(); i++)
        {
            axes[i].axis->Delete();
            if (i > 0)
            {
                axes[i].axisCap1->Delete();
                axes[i].axisCap2->Delete();
            }
        }
        axes.resize(n);
    }
    else if (n > axes.size())
    {
        for (int i=1; i<axes.size(); i++)
        {
            axes[i].axisCap1->SetMajorTickMinimum(-floor(30./(n-1.)-1.));
            axes[i].axisCap1->SetMajorTickMaximum(floor(30./(n-1.)-1.));
            axes[i].axisCap2->SetMajorTickMinimum(-floor(30./(n-1.)-1.));
            axes[i].axisCap2->SetMajorTickMaximum(floor(30./(n-1.)-1.));
        }
        for (int i=axes.size(); i<n; i++)
        {
            vtkVisItAxisActor2D *ax;
            ax = vtkVisItAxisActor2D::New();
            ax->SetMinorTicksVisible(tickVisibility);
            ax->SetTickVisibility(tickVisibility || tickLabelVisibility);
            ax->SetFontFamilyToCourier();
            ax->SetShadow(0);
            ax->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
            ax->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
            ax->PickableOff();
            ax->SetEndStringVOffsetFactor(0);
            ax->SetEndStringHOffsetFactor(-0.5);
            ax->SetUseOrientationAngle(1);
            ax->SetTitleAtEnd(0);
            ax->SetOrientationAngle(0.);

            AxisInfo a(ax, 0, 3);
            axes.push_back(a);

            // Update the properties of this new axis
            axes[i].axis->SetVisibility(axisVisibility);
            axes[i].axis->SetLabelVisibility(false);
            axes[i].axis->SetTitleVisibility(titleVisibility);
            axes[i].axis->SetTickLocation(2);
            axes[i].axis->SetDrawGridlines(false);
            axes[i].axis->SetAdjustLabels(autoSetTicks);
            axes[i].axis->SetMajorTickMinimum(majorTickMinimum);
            axes[i].axis->SetMajorTickMaximum(majorTickMaximum);
            axes[i].axis->SetMajorTickSpacing(majorTickSpacing);
            axes[i].axis->SetMinorTickSpacing(minorTickSpacing);
            axes[i].axis->SetLabelFontHeight(labelFontHeight);
            axes[i].axis->SetTitleFontHeight(titleFontHeight);
            axes[i].axis->GetProperty()->SetLineWidth(lineWidth);
            axes[i].axis->GetProperty()->SetColor(fr, fg, fb);

            if (i > 0)
            {
                vtkVisItAxisActor2D *ax;
                ax = vtkVisItAxisActor2D::New();
                ax->SetTickVisibility(1);
                ax->SetFontFamilyToCourier();
                ax->SetShadow(0);
                ax->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
                ax->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
                ax->PickableOff();
                ax->SetUseOrientationAngle(1);
                ax->SetOrientationAngle(-1.5707963);

                ax->SetVisibility(axisVisibility);

                ax->SetLabelVisibility(false);
                ax->SetTitleVisibility(titleVisibility);
                ax->SetTickLocation(0);
                ax->SetDrawGridlines(false);
                ax->SetAdjustLabels(false);
                ax->SetMajorTickMinimum(-floor(30./(n-1.)-1.));
                ax->SetMajorTickMaximum(floor(30./(n-1.)-1.));
                ax->SetMajorTickSpacing(1.);
                ax->SetMinorTickSpacing(1.);
                ax->SetLabelFontHeight(labelFontHeight);
                ax->SetTitleFontHeight(titleFontHeight);
                ax->GetProperty()->SetLineWidth(lineWidth);
                ax->GetProperty()->SetColor(fr, fg, fb);

                axes[i].axisCap1 = ax;

                ax = vtkVisItAxisActor2D::New();
                ax->SetTickVisibility(1);
                ax->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
                ax->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
                ax->PickableOff();
                ax->SetUseOrientationAngle(1);
                ax->SetOrientationAngle(1.5707963);

                ax->SetVisibility(axisVisibility);

                ax->SetLabelVisibility(false);
                ax->SetTitleVisibility(false);
                ax->SetTickLocation(0);
                ax->SetDrawGridlines(false);
                ax->SetAdjustLabels(false);
                ax->SetMajorTickMinimum(-floor(30./(n-1.)-1.));
                ax->SetMajorTickMaximum(floor(30./(n-1.)-1.));
                ax->SetMajorTickSpacing(1.);
                ax->SetMinorTickSpacing(1.);
                ax->GetProperty()->SetLineWidth(lineWidth);
                ax->GetProperty()->SetColor(fr, fg, fb);

                axes[i].axisCap2 = ax;
            }
        }
    }

    if (axesCurrentlyInWindow)
    {
        AddAxesToWindow();
    }

    UpdateLabelTextAttributes(fr,fg,fb);
    UpdateTitleTextAttributes(fr,fg,fb);
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::AdjustValues
//
//  Purpose:
//    Determine a good power scaling for the given axis.
//
//  Arguments:
//    minval,maxval    the range of the given axis
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

bool
VisWinAxesParallel::AdjustValues(double minval, double maxval)
{
    int curPow;
    if (autolabelScaling)
    {
        curPow = LabelExponent(minval, maxval);
    }
    else
    {
        curPow = userPow;
    }

    if (curPow != axisPow)
    {
        axisPow = curPow;
        return true;
    }
    return false;
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::AdjustRange
//
//  Purpose:
//    Determine a good format string for the given axis.
//
//  Arguments:
//    minval,maxval    the range of the given axis
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::AdjustRange(double minval, double maxval)
{
    if (axisPow != 0)
    {
        minval /= pow(10., axisPow);
        maxval /= pow(10., axisPow);
    }
    int axisDigits = Digits(minval, maxval);
    char  format[16];
    SNPRINTF(format, 16, "%%.%df", axisDigits);
    axes[0].axis->SetLabelFormat(format);
}


// ****************************************************************************
//  Method: VisWinAxesParallel::GetRange
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
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::GetRange(double &min_x, double &max_x,
                          double &min_y, double &max_y)
{
    VisWindow *vw = mediator;

    switch (vw->GetWindowMode())
    {
      case WINMODE_AXISPARALLEL:
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
//  Method: VisWinAxesParallel::AddAxesToWindow
//
//  Purpose:
//      Adds the axes to the vis window.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::AddAxesToWindow(void)
{
    if (addedAxes)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    for (int i=0; i < axes.size(); i++)
    {
        foreground->AddActor2D(axes[i].axis);
        if (i > 0)
        {
            foreground->AddActor2D(axes[i].axisCap1);
            foreground->AddActor2D(axes[i].axisCap2);
        }
    }

    addedAxes = true;
}


// ****************************************************************************
//  Method: VisWinAxesParallel::RemoveAxesFromWindow
//
//  Purpose:
//      Removes the axes from the vis window.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

void
VisWinAxesParallel::RemoveAxesFromWindow(void)
{
    if (! addedAxes)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    for (int i=0; i < axes.size(); i++)
    {
        foreground->RemoveActor2D(axes[i].axis);
        if (i > 0)
        {
            foreground->RemoveActor2D(axes[i].axisCap1);
            foreground->RemoveActor2D(axes[i].axisCap2);
        }
    }

    addedAxes = false;
}


// ****************************************************************************
//  Method: VisWinAxesParallel::ShouldAddAxes
//
//  Purpose:
//      Hides from routines that would like to add axes the logic about what
//      state the VisWindow must be in.  It should only be added if we are
//      in 2D mode and there are plots.
//
//  Returns:    true if the axes should be added to the vis window, false
//              otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
// ****************************************************************************

bool
VisWinAxesParallel::ShouldAddAxes(void)
{
    return (mediator.GetMode() == WINMODE_AXISPARALLEL &&
            mediator.HasPlots());
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::UpdateLabelTextAttributes
//
//  Purpose:
//    Update text style for labels
//
//  Arguments:
//    fr,fg,fb   the new foreground color
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::UpdateLabelTextAttributes(double fr, double fg, double fb)
{
    if (axes.size() > 0)
    {
        vtkVisItAxisActor2D *axis = axes[0].axis;
        // Set the colors
        if(labelTextAttributes.useForegroundColor)
            axis->GetLabelTextProperty()->SetColor(fr, fg, fb);
        else
        {
            axis->GetLabelTextProperty()->SetColor(
                labelTextAttributes.color[0],
                labelTextAttributes.color[1],
                labelTextAttributes.color[2]);
        }

        axis->GetLabelTextProperty()->SetFontFamily((int)labelTextAttributes.font);
        axis->GetLabelTextProperty()->SetBold(labelTextAttributes.bold?1:0);
        axis->GetLabelTextProperty()->SetItalic(labelTextAttributes.italic?1:0);

        // Pass the opacity in the line offset.
        axis->GetLabelTextProperty()->SetLineOffset(labelTextAttributes.color[3]);
    }
}


// ****************************************************************************
//  Method:  VisWinAxesParallel::UpdateTitleTextAttributes
//
//  Purpose:
//    Update text style for titles
//
//  Arguments:
//    fr,fg,fb   the new foreground color
//
//  Programmer:  Eric Brugger
//  Creation:    January 20, 2009
//
// ****************************************************************************

void
VisWinAxesParallel::UpdateTitleTextAttributes(double fr, double fg, double fb)
{ 
    if (axes.size() > 0)
    {
        vtkVisItAxisActor2D *axis = axes[0].axis;
        // Set the colors
        if(titleTextAttributes.useForegroundColor)
            axis->GetTitleTextProperty()->SetColor(fr, fg, fb);
        else
        {
            axis->GetTitleTextProperty()->SetColor(
                titleTextAttributes.color[0],
                titleTextAttributes.color[1],
                titleTextAttributes.color[2]);
        }

        axis->GetTitleTextProperty()->SetFontFamily((int)titleTextAttributes.font);
        axis->GetTitleTextProperty()->SetBold(titleTextAttributes.bold?1:0);
        axis->GetTitleTextProperty()->SetItalic(titleTextAttributes.italic?1:0);

        // Pass the opacity in the line offset.
        axis->GetTitleTextProperty()->SetLineOffset(titleTextAttributes.color[3]);
    }
    for(int i = 1; i < axes.size(); ++i)
    {
        vtkVisItAxisActor2D *axis = axes[i].axisCap1;
        // Set the colors
        if(titleTextAttributes.useForegroundColor)
            axis->GetTitleTextProperty()->SetColor(fr, fg, fb);
        else
        {
            axis->GetTitleTextProperty()->SetColor(
                titleTextAttributes.color[0],
                titleTextAttributes.color[1],
                titleTextAttributes.color[2]);
        }

        axis->GetTitleTextProperty()->SetFontFamily((int)titleTextAttributes.font);
        axis->GetTitleTextProperty()->SetBold(titleTextAttributes.bold?1:0);
        axis->GetTitleTextProperty()->SetItalic(titleTextAttributes.italic?1:0);

        // Pass the opacity in the line offset.
        axis->GetTitleTextProperty()->SetLineOffset(titleTextAttributes.color[3]);
    }
}
