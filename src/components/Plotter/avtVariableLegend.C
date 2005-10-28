// ************************************************************************* //
//                             avtVariableLegend.C                           //
// ************************************************************************* //

#include <vtkLookupTable.h>
#include <vtkVerticalScalarBarActor.h>

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
//    Changed sBar to be of type vtkVerticalScalarBarActor, a modified
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
// ****************************************************************************

avtVariableLegend::avtVariableLegend()
{
    min = 0.;
    max = 1.;

    lut = NULL;
 
    sBar = vtkVerticalScalarBarActor::New();
    sBar->SetShadow(0);

    size[0] = 0.08;
    size[1] = 0.26;
    sBar->SetPosition2(size[0], size[1]);

    SetLegendPosition(0.05, 0.72);

    barVisibility = 1;
    rangeVisibility = 1;

    //
    // Set the legend to also point to sBar, so the base methods will work
    // without us re-defining them.
    //
    legend = sBar;
    legend->Register(NULL);
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
// ****************************************************************************

avtVariableLegend::avtVariableLegend(int)
{
    min = 0.;
    max = 1.;
    lut = NULL;
    sBar = NULL;
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
// ****************************************************************************

void
avtVariableLegend::GetLegendSize(float, float &w, float &h)
{
    w = 0.08;

    if (barVisibility)
    {
        h = 0.26;
    }
    else
    {
        float nLines = 0.51;

        if (title != NULL)        nLines += 1.0;
        if (databaseInfo != NULL) nLines += 2.0;
        if (varName != NULL)      nLines += 1.0;
        if (varUnits != NULL)     nLines += 1.0;
        if (message != NULL)      nLines += 1.0;
        if (rangeVisibility)      nLines += 2.5;

        h = nLines * fontHeight;

        sBar->SetPosition2(w, h);
    }
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
avtVariableLegend::SetColorBarVisibility(const int val)
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
// ****************************************************************************

void
avtVariableLegend::SetRange(float nmin, float nmax)
{
    min = nmin;
    max = nmax;

    if (min == max)
    {
        //
        //  Set a message and don't build labels.
        //
        SetMessage("Constant.");
        sBar->SetNumberOfLabels(0);
        sBar->SetRange(min, max);
    }
    else if (min == FLT_MAX || max == -FLT_MAX )
    {
        debug5 << "avtVariableLegend did not get valid range." << endl;
        //
        //  We didn't get good values for the range. 
        //  Don't label, don't set range of lut. 
        sBar->SetNumberOfLabels(0);
    }
    else 
    {
        sBar->SetNumberOfLabelsToDefault();
        if (lut != NULL)
            lut->SetTableRange(min, max);
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
avtVariableLegend::GetRange(float &amin, float &amax)
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
avtVariableLegend::SetScaling(int mode, float skew)
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
// ****************************************************************************

void
avtVariableLegend::SetVarRangeVisibility(const int val )
{
    rangeVisibility = val;
    sBar->SetRangeVisibility(val);
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
avtVariableLegend::SetVarRange(float nmin, float nmax)
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
// ****************************************************************************

void
avtVariableLegend::ChangePosition(float x, float y)
{
    sBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    sBar->GetPositionCoordinate()->SetValue(x, y, 0.);
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
//
// ****************************************************************************

void
avtVariableLegend::ChangeFontHeight(float fh)
{
    sBar->SetFontHeight(fh);
}


