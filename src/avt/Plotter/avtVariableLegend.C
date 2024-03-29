// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVariableLegend.C                           //
// ************************************************************************* //

#include <vtkLookupTable.h>
#include <vtkVisItScalarBarActor.h>

#include <avtVariableLegend.h>

#include <DebugStream.h>
#include <float.h>

// ****************************************************************************
//  Method: avtVariableLegend constructor
//
//  Programmer:  Hank Childs
//  Creation:    October 4, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Dec  8 15:02:31 PST 2000 
//    Changed sBar to be of type vtkVisItScalarBarActor, a modified
//    version of vtkScalarBarActor, better suited to VisIt.
//
//    Hank Childs, Thu Dec 28 10:13:43 PST 2000
//    Removed min and max arguments.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001 
//    Legends don't own the lut.  It must be set by the plot using this
//    legend.  Don't build it here, initialize it to NULL.  
//    Moved setting sBar's lut to method SetLookupTable.
//
//    Kathleen Bonnell, Wed Oct 10 17:31:26 PDT 2001 
//    Initialize new member 'title'
//
//    Hank Childs, Thu Mar 14 15:57:09 PST 2002
//    Make the legend a bit smaller.
//
//    Eric Brugger, Mon Jul 14 15:54:19 PDT 2003
//    Remove member title.  Remove initialization of scalar bar title.
//
//    Eric Brugger, Wed Jul 16 08:20:29 PDT 2003
//    I added barVisibility and rangeVisibility.  I changed the default
//    size and position of the legend.
//
//    Brad Whitlock, Wed Mar 21 11:26:58 PDT 2007
//    Added labelVisibility and scale.
//
//    Mark C. Miller, Fri Sep  7 10:18:38 PDT 2007
//    Moved call(s) to class methods to *after* all members have been
//    initialized. If we don't do this, then we can wind up invoking methods
//    on the object we are constructing here before all state variables have
//    been initialized.
//
//    Hank Childs, Fri Jan 23 15:39:46 PST 2009
//    Initialize minmaxVisibility.
//
//    Kathleen Bonnell, Thu Oct  1 14:24:19 PDT 2009
//    Initialize numTicks.  Set LegendType for sBar.
//
//    Alister Maguire, Wed Jan 23 11:04:08 PST 2019
//    Initialized nanColor. 
//
// ****************************************************************************

avtVariableLegend::avtVariableLegend()
{
    min = 0.;
    max = 1.;

    lut = NULL;
 
    sBar = vtkVisItScalarBarActor::New();
    sBar->SetShadow(0);

    scale[0] = 1.;
    scale[1] = 1.;
    size[0] = 0.08;
    size[1] = 0.26;
    sBar->SetPosition2(size[0], size[1]);
    sBar->SetType(vtkVisItScalarBarActor::VTK_CONTINUOUS);

    barVisibility = true;
    rangeVisibility = true;
    titleVisibility = true;
    labelVisibility = 1;
    minmaxVisibility = true;
    numTicks = 5;

    //
    // Set the nan color to grey by default.
    //
    nanColor[3] = 1.0;
    for (int i = 0; i < 3; ++i)
        nanColor[i] = .41;

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
    // See note of Fri Sep  7 10:18:38 PDT 2007 above
    //
    SetLegendPosition(0.05, 0.72);
}


// ****************************************************************************
//  Method: avtVariableLegend::avtVariableLegend
//
//  Purpose: 
//    Constructor for the avtVariableLegend class.
//
//  Arguments:
//    arg : This argument is pretty much ignored. It is just to mark this as
//          a different constructor.
//
//  Returns:    
//
//  Note:       This constructor performs the same initialization as the default
//              constructor except that it does not create the scalar bar
//              actor.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Apr 20 09:07:33 PDT 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    Don't build lut, it must be set by plot using this legend.  Intitialize
//    it to NULL.
//   
//    Kathleen Bonnell, Wed Oct 10 17:31:26 PDT 2001 
//    Initialize new member 'title'. 
//    
//    Eric Brugger, Mon Jul 14 15:54:19 PDT 2003
//    Remove member title.
//
//    Brad Whitlock, Wed Mar 21 23:55:51 PST 2007
//    Initialize scale and other members.
//
//    Hank Childs, Fri Jan 23 15:40:11 PST 2009
//    Initialize minmaxVisibility.
//
//    Kathleen Bonnell, Thu Oct  1 14:24:19 PDT 2009
//    Initialize numTicks.
//
// ****************************************************************************

avtVariableLegend::avtVariableLegend(int)
{
    min = 0.;
    max = 1.;
    lut = NULL;
    sBar = NULL;
    scale[0] = 1.;
    scale[1] = 1.;
    size[0] = 0.08;
    size[1] = 0.26;
    barVisibility = true;
    rangeVisibility = true;
    labelVisibility = 1;
    minmaxVisibility = true;
    titleVisibility = true;
    numTicks = 5;
}


// ****************************************************************************
//  Method: avtVariableLegend destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    This legend no longer owns the lut, should not try to delete it.
//
//    Kathleen Bonnell, Wed Oct 10 17:31:26 PDT 2001 
//    Delete new member 'title'. 
//
//    Eric Brugger, Mon Jul 14 15:54:19 PDT 2003
//    Remove member title.
//
// ****************************************************************************

avtVariableLegend::~avtVariableLegend()
{
    if (sBar != NULL)
    {
        sBar->Delete();
        sBar = NULL;
    }
}


// ****************************************************************************
//  Method: avtVariableLegend::GetLegendSize
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
//    Eric Brugger, Thu Jul 17 08:47:55 PDT 2003
//    Added maxSize argument.  It is unused in this routine so it is unnamed.
//
//    Brad Whitlock, Tue Jul 20 17:16:12 PST 2004
//    Added varUnits.
//
//    Brad Whitlock, Wed Mar 21 11:49:05 PDT 2007
//    Added scaling and prevented position2 from being set.
//
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Adjust computed size based on orientation.
//
//    Hank Childs, Fri Jan 23 15:56:30 PST 2009
//    Add support for user setting min/max visibility.
//
// ****************************************************************************

void
avtVariableLegend::GetLegendSize(double, double &w, double &h)
{

    if (barVisibility)
    {
        if (orientation == VerticalTextOnRight ||
            orientation == VerticalTextOnLeft) 
        {
            w = 0.08 * scale[0];
            h = 0.26 * scale[1];
        }
        else
        {
            w = 0.26 * scale[0];
            h = 0.14 * scale[1];
        }
    }
    else
    {
        w = 0.08 * scale[0];

        double nLines = 0.51;

        if (title != NULL)        nLines += 1.0;
        if (databaseInfo != NULL) nLines += 2.0;
        if (varName != NULL)      nLines += 1.0;
        if (varUnits != NULL)     nLines += 1.0;
        if (message != NULL)      nLines += 1.0;
        if (rangeVisibility && minmaxVisibility)      nLines += 2.5;

        h = nLines * fontHeight * scale[1];
    }

    size[0] = w;
    size[1] = h;
}

// ****************************************************************************
// Method: avtVariableLegend::SetTitleVisibility
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
avtVariableLegend::SetTitleVisibility(bool val)
{
    titleVisibility = val;
    sBar->SetTitleVisibility(val);
}

// ****************************************************************************
// Method: avtVariableLegend::GetTitleVisibility
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
avtVariableLegend::GetTitleVisibility() const
{
    return titleVisibility;
}

// ****************************************************************************
// Method: avtVariableLegend::SetLabelVisibility
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
//    Kathleen Bonnell, Thu Oct  1 14:26:46 PDT 2009
//    LabelVisibility has been changed to DrawMode in sbar.
//   
// ****************************************************************************

void
avtVariableLegend::SetLabelVisibility(int val)
{
    labelVisibility = val;
    sBar->SetDrawMode((vtkVisItScalarBarActor::TickLabelDrawMode)val);
}

// ****************************************************************************
// Method: avtVariableLegend::GetLabelVisibility
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

int
avtVariableLegend::GetLabelVisibility() const
{
    return labelVisibility;
}

// ****************************************************************************
// Method: avtVariableLegend::SetMinMaxVisibility
//
// Purpose: 
//   Sets whether the min/max is visible.
//
// Arguments:
//   val : True if min/max is to be visible.
//
// Programmer: Hank Childs
// Creation:   January 23, 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtVariableLegend::SetMinMaxVisibility(bool val)
{
    minmaxVisibility = val;
    sBar->SetRangeVisibility(minmaxVisibility && rangeVisibility);
}

// ****************************************************************************
// Method: avtVariableLegend::GetMinMaxVisibility
//
// Purpose: 
//   Returns whether min/max is visible.
//
// Returns:    Whether min/max is visible.
//
// Programmer: Hank Childs
// Creation:   January 23, 2009
//
// Modifications:
//   
// ****************************************************************************

bool
avtVariableLegend::GetMinMaxVisibility() const
{
    return minmaxVisibility;
}

// ****************************************************************************
// Method: avtVariableLegend::SetNumberFormat
//
// Purpose: 
//   Sets the number format string.
//
// Arguments:
//   fmt : The new format string.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:32:10 PST 2007
//
// Modifications:
//   
//    Hank Childs, Fri Jan 23 16:25:24 PST 2009
//    Re-enable usage of number format for range string.
//
// ****************************************************************************

void
avtVariableLegend::SetNumberFormat(const char *fmt)
{
    // Set the label format.
    sBar->SetLabelFormat(fmt);

    // Use the format in the min/max range label.
    char rangeFormat[200];
    snprintf(rangeFormat, 200, "Max: %s\nMin: %s", fmt, fmt);
    sBar->SetRangeFormat(rangeFormat);
}

// ****************************************************************************
// Method: avtVariableLegend::SetLegendScale
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
avtVariableLegend::SetLegendScale(double xScale, double yScale)
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
// Method: avtVariableLegend::SetBoundingBoxVisibility
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
avtVariableLegend::SetBoundingBoxVisibility(bool val)
{
    sBar->SetBoundingBoxVisibility(val?1:0);
}

// ****************************************************************************
// Method: avtVariableLegend::SetBoundingBoxColor
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
avtVariableLegend::SetBoundingBoxColor(const double *color)
{
    sBar->SetBoundingBoxColor((double*)color);
}

// ****************************************************************************
// Method: avtVariableLegend::SetOrientation
//
// Purpose: 
//   Set the orientation of the legend -- allowing for horizontal, vertical, ...
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:34:59 PST 2007
//
// Modifications:
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Filled in this stub method.
//   
// ****************************************************************************

void
avtVariableLegend::SetOrientation(LegendOrientation l)
{
    avtLegend::SetOrientation(l);
    sBar->SetOrientation((vtkVisItScalarBarActor::BarOrientation)(int)l);
}

// ****************************************************************************
// Method: avtVariableLegend::SetFont
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
avtVariableLegend::SetFont(int family, bool bold, bool italic, bool shadow)
{
    sBar->SetFontFamily(family);
    sBar->SetBold(bold?1:0);
    sBar->SetItalic(italic?1:0);
    sBar->SetShadow(shadow?1:0);
}

// ****************************************************************************
//  Method: avtVariableLegend::SetColorBarVisibility
//
//  Purpose:
//      Turns on/off the visibility of the color bar.
//
//  Arguments:
//      val     The new value (On 1, Off 0).
//
//  Programmer: Eric Brugger
//  Creation:   July 15, 2003
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 08:20:29 PDT 2003
//    Add code to track the color bar visibility.
//
// ****************************************************************************

void
avtVariableLegend::SetColorBarVisibility(const bool val)
{
    barVisibility = val;
    sBar->SetColorBarVisibility(val);
}


// ****************************************************************************
//  Method: avtVariableLegend::SetRange
//
//  Purpose:
//      Sets the range of the scalar bars.
//
//  Arguments:
//      nmin    The new minimum.
//      nmax    The new maximum.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Jul 23 17:44:34 PDT 2001
//    Added test for constant range so that labels, title message can 
//    get set appropriately.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001 
//    Removed unnecessary call to lut->Build(). 
//    
//    Kathleen Bonnell, Mon Dec 17 16:46:31 PST 2001 
//    Set number of labels in each if-statement, in case this
//    method is called multiple times with different parameters. 
//    
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002  
//    Set the sBar's Range. 
//    
//    Kathleen Bonnell, Mon Jul  1 15:20:03 PDT 2002 
//    Set the sBar's Range, even if the range is constant. 
//    
//    Kathleen Bonnell, Wed Mar 19 14:31:42 PST 200
//    For constant or invalid range, set number of labels to zero. 
//
//    Hank Childs, Mon Jul 14 09:54:14 PDT 2003
//    Do not assume that the lut is non-NULL. ['3494]
//    
//    Kathleen Bonnell, Thu Oct  1 14:27:34 PDT 2009
//    Use numTicks to set NumberOfLabels instead of setting to default.
//
//    Alister Maguire, Wed Jan 16 13:54:14 PST 2019
//    Tell the lookup table to grey out nan values. 
//
//    Kathleen Biagas, Tue Sep 19, 2023
//    Call 'SetMessage(NULL)' when not constant.
//
// ****************************************************************************

void
avtVariableLegend::SetRange(double nmin, double nmax)
{
    min = nmin;
    max = nmax;

    if (min == max)
    {
        //
        //  Set a message and don't build labels.
        //
        SetMessage("Constant");
        sBar->SetNumberOfLabels(0);
        sBar->SetRange(min, max);
    }
    else if (min == FLT_MAX || max == -FLT_MAX )
    {
        SetMessage(NULL);
        debug5 << "avtVariableLegend did not get valid range." << endl;
        //
        //  We didn't get good values for the range. 
        //  Don't label, don't set range of lut. 
        sBar->SetNumberOfLabels(0);
    }
    else 
    {
        SetMessage(NULL);
        sBar->SetNumberOfLabels(numTicks);
        if (lut != NULL)
        {
            lut->SetTableRange(min, max);
            lut->SetNanColor(nanColor);
        }
        sBar->SetRange(min, max);
    }
}


// ****************************************************************************
//  Method: avtVariableLegend::GetRange
//
//  Purpose:
//      Get the variable range.
//
//  Arguments:
//      amin    reference that will contain the minimum after the call.
//      amax    reference that will contain the minimum after the call.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2001
//
// ****************************************************************************

void
avtVariableLegend::GetRange(double &amin, double &amax)
{
    amin = min;
    amax = max;
}


// ****************************************************************************
//  Method: avtVariableLegend::SetScaling
//
//  Purpose:
//    Sets the scaling mode of the legend.
//
//  Arguments
//    mode   0 = Linear, 1 = Log, 2 = Skew;
//    skew   The Skew factor to use for skew scaling. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 30, 2001
//
// ****************************************************************************

void
avtVariableLegend::SetScaling(int mode, double skew)
{
    switch (mode)
    {
        case 0 : sBar->LogScalingOff();
                 sBar->SkewScalingOff();
                 break;
        case 1 : sBar->LogScalingOn();
                 break;
        case 2 : sBar->SkewScalingOn();
                 sBar->SetSkewFactor(skew);
                 break;
    } 
}


// ****************************************************************************
//  Method: avtVariableLegend::SetLookupTable
//
//  Purpose: 
//    Specifies the lookuptable this legend should use. Sets the
//    scalarbar actor to use the same lut.
//
//  Arguments:
//    LUT       The lookup table.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 28, 2001 
//
// ****************************************************************************

void
avtVariableLegend::SetLookupTable(vtkLookupTable *LUT)
{
    if (lut == LUT)
    {
        // no need to make changes
        return;
    }

    lut = LUT;
    sBar->SetLookupTable(lut);
}


// ****************************************************************************
//  Method: avtVariableLegend::SetVarRangeVisibility
//
//  Purpose:
//      Turns on/off the visibility of the variable range.
//
//  Arguments:
//      val     The new value (On 1, Off 0).
//
//  Programmer: Eric Brugger
//  Creation:   July 15, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Jan 23 15:53:59 PST 2009
//    Incorporate user request through minmax visibility.
//
// ****************************************************************************

void
avtVariableLegend::SetVarRangeVisibility(const bool val )
{
    rangeVisibility = val;
    sBar->SetRangeVisibility(rangeVisibility && minmaxVisibility);
}


// ****************************************************************************
//  Method: avtVariableLegend::SetVarRange 
//
//  Purpose:
//      Sets the range of the var used in limit text.
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
avtVariableLegend::SetVarRange(double nmin, double nmax)
{
    sBar->SetVarRange(nmin, nmax);
}


// ****************************************************************************
//  Method: avtVariableLegend::ChangePosition
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
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Mar 21 20:20:40 PST 2007
//    I made it set position2.
//
// ****************************************************************************

void
avtVariableLegend::ChangePosition(double x, double y)
{
    sBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    sBar->GetPositionCoordinate()->SetValue(x, y, 0.);

    // Set the position 2, incorporating the scale.
    double tmp = 0.0, w, h;
    GetLegendSize(tmp, w, h);
    sBar->SetPosition2(w, h);    
}


// ****************************************************************************
//  Method: avtVariableLegend::ChangeTitle
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
avtVariableLegend::ChangeTitle(const char *t)
{
    sBar->SetTitle(t);
}


// ****************************************************************************
//  Method: avtVariableLegend::ChangeFontHeight
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
avtVariableLegend::ChangeFontHeight(double fh)
{
    double minScale = (scale[0] < scale[1]) ? scale[0] : scale[1];
    sBar->SetFontHeight(fh * minScale);
}


// ****************************************************************************
// Method: avtVariableLegend::SetNumTicks
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//    Kathleen Bonnell, Tue Oct  6 11:07:53 PDT 2009
//    Handle constant-legends.
//
// ****************************************************************************

void                          
avtVariableLegend::SetNumTicks(int nt)
{
    numTicks = nt;
    if (min != max)
        sBar->SetNumberOfLabels(numTicks);
    else 
        sBar->SetNumberOfLabels(0);
}


// ****************************************************************************
// Method: avtVariableLegend::SetUseSuppliedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtVariableLegend::SetUseSuppliedLabels(bool val)
{
    sBar->SetUseSuppliedLabels(val);
}


// ****************************************************************************
// Method: avtVariableLegend::GetUseSuppliedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
avtVariableLegend::GetUseSuppliedLabels()
{
    return sBar->GetUseSuppliedLabels();
}


// ****************************************************************************
// Method: avtVariableLegend::SetMinMaxInclusive
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtVariableLegend::SetMinMaxInclusive(bool val)
{
    sBar->SetMinMaxInclusive(val);
}


// ****************************************************************************
// Method: avtVariableLegend::SetSuppliedValues
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtVariableLegend::SetSuppliedValues(const doubleVector &v)
{
    sBar->SetSuppliedValues(v);
}


// ****************************************************************************
// Method: avtVariableLegend::SetSuppliedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtVariableLegend::SetSuppliedLabels(const stringVector &l)
{
    sBar->SetSuppliedLabels(l);
}


// ****************************************************************************
// Method: avtVariableLegend::GetCalculatedLabels
//
// Programmer: Kathleen Bonnell
// Creation:   Wed Sep 16 13:25:54 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtVariableLegend::GetCalculatedLabels(doubleVector &v)
{
    sBar->GetCalculatedValues(v);
}


// ****************************************************************************
// Method: avtVariableLegend::SetBelowRangeColor
//
// Programmer: Kathleen Biagas
// Creation:   December 26, 2018
//
// Modifications:
//   Kathleen Biagas, Mon July 18, 2022
//   Test for lut before setting.
//
// ****************************************************************************

void
avtVariableLegend::SetBelowRangeColor(double r, double g, double b, double a)
{
    if(lut)
        lut->SetBelowRangeColor(r,g,b,a);
}


// ****************************************************************************
// Method: avtVariableLegend::SetAboveRangeColor
//
// Programmer: Kathleen Biagas
// Creation:   December 26, 2018
//
// Modifications:
//   Kathleen Biagas, Mon July 18, 2022
//   Test for lut before setting.
//
// ****************************************************************************

void
avtVariableLegend::SetAboveRangeColor(double r, double g, double b, double a)
{
    if (lut)
        lut->SetAboveRangeColor(r,g,b,a);
}


// ****************************************************************************
// Method: avtVariableLegend::UseBelowRangeColor
//
// Programmer: Kathleen Biagas
// Creation:   December 26, 2018
//
// Modifications:
//   Kathleen Biagas, Mon July 18, 2022
//   Test for lut before setting.
//
// ****************************************************************************

void
avtVariableLegend::UseBelowRangeColor(bool v)
{
    if(lut)
        lut->SetUseBelowRangeColor(v);
}


// ****************************************************************************
// Method: avtVariableLegend::UseAboveRangeColor
//
// Programmer: Kathleen Biagas
// Creation:   December 26, 2018
//
// Modifications:
//   Kathleen Biagas, Mon July 18, 2022
//   Test for lut before setting.
//
// ****************************************************************************

void
avtVariableLegend::UseAboveRangeColor(bool v)
{
    if(lut)
        lut->SetUseAboveRangeColor(v);
}


// ****************************************************************************
// Method: avtVariableLegend::SetNanColor
//
// Programmer: Alister Maguire
// Creation:   January 23, 2019
//
// Modifications:
//   Kathleen Biagas, Mon July 18, 2022
//   Test for lut before setting.
//
// ****************************************************************************

void
avtVariableLegend::SetNanColor(double r, double g, double b, double a)
{
    nanColor[0] = r;
    nanColor[1] = g;
    nanColor[2] = b;
    nanColor[3] = a;
    if (lut)
        lut->SetNanColor(nanColor);
}
