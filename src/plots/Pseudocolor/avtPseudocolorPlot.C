// ************************************************************************* //
//                           avtPseudocolorPlot.C                            //
// ************************************************************************* //

#include <avtPseudocolorPlot.h>

#include <PseudocolorAttributes.h>

#include <avtExtents.h>
#include <avtLookupTable.h>
#include <avtPointToGlyphFilter.h>
#include <avtShiftCenteringFilter.h>
#include <avtVariableLegend.h>
#include <avtVariableMapper.h>

#include <DebugStream.h>
#include <InvalidLimitsException.h>


// ****************************************************************************
//  Method: avtPseudocolorPlot constructor
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001
//    Initialize atts, set the title for the legend.
//
//    Kathleen Bonnell, Tue Apr 24 12:22:01 PDT 2001 
//    Initialize filter.
// 
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Removed previousMode.
// 
//    Hank Childs, Sun Jun 23 12:22:58 PDT 2002
//    Initialize glyphPoints.
//
//    Eric Brugger, Wed Jul 16 11:09:02 PDT 2003
//    Modified to work with the new way legends are managed.
//
// ****************************************************************************

avtPseudocolorPlot::avtPseudocolorPlot()
{
    varMapper = new avtVariableMapper;
    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Pseudocolor");

    colorsInitialized = false;

    avtLUT  = new avtLookupTable;

    filter = NULL;
    glyphPoints = new avtPointToGlyphFilter;

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;
}


// ****************************************************************************
//  Method: avtPseudocolorPlot destructor
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 24 12:22:01 PDT 2001 
//    Destruct filter.
// 
//    Hank Childs, Sun Jun 23 12:22:58 PDT 2002
//    Destruct glyphPoints.
//
// ****************************************************************************

avtPseudocolorPlot::~avtPseudocolorPlot()
{
    if (filter != NULL)
    {
        delete filter;
        filter = NULL;
    }

    if (glyphPoints != NULL)
    {
        delete glyphPoints;
        glyphPoints = NULL;
    }

    if (varMapper != NULL)
    {
        delete varMapper;
        varMapper = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}


// ****************************************************************************
//  Method:  avtPseudocolorPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************

avtPlot*
avtPseudocolorPlot::Create()
{
    return new avtPseudocolorPlot;
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

avtMapper *
avtPseudocolorPlot::GetMapper(void)
{
    return varMapper;
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a pseudo color plot -- that is
//      none.  It is still unclear to me if a facelist should occur here or
//      if a triangle stripper should occur.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the pseudo color plot has been applied.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 24 12:22:01 PDT 2001
//    Added avtShiftCenteringFilter.
//
//    Hank Childs, Fri Jun 15 09:37:30 PDT 2001
//    Use more general data object instead of derived types.
//
//    Hank Childs, Fri Feb  8 18:33:39 PST 2002
//    Removed references to PC atts in the shift centering filter.
//
//    Hank Childs, Sun Jun 23 12:42:22 PDT 2002
//    Add the point to glyph filter if needed.
//
//    Kathleen Bonnell, Mon Jun 24 15:09:37 PDT 2002
//    Only use shift-centering filter if needed, fix potential
//    memory leak.
//
//    Kathleen Bonnell, Tue Oct 22 08:41:29 PDT 2002  
//    Moved the glyph filter to ApplyRenderingTransformtion, so that the 
//    output of this method (used as query input), would still have topo dim
//    of 0 for point meshes, and thus be possibly unquery-able. 
//
//    Hank Childs, Wed Feb 25 15:48:23 PST 2004
//    Do not consider the centering of the current dataset if we are asked to
//    shift the centering.
//
// ****************************************************************************

avtDataObject_p
avtPseudocolorPlot::ApplyOperators(avtDataObject_p input)
{
    avtDataObject_p dob = input; 

    if ((atts.GetCentering() == 1) || (atts.GetCentering() == 2))
    {
        //
        // It was requested that we shift centering.  If we asked for zonal
        // data and the data is already zonal, then this will effectively
        // be a no-op.
        //
        if (filter != NULL)
        {
            delete filter;
        }
        filter = new avtShiftCenteringFilter(atts.GetCentering());
        filter->SetInput(dob);
        dob = filter->GetOutput();
    }

    return dob;
}

// ****************************************************************************
//  Method: avtPseudocolorPlot::ApplyRenderingTransormation
//
//  Purpose:
//      Applies the rendering transformation associated with a pseudo color 
//      plot.  Namely, a glyph filter for point meshes. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the glyph points has (possibly) 
//              been applied.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
//  Modifications:
//
//    Jeremy Meredith, Fri Dec 20 11:36:03 PST 2002
//    Added scaling of point variables by a scalar field.
//
//    Hank Childs, Thu Aug 21 22:05:14 PDT 2003
//    Added support for different types of point glyphs.
//
// ****************************************************************************

avtDataObject_p
avtPseudocolorPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    avtDataObject_p dob = input; 

    if (dob->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        glyphPoints->SetPointSize(atts.GetPointSize());
        glyphPoints->SetScaleByVariableEnabled(atts.GetPointSizeVarEnabled());
        if (atts.GetPointSizeVarEnabled())
        {
            glyphPoints->SetScaleVariable(atts.GetPointSizeVar());
        }
        else
        {
            glyphPoints->SetScaleVariable("default");
        }
        glyphPoints->SetInput(dob);
        glyphPoints->SetGlyphType((int) atts.GetPointType());
        dob = glyphPoints->GetOutput();
    }

    return dob;
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a pseudo color plot.  Right
//      now this only means adding a legend.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 16:59:34 PST 2001
//    Add shift factor.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Add error checking for Log scale, added calls to retrieve the 
//    variable's range and set it in the legend. (Controls display of
//    limit text, even if artificial limits have been set.)
//
//    Kathleen Bonnell, Thu Apr  5 11:29:33 PDT 2001 
//    Set render order when opacity < 1. 
//    
//    Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001 
//    Reflect change in InvalidLimitsException signature. 
//    
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Kathleen Bonnell, Thu Aug 30 16:45:39 PDT 2001 
//    Use avtLUT to set the legend's lut.  Added call to SetScaling
//    before throwing exception.
//
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002
//    Move code that sets legend range to new method.
//
//    Kathleen Bonnell, Mon Sep 29 13:07:50 PDT 2003 
//    Set AntialiasedRenderOrder. 
//
//    Kathleen Bonnell, Wed Apr 28 17:37:47 PDT 2004 
//    Chagned 'SetLegendRanges' to 'SetLimitsMode', as the legend ranges are
//    dependent upon values set in SetLimitsMode. 
//
// ****************************************************************************

void
avtPseudocolorPlot::CustomizeBehavior()
{
    SetLimitsMode(atts.GetLimitsMode());

    if (atts.GetOpacity() < 1.)
    {
       behavior->SetRenderOrder(MUST_GO_LAST);
       behavior->SetAntialiasedRenderOrder(MUST_GO_LAST);
    }
    else
    {
       behavior->SetRenderOrder(DOES_NOT_MATTER);
       behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
    }

    behavior->SetLegend(varLegendRefPtr);
    behavior->SetShiftFactor(0.);
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::SetAtts
//
//  Purpose:
//      Sets the atts for the pseudocolor plot.
//
//  Arguments:
//      atts    The attributes for this pseudocolor plot.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made this method take a generic AttributeGroup since it is now virtual.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Made atts a member. Added call to SetScaling method.
//
//    Jeremy Meredith, Tue Jun  5 20:33:13 PDT 2001
//    Added code to set a flag if the plot needs recalculation.  Also
//    store the attributes as a class member.
//
//    Brad Whitlock, Thu Jun 14 16:58:48 PST 2001
//    I added code to update the color tables if necessary.
//
//    Jeremy Meredith, Thu Aug 23 14:11:40 PDT 2001
//    Made it use the color table name "Default" instead of the boolean flag.
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//    Moved Min/Max code to new method SetLimitsMode. 
//
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002 
//    varLegends requires SetScaling come before SetLimitsMode. 
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Removed previousMode.
// 
// ****************************************************************************

void
avtPseudocolorPlot::SetAtts(const AttributeGroup *a)
{
    const PseudocolorAttributes *newAtts = (const PseudocolorAttributes *)a;

    // See if the colors will need to be updated.
    bool updateColors = (!colorsInitialized) ||
         (atts.GetColorTableName() != newAtts->GetColorTableName()); 

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    SetOpacity(atts.GetOpacity());
    SetLighting(atts.GetLightingFlag());
    SetLegend(atts.GetLegendFlag());

    // Update the plot's colors if needed.
    if(updateColors || atts.GetColorTableName() == "Default")
    {
        colorsInitialized = true;
        SetColorTable(atts.GetColorTableName().c_str());
    }

    SetScaling(atts.GetScaling(), atts.GetSkewFactor());
    SetLimitsMode(atts.GetLimitsMode());
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::GetDataExtents
//
//  Purpose:
//      Gets the data extents used by the plot.
//
//  Arguments:
//      extents The extents used by the plot.
//
//  Programmer: Eric Brugger
//  Creation:   March 25, 2004
//
// ****************************************************************************

void
avtPseudocolorPlot::GetDataExtents(vector<double> &extents)
{
    float min, max;

    varLegend->GetRange(min, max);

    extents.push_back(min);
    extents.push_back(max);
}


// ****************************************************************************
// Method: avtPseudocolorPlot::SetColorTable
//
// Purpose: 
//   Sets the plot's color table if the color table is the same as that of
//   the plot or we are using the default color table for the plot.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    Returns true if the color table is updated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:52:49 PST 2001
//
// Modifications:
//
//    Jeremy Meredith, Thu Aug 23 14:11:40 PDT 2001
//    Made it use the color table name "Default" instead of the boolean flag.
//
//    Kathleen Bonnell, Thu Aug 30 16:10:15 PDT 2001 
//    Main functionality now resides in avtLookupTable::SetColorTable. 
//
//    Kathleen Bonnell, Fri Aug 31 15:21:45 PDT 2001 
//    Added logic (that existed here before I moved functionality to
//    avtLookupTable) to ensure that changes to active color in color table
//    window will have effect if this plot's color table is "Default". 
//
// ****************************************************************************

bool
avtPseudocolorPlot::SetColorTable(const char *ctName)
{
    bool namesMatch = (atts.GetColorTableName() == std::string(ctName));

    if (atts.GetColorTableName() == "Default")
        return avtLUT->SetColorTable(NULL, namesMatch); 
    else
        return avtLUT->SetColorTable(ctName, namesMatch);
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

void
avtPseudocolorPlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        varLegend->LegendOn();
    }
    else
    {
        varLegend->LegendOff();
    }
}

// ****************************************************************************
//  Method: avtPseudocolorPlot::SetScaling
//
//  Purpose:
//    Set up mapper and legend to perform needed scaling 
//
//  Arguments:
//    mode   Which scaling mode to use: 0 = Linear, 1 = Log, 2 = Skew
//    skew   The skew factor to use in the case of skew mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 30, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug 30 14:35:32 PDT 2001
//    Set mapper's lut via one retrieved from avtLUT.
//
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002    
//    Set legend's lut and scaling. 
//
// ****************************************************************************

void
avtPseudocolorPlot::SetScaling(int mode, double skew)
{
    varLegend->SetLookupTable(avtLUT->GetLookupTable());
    varLegend->SetScaling(mode, skew);

    if (mode == 1)
    {
       varMapper->SetLookupTable(avtLUT->GetLogLookupTable());
    }
    else if (mode == 2)
    {
       avtLUT->SetSkewFactor(skew);
       varMapper->SetLookupTable(avtLUT->GetSkewLookupTable());
    }
    else 
    {
       varMapper->SetLookupTable(avtLUT->GetLookupTable());
    }
}



// ****************************************************************************
//  Method: avtPseudocolorPlot::SetLighting
//
//  Purpose:
//      Turns the lighting on or off.
//
//  Arguments:
//      lightingOn   true if the lighting should be turned on, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

void
avtPseudocolorPlot::SetLighting(bool lightingOn)
{
    if (lightingOn)
    {
        varMapper->TurnLightingOn();
    }
    else
    {
        varMapper->TurnLightingOff();
    }
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::SetLimitsMode
//
//  Purpose:  To determine the proper limits the mapper should be using.
//
//  Arguments:
//    limitsMode  Specifies which type of limits.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     October 4, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Nov 7 16:36:37 PST 2001
//    Made it use the generated name for the user-specified limits mode.
//
//    Kathleen Bonnell, Tue Dec 18 08:58:09 PST 2001
//    Added code to set the legend range from the mapper's range. 
//
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002    
//    Use new method to set legend range. 
//
//    Kathleen Bonnell, Thu Mar 28 08:21:21 PST 2002 
//    Handle cases where min > max. 
//
//    Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002  
//    Always allow user to specify Min/Max. 
// 
//    Eric Brugger, Thu Mar 25 17:12:04 PST 2004
//    I added code to use the data extents from the base class if set.
//
// ****************************************************************************

void
avtPseudocolorPlot::SetLimitsMode(int limitsMode)
{
    float min, max;
    //
    //  Retrieve the actual range of the data
    //
    varMapper->GetVarRange(min, max);

    float userMin = atts.GetMinFlag() ? atts.GetMin() : min;
    float userMax = atts.GetMaxFlag() ? atts.GetMax() : max;
      
    if (dataExtents.size() == 2)
    {
        varMapper->SetMin(dataExtents[0]);
        varMapper->SetMax(dataExtents[1]);
    }
    else if (atts.GetMinFlag() && atts.GetMaxFlag())
    {
        if (userMin >= userMax)
        {
            EXCEPTION1(InvalidLimitsException, false); 
        }
        else
        {
            varMapper->SetMin(userMin);
            varMapper->SetMax(userMax);
        }
    } 
    else if (atts.GetMinFlag())
    {
        varMapper->SetMin(userMin);
        if (userMin > userMax)
        {
            varMapper->SetMax(userMin);
        }
        else
        {
            varMapper->SetMaxOff();
        }
    }
    else if (atts.GetMaxFlag())
    {
        varMapper->SetMax(userMax);
        if (userMin > userMax)
        {
            varMapper->SetMin(userMax);
        }
        else
        {
            varMapper->SetMinOff();
        }
    }
    else
    {
        varMapper->SetMinOff();
        varMapper->SetMaxOff();
    }
    varMapper->SetLimitsMode(limitsMode);

    SetLegendRanges();
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::SetOpacity
//
//  Purpose:
//      Allows for plots to be non-opaque.
//
//  Arguments:
//      opacity     The new opacity.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 29 13:07:50 PDT 2003 
//    Set AntialiasedRenderOrder. 
//
// ****************************************************************************

void
avtPseudocolorPlot::SetOpacity(float opacity)
{
    varMapper->SetOpacity(opacity);
    if (opacity < 1.)
    {
       behavior->SetRenderOrder(MUST_GO_LAST);
       behavior->SetAntialiasedRenderOrder(MUST_GO_LAST);
    }
    else
    {
       behavior->SetRenderOrder(DOES_NOT_MATTER);
       behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
    }
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::SetLegendRanges
//
//  Purpose:
//    Sets the legend ranges (color bar labels and limits text). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2002 
//
//  Modifications
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Removed call to SetScaling before throwing exception.
// 
//    Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002  
//    Always allow user to specify Min/Max. 
// 
// ****************************************************************************

void
avtPseudocolorPlot::SetLegendRanges()
{
    float min, max;

    //
    // set and get the range for the legend's color bar labels 
    //
    bool validRange = false;
    if (atts.GetLimitsMode() == PseudocolorAttributes::OriginalData)
    {
        validRange = varMapper->GetRange(min, max);
    }
    else
    {
        validRange = varMapper->GetCurrentRange(min, max);
    }

    varLegend->SetRange(min, max);

    // 
    // Perform error checking if log scaling is to be used.
    // 
    if (atts.GetScaling() == PseudocolorAttributes::Log &&
       ( min <= 0. || max <= 0. ) && validRange == true)
    {
        EXCEPTION1(InvalidLimitsException, true); 
    }
 
    varLegend->SetScaling(atts.GetScaling(), atts.GetSkewFactor());

    //
    // set and get the range for the legend's limits text
    //
    varMapper->GetVarRange(min, max);
    varLegend->SetVarRange(min, max);
}


// ****************************************************************************
//  Method: avtPseudocolorPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
// ****************************************************************************
 
void
avtPseudocolorPlot::ReleaseData(void)
{
    avtSurfaceDataPlot::ReleaseData();
 
    if (filter != NULL)
    {
        filter->ReleaseData();
    }
    if (glyphPoints != NULL)
    {
        glyphPoints->ReleaseData();
    }
}


// ****************************************************************************
//  Method:  avtPseudocolorPlot::GetSmoothingLevel
//
//  Purpose:
//    Returns the amount of smoothing needed inside the avtPlot.
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2002
//
// ****************************************************************************

int
avtPseudocolorPlot::GetSmoothingLevel()
{
    return atts.GetSmoothingLevel();
}
