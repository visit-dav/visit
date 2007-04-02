// ************************************************************************* //
//                             avtLevelsLegend.C                             //
// ************************************************************************* //

#include <avtLevelsLegend.h>

#include <math.h>

#include <vtkLookupTable.h>
#include <vtkVerticalScalarBarActor.h>

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
// ****************************************************************************

avtLevelsLegend::avtLevelsLegend()
{
    min = 0.;
    max = 1.;

    nLevels = 0;

    lut = NULL;

    sBar = vtkVerticalScalarBarActor::New();
    sBar->SetShadow(0);
    sBar->UseDefinedLabelsOn();

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
// ****************************************************************************

void
avtLevelsLegend::GetLegendSize(float maxSize, float &w, float &h)
{
    w = 0.08;

    //
    // The fudge factor added to nLines when barVisibility is true is
    // there to make sure that there is enough room for all the levels
    // because this algorithm doesn't exactly match the algorithm in
    // vtkVerticalScalarBarActor.
    //
    float fudge = 0.5;
    float nLines = 0.0;
    if (title != NULL)        nLines += 1.0;
    if (databaseInfo != NULL) nLines += 2.0;
    if (varName != NULL)      nLines += 1.0;
    if (message != NULL)      nLines += 1.0;
    if (rangeVisibility)      nLines += 2.0;
    if (barVisibility)        nLines += nLevels * 1.1 + 1.0 + fudge;

    h = nLines * fontHeight;

    //
    // If the legend is larger than the maximum size, then try to shrink
    // the color bar so that it fits within the maximum size.
    //
    if (h > maxSize && barVisibility)
    {
        float hTitles = h - (nLevels * 1.1 + 1.0 + fudge) * fontHeight;
        float nLevelsSpace = (maxSize - hTitles) / (fontHeight * 1.1);
        if (nLevelsSpace > 2.)
        {
            h = maxSize;
        }
    }

    sBar->SetPosition2(w, h);
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
avtLevelsLegend::SetRange(float nmin, float nmax)
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
avtLevelsLegend::SetVarRange(float nmin, float nmax)
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
// ****************************************************************************

void
avtLevelsLegend::ChangePosition(float x, float y)
{
    sBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    sBar->GetPositionCoordinate()->SetValue(x, y, 0.);
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
//
// ****************************************************************************

void
avtLevelsLegend::ChangeFontHeight(float fh)
{
    sBar->SetFontHeight(fh);
}
