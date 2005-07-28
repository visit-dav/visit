// ************************************************************************* //
//                             avtTruecolorPlot.C                                 //
// ************************************************************************* //

#include <avtTruecolorPlot.h>

#include <avtTruecolorFilter.h>

#include <vtkDataSetMapper.h>
#include <avtVariableMapper.h>

// ****************************************************************************
//  Method: avtTruecolorPlot constructor
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtTruecolorPlot::avtTruecolorPlot()
{
    TruecolorFilter = new avtTruecolorFilter();
    myMapper   = new avtVariableMapper();
}


// ****************************************************************************
//  Method: avtTruecolorPlot destructor
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtTruecolorPlot::~avtTruecolorPlot()
{
    if (myMapper != NULL)
    {
        delete myMapper;
        myMapper = NULL;
    }
    if (TruecolorFilter != NULL)
    {
        delete TruecolorFilter;
        TruecolorFilter = NULL;
    }
}


// ****************************************************************************
//  Method:  avtTruecolorPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtPlot*
avtTruecolorPlot::Create()
{
    return new avtTruecolorPlot;
}


// ****************************************************************************
//  Method: avtTruecolorPlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtMapper *
avtTruecolorPlot::GetMapper(void)
{
    return myMapper;
}


// ****************************************************************************
//  Method: avtTruecolorPlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Truecolor plot.  
//      The output from this method is a query-able object.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Truecolor plot has been applied.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

avtDataObject_p
avtTruecolorPlot::ApplyOperators(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtTruecolorPlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation associated with a Truecolor plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Truecolor plot has been applied.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
//  Modifications:
//
//     Chris Wojtan Mon Jun 21 15:47 2004
//     Set the TruecolorFilter's variable name before sending the data through the filter
//
// ****************************************************************************

avtDataObject_p
avtTruecolorPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    // the variable name should be correctly set by this time
    TruecolorFilter->SetVarName(varname);

    TruecolorFilter->SetInput(input);
    return TruecolorFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtTruecolorPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Truecolor plot.  This includes
//      behavior like shifting towards or away from the screen.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

void
avtTruecolorPlot::CustomizeBehavior(void)
{
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
}


// ****************************************************************************
//  Method: avtTruecolorPlot::CustomizeMapper
//
//  Purpose:
//      A hook from the base class that allows the plot to change its mapper
//      based on the dataset input. 
//
//  Arguments:
//      doi     The data object information.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
// ****************************************************************************

void
avtTruecolorPlot::CustomizeMapper(avtDataObjectInformation &doi)
{
}


// ****************************************************************************
//  Method: avtTruecolorPlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Truecolor plot.
//
//  Arguments:
//      atts    The attributes for this Truecolor plot.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005
//    Add SetLighting call.
//
// ****************************************************************************

void
avtTruecolorPlot::SetAtts(const AttributeGroup *a)
{
    const TruecolorAttributes *newAtts = (const TruecolorAttributes *)a;
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;
    SetOpacity(atts.GetOpacity());
    SetLighting(atts.GetLightingFlag());
}


// ****************************************************************************
//  Method: avtTruecolorPlot::SetOpacity
//
//  Purpose:
//      Allows for plots to be non-opaque.
//
//  Arguments:
//      opacity     The new opacity.
//
//  Programmer: Chris Wojtan
//  Creation:   June 14, 2004
//
// ****************************************************************************

void
avtTruecolorPlot::SetOpacity(float opacity)
{
    myMapper->SetOpacity(opacity);
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
//  Method: avtTruecolorPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   November 9, 2004
//
// ****************************************************************************

void
avtTruecolorPlot::ReleaseData(void)
{
    avtSurfaceDataPlot::ReleaseData();

    if (TruecolorFilter != NULL)
    {
        TruecolorFilter->ReleaseData();
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
//  Programmer: Kathleen Bonnell 
//  Creation:   July 21, 2005 
//
//  Modifications:
//
// ****************************************************************************

void
avtTruecolorPlot::SetLighting(bool lightingOn)
{
    if (lightingOn)
    {
        myMapper->TurnLightingOn();
        myMapper->SetSpecularIsInappropriate(false);
    }
    else
    {
        myMapper->TurnLightingOff();
        myMapper->SetSpecularIsInappropriate(true);
    }
}


