// ************************************************************************* //
//                             avtLevelsLegend.C                             //
// ************************************************************************* //

#include <vtkLookupTable.h>
#include <vtkVerticalScalarBarActor.h>

#include <avtLevelsLegend.h>


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
// ****************************************************************************

avtLevelsLegend::avtLevelsLegend()
{
    min = 0.;
    max = 1.;

    lut = NULL;

    sBar = vtkVerticalScalarBarActor::New();
    sBar->SetShadow(0);
    sBar->UseDefinedLabelsOn();

    size[0] = 0.08;
    size[1] = 0.17;
    sBar->SetWidth(size[0]);
    sBar->SetHeight(size[1]);

    SetLegendPosition(0.1, 0.7);

    //
    // Set the legend to also point to sBar, so the base methods will work
    // without us re-defining them.
    //
    legend = sBar;
    legend->Register(NULL);
    varName = NULL;
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
    if (varName != NULL)
    {
        delete [] varName;
        varName = NULL;
    }
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetColorBar
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
avtLevelsLegend::SetColorBar(const int val)
{
    sBar->SetColorBarVisibility(val);
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
// ****************************************************************************

void
avtLevelsLegend::SetLevels(const std::vector<double> &levels)
{
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
// ****************************************************************************

void
avtLevelsLegend::SetLevels(const std::vector<std::string> &levels)
{
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
//  Method: avtLevelsLegend::SetMessage
//
//  Purpose:
//      Appends a message to the title of this legend.
//
//  Arguments:
//      msg    The message. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
// ****************************************************************************

void
avtLevelsLegend::SetMessage(const char *msg)
{
    char *title = sBar->GetTitle();
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
avtLevelsLegend::SetVarRange(float nmin, float nmax)
{
    sBar->SetVarRange(nmin, nmax);
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
avtLevelsLegend::SetRange(float nmin, float nmax)
{
    min = nmin;
    max = nmax;
    sBar->SetRange(min, max);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetRange
//
//  Purpose:
//      Turns on/off the visibility of the range. 
//
//  Arguments:
//      val     The new value (On 1, Off 0).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
// ****************************************************************************

void
avtLevelsLegend::SetRange(const int val )
{
    sBar->SetRangeVisibility(val);
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetTitle
//
//  Purpose:
//      Sets the title for the legend. 
//
//  Arguments:
//      title    The title. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002 
//    Add the varName to the title, if it has already been set.
//
// ****************************************************************************

void
avtLevelsLegend::SetTitle(const char *title)
{
    sBar->SetTitle(title);
    if (varName != NULL)
    {
        char *subtitle = "\nVar:  ";
        int size = strlen(title) + strlen(subtitle) + strlen(varName) + 1;
        char *vartitle = new char [size];
        strcpy(vartitle, title);
        strcat(vartitle, subtitle);
        strcat(vartitle, varName);
        sBar->SetTitle(vartitle);
        delete [] vartitle;
    }
}


// ****************************************************************************
//  Method: avtLevelsLegend::SetVarName
//
//  Purpose:
//      Appends the variable name to the title of this legend.
//
//  Arguments:
//      name    The variable name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002 
//    Store the var name for future reference.
//
// ****************************************************************************

void
avtLevelsLegend::SetVarName(const char *name)
{
    char *title = sBar->GetTitle();
    char *subtitle = "\nVar:  ";
    int size = strlen(title) + strlen(subtitle) + strlen(name) + 1;
    if (varName != NULL)
        delete [] varName;
    varName = new char [strlen(name) + 1];
    strcpy(varName, name);

    char *vartitle = new char [size];
    strcpy(vartitle, title);
    strcat(vartitle, subtitle);
    strcat(vartitle, name);
    sBar->SetTitle(vartitle);
    delete [] vartitle;
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
// ****************************************************************************

void
avtLevelsLegend::ChangePosition(float x, float y)
{
    sBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    sBar->GetPositionCoordinate()->SetValue(x, y, 0.);
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

