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
// ****************************************************************************

avtVariableLegend::avtVariableLegend()
{
    min = 0.;
    max = 1.;

    lut = NULL;
 
    title = NULL;
    sBar = vtkVerticalScalarBarActor::New();
    sBar->SetShadow(0);
    sBar->SetTitle("Variable Plot");

    size[0] = 0.08;
    size[1] = 0.17;
    sBar->SetWidth(size[0]);
    sBar->SetHeight(size[1]);

    SetLegendPosition(0.05, 0.7);

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
// ****************************************************************************

avtVariableLegend::avtVariableLegend(int arg)
{
    min = 0.;
    max = (float)arg/arg;
    lut = NULL;
    sBar = NULL;
    title = NULL;
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
// ****************************************************************************

avtVariableLegend::~avtVariableLegend()
{
    if (sBar != NULL)
    {
        sBar->Delete();
        sBar = NULL;
    }
    if (title != NULL)
    {
        delete [] title;
        title = NULL;
    }
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
//  Method: avtVariableLegend::GetVarRange
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
avtVariableLegend::GetVarRange(float &amin, float &amax)
{
    amin = min;
    amax = max;
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
        lut->SetTableRange(min, max);
        sBar->SetRange(min, max);
    }
}


// ****************************************************************************
//  Method: avtVariableLegend::SetTitle
//
//  Purpose:
//    Sets the title for this legend. 
//
//  Arguments:
//    nTitle    The title name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 09, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 10 17:31:26 PDT 2001
//    Reflect that 'title' is now a member.
//
// ****************************************************************************

void
avtVariableLegend::SetTitle(const char *nTitle)
{
    int size = strlen(nTitle) + 1;
    if (title != NULL)
    {
        delete [] title;
    }
    title = new char[size];
    strcpy(title, nTitle);
    sBar->SetTitle(title);
}


// ****************************************************************************
//  Method: avtVariableLegend::SetVarName
//
//  Purpose:
//      Adds the variable name to the title of this plot.
//
//  Arguments:
//      name    The variable name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 11, 2000 
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 15 13:16:41 PST 2001
//    Modified to preserve original title, and append var name to end.
//
//    Kathleen Bonnell, Wed Oct 10 17:31:26 PDT 2001 
//    Append var name to new member 'title'. 
//
// ****************************************************************************

void
avtVariableLegend::SetVarName(const char *name)
{
    char *subtitle = "\nVar:  ";
    int size = strlen(title) + strlen(subtitle) + strlen(name) + 1;
    char *vartitle = new char [size];
    strcpy(vartitle, title);
    strcat(vartitle, subtitle);
    strcat(vartitle, name);
    delete [] title;
    title = new char [size];
    strcpy(title, vartitle);

    sBar->SetTitle(title);
    delete [] vartitle;
}

// ****************************************************************************
//  Method: avtVariableLegend::SetMessage
//
//  Purpose:
//      Appends a message to the title of this legend.
//
//  Arguments:
//      msg    The message.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 09, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 10 17:31:26 PDT 2001
//    Allow for NULL message.  title is now a member.
//
// ****************************************************************************

void
avtVariableLegend::SetMessage(const char *msg)
{
    if (msg == NULL)
    {
        sBar->SetTitle(title);
        return;
    }
    char *nl = "\n";
    int size = strlen(title) + strlen(nl) + strlen(msg) + 1;
    char *msgtitle = new char [size];
    strcpy(msgtitle, title);
    strcat(msgtitle, nl);
    strcat(msgtitle, msg);

    sBar->SetTitle(msgtitle);
    delete [] msgtitle;
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
//  Method: avtVariableLegend::SetColorBar
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
// ****************************************************************************

void
avtVariableLegend::SetColorBar(const int val)
{
    sBar->SetColorBarVisibility(val);
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

