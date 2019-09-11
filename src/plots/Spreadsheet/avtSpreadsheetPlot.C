// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtSpreadsheetPlot.C                          //
// ************************************************************************* //

#include <avtSpreadsheetPlot.h>
#include <avtUserDefinedMapper.h>
#include <avtSpreadsheetFilter.h>
#include <avtSpreadsheetRenderer.h>

// ****************************************************************************
//  Method: avtSpreadsheetPlot constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtSpreadsheetPlot::avtSpreadsheetPlot() : avtVolumeDataPlot(), atts()
{
    SpreadsheetFilter = new avtSpreadsheetFilter();
    renderer = avtSpreadsheetRenderer::New();

    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    mapper = new avtUserDefinedMapper(cr);
    colorsInitialized = false;
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtSpreadsheetPlot::~avtSpreadsheetPlot()
{
    if (mapper != NULL)
    {
        delete mapper;
        mapper = NULL;
    }
    if (SpreadsheetFilter != NULL)
    {
        delete SpreadsheetFilter;
        SpreadsheetFilter = NULL;
    }
    renderer = NULL;
}


// ****************************************************************************
//  Method:  avtSpreadsheetPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtPlot*
avtSpreadsheetPlot::Create()
{
    return new avtSpreadsheetPlot;
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtMapperBase *
avtSpreadsheetPlot::GetMapper(void)
{
    return mapper;
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Spreadsheet plot.  
//      The output from this method is a query-able object.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Spreadsheet plot has been applied.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtDataObject_p
avtSpreadsheetPlot::ApplyOperators(avtDataObject_p input)
{
    SpreadsheetFilter->SetInput(input);
    return SpreadsheetFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation associated with a Spreadsheet plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Spreadsheet plot has been applied.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtDataObject_p
avtSpreadsheetPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Spreadsheet plot.  This 
//      includes behavior like shifting towards or away from the screen.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
//  Modifications:
//
//    Hank Childs, Fri Aug 31 11:22:41 PDT 2007
//    Declare that the plot can adapt to any window mode.
//
// ****************************************************************************

void
avtSpreadsheetPlot::CustomizeBehavior(void)
{
    behavior->SetRenderOrder(MUST_GO_LAST);
    if(behavior->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_CURVE);
    else
        behavior->GetInfo().GetAttributes().SetAdaptsToAnyWindowMode(true);
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot::CustomizeMapper
//
//  Purpose:
//      A hook from the base class that allows the plot to change its mapper
//      based on the dataset input. 
//
//  Arguments:
//      doi     The data object information.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

void
avtSpreadsheetPlot::CustomizeMapper(avtDataObjectInformation &doi)
{
/* Example of usage.
    int dim = doi.GetAttributes().GetCurrentSpatialDimension();
    if (dim == 2)
    {
    }
    else
    {
    }
 */
}


// ****************************************************************************
//  Method: avtSpreadsheetPlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Spreadsheet plot.
//
//  Arguments:
//      atts    The attributes for this Spreadsheet plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

void
avtSpreadsheetPlot::SetAtts(const AttributeGroup *a)
{
    const SpreadsheetAttributes *newAtts = (const SpreadsheetAttributes *)a;

    // See if the colors will need to be updated.
    bool updateColors = (!colorsInitialized) ||
         (atts.GetColorTableName() != newAtts->GetColorTableName()); 

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    // Give the filter the new attributes.
    SpreadsheetFilter->SetAtts(atts);

    // Update the plot's colors if needed.
    if(updateColors || atts.GetColorTableName() == "Default")
    {
        colorsInitialized = true;
        SetColorTable(atts.GetColorTableName().c_str());
    }

    // Give the renderer the new attributes.
    renderer->SetAtts(a);
}

// ****************************************************************************
// Method: avtSpreadsheetPlot::SetColorTable
//
// Purpose: 
//   Tells the renderer about the new color table.
//
// Arguments:
//   ctName : The name of the new or modified color table.
//
// Returns:    True if a redraw is required; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 15:43:09 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtSpreadsheetPlot::SetColorTable(const char *ctName)
{
    return renderer->SetColorTable(ctName);
}

// ****************************************************************************
//  Method: avtSpreadsheetPlot::SetForegroundColor
//
//  Purpose:
//    Sets the foreground color.
//
//  Returns:    True if using this color will require the plot to be redrawn.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 20 11:16:45 PDT 2007
//
//  Modifications:
//
// ****************************************************************************

bool
avtSpreadsheetPlot::SetForegroundColor(const double *fg)
{
    return renderer->SetForegroundColor(fg);
}
