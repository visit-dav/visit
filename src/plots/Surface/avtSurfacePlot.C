// ************************************************************************* //
//                             avtSurfacePlot.C                              //
// ************************************************************************* //
 
#include <avtSurfacePlot.h>
#include <avtSurfaceFilter.h>
#include <avtWireframeFilter.h>
#include <avtSurfaceAndWireframeRenderer.h>

#include <InvalidDimensionsException.h>
#include <InvalidLimitsException.h>
#include <SurfaceAttributes.h>
#include <SurfaceFilterAttributes.h>

#include <avtVariableLegend.h>
#include <avtUserDefinedMapper.h>
#include <avtLookupTable.h>
#include <vtkProperty.h>

#include <snprintf.h>

// ****************************************************************************
//  Method: avtSurfacePlot constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Aug 20 09:09:21 PDT 2001
//    Change from avtVariableMapper to avtUserDefinedMapper and
//    avtSurfaceAndWireframeRenderer.  Added intialization of
//    new member 'property', and 'LUT'.
//
//    Kathleen Bonnell, Thu Aug 30 11:09:23 PDT 2001 
//    Change from vtkLookupTable to avtLookupTable. 
//    
//    Kathleen Bonnell, Thu Sep 27 13:49:17 PDT 2001 
//    Use new renderer methods for setting primitive drawing. 
//    
//    Hank Childs, Tue Apr 23 20:16:50 PDT 2002
//    Instantiate renderer through 'New' to insulate from graphics library
//    issues.
//
//    Hank Childs, Mon May 20 10:47:18 PDT 2002
//    Reflect that our renderer is now reference counted.
//
//    Hank Childs, Thu Sep 12 14:01:37 PDT 2002          
//    Initialize surfaceFilter.
//
//    Eric Brugger, Wed Jul 16 11:22:08 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004
//    Added avtWireframeFilter.
//
// ****************************************************************************

avtSurfacePlot::avtSurfacePlot()
{
    renderer = avtSurfaceAndWireframeRenderer::New();
    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    mapper = new avtUserDefinedMapper(cr);

    avtLUT = new avtLookupTable();
    property = vtkProperty::New();

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Surface");

    colorsInitialized = false;

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;

    renderer->SurfaceLinesOff();
    renderer->EdgePolysOff();

    surfaceFilter = NULL;
    wireFilter = NULL;
}


// ****************************************************************************
//  Method: avtSurfacePlot destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Aug 20 09:09:21 PDT 2001
//    Added destruction of renderer, property, and LUT. 
//
//    Hank Childs, Mon May 20 10:47:18 PDT 2002
//    Do not delete the renderer, because it is now reference counted.
//
//    Hank Childs, Thu Sep 12 14:01:37 PDT 2002          
//    Delete surface surfaceFilter.
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004
//    Added avtWireframeFilter.
//
// ****************************************************************************

avtSurfacePlot::~avtSurfacePlot()
{
    if (mapper != NULL)
    {
        delete mapper;
        mapper = NULL;
    }
    if (property != NULL)
    {
        property->Delete();
        property = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }
    if (surfaceFilter != NULL)
    {
        delete surfaceFilter;
        surfaceFilter = NULL;
    }
    if (wireFilter != NULL)
    {
        delete wireFilter;
        wireFilter = NULL;
    }

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}


// ****************************************************************************
//  Method:  avtSurfacePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************

avtPlot*
avtSurfacePlot::Create()
{
    return new avtSurfacePlot;
}


// ****************************************************************************
//  Method: avtSurfacePlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
// ****************************************************************************

avtMapper *
avtSurfacePlot::GetMapper(void)
{
    return mapper;
}


// ****************************************************************************
//  Method: avtSurfacePlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a surface plot -- 
//      an avtSurfaceFilter.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the surfaceFilter is applied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jun 15 09:40:58 PDT 2001
//    Use more general data objects instead of derived types.
//
//    Hank Childs, Thu Sep 12 14:01:37 PDT 2002          
//    Clean up memory leak.
//
// ****************************************************************************

avtDataObject_p
avtSurfacePlot::ApplyOperators(avtDataObject_p input)
{
    if (surfaceFilter != NULL)
    {
        delete surfaceFilter;
    }

    SurfaceFilterAttributes sf_atts;
    sf_atts.SetLimitsMode(
                   (SurfaceFilterAttributes::LimitsMode) atts.GetLimitsMode());
    sf_atts.SetMinFlag(atts.GetMinFlag());
    sf_atts.SetMaxFlag(atts.GetMaxFlag());
    sf_atts.SetScaling((SurfaceFilterAttributes::Scaling) atts.GetScaling());
    sf_atts.SetSkewFactor(atts.GetSkewFactor());
    sf_atts.SetMin(atts.GetMin());
    sf_atts.SetMax(atts.GetMax());
    sf_atts.SetVariable("default");
    sf_atts.SetUseXYLimits(true);
    sf_atts.SetGenerateNodalOutput(true);
    surfaceFilter = new avtSurfaceFilter((const AttributeGroup *)&sf_atts);

    surfaceFilter->SetInput(input);
    return surfaceFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtSurfacePlot::ApplyRenderingTransformation
//
//  Purpose:
//    Applies the rendering transformation associated with a surface plot -- 
//    avtWireframeFilter. 
//
//  Arguments:
//    input   The input data object.
//
//  Returns:    The data object after the wireframeFilter is applied. 
// 
//  Programmer:  Kathleen Bonnell
//  Creation:    October 22, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004
//    Added avtWireframeFilter.
//
// ****************************************************************************

avtDataObject_p
avtSurfacePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    if (wireFilter != NULL)
    {
        delete wireFilter;
    }
    wireFilter = new avtWireframeFilter((const AttributeGroup *)&atts);

    wireFilter->SetInput(input);
    return wireFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtSurfacePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a surface plot.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001
//    Added error checking on min/max before using it to set legend's range.
//    Added calls to retrieve from mapper, set in legend the variable's true
//    extents.  Turn off color bar when coloring surface by constant color.
//
//    Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001 
//    Reflect change in InvalidLimitsException signature. 
//    
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Kathleen Bonnell, Tue Aug 21 10:15:07 PDT 2001 
//    GetRange is now a call to the base avtMapper class,
//    so does not take into account user-defined min/max.
//    Added that functionality.  Set renderer's scalar range
//    and property.  Removed setting of special lookup table
//    for constant coloring, as that is covered in the renderer.
//
//    Kathleen Bonnell, Thu Aug 30 11:09:23 PDT 2001
//    Set the legend's lut from avtLUT.
//
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//    Moved the bulk of the code to method SetLimitsMode, so that
//    changes to 'coloring' attributes won't require this plot to be
//    recalculated. 
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002  
//    Moved code that sets legend's LUT to SetScaling method. 
//
//    Kathleen Bonnell, Mon Sep 29 12:31:18 PDT 2003 
//    Set AntialiasedRenderOrder depending upon whether or not the wireframe
//    is being drawn without the surface. 
//
// ****************************************************************************

void
avtSurfacePlot::CustomizeBehavior()
{
    //
    //  In case the correct data ranges weren't available before.
    //
    SetLimitsMode(atts.GetLimitsMode());

    renderer->SetProperty(property);

    behavior->SetLegend(varLegendRefPtr);
    behavior->SetShiftFactor(0.9);
    behavior->SetRenderOrder(DOES_NOT_MATTER);
    if (atts.GetWireframeFlag() && !atts.GetSurfaceFlag())
      behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
    else 
      behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetAtts
//
//  Purpose:
//    Sets the atts for the surface plot.
//
//  Arguments:
//    atts      The attributes for this surface plot.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Mar 30 08:56:47 PDT 2001
//    Added calls to deal with artificial limits set by user.  Added call
//    to SetLineWidth and SetScaling.
//
//    Jeremy Meredith, Tue Jun  5 20:33:13 PDT 2001
//    Added code to set a flag if the plot needs recalculation.  Also
//    store the attributes as a class member.
//
//    Brad Whitlock, Fri Jun 15 14:24:28 PST 2001
//    Added code to set the colors table.
//
//    Kathleen Bonnell, Thu Jun 21 17:33:08 PDT 2001 
//    Added code to set the line style. 
//
//    Kathleen Bonnell, Mon Aug 20 09:09:21 PDT 2001 
//    Added calls to set renderer and property values
//    based on atts.
//
//    Jeremy Meredith, Thu Aug 23 14:11:40 PDT 2001
//    Made it use the color table name "Default" instead of the boolean flag.
//
//    Kathleen Bonnell, Thu Aug 30 16:45:39 PDT 2001 
//    Removed an if-test before call to SetColorTable.  The case is
//    handled from within SetColorTable.
//
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//    Added call to SetLimitsMode. 
//    
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Re-ordered method calls. 
//    
//    Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002
//    Notify renderer when lut colors are updated. 
//    
//    Kathleen Bonnell, Mon Sep 29 12:31:18 PDT 2003 
//    Set AntialiasedRenderOrder depending upon whether or not the wireframe
//    is being drawn without the surface. 
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004 
//    Set renderer's CanApplyGlobalRepresentation flag. 
//    
// ****************************************************************************

void
avtSurfacePlot::SetAtts(const AttributeGroup *a)
{
    const SurfaceAttributes *newAtts = (const SurfaceAttributes *)a;

    // See if the colors will need to be updated.
    bool updateColors = (!colorsInitialized) ||
       (atts.GetColorTableName() != newAtts->GetColorTableName());

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;
 
    if (atts.GetSurfaceFlag())
    {
        SetSurfaceAttributes(atts.GetColorByZFlag()); 
    }
    SetWireframeAttributes(atts.GetWireframeFlag());
    SetLighting(atts.GetLightingFlag());
    SetLineWidth(Int2LineWidth(atts.GetLineWidth()));
    SetLineStyle(Int2LineStyle(atts.GetLineStyle()));
    renderer->CanApplyGlobalRepresentation(atts.GetSurfaceFlag());
    SetRepresentation(!atts.GetSurfaceFlag() && atts.GetWireframeFlag());
    SetLegend(atts.GetLegendFlag());

    // Update the plot's colors if needed.
    if (atts.GetColorByZFlag() &&
       (updateColors || atts.GetColorTableName() == "Default"))
    {
        colorsInitialized = true;
        SetColorTable(atts.GetColorTableName().c_str());
    }

    SetScaling(atts.GetScaling(), atts.GetSkewFactor());
    SetLimitsMode(atts.GetLimitsMode());
    renderer->LUTColorsChanged(updateColors);

    if (atts.GetWireframeFlag() && !atts.GetSurfaceFlag())
        behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
    else 
        behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
}

// ****************************************************************************
// Method: avtSurfacePlot::SetColorTable
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
//    Kathleen Bonnell, Thu Aug 30 11:09:23 PDT 2001
//    Main functionality now resides in avtLookupTable::SetColorTable. 
//
//    Kathleen Bonnell, Fri Aug 31 15:21:45 PDT 2001 
//    Added logic (which existed here prior to moving functionality to
//    avtLookupTable) to ensure changing active color in color table window
//    affects this plot if its color table is default.  
//
// ****************************************************************************

bool
avtSurfacePlot::SetColorTable(const char *ctName)
{
    bool retval = false;
    if (atts.GetColorByZFlag())
    {
        bool namesMatch = (atts.GetColorTableName() == std::string(ctName));
        if (atts.GetColorTableName() == "Default")
            retval = avtLUT->SetColorTable(NULL, namesMatch);
        else
            retval = avtLUT->SetColorTable(ctName, namesMatch);
    }
    return retval;
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetLineWidth
//
//  Purpose:
//    Tells the mapper to set it's line width.
//
//  Arguments:
//    w         The width for the lines. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 21 10:54:58 PDT 2001
//    Changed parameter from int to _LineWidth.  Set line width
//    for property instead of mapper.
//
// ****************************************************************************

void
avtSurfacePlot::SetLineWidth(_LineWidth lw)
{
    property->SetLineWidth(LineWidth2Int(lw));
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetLineStyle
//
//  Purpose:
//    Tells the mapper to set its line style.
//
//  Arguments:
//    ls        The width for the lines. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 21, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 21 10:54:58 PDT 2001
//    Changed parameter from int to _LineStyle.  Set line style
//    for property instead of mapper.
//
// ****************************************************************************

void
avtSurfacePlot::SetLineStyle(_LineStyle ls)
{
    property->SetLineStipplePattern(LineStyle2StipplePattern(ls));
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetLegend
//
//  Purpose:
//    Turns the legend on or off.
//
//  Arguments:
//    legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
// ****************************************************************************

void
avtSurfacePlot::SetLegend(bool legendOn)
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
//  Method: avtSurfacePlot::SetLighting
//
//  Purpose:
//    Turns the lighting on or off.
//
//  Arguments:
//    lightingOn   true if the lighting should be turned on, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 21 10:54:58 PDT 2001
//    Set property attributes instead of mapper. 
//
//    Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002
//    Tell the renderer to ignore global lighting updates when our local 
//    lighting is off. 
//
// ****************************************************************************

void
avtSurfacePlot::SetLighting(bool lightingOn)
{
    if (lightingOn)
    {
        property->SetAmbient(0.0); 
        property->SetDiffuse(1.0); 
        renderer->IgnoreLighting(false);
    }
    else
    {
        property->SetAmbient(1.0); 
        property->SetDiffuse(0.0); 
        renderer->IgnoreLighting(true);
    }
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetScaling
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
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Kathleen Bonnell, Mon Aug 20 19:01:04 PDT 2001 
//    Changed functionality to only create the specific type
//    of lookup table.  Removed call to set hue range, as it
//    is unnecessary with Brads' new color tables.
//
//    Kathleen Bonnell, Thu Aug 30 11:09:23 PDT 2001
//    Use avtLUT methods to set the renderer's lut.
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002  
//    Set legend's LUT. 
//
// ****************************************************************************

void
avtSurfacePlot::SetScaling(const int mode, const double skew)
{
    varLegend->SetLookupTable(avtLUT->GetLookupTable());
    varLegend->SetScaling(mode, skew);

    if (mode == SurfaceAttributes::Log &&
        atts.GetColorByZFlag()) 
    {
        renderer->SetLookupTable(avtLUT->GetLogLookupTable());
    }
    else if (mode == SurfaceAttributes::Skew)
    {
        avtLUT->SetSkewFactor(skew);
        renderer->SetLookupTable(avtLUT->GetSkewLookupTable());
    }
    else
    {
        renderer->SetLookupTable(avtLUT->GetLookupTable());
    }
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetRepresentation
//
//  Purpose:
//    Sets the 'visibility' of the surface, by setting the appropriate
//    represention in the property. 
//
//  Arguments:
//    wireMode    True if we want to hide the surface, false otherwise. 
//
//  Programmer:   Kathleen Bonnell
//  Creation:     August 21, 2001 
//
// ****************************************************************************

void
avtSurfacePlot::SetRepresentation(bool wireMode)
{
    if (wireMode)
    {
        property->SetRepresentationToWireframe();
    }
    else 
    {
        property->SetRepresentationToSurface();
    }
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetWireframeAttributes
//
//  Purpose:
//    Sets the necessary attributes for drawing the wireframe. 
//
//  Arguments:
//    on          True if we want to draw the wireframe. 
//
//  Programmer:   Kathleen Bonnell
//  Creation:     August 21, 2001 
//
// ****************************************************************************

void
avtSurfacePlot::SetWireframeAttributes(bool on)
{
    if (on)
    {
        const unsigned char * col = atts.GetWireframeColor().GetColor();
        double rgb[3];
        rgb[0] = (double) col[0] / 255.0;
        rgb[1] = (double) col[1] / 255.0;
        rgb[2] = (double) col[2] / 255.0;
        property->SetEdgeColor(rgb);
        property->EdgeVisibilityOn();
        renderer->ResolveTopologyOn();
    }
    else 
    {
        property->EdgeVisibilityOff();
        renderer->ResolveTopologyOff();
    }
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetSurfaceAttributes
//
//  Purpose:
//    Sets the coloring mode for the surface.
//
//  Arguments:
//    useScalars  True if we want to color by the scalars, false otherwise. 
//
//  Programmer:   Kathleen Bonnell
//  Creation:     August 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002
//    Set property's modified tag when useScalars is true & renderer's scalar
//    vis is off, because it will force an update of the property display list.
//    Allowing the correct lut colors to be used.
// ****************************************************************************

void
avtSurfacePlot::SetSurfaceAttributes(bool useScalars)
{
    if (useScalars)
    {
        if (!renderer->GetScalarVisibility())
            property->Modified(); 
        renderer->ScalarVisibilityOn(); 
    }
    else
    {
        renderer->ScalarVisibilityOff();
        const unsigned char * col = atts.GetSurfaceColor().GetColor();
        double rgb[3];
        rgb[0] = (float) col[0] / 255.0;
        rgb[1] = (float) col[1] / 255.0;
        rgb[2] = (float) col[2] / 255.0;
        property->SetColor(rgb);
    }
}


// ****************************************************************************
//  Method: avtSurfacePlot::SetLimitsMode
//
//  Purpose:
//    Sets the limits used for coloring.
//
//  Arguments:
//    mode        Which type of limits to use. 
//
//  Programmer:   Kathleen Bonnell
//  Creation:     October 4, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Test for invalid user-specified min/max.
//
//    Kathleen Bonnell, Wed May 29 13:40:22 PDT 2002 
//    Always allow user-specified min/max.
//
//    Eric Brugger, Wed Jul 16 11:22:08 PDT 2003
//    Modified to work with the new way legends are managed.
//
// ****************************************************************************

void
avtSurfacePlot::SetLimitsMode(int mode)
{
    double origMin, origMax; 
    double userMin, userMax;

    if (!mapper->GetDataRange(origMin, origMax))
    {
        // we don't have input yet.
        return;


    }
    if (mode == SurfaceAttributes::CurrentPlot)
    {
        if (!mapper->GetCurrentDataRange(userMin, userMax))
        {
            return;
        }
        userMin = (atts.GetMinFlag() ? atts.GetMin() : userMin);
        userMax = (atts.GetMaxFlag() ? atts.GetMax() : userMax);
    }
    else
    {
        userMin = (atts.GetMinFlag() ? atts.GetMin() : origMin);
        userMax = (atts.GetMaxFlag() ? atts.GetMax() : origMax);
    }

    if ((userMin >= userMax) && atts.GetMinFlag() && atts.GetMaxFlag())
    {
        EXCEPTION1(InvalidLimitsException, false); 
    }
    else if ((userMin > userMax) && atts.GetMinFlag())
    {
        userMax = userMin;
    }
    else if ((userMin > userMax) && atts.GetMaxFlag())
    {
        userMin = userMax;
    }
   
    //  
    //  Before setting the range for the legend and renderer, need to 
    //  check for non-positive limits if log scaling is to be used. 
    //  
    if (atts.GetScaling() == SurfaceAttributes::Log  &&
        (userMin <=0. || userMax <= 0.) )
    {
        EXCEPTION1(InvalidLimitsException, true); 
    }

    renderer->SetScalarRange(userMin, userMax);

    // this sets range for labels
    varLegend->SetRange(userMin, userMax);
    
    // this sets range for limit text 
    varLegend->SetVarRange(origMin, origMax);
    
    if (!atts.GetColorByZFlag()) 
    {
        varLegend->SetColorBarVisibility(0);
        char msg[250];
        SNPRINTF(msg, 250, "Z-min:  %# -9.4g\nZ-max:  %# -9.4g", 
                 origMin, origMax);
        varLegend->SetMessage(msg);
    }
    else
    {
        // 
        // Make sure we render the color bar, in case we turned it off 
        // previously. 
        // 
        varLegend->SetColorBarVisibility(1);
        varLegend->SetMessage(NULL);
    }
}


// ****************************************************************************
//  Method: avtSurfacePlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004
//    Added avtWireframeFilter.
//
// ****************************************************************************
 
void
avtSurfacePlot::ReleaseData(void)
{
    avtSurfaceDataPlot::ReleaseData();
 
    if (surfaceFilter != NULL)
    {
        surfaceFilter->ReleaseData();
    }
    if (wireFilter != NULL)
    {
        wireFilter->ReleaseData();
    }
}


