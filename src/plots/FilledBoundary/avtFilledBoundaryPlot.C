// ************************************************************************* //
//                            avtFilledBoundaryPlot.C                        //
// ************************************************************************* //

#include <avtFilledBoundaryPlot.h>

#include <algorithm>

#include <FilledBoundaryAttributes.h>

#include <avtColorTables.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtFacelistFilter.h>
#include <avtGhostZoneFilter.h>
#include <avtLevelsLegend.h>
#include <avtLevelsPointGlyphMapper.h>
#include <avtLookupTable.h>
#include <avtFilledBoundaryFilter.h>
#include <avtFeatureEdgesFilter.h>
#include <avtSmoothPolyDataFilter.h>

#include <DebugStream.h>
#include <InvalidColortableException.h>
#include <LineAttributes.h>
#include <maptypes.h>

using std::sort;
using std::pair;


// ****************************************************************************
//  Method: avtFilledBoundaryPlot constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 10:29:53 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Changed mapper to type avtLevelsPointGlyphMapper. 
//
// ****************************************************************************

avtFilledBoundaryPlot::avtFilledBoundaryPlot()
{
    levelsMapper = new avtLevelsPointGlyphMapper();
    levelsLegend = new avtLevelsLegend();
    levelsLegend->SetTitle("Filled Boundary");
    // there is no 'range' per se, so turn off range visibility.
    levelsLegend->SetVarRangeVisibility(0);
    // Tell the legend to draw top-to-bottom (default is bottom-to-top).
    levelsLegend->SetReverseOrder(true);

    avtLUT = new avtLookupTable();

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a
    // LevelsLegend.
    //
    levLegendRefPtr = levelsLegend;

    wf    = new avtFeatureEdgesFilter();
    gzfl  = new avtGhostZoneAndFacelistFilter();
    gz    = new avtGhostZoneFilter();
    fl    = new avtFacelistFilter();
    sub   = new avtFilledBoundaryFilter();
    smooth= new avtSmoothPolyDataFilter();
}


// ****************************************************************************
//  Method: avtLevelsMapper destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
// ****************************************************************************

avtFilledBoundaryPlot::~avtFilledBoundaryPlot()
{
    if (levelsMapper != NULL)
    {
        delete levelsMapper;
        levelsMapper = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }

    if (wf != NULL)
    {
        delete wf;
        wf = NULL;
    }
    if (gzfl != NULL)
    {
        delete gzfl;
        gzfl = NULL;
    }
    if (gz != NULL)
    {
        delete gz;
        gz = NULL;
    }
    if (fl != NULL)
    {
        delete fl;
        fl = NULL;
    }
    if (sub != NULL)
    {
        delete sub;
        sub = NULL;
    }
    if (smooth != NULL)
    {
        delete smooth;
        smooth = NULL;
    }
 
    //
    // Do not delete the levelsLegend since it is being held by levLegendRefPtr.    
    //
}


// ****************************************************************************
//  Method:  avtFilledBoundaryPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

avtPlot*
avtFilledBoundaryPlot::Create()
{
    return new avtFilledBoundaryPlot;
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::SetAtts
//
//  Purpose:
//      Sets the plot with the attributes.
//
//  Arguments:
//      atts    The attributes for this boundary plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 29 13:07:50 PDT 2003
//    Set AntialiasedRenderOrder dependent upon wireframe mode.
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004 
//    Ensure that specular properties aren't used in wireframe mode. 
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Incorporate point controls (point size, point type, point size var). 
//
// ****************************************************************************

void
avtFilledBoundaryPlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const FilledBoundaryAttributes*)a);
    atts = *(const FilledBoundaryAttributes*)a;

    SetColors();
    SetLegend(atts.GetLegendFlag());
    SetLineStyle(atts.GetLineStyle());
    SetLineWidth(atts.GetLineWidth());
    sub->SetPlotAtts(&atts);
    if (!atts.GetWireframe())
    {
        behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
        levelsMapper->SetSpecularIsInappropriate(false);
    }
    else 
    {
        behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
        levelsMapper->SetSpecularIsInappropriate(true);
    }

    levelsMapper->SetScale(atts.GetPointSize());
    if (atts.GetPointSizeVarEnabled() &&
        atts.GetPointSizeVar() != "default" &&
        atts.GetPointSizeVar() != "" &&
        atts.GetPointSizeVar() != "\0")
    {
        levelsMapper->ScaleByVar(atts.GetPointSizeVar());
    }
    else 
    {
        levelsMapper->DataScalingOff();
    }
    levelsMapper->SetGlyphType((int)atts.GetPointType());
}

// ****************************************************************************
// Method: avtFilledBoundaryPlot::SetColorTable
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
// Programmer: Jeremy Meredith
// Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

bool
avtFilledBoundaryPlot::SetColorTable(const char *ctName)
{
    bool retval = false;

    if(atts.GetColorType() == FilledBoundaryAttributes::ColorByColorTable)
    {
        // If the color table is "Default" or is the color table that is being
        // changed, set the colors.
        bool usesCT = (atts.GetColorTableName() == std::string(ctName));
        if (usesCT || atts.GetColorTableName() == "Default")
        {
            SetColors();
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: avtFilledBoundaryPlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtFilledBoundaryPlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        levelsLegend->LegendOn();
    }
    else
    {
        levelsLegend->LegendOff();
    }
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::SetLineStyle
//
//  Purpose:
//      Sets the line style.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtFilledBoundaryPlot::SetLineStyle(int ls)
{
    levelsMapper->SetLineStyle(Int2LineStyle(ls));
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::SetLineWidth
//
//  Purpose:
//      Sets the line width.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtFilledBoundaryPlot::SetLineWidth(int lw)
{
    levelsMapper->SetLineWidth(Int2LineWidth(lw));
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::GetMapper
//
//  Purpose:
//      Gets the levels mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

avtMapper *
avtFilledBoundaryPlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::ApplyOperators
//
//  Purpose:
//      Does the implied operators for a boundary plot, that is none.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the boundary plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtFilledBoundaryPlot::ApplyOperators(avtDataObject_p input)
{
    return input;
}

// ****************************************************************************
//  Method: avtFilledBoundaryPlot::RenderingTransformation
//
//  Purpose:
//      Does the rendering transformation for a boundary plot, namely, the
//      boundary, ghost-zone and facelist filters. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the boundary plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
// ****************************************************************************

avtDataObject_p
avtFilledBoundaryPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    int type = atts.GetBoundaryType();

    if (!atts.GetWireframe())
    {
        if ((type == FilledBoundaryAttributes::Domain ||
             type == FilledBoundaryAttributes::Group) && 
             atts.GetDrawInternal())
        {
            gzfl->SetUseFaceFilter(false);
        }
        else
        {
            gzfl->SetUseFaceFilter(true);
        }

        // Set the amount of smoothing required
        smooth->SetSmoothingLevel(atts.GetSmoothingLevel());
    
        //
        // Apply the needed filters
        //
        if ((type==FilledBoundaryAttributes::Domain || 
             type==FilledBoundaryAttributes::Group) &&
             atts.GetDrawInternal())
        {
            // We're doing a non-wireframe domain boundary plot
            // where we require internal faces:
            //   - strip ghost zones first to keep domain boundaries
            //   - find the external faces of every domain
            //   - do the boundary (smoothing if needed)
            gz->SetInput(input);
            fl->SetInput(gz->GetOutput());
            if (atts.GetSmoothingLevel() > 0)
            {
                smooth->SetInput(fl->GetOutput());
                sub->SetInput(smooth->GetOutput());
            }
            else
            {
                sub->SetInput(fl->GetOutput());
            }
            return sub->GetOutput();
        }
        else
        {
            // We're doing any other non-wireframe boundary plot:
            //   - do the facelist and ghost zones in the needed order
            //   - do the boundary (smoothing if needed)
            gzfl->SetInput(input);
            if (atts.GetSmoothingLevel() > 0)
            {
                smooth->SetInput(gzfl->GetOutput());
                sub->SetInput(smooth->GetOutput());
            }
            else
            {
                sub->SetInput(gzfl->GetOutput());
            }
            return sub->GetOutput();
        }
    }
    else
    {
        if (type==FilledBoundaryAttributes::Domain || 
            type==FilledBoundaryAttributes::Group)
        {
            // We're doing a wireframe domain boundary plot:
            //   - strip ghost zones first to keep domain boundaries
            //   - find the external faces of every domain
            //   - do the boundary (smoothing if needed)
            //   - find feature edges
            gz->SetInput(input);
            fl->SetInput(gz->GetOutput());
            if (atts.GetSmoothingLevel() > 0)
            {
                smooth->SetInput(fl->GetOutput());
                sub->SetInput(smooth->GetOutput());
            }
            else
            {
                sub->SetInput(fl->GetOutput());
            }
            wf->SetInput(sub->GetOutput());
            return wf->GetOutput();
        }
        else
        {
            // We're doing any other wireframe boundary plot:
            //   - find the external faces first
            //   - do the boundary (smoothing if needed)
            //   - find feature edges
            //   - strip ghost zones last to remove domain boundaries
            fl->SetInput(input);
            if (atts.GetSmoothingLevel() > 0)
            {
                smooth->SetInput(fl->GetOutput());
                sub->SetInput(smooth->GetOutput());
            }
            else
            {
                sub->SetInput(fl->GetOutput());
            }
            wf->SetInput(sub->GetOutput());
            gz->SetInput(wf->GetOutput());
            return gz->GetOutput();
        }
    }

    return input;
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.  Since we do not yet have a 
//      levels mapper, this is only satisfying the requirement that the hook
//      must be defined so the type can be concrete.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 29 13:07:50 PDT 2003
//    Set AntialiasedRenderOrder dependent upon wireframe mode.
//
// ****************************************************************************

void
avtFilledBoundaryPlot::CustomizeBehavior(void)
{
    SortLabels();
    SetColors();
    levelsLegend->SetLookupTable(avtLUT->GetLookupTable());

    behavior->SetLegend(levLegendRefPtr);
    if (atts.GetWireframe())
    {
        behavior->SetShiftFactor(0.7);
        behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
    }
    else
    {
        behavior->SetShiftFactor(0.);
        behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
    }
}

// ****************************************************************************
//  Method: avtFilledBoundaryPlot::SetColors
//
//  Purpose:
//    Sets up colors for the mapper and legend to use.   Either a single
//    color based on the flag set in atts, or colors corrsponding to the
//    current subselection.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 10:29:53 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Brad Whitlock, Thu Oct 23 14:57:44 PST 2003
//    I fixed it so the global plot opacity affects the plot when the plot
//    is colored using a single color.
//
//    Jeremy Meredith, Wed Apr 14 16:31:29 PDT 2004
//    Add the "mixed" color to the end of the list when we are using
//    clean-zones only.  It was formerly included in the MultiColor list,
//    but this is a much, well, "cleaner" implementation.
//
//    Jeremy Meredith, Tue Jul  6 16:18:38 PDT 2004
//    Fixed some problems with clean-zones-only.  It was possible to have the
//    "mixed" mapper remain by turning off CZO mode, in which case not enough
//    colors were in the list and we got an exception.  Instead, we always
//    add the color and let the other pieces figure out whether or not it
//    should show up in the legend.
//
// ****************************************************************************

void 
avtFilledBoundaryPlot::SetColors()
{
    vector < string > allLabels = atts.GetBoundaryNames();
    vector < string > labels; 
    LevelColorMap levelColorMap;

    behavior->GetInfo().GetAttributes().GetLabels(labels);

    if (labels.size() == 0)
    {
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage("No subsets present");
    }  
    else 
    {
        levelsLegend->SetColorBarVisibility(1);
        levelsLegend->SetMessage(NULL);
    }  

    if (atts.GetColorType() == FilledBoundaryAttributes::ColorBySingleColor)
    {
        ColorAttribute ca(atts.GetSingleColor());
        ca.SetAlpha((unsigned char)(float(ca.Alpha()) * atts.GetOpacity()));
        ColorAttributeList cal;
        cal.AddColorAttribute(ca);

        avtLUT->SetLUTColorsWithOpacity(ca.GetColor(), 1);
        levelsMapper->SetColors(cal);
        // 
        //  Send an empty color map, rather than one where all
        //  entries map to same value. 
        //
        levelsLegend->SetLabelColorMap(levelColorMap);
        levelsLegend->SetLevels(labels);
        return;
    }
    else if (atts.GetColorType() == FilledBoundaryAttributes::ColorByMultipleColors)
    {
        ColorAttributeList cal(atts.GetMultiColor());

        //
        // If we are doing clean-zones-only, we will need a mixed color
        //
        allLabels.push_back("mixed");
        cal.AddColorAttribute(atts.GetMixedColor());

        int numColors = cal.GetNumColorAttributes();

        //
        //  Create colors from original color table. 
        //
        unsigned char *colors = new unsigned char[numColors * 4];
        unsigned char *cptr = colors;
        for(int i = 0; i < numColors; i++)
        {
            unsigned char c = (unsigned char)(cal[i].Alpha() * atts.GetOpacity());
            cal[i].SetAlpha(c);
            *cptr++ = (unsigned char)cal[i].Red();
            *cptr++ = (unsigned char)cal[i].Green();
            *cptr++ = (unsigned char)cal[i].Blue();
            *cptr++ = (unsigned char)cal[i].Alpha();

            //
            //  Create a label-to-color-index mapping 
            //
            levelColorMap.insert(LevelColorMap::value_type(allLabels[i], i));
        }

        avtLUT->SetLUTColorsWithOpacity(colors, numColors);
        levelsMapper->SetColors(cal);
        levelsLegend->SetLevels(labels);

        levelsMapper->SetLabelColorMap(levelColorMap);
        levelsLegend->SetLabelColorMap(levelColorMap);

        delete [] colors;
    }
    else // ColorByColorTable
    {
        ColorAttributeList cal(atts.GetMultiColor());

        //
        // If we are doing clean-zones-only, we will need a mixed color
        //
        allLabels.push_back("mixed");
        cal.AddColorAttribute(atts.GetMixedColor());

        //
        // It is a litte more complicated to handle c.z.o. here relative
        // to when using MultiColor, because we want to interpolate the
        // color table using only the size of array *before* adding the
        // mixed color.  We use two "numColors" variables to handle this.
        //
        int numColorsFull = cal.GetNumColorAttributes();
        int numColors     = numColorsFull - 1;
        unsigned char *colors = new unsigned char[numColorsFull * 4];
        unsigned char *cptr = colors;
        avtColorTables *ct = avtColorTables::Instance();
        int opacity = int(atts.GetOpacity()*255.);

        //
        // Detect if we're using the default color table or a color table
        // that does not exist anymore.
        //
        string ctName(atts.GetColorTableName());
        if(ctName == "Default")
            ctName = string(ct->GetDefaultDiscreteColorTable());
        else if(!ct->ColorTableExists(ctName.c_str()))
        {
            delete [] colors;
            EXCEPTION1(InvalidColortableException, ctName);
        }

        //
        //  Create a label-to-color-index mapping 
        //
        for(int i = 0; i < numColorsFull; ++i)
            levelColorMap.insert(LevelColorMap::value_type(allLabels[i], i));

        // 
        // Add a color for each boundary name.
        //
        if(ct->IsDiscrete(ctName.c_str()))
        {
            // The CT is discrete, get its color color control points.
            for(int i = 0; i < numColors; ++i)
            {
                unsigned char rgb[3] = {0,0,0};
                ct->GetControlPointColor(ctName.c_str(), i, rgb);
                *cptr++ = rgb[0];
                *cptr++ = rgb[1];
                *cptr++ = rgb[2];
                *cptr++ = opacity;

                cal[i].SetRgba(rgb[0], rgb[1], rgb[2], opacity);
            }
        }
        else
        {
            // The CT is continuous, sample the CT so we have a unique color
            // for each element.
            unsigned char *rgb = ct->GetSampledColors(ctName.c_str(), numColors);
            if(rgb)
            {
                for(int i = 0; i < numColors; ++i)
                {
                     int j = i * 3;
                     *cptr++ = rgb[j];
                     *cptr++ = rgb[j+1];
                     *cptr++ = rgb[j+2];
                     *cptr++ = opacity;

                     cal[i].SetRgba(rgb[j], rgb[j+1], rgb[j+2], opacity);
                }
                delete [] rgb;
            }
        }
        *cptr++ = (unsigned char)cal[numColors].Red();
        *cptr++ = (unsigned char)cal[numColors].Green();
        *cptr++ = (unsigned char)cal[numColors].Blue();
        *cptr++ = (unsigned char)cal[numColors].Alpha();

        avtLUT->SetLUTColorsWithOpacity(colors, numColorsFull);
        levelsMapper->SetColors(cal);
        levelsLegend->SetLevels(labels);

        levelsMapper->SetLabelColorMap(levelColorMap);
        levelsLegend->SetLabelColorMap(levelColorMap);

        delete [] colors;
    }
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
//    Hank Childs, Mon Nov 17 13:19:34 PST 2003
//    Release data from smooth filter.
//
// ****************************************************************************
 
void
avtFilledBoundaryPlot::ReleaseData(void)
{
    avtVolumeDataPlot::ReleaseData();
 
    if (wf != NULL)
    {
        wf->ReleaseData();
    }
    if (gzfl != NULL)
    {
        gzfl->ReleaseData();
    }
    if (gz != NULL)
    {
        gz->ReleaseData();
    }
    if (fl != NULL)
    {
        fl->ReleaseData();
    }
    if (sub != NULL)
    {
        sub->ReleaseData();
    }
    if (smooth != NULL)
    {
        smooth->ReleaseData();
    }
}


// ****************************************************************************
//  Method: avtFilledBoundaryPlot::SortLabels
//
//  Purpose:
//    Ensure that the labels used by the legend are in the same order
//    as listed in plot atts.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note: original copied from avtSubsetPlot
//
//  Modifications:
//
//    Hank Childs, Thu Mar 18 21:18:06 PST 2004
//    Re-wrote quadratic algorithm.
//
//    Jeremy Meredith, Thu Apr 15 12:19:21 PDT 2004
//    Allowed for zero-length arrays.  This probably shouldn't happen in
//    practice, but a different bug was causing a crash here.
//
// ****************************************************************************
 
void
avtFilledBoundaryPlot::SortLabels()
{
    int   i;

    vector < string > originalLabels = atts.GetBoundaryNames();

    //
    // Construct pairs of (label, index-into-ordered-list)
    //
    vector < pair < string, int > > originalLabelPairs;
    for (i = 0 ; i < originalLabels.size() ; i++)
    {
        originalLabelPairs.push_back(pair<string, int>(originalLabels[i], i));
    }
    sort(originalLabelPairs.begin(), originalLabelPairs.end());
    
    vector < string > usedLabels;
    behavior->GetInfo().GetAttributes().GetLabels(usedLabels);
    sort(usedLabels.begin(), usedLabels.end());
    int origLabelIndex = 0;
    vector < pair < int, string > > sortedUsedLabels;
    for (i = 0 ; i < usedLabels.size() ; i++)
    {
        while (origLabelIndex < originalLabelPairs.size() &&
               usedLabels[i] != originalLabelPairs[origLabelIndex].first)
        {
            origLabelIndex++;
        }

        if (origLabelIndex >= originalLabelPairs.size())
            break;

        sortedUsedLabels.push_back(
         pair<int, string>(originalLabelPairs[origLabelIndex].second, 
                           usedLabels[i]));
    }

    if (origLabelIndex >= originalLabelPairs.size())
    {
        // We have had an internal error -- there is a label in the subset
        // plot that is not in "master" list of labels.
        // Do nothing -- this means the labels won't be sorted.
        debug1 << "INTERNAL ERROR: the plot has a label that is not contained "
               << "in the master list of labels.  Not sorting the labels."
               << endl;
        return;
    }

    
    sort(sortedUsedLabels.begin(), sortedUsedLabels.end());
    vector < string > sortedLabels(sortedUsedLabels.size());
    for (i = 0; i < sortedUsedLabels.size(); i++)
    {
        sortedLabels[i] = sortedUsedLabels[i].second;
    }
    behavior->GetInfo().GetAttributes().SetLabels(sortedLabels);
}


