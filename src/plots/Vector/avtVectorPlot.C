// ************************************************************************* //
//                            avtVectorPlot.C                                //
// ************************************************************************* //

#include <avtVectorPlot.h>

#include <vtkVectorGlyph.h>

#include <avtGhostZoneFilter.h>
#include <avtLookupTable.h>
#include <avtVariableLegend.h>
#include <avtVectorFilter.h>
#include <avtVectorGlyphMapper.h>

#include <LineAttributes.h>

// ****************************************************************************
//  Method: avtVectorPlot constructor
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug 30 10:47:07 PDT 2001
//    Intialize avtLUT.
//
//    Brad Whitlock, Mon Dec 2 11:59:27 PDT 2002
//    I added a legend.
//
//    Eric Brugger, Wed Jul 16 11:32:57 PDT 2003
//    Modified to work with the new way legends are managed.
//
// ****************************************************************************

avtVectorPlot::avtVectorPlot()
{
    colorsInitialized = false;
    glyph        = vtkVectorGlyph::New();
    vectorFilter = new avtVectorFilter(true, 10);
    ghostFilter  = new avtGhostZoneFilter();
    glyphMapper  = new avtVectorGlyphMapper(glyph->GetOutput());
    avtLUT       = new avtLookupTable();

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Vector");

    //
    // This is to allow the legend to be reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;
}


// ****************************************************************************
//  Method: avtVectorPlot destructor
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug 30 10:47:07 PDT 2001
//    Delete avtLUT.
//
// ****************************************************************************

avtVectorPlot::~avtVectorPlot()
{
    if (glyphMapper != NULL)
    {
        delete glyphMapper;
        glyphMapper = NULL;
    }
    if (vectorFilter != NULL)
    {
        delete vectorFilter;
        vectorFilter = NULL;
    }
    if (ghostFilter != NULL)
    {
        delete ghostFilter;
        ghostFilter = NULL;
    }
    if (glyph != NULL)
    {
        glyph->Delete();
        glyph = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }
}


// ****************************************************************************
//  Method:  avtVectorPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Hank Childs
//  Creation:    March 21, 2001
//
// ****************************************************************************

avtPlot*
avtVectorPlot::Create()
{
    return new avtVectorPlot;
}


// ****************************************************************************
//  Method:  avtVectorPlot::SetCellCountMultiplierForSRThreshold
//
//  Purpose: Sets the number of polygons each point in the plot's output will
//  be glyphed into.
//
//  Programmer:  Mark C. Miller 
//  Creation:    August 11, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 12 14:15:55 PDT 2004
//    Changed some code to get it to compile.
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Changed to Set... (Get is now done in avtPlot.C)
//
// ****************************************************************************

void
avtVectorPlot::SetCellCountMultiplierForSRThreshold(const avtDataObject_p dob)
{
    cellCountMultiplierForSRThreshold = 6.0;
    if (*dob)
    {
        int dim = dob->GetInfo().GetAttributes().GetSpatialDimension();
        if (dim == 2)
            cellCountMultiplierForSRThreshold = 2.0;
        else
            cellCountMultiplierForSRThreshold = 6.0;
    }
}

// ****************************************************************************
//  Method: avtVectorPlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2000
//
// ****************************************************************************

avtMapper *
avtVectorPlot::GetMapper(void)
{
    return glyphMapper;
}


// ****************************************************************************
//  Method: avtVectorPlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a vector plot.  This will create
//      the vectors as poly data.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the vector plot has been applied.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2000
//
//  Modifications:
//    Hank Childs, Tue Jul 17 07:42:08 PDT 2001
//    Added a filter for ghost zones.
//
//    Kathleen Bonnell, Mon Aug  9 14:27:08 PDT 2004 
//    Added code to create a name for the magnitude scalar variable.
//    Tell the vectorFilter to create that scalar var.
//
// ****************************************************************************

avtDataObject_p
avtVectorPlot::ApplyOperators(avtDataObject_p input)
{
    ghostFilter->SetInput(input);
    ComputeMagVarName(varname);
    vectorFilter->SetInput(ghostFilter->GetOutput());
    vectorFilter->SetMagVarName(magVarName); 
    return vectorFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtVectorPlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The input data object. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtVectorPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input; 
}


// ****************************************************************************
//  Method: avtVectorPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a vector plot.  Right
//      now this only means defining the shift factor.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Apr  7 15:39:46 PDT 2001
//    Guess we should shift vectors after all because it can be put behind
//    PC plots in 2D mode.
//
//    Kathleen Bonnell, Mon Sep 29 12:31:18 PDT 2003
//    Set AntialiasedRenderOrder, so that vector lines get drawn without
//    the 'halo'. 
//    
// ****************************************************************************

void
avtVectorPlot::CustomizeBehavior(void)
{
    behavior->SetShiftFactor(0.6);
    behavior->SetLegend(varLegendRefPtr);
    behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
}


// ****************************************************************************
//  Method: avtVectorPlot::CustomizeMapper
//
//  Purpose:
//      A hook from the base class that allows the plot to change its mapper
//      based on the dataset input.  This method will modify the input to the
//      mapper instead, which is the glyph.  It will have flat heads for 2D and
//      cone heads for 3D.
//
//  Arguments:
//      doi     The data object information.
//
//  Programmer: Hank Childs
//  Creation:   April 7, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Aug 27 12:21:22 PDT 2001
//    Set the render order to MUST_GO_LAST for 2D, DOES_NOT_MATTER otherwise.
//   
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Brad Whitlock, Wed Dec 4 15:55:40 PST 2002
//    I added code to set the legend ranges.
//
//    Kathleen Bonnell, Thu Aug 12 19:28:34 PDT 2004 
//    I added call to ComputeMagVarName and SetMapperColors.
//
// ****************************************************************************

void
avtVectorPlot::CustomizeMapper(avtDataObjectInformation &doi)
{
    ComputeMagVarName(varname);
    SetMapperColors();
    int dim = doi.GetAttributes().GetSpatialDimension();
    if (dim == 2)
    {
        //
        // We will get a flat head if the "cone head" feature is off.
        //
        glyph->SetConeHead(0);
        behavior->SetRenderOrder(MUST_GO_LAST);
    }
    else
    {
        glyph->SetConeHead(1);
        behavior->SetRenderOrder(DOES_NOT_MATTER);
    }

    //
    // Set the legend ranges.
    //
    SetLegendRanges();
}


// ****************************************************************************
//  Method: avtVectorPlot::SetAtts
//
//  Purpose:
//      Sets the atts for the pseudocolor plot.
//
//  Arguments:
//      atts    The attributes for this pseudocolor plot.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Jun  5 20:33:13 PDT 2001
//    Added code to set a flag if the plot needs recalculation.
//
//    Brad Whitlock, Fri Jun 15 15:14:39 PST 2001
//    Added code to set the color table used when coloring by magnitude.
//
//    Kathleen Bonnell, Mon Jun 25 12:45:06 PDT 2001 
//    Added code to set the line style. 
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001 
//    Use enum types from LineAttributes to ensure proper 
//    pattern is sent down to vtk 
//
//    Jeremy Meredith, Thu Aug 23 14:11:40 PDT 2001
//    Made it use the color table name "Default" instead of the boolean flag.
//
//    Jeremy Meredith, Fri Nov 21 11:30:35 PST 2003
//    Added origin offset.
//
//    Kathleen Bonnell, Thu Aug 12 12:05:01 PDT 2004
//    Set lut single color when ColorByMag is off. 
//
//    Kathleen Bonnell, Mon Aug  9 14:33:26 PDT 2004 
//    Moved some code into SetMapperColors and added call to this new method. 
//
// ****************************************************************************

void
avtVectorPlot::SetAtts(const AttributeGroup *a)
{
    const VectorAttributes *newAtts = (const VectorAttributes *)a;

    // See if the colors will need to be updated.
    bool updateColors = (!colorsInitialized) ||
       (atts.GetColorTableName() != newAtts->GetColorTableName());

    // See if any attributes that require the plot to be regenerated were
    // changed and copy the state object.
    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);
    atts = *newAtts;

    // Set internal plot state using the information in the atts state object.
    bool useStride = atts.GetUseStride();
    if (useStride)
    {
        vectorFilter->SetStride(atts.GetStride());
    }
    else
    {
        vectorFilter->SetNVectors(atts.GetNVectors());
    }

    glyph->SetMakeHead(atts.GetHeadOn());
    glyph->SetHeadSize(atts.GetHeadSize());

    float offset;
    switch (atts.GetVectorOrigin())
    {
      case VectorAttributes::Head:    offset = -.5;  break;
      case VectorAttributes::Middle:  offset =   0;  break;
      case VectorAttributes::Tail:    offset = +.5;  break;
    }
    glyph->SetOriginOffset(offset);

    glyphMapper->SetScale(atts.GetScale());

    SetMapperColors();

    glyphMapper->SetLineWidth(Int2LineWidth(atts.GetLineWidth()));
    glyphMapper->SetLineStyle(Int2LineStyle(atts.GetLineStyle()));

    // Update the plot's colors if needed.
    if (atts.GetColorByMag() &&
       (updateColors || atts.GetColorTableName() == "Default"))
    {
        colorsInitialized = true;
        SetColorTable(atts.GetColorTableName().c_str());
    }

    //
    // Update the legend.
    //
    SetLegend(atts.GetUseLegend());
}

// ****************************************************************************
//  Method: avtVectorPlot::SetColorTable
//
//  Purpose: 
//      Sets the plot's color table if the color table is the same as that of
//      the plot or we are using the default color table for the plot.
//
//  Arguments:
//      ctName : The name of the color table to use.
//
//  Returns:    Returns true if the color table is updated.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 14 16:52:49 PST 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 23 14:11:40 PDT 2001
//    Made it use the color table name "Default" instead of the boolean flag.
//
//    Kathleen Bonnell, Thu Aug 30 10:47:07 PDT 2001 
//    Main functionality now resides in avtLookupTable::SetColorTable. 
//
//    Kathleen Bonnell, Fri Aug 31 15:21:45 PDT 2001 
//    Added logic (that got lost when I moved functionality to 
//    avtLookupTable) to ensure that changing active color in color table
//    window will affect this plot if its color table is "Default". 
//
//    Brad Whitlock, Wed Dec 4 15:54:38 PST 2002
//    I added code to set the LUT colors for the case where we are using
//    a single color.
//
// ****************************************************************************

bool
avtVectorPlot::SetColorTable(const char *ctName)
{
    bool retval = false;
    if (atts.GetColorByMag())
    {
        bool namesMatch = (atts.GetColorTableName() == std::string(ctName));
        if (atts.GetColorTableName() == "Default")
        {
            retval =  avtLUT->SetColorTable(NULL, namesMatch);
        }
        else
        {
            retval =  avtLUT->SetColorTable(ctName, namesMatch);
        }

        if (retval)
        {
            glyphMapper->SetLookupTable(avtLUT->GetLookupTable());
        }
    }
    else
    {
        avtLUT->SetLUTColors(atts.GetVectorColor().GetColor(), 1);
    }

    return retval;
}

// ****************************************************************************
//  Method: avtVectorPlot::SetLegend
//
//  Purpose:
//    Turns the legend on or off.
//
//  Arguments:
//    legendOn  : true if the legend should be turned on, false otherwise.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Dec 2 12:04:49 PDT 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:48:03 PDT 2003
//    Remove call to SetColorTable, as it is unnecessary here.
//
// ****************************************************************************

void
avtVectorPlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        // Set scaling.
        varLegend->LegendOn();
        varLegend->SetLookupTable(avtLUT->GetLookupTable());
        varLegend->SetScaling();
    }
    else
    {
        varLegend->LegendOff();
    }
}

// ****************************************************************************
// Method: avtVectorPlot::SetLegendRanges
//
// Purpose: 
//   Sets the range to use for the legend.
//
// Programmer: Brad Whitlock
// Date:       Mon Dec 2 12:07:05 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtVectorPlot::SetLegendRanges()
{
    float min = 0., max = 1.;
    glyphMapper->GetRange(min, max);

    //
    // Set the range for the legend's text and colors.
    //
    varLegend->SetVarRange(min, max);
    varLegend->SetRange(min, max);
}

// ****************************************************************************
//  Method: avtVectorPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
// ****************************************************************************
 
void
avtVectorPlot::ReleaseData(void)
{
    avtPointDataPlot::ReleaseData();
 
    if (vectorFilter != NULL)
    {
        vectorFilter->ReleaseData();
    }
    if (ghostFilter != NULL)
    {
        ghostFilter->ReleaseData();
    }
}


// ****************************************************************************
//  Method: avtVectorPlot::ComputeMagVarName
//
//  Purpose:
//    Appends '_AVT_mag' to the name of the vector variable -- to creat a name
//    for the scalar magnitude variable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 9, 2004 
//
// ****************************************************************************

void
avtVectorPlot::ComputeMagVarName(const string &vn)
{
    magVarName = vn + string("_AVT_mag");
}

// ****************************************************************************
//  Method: avtVectorPlot::SetMapperColors
//
//  Purpose:
//    Tells the glyphMapper how to color the data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2004 
//
// ****************************************************************************

void
avtVectorPlot::SetMapperColors()
{
    if (atts.GetColorByMag())
    {
        glyphMapper->ColorByScalarOn(magVarName);
    }
    else
    {
        const unsigned char *col = atts.GetVectorColor().GetColor();
        avtLUT->SetLUTColors(col, 1);
        glyphMapper->ColorByMagOff(col);
    }
}
