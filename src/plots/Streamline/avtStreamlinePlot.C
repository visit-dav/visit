// ************************************************************************* //
//                             avtStreamlinePlot.C                           //
// ************************************************************************* //

#include <avtStreamlinePlot.h>

#include <avtShiftCenteringFilter.h>
#include <avtStreamlineFilter.h>
#include <avtVariableLegend.h>
#include <avtVariableMapper.h>
#include <avtLookupTable.h>

#include <StreamlineAttributes.h>

// ****************************************************************************
//  Method: avtStreamlinePlot constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtStreamlinePlot::avtStreamlinePlot()
{
    colorsInitialized = false;
    streamlineFilter = new avtStreamlineFilter;
    shiftCenteringFilter = NULL;

    avtLUT = new avtLookupTable; 
    varMapper = new avtVariableMapper;
    varMapper->SetLookupTable(avtLUT->GetLookupTable());

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Streamline Plot");

    //
    // This is to allow the legend to be reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;
}

// ****************************************************************************
//  Method: avtStreamlinePlot destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtStreamlinePlot::~avtStreamlinePlot()
{
    if (streamlineFilter != NULL)
    {
        delete streamlineFilter;
        streamlineFilter = NULL;
    }
    if (shiftCenteringFilter != NULL)
    {
        delete shiftCenteringFilter;
        shiftCenteringFilter = NULL;
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
//  Method:  avtStreamlinePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtPlot*
avtStreamlinePlot::Create()
{
    return new avtStreamlinePlot;
}

// ****************************************************************************
//  Method: avtStreamlinePlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtMapper *
avtStreamlinePlot::GetMapper(void)
{
    return varMapper;
}

// ****************************************************************************
//  Method: avtStreamlinePlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Streamline plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Streamline plot has been applied.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtStreamlinePlot::ApplyOperators(avtDataObject_p input)
{
    avtDataObject_p dob = input; 

    // If the variable centering is zonal, convert it to nodal or the
    // streamline filter will not play with it.
    avtCentering centering = input->GetInfo().GetAttributes().GetCentering();
    if(centering == AVT_ZONECENT)
    {
        if(shiftCenteringFilter != NULL)
            delete shiftCenteringFilter;
        shiftCenteringFilter = new avtShiftCenteringFilter(AVT_ZONECENT);
        shiftCenteringFilter->SetInput(input);
        dob = shiftCenteringFilter->GetOutput();
    }

    // Add the streamline filter.
    streamlineFilter->SetInput(dob);
    dob = streamlineFilter->GetOutput();

    return dob;
}

// ****************************************************************************
// Method: avtStreamlinePlot::ApplyRenderingTransformation
//
// Purpose: 
//   Performs the rendering transforamtion for a Streamline plot.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The input data object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 24 14:20:21 PST 2002
//
// Modifications:
//   
// ****************************************************************************

avtDataObject_p
avtStreamlinePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}

// ****************************************************************************
//  Method: avtStreamlinePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Streamline plot.  This includes
//      behavior like shifting towards or away from the screen.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlinePlot::CustomizeBehavior(void)
{
    SetLegendRanges();
    behavior->SetLegend(varLegendRefPtr);
}

// ****************************************************************************
//  Method: avtStreamlinePlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Streamline plot.
//
//  Arguments:
//      atts    The attributes for this Streamline plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlinePlot::SetAtts(const AttributeGroup *a)
{
    const StreamlineAttributes *newAtts = (const StreamlineAttributes *)a;

    // See if the colors will need to be updated.
    bool updateColors = (!colorsInitialized) ||
       (atts.GetColorTableName() != newAtts->GetColorTableName());

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    //
    // Set the filter's attributes based on the plot attributes.
    //
    streamlineFilter->SetSourceType(atts.GetSourceType());
    streamlineFilter->SetStepLength(atts.GetStepLength());
    streamlineFilter->SetMaxTime(atts.GetMaxTime());
    streamlineFilter->SetShowTube(atts.GetShowTube());
    streamlineFilter->SetShowStart(atts.GetShowStart());
    streamlineFilter->SetTubeRadius(atts.GetTubeRadius());
    streamlineFilter->SetPointDensity(atts.GetPointDensity());

    streamlineFilter->SetPointSource(atts.GetPointSource());
    streamlineFilter->SetLineSource(atts.GetLineStart(),
                                    atts.GetLineEnd());
    streamlineFilter->SetPlaneSource(atts.GetPlaneOrigin(),
                                     atts.GetPlaneNormal(),
                                     atts.GetPlaneUpAxis(),
                                     atts.GetPlaneRadius());
    streamlineFilter->SetSphereSource(atts.GetSphereOrigin(),
                                      atts.GetSphereRadius());
    streamlineFilter->SetBoxSource(atts.GetBoxExtents());

    //
    // Set whether or not lighting is on.
    //
    SetLighting(atts.GetLightingFlag());

    //
    // Update the plot's colors if needed.
    //
    if(atts.GetColorBySpeed())
    {
        if (updateColors || atts.GetColorTableName() == "Default")
        {
            colorsInitialized = true;
            SetColorTable(atts.GetColorTableName().c_str());
        }
    }
    else
    {
        // Set a single color into the LUT.
        avtLUT->SetLUTColors(atts.GetSingleColor().GetColor(), 1);
    }

    SetLineWidth(atts.GetLineWidth());

    //
    // Update the legend.
    //
    SetLegend(atts.GetLegendFlag());
}

// ****************************************************************************
// Method: avtStreamlinePlot::SetColorTable
//
// Purpose: 
//   Sets the color table for the plot.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    If the color table changed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 15:40:41 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtStreamlinePlot::SetColorTable(const char *ctName)
{
    bool namesMatch = (atts.GetColorTableName() == std::string(ctName));

    if (atts.GetColorTableName() == "Default")
        return avtLUT->SetColorTable(NULL, namesMatch);
    else
        return avtLUT->SetColorTable(ctName, namesMatch);
}

// ****************************************************************************
//  Method: avtStreamlinePlot::SetLegend
//
//  Purpose:
//    Turns the legend on or off.
//
//  Arguments:
//    legendOn  : true if the legend should be turned on, false otherwise.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 7 15:54:28 PST 2002
//
// ****************************************************************************

void
avtStreamlinePlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        // Set scaling.
        varLegend->LegendOn();
        varLegend->SetLookupTable(avtLUT->GetLookupTable());
        varLegend->SetScaling();
        varMapper->SetLookupTable(avtLUT->GetLookupTable());

        //
        //  Retrieve the actual range of the data
        //
        varMapper->SetMin(0.);
        varMapper->SetMaxOff();
        varMapper->SetLimitsMode(0);
        SetLegendRanges();
    }
    else
    {
        varLegend->LegendOff();
    }
}

// ****************************************************************************
//  Method: avtStreamlinePlot::SetLighting
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
avtStreamlinePlot::SetLighting(bool lightingOn)
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
// Method: avtStreamlinePlot::SetLegendRanges
//
// Purpose: 
//   Sets the range to use for the legend.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 16:23:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlinePlot::SetLegendRanges()
{
    float min, max;
    varMapper->GetVarRange(min, max);

    //
    // Set the range for the legend's text and colors.
    //
    varLegend->SetVarRange(0., max);
    varLegend->SetRange(0., max);
}

// ****************************************************************************
// Method: avtStreamlinePlot::SetLineWidth
//
// Purpose: 
//   Sets the streamline's line width.
//
// Arguments:
//   lw : The new line width.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 15:52:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlinePlot::SetLineWidth(int lw)
{
    varMapper->SetLineWidth(Int2LineWidth(lw));
}

// ****************************************************************************
// Method: avtStreamlinePlot::ReleaseData
//
// Purpose: 
//   Tells the the plot's filters to release their data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 15:52:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlinePlot::ReleaseData(void)
{
    avtLineDataPlot::ReleaseData();

    if(streamlineFilter != NULL)
        streamlineFilter->ReleaseData();
    if(shiftCenteringFilter != NULL)
        shiftCenteringFilter->ReleaseData();
}
