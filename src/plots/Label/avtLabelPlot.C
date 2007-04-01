// ************************************************************************* //
//                             avtLabelPlot.C                                //
// ************************************************************************* //

#include <avtLabelPlot.h>

#include <avtCallback.h>
#include <avtCondenseDatasetFilter.h>
#include <avtExtents.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtLabelFilter.h>
#include <avtMesaLabelRenderer.h>
#include <avtOpenGLLabelRenderer.h>
#include <vtkLookupTable.h>
#include <avtUserDefinedMapper.h>
#include <avtVariableLegend.h>

#include <avtVertexNormalsFilter.h>
#include <TimingsManager.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

// ****************************************************************************
// Method: avtLabelPlot constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtLabelPlot::avtLabelPlot() : avtSurfaceDataPlot()
{
    labelFilter = NULL;
    ghostAndFaceFilter = NULL;
    condenseFilter = NULL;
    normalFilter = NULL;

    if (avtCallback::GetNowinMode())
        renderer = new avtMesaLabelRenderer;
    else
        renderer = new avtOpenGLLabelRenderer;

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Label");
    vtkLookupTable *lut = vtkLookupTable::New();
    varLegend->SetLookupTable(lut);
    lut->Delete();
    varLegend->SetColorBarVisibility(0);
    varLegend->SetVarRangeVisibility(0);
    varLegendRefPtr = varLegend;

    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    labelMapper = new avtUserDefinedMapper(cr);
}


// ****************************************************************************
// Method: avtLabelPlot destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtLabelPlot::~avtLabelPlot()
{
    if (ghostAndFaceFilter != NULL)
    {
        delete ghostAndFaceFilter;
        ghostAndFaceFilter = NULL;
    }
    if (condenseFilter != NULL)
    {
        delete condenseFilter;
        condenseFilter = NULL;
    }
    if (normalFilter != NULL)
    {
        delete normalFilter;
        normalFilter = NULL;
    }
    if (labelFilter != NULL)
    {
        delete labelFilter;
        labelFilter = NULL;
    }

    renderer = NULL;

    if (labelMapper != NULL)
    {
        delete labelMapper;
        labelMapper = NULL;
    }

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}

// ****************************************************************************
//  Method:  avtLabelPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 7 14:58:26 PST 2004
//
// ****************************************************************************

avtPlot*
avtLabelPlot::Create()
{
    return new avtLabelPlot;
}

// ****************************************************************************
// Method: avtLabelPlot::GetMapper
//
// Purpose: 
//   Returns the label plot's mapper.
//
// Returns:    A pointer to the plot's mapper.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:48:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

avtMapper *
avtLabelPlot::GetMapper(void)
{
    return labelMapper;
}

// ****************************************************************************
// Method: avtLabelPlot::SetForegroundColor
//
// Purpose: 
//   Sets the foreground color into the label plot's renderer.
//
// Arguments:
//   c : The new foreground color.
//
// Returns:    True if the color was set; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:49:08 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
avtLabelPlot::SetForegroundColor(const double *c)
{
    return renderer->SetForegroundColor(c);
}

// ****************************************************************************
// Method: avtLabelPlot::SetLegend
//
// Purpose: 
//   Sets whether the legend is on or off.
//
// Arguments:
//   legendOn : Whether the legend should be on.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:50:17 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelPlot::SetLegend(bool legendOn)
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
// Method: avtLabelPlot::ApplyOperators
//
// Purpose:
//     Applies the operators associated with a Label plot.  
//     The output from this method is a query-able object.
//
// Arguments:
//     input   The input data object.
//
// Returns:    The data object after the Label plot has been applied.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtDataObject_p
avtLabelPlot::ApplyOperators(avtDataObject_p input)
{
    debug3 << "avtLabelPlot::ApplyOperators" << endl;
    return input;
}


// ****************************************************************************
// Method: avtLabelPlot::ApplyRenderingTransformation
//
// Purpose:
//     Applies the rendering transformation associated with a Label plot.  
//
// Arguments:
//     input   The input data object.
//
// Returns:    The data object after the Label plot has been applied.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005
//   Removed TRY-CATCH block in favor of testing for ValidVariable.
//
// ****************************************************************************

avtDataObject_p
avtLabelPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    debug3 << "avtLabelPlot::ApplyRenderingTransformation: start" << endl;

    avtDataObject_p dob = input;
    int total = visitTimer->StartTimer();
    int onefilter;

    //
    // Throw out the ghost cells.
    //
    onefilter = visitTimer->StartTimer();
    if(ghostAndFaceFilter != NULL)
        delete ghostAndFaceFilter;
    ghostAndFaceFilter = new avtGhostZoneAndFacelistFilter;
    ghostAndFaceFilter->SetUseFaceFilter(true);
    ghostAndFaceFilter->SetInput(dob);
    dob = ghostAndFaceFilter->GetOutput();
    visitTimer->StopTimer(onefilter, "avtGhostZoneAndFacelistFilter");

    //
    // If the variable that we're plotting is node centered or if it is
    // a mesh then remove any points that are not used in cells so we
    // don't end up adding labels for them.
    //
    if(condenseFilter != NULL)
    {
        delete condenseFilter; 
        condenseFilter = NULL;
    }
    bool removeExtraNodes = true;
    if (dob->GetInfo().GetAttributes().ValidVariable(varname))
    {
        removeExtraNodes = (AVT_NODECENT == 
            dob->GetInfo().GetAttributes().GetCentering(varname));
    }
    else 
    {
        debug1 << "We could not determine the variable centering for "
               << varname << " so let's assume that we need to remove "
               <<"extra nodes." << endl;
    }
    
    if(removeExtraNodes)
    {
        onefilter = visitTimer->StartTimer();
        condenseFilter = new avtCondenseDatasetFilter;
        condenseFilter->BypassHeuristic(true);
        condenseFilter->KeepAVTandVTK(true);
        condenseFilter->SetInput(dob);
        dob = condenseFilter->GetOutput();
        visitTimer->StopTimer(onefilter, "avtCondenseDatasetFilter");
    }

    //
    // Create vectors if the dataset's spatial dimension is greater
    // than 2 so we get normals that we can quantize in the label filter.
    //
    if(normalFilter != NULL)
    {
        delete normalFilter;
        normalFilter = NULL;
    }
    if(dob->GetInfo().GetAttributes().GetSpatialDimension() > 2)
    {
        onefilter = visitTimer->StartTimer();
        dob->GetInfo().GetValidity().SetNormalsAreInappropriate(false);
        normalFilter = new avtVertexNormalsFilter;
        normalFilter->SetInput(dob);
        dob = normalFilter->GetOutput();
        dob->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
        visitTimer->StopTimer(onefilter, "avtVertexNormalsFilter");
    }

    //
    // Create additional label information.
    //
    onefilter = visitTimer->StartTimer();
    if(labelFilter != NULL)
        delete labelFilter;
    labelFilter = new avtLabelFilter;
    labelFilter->SetLabelVariable(varname);
    labelFilter->SetInput(dob);
    dob = labelFilter->GetOutput();
    visitTimer->StopTimer(onefilter, "avtLabelFilter");
    visitTimer->StopTimer(total, "avtLabelPlot::ApplyRenderingTransformation");

    return dob;
}


// ****************************************************************************
// Method: avtLabelPlot::CustomizeBehavior
//
// Purpose:
//     Customizes the behavior as appropriate for a Label plot.  This includes
//     behavior like shifting towards or away from the screen.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

void
avtLabelPlot::CustomizeBehavior(void)
{
    behavior->SetLegend(varLegendRefPtr);
    behavior->SetShiftFactor(0.5);
    behavior->SetRenderOrder(MUST_GO_LAST);
    behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
}


// ****************************************************************************
// Method: avtLabelPlot::CustomizeMapper
//
// Purpose:
//     A hook from the base class that allows the plot to change its mapper
//     based on the dataset input. 
//
// Arguments:
//     doi     The data object information.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005
//   Removed TRY-CATCH block in favor of testing for ValidVariable.
//
// ****************************************************************************

void
avtLabelPlot::CustomizeMapper(avtDataObjectInformation &doi)
{
    //
    // Tell the renderer the variable that we care about.
    //
    renderer->SetVariable(varname);

    //
    // Tell the renderer whether to treat the label data as ASCII.
    //
    if (doi.GetAttributes().ValidVariable(varname))
    {
        renderer->SetTreatAsASCII(doi.GetAttributes().GetTreatAsASCII(varname));
    }
    else 
    {
        // Could not get the information so set the flag to false.
        renderer->SetTreatAsASCII(false);
    }

    //
    // Tell the renderer whether the data is 3D or not so it can make
    // some 2D optimizations.
    //
    renderer->Set3D(doi.GetAttributes().GetSpatialDimension() == 3);

    //
    // Set the dataset's extents into the renderer so we can create
    // 2D label bins in world space.
    //
    // These also returned values that might also work.
    // GetCumulativeCurrentSpatialExtents
    // GetSpatialExtents
    // GetAnySpatialExtents
    double e[6];
    doi.GetAttributes().GetTrueSpatialExtents()->CopyTo(e);
    float fe[6];
    for(int i = 0; i < 6; ++i)
        fe[i] = float(e[i]);
    renderer->SetExtents(fe);
}

// ****************************************************************************
// Method: avtLabelPlot::EnhanceSpecification
//
// Purpose: 
//   Turns on global cell and node numbers in the data specification.
//
// Arguments:
//   spec : The input data specification.
//
// Returns:    A new data specification.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:53:04 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

avtPipelineSpecification_p
avtLabelPlot::EnhanceSpecification(avtPipelineSpecification_p spec)
{
    debug3 << "avtLabelPlot::EnhanceSpecification: 0" << endl;
    avtDataSpecification_p ds = spec->GetDataSpecification();

    //
    // The pipeline specification should really be const -- it is used
    // elsewhere, so we can't modify it and return it.  Make a copy and in
    // the new copy, indicate that we need structured indices.
    //
    avtDataSpecification_p nds = new avtDataSpecification(ds);
    nds->TurnZoneNumbersOn();
    nds->TurnNodeNumbersOn();
    nds->SetNeedStructuredIndices(true);
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec, nds);

    debug3 << "avtLabelPlot::EnhanceSpecification: 1" << endl;

    return rv;
}

// ****************************************************************************
// Method: avtLabelPlot::SetAtts
//
// Purpose:
//     Sets the atts for the Label plot.
//
// Arguments:
//     atts    The attributes for this Label plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 17:00:04 PST 2004
//   I fixed a bug where I forgot to set the new attributes into the current
//   attributes.
//
// ****************************************************************************

void
avtLabelPlot::SetAtts(const AttributeGroup *a)
{
    const LabelAttributes *newAtts = (const LabelAttributes *)a;

    // Set the label plot attributes into the renderer.
    renderer->SetAtts(newAtts);

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    // Set whether the legend is on or off.
    SetLegend(atts.GetLegendFlag());
}

// ****************************************************************************
// Method: avtLabelPlot::ReleaseData
//
// Purpose: 
//   Causes the label plot to release its data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:54:25 PDT 2004
//
// Modifications:
//   
//   Hank Childs, Tue Nov  2 05:16:53 PST 2004
//   Go ahead and release data for the normals filter, now that a memory issue
//   has been cleaned up.
//
// ****************************************************************************

void
avtLabelPlot::ReleaseData(void)
{
    debug3 << "avtLabelPlot::ReleaseData: 0" << endl;
    if (labelFilter != NULL)
    {
        labelFilter->ReleaseData();
    }
    if (ghostAndFaceFilter != NULL)
    {
        ghostAndFaceFilter->ReleaseData();
    }
    if (condenseFilter != NULL)
    {
        condenseFilter->ReleaseData();
    }
    if (normalFilter != NULL)
    {
        normalFilter->ReleaseData();
    }
    debug3 << "avtLabelPlot::ReleaseData: 1" << endl;
}
