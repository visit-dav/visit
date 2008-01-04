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
//                             avtLevelsLegend.C                             //
// ************************************************************************* //

#include <avtLevelsLegend.h>

#include <math.h>

#include <vtkLookupTable.h>
#include <vtkVerticalScalarBarActor.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtLevelsLegend constructor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Sep 24 16:02:31 PDT 2001
//    Intialize lut to NULL.
//
//    Hank Childs, Thu Mar 14 15:58:51 PST 2002 
//    Make legend smaller.
//
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002  
//    Initialize varName. 
//
//    Eric Brugger, Mon Jul 14 15:53:07 PDT 2003
//    Remove member varName.
//
//    Eric Brugger, Wed Jul 16 08:07:17 PDT 2003
//    I added nLevels, barVisibility and rangeVisibility.  I changed the
//    default size and position of the legend.
//
//    Brad Whitlock, Wed Mar 21 21:43:27 PST 2007
//    Added scale.
//
//    Mark C. Miller, Thu Nov 15 16:13:12 PST 2007
//    Moved call to SetLegendPosition() to bottom of constructor. Without this
//    we can wind up invoking methods on the object we are constructing here
//    before all its state variables have been initialized.
//
// ****************************************************************************

avtLevelsLegend::avtLevelsLegend()
{
    min = 0.;
    max = 1.;
    scale[0] = scale[1] = 1.;
    maxScale = 1.;
    setMaxScale = true;

    nLevels = 0;

    lut = NULL;

    sBar = vtkVerticalScalarBarActor::New();
    sBar->SetShadow(0);
    sBar->UseDefinedLabelsOn();

    size[0] = 0.08;
    size[1] = 0.26;
    sBar->SetPosition2(size[0], size[1]);

    barVisibility = 1;
    rangeVisibility = 1;
    labelVisibility = true;
    titleVisibility = true;

    //
    // Set the legend to also point to sBar, so the base methods will work
    // without us re-defining them.
    //
    legend = sBar;
    legend->Register(NULL);

    //
    // WARNING: DO NOT INITIALIZE avtLevelsLegend STATE VARIABLES BELOW HERE
    // WARNING: DO NOT INITIALIZE avtLevelsLegend STATE VARIABLES BELOW HERE
    // WARNING: DO NOT INITIALIZE avtLevelsLegend STATE VARIABLES BELOW HERE
    // See note of Thu Nov 15 16:13:12 PST 2007 above.
    //
    SetLegendPosition(0.05, 0.72);
}


// ****************************************************************************
//  Method: avtLevelsLegend destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 24 16:02:31 PDT 2001
//    Delete lut.
//
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002  
//    Delete varName. 
//
//    Eric Brugger, Mon Jul 14 15:53:07 PDT 2003
//    Remove member varName.
//
// ****************************************************************************

avtLevelsLegend::~avtLevelsLegend()
{
    if (sBar != NULL)
    {
        sBar->Delete();
        sBar = NULL;
    }
    if (lut != NULL)
    {
        lut->Delete();
        lut = NULL;
    }
}


// ****************************************************************************
//  Method: avtLevelsLegend::GetLegendSize
//
//  Purpose:
//      Gets the legend's size.
//
//  Arguments:
//      w        The legend's width.
//      h        The legend's height.
//
//  Programmer:  Eric Brugger
//  Creation:    July 15, 2003
//
//  Modifications:
//    Eric Brugger, Thu Jul 17 08:20:28 PDT 2003
//    Added maxSize argument.
//
//    Brad Whitlock, Wed Mar 21 21:41:53 PST 2007
//    Added scaling.
//
//    Hank Childs, Wed May 30 11:32:47 PDT 2007
//    Increase fudge factor (contours of 3 isolevels weren't showing up)
//
// ****************************************************************************

void
avtLevelsLegend::GetLegendSize(double maxHeight, double &w, double &h)
{
    w = 0.08 * scale[0];

    //
    // The fudge factor added to nLines when barVisibility is true is
    // there to make sure that there is enough room for all the levels
    // because this algorithm doesn't exactly match the algorithm in
    // vtkVerticalScalarBarActor.
    //
    double fudge = 0.7;
    double nLines = 0.0;
    if (title != NULL)        nLines += 1.0;
    if (databaseInfo != NULL) nLines += 2.0;
    if (varName != NULL)      nLines += 1.0;
    if (message != NULL)      nLines += 1.0;
    if (rangeVisibility)      nLines += 2.0;
    if (barVisibility)        nLines += nLevels * 1.1 + 1.0 + fudge;

    h = nLines * fontHeight * scale[1];

    //
    // If the legend is larger than the maximum size, then try to shrink
    // the color bar so that it fits within the maximum size.
    //
    if (h > maxHeight && barVisibility)
    {
        double hTitles = h - (nLevels * 1.1 + 1.0 + fudge) * fontHeight;
        double nLevelsSpace = (maxHeight - hTitles) / (fontHeight * 1.1);
        if (nLevelsSpace > 2.)
        {
            h = maxHeight;
        }
    }

    if(setMaxScale)
        maxScale = maxHeight;

    size[0] = w;
    size[1] = h;
}

// ****************************************************************************
// Method: avtLevelsLegend::SetTitleVisibility
//
// Purpose: 
//   Sets whether titles are visible.
//
// Arguments:
//   val : True if titles are to be visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:31:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetTitleVisibility(bool val)
{
    titleVisibility = val;
    sBar->SetTitleVisibility(val?1:0);
}

// ****************************************************************************
// Method: avtLevelsLegend::GetTitleVisibility
//
// Purpose: 
//   Returns whether titles are visible.
//
// Returns:    Whether titles are visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:31:48 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtLevelsLegend::GetTitleVisibility() const
{
    return titleVisibility;
}

// ****************************************************************************
// Method: avtLevelsLegend::SetLabelVisibility
//
// Purpose: 
//   Sets whether labels are visible.
//
// Arguments:
//   val : True if labels are to be visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:31:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetLabelVisibility(bool val)
{
    labelVisibility = val;
    sBar->SetLabelVisibility(val?1:0);
}

// ****************************************************************************
// Method: avtLevelsLegend::GetLabelVisibility
//
// Purpose: 
//   Returns whether labels are visible.
//
// Returns:    Whether labels are visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:31:48 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtLevelsLegend::GetLabelVisibility() const
{
    return labelVisibility;
}

// ****************************************************************************
// Method: avtLevelsLegend::SetLegendScale
//
// Purpose: 
//   Set the legend scale.
//
// Arguments:
//   xScale : The scale factor in X.
//   yScale : The scale factor in Y.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:33:20 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetLegendScale(double xScale, double yScale)
{
    // Scale the color bar
    double colorBarScale = sBar->GetBarWidth() / scale[0];
    colorBarScale *= xScale;
    sBar->SetBarWidth(colorBarScale);

    // Save the scales.
    scale[0] = xScale;
    scale[1] = yScale;
}

// ****************************************************************************
// Method: avtLevelsLegend::SetBoundingBoxVisibility
//
// Purpose: 
//   Sets whether the bounding box around the legend will be visible.
//
// Arguments:
//   val : True to make the box visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:34:03 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetBoundingBoxVisibility(bool val)
{
    sBar->SetBoundingBoxVisibility(val?1:0);
}

// ****************************************************************************
// Method: avtLevelsLegend::SetBoundingBoxColor
//
// Purpose: 
//   Set the bounding box color.
//
// Arguments:
//   color : An rgba tuple of colors.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:34:36 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetBoundingBoxColor(const double *color)
{
    sBar->SetBoundingBoxColor((double*)color);
}

// ****************************************************************************
// Method: avtLevelsLegend::SetOrientation
//
// Purpose: 
//   Set the orientation of the legend -- allowing for horizontal, vertical, ...
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:34:59 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetOrientation(LegendOrientation)
{
    debug1 << "avtLevelsLegend::SetOrientation: NOT IMPLEMENTED" << endl;
}

// ****************************************************************************
// Method: avtLevelsLegend::SetFont
//
// Purpose: 
//   Set the font properties for the legend.
//
// Arguments:
//   family : VTK_ARIAL, VTK_COURIER, VTK_TIMES
//   bold   : True to make the text bold.
//   italic : True to make the text italic.
//   shadow : True to make the text shadowed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:35:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsLegend::SetFont(int family, bool bold, bool italic, bool shadow)
{
    sBar->SetFontFamily(family);
    sBar->SetBold(bold?1:0);
    sBar->SetItalic(italic?1:0);
    sBar->SetShadow(shadow?1:0);
}

// ****************************************************************************
//  Method: avtLevelsLegend::SetColorBarVisibility
//
//  Purpose:
//      Turns on/off the visibility of the color bar. 
//
//  Arguments:
//      val    On (1) or Off (0).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2001 
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 08:07:17 PDT 2003
//    Add code to track the color bar visibility.
//
// ****************************************************************************

void
avtLevelsLegend::SetColorBarVisibility(const int val)
{
    barVisibility = val;
    sBar->SetColorBarVisibility(val);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetRange
//
//  Purpose:
//      Sets the range of the scalar bars.
//
//  Arguments:
//      nmin    The new minimum.
//      nmax    The new maximum.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
// ****************************************************************************

void
avtLevelsLegend::SetRange(double nmin, double nmax)
{
    min = nmin;
    max = nmax;
    sBar->SetRange(min, max);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetLevels
//
//  Purpose:
//      Sets the levels for the scalar bars.
//
//  Arguments:
//      levels  The levels to use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 08:07:17 PDT 2003
//    Add code to track the number of levels.
//
// ****************************************************************************

void
avtLevelsLegend::SetLevels(const std::vector<double> &levels)
{
    nLevels = levels.size();
    sBar->SetDefinedLabels(levels);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetLevels
//
//  Purpose:
//      Sets the levels for the scalar bars.
//
//  Arguments:
//      levels  The levels to use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 08:07:17 PDT 2003
//    Add code to track the number of levels.
//
// ****************************************************************************

void
avtLevelsLegend::SetLevels(const std::vector<std::string> &levels)
{
    nLevels = levels.size();
    sBar->SetDefinedLabels(levels);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetLookupTable
//
//  Purpose:
//      Sets the lookup table for the scalar bars.
//
//  Arguments:
//      lut    The lookup table to use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications: 
//  
//    Kathleen Bonnell, Mon Sep 24 16:02:31 PDT 2001
//    Copy LUT over to new member lut, register it and set its range.
//
// ****************************************************************************

void
avtLevelsLegend::SetLookupTable(vtkLookupTable *LUT) 
{
    if (lut == LUT)
    {
        return;
    }
    if (lut != NULL)
    {
        lut->Delete();
    }
    lut = LUT;
    lut->Register(NULL);
    lut->SetTableRange(0, lut->GetNumberOfColors()); 
    sBar->SetLookupTable(lut);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetLabelColorMap
//
//  Purpose:
//    Sets the label-to-colorindex map for sBar.   
//
//  Arguments:
//    cm       The new color map.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 10, 2002 
//
//  Modifications:
//    Brad Whitlock, Fri Nov 15 10:13:59 PDT 2002
//    I changed the map type.
//
// ****************************************************************************
void
avtLevelsLegend::SetLabelColorMap(const LevelColorMap &cm)
{
    sBar->SetLabelColorMap(cm);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetReverseOrder
//
//  Purpose:
//    Sets the reverse-ordering of sBar.   
//    Default is for the legend to be drawn bottom-to-top, min-to-max.
//    Reverse allows for labels and colors to be drawn top-to-bottom.
//
//  Arguments:
//    rev       The new order. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 19, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtLevelsLegend::SetReverseOrder(const bool rev)
{
    if (rev)
        sBar->ReverseOrderOn();
     else
        sBar->ReverseOrderOff();
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetVarRangeVisibility
//
//  Purpose:
//      Turns on/off the visibility of the variable range.
//
//  Arguments:
//      val     The new value (On 1, Off 0).
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 1, 2001
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 08:07:17 PDT 2003
//    Add code to track the number of levels.
//
// ****************************************************************************

void
avtLevelsLegend::SetVarRangeVisibility(const int val )
{
    rangeVisibility = val;
    sBar->SetRangeVisibility(val);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetVarRange
//
//  Purpose:
//      Sets the var range to be displayed as limits text. 
//
//  Arguments:
//      nmin    The new minimum.
//      nmax    The new maximum.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 2, 2001 
//
// ****************************************************************************

void
avtLevelsLegend::SetVarRange(double nmin, double nmax)
{
    sBar->SetVarRange(nmin, nmax);
}


// ****************************************************************************
//  Method: avtLevelsLegend::ChangePosition
//
//  Purpose:
//      Because the base type doesn't know what kind of 2D actor we have used,
//      this is the hook that allows it to do the bookkeeping and the derived
//      type to do the actual work of changing the legend's position.
//
//  Arguments:
//      x       The new x-position.
//      y       The new y-position.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications:
//    Brad Whitlock, Wed Mar 21 21:46:36 PST 2007
//    Set position2.
//
// ****************************************************************************

void
avtLevelsLegend::ChangePosition(double x, double y)
{
    sBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    sBar->GetPositionCoordinate()->SetValue(x, y, 0.);

    // Set the position 2, incorporating the scale.
    double w, h;
    setMaxScale = false;
    GetLegendSize(maxScale, w, h);
    setMaxScale = true;
    sBar->SetPosition2(w, h); 
}


// ****************************************************************************
//  Method: avtLevelsLegend::ChangeTitle
//
//  Purpose:
//      Because the base type doesn't know what kind of 2D actor we have used,
//      this is the hook that allows it to do the bookkeeping and the derived
//      type to do the actual work of changing the legend's title.
//
//  Arguments:
//      t       The new title.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 25, 2005 
// 
//  Modifications:
//
// ****************************************************************************

void
avtLevelsLegend::ChangeTitle(const char *t)
{
    sBar->SetTitle(t);
}


// ****************************************************************************
//  Method: avtLevelsLegend::ChangeFontHeight
//
//  Purpose:
//      Because the base type doesn't know what kind of 2D actor we have used,
//      this is the hook that allows it to do the bookkeeping and the derived
//      type to do the actual work of changing the legend's font height.
//
//  Arguments:
//      fh      The new font height.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 25, 2005 
// 
//  Modifications:
//    Brad Whitlock, Wed Mar 21 20:19:59 PST 2007
//    Scale the text taking scale into account.
//
// ****************************************************************************

void
avtLevelsLegend::ChangeFontHeight(double fh)
{
    double minScale = (scale[0] < scale[1]) ? scale[0] : scale[1];
    sBar->SetFontHeight(fh * minScale);
}
