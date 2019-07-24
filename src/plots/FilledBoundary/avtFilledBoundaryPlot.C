// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFilledBoundaryPlot.C                        //
// ************************************************************************* //

#include <avtFilledBoundaryPlot.h>

#include <FilledBoundaryAttributes.h>

#include <avtColorTables.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtFacelistFilter.h>
#include <avtGhostZoneFilter.h>
#include <avtLevelsLegend.h>
#include <avtLevelsMapper.h>
#include <avtLevelsPointGlyphMapper.h>
#include <avtLookupTable.h>
#include <avtFilledBoundaryFilter.h>
#include <avtFeatureEdgesFilter.h>
#include <avtSmoothPolyDataFilter.h>

#include <DebugStream.h>
#include <InvalidColortableException.h>
#include <LineAttributes.h>
#include <maptypes.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using std::sort;
using std::string;
using std::pair;
using std::vector;


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
//    Kathleen Biagas, Tue Aug 23 11:19:44 PDT 2016
//    Added LevelsMapper as points and surfaces no longer handled by
//    same mapper.
//
// ****************************************************************************

avtFilledBoundaryPlot::avtFilledBoundaryPlot()
{
    glyphMapper  = new avtLevelsPointGlyphMapper();
    levelsMapper = new avtLevelsMapper();
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
    if (glyphMapper != NULL)
    {
        delete glyphMapper;
        glyphMapper = NULL;
    }
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
//    Brad Whitlock, Wed Jul 20 13:26:13 PST 2005
//    I made the pointSize in the atts be used for to set the point size for
//    points, which is not the same as what's used for Box, Axis, Icosahedra.
//
//    Brad Whitlock, Tue Jan  8 11:44:18 PST 2013
//    I added some new glyph types.
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
    SetLineWidth(atts.GetLineWidth());
    sub->SetPlotAtts(&atts);
    if (!atts.GetWireframe())
    {
        behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
        levelsMapper->SetSpecularIsInappropriate(false);
        glyphMapper->SetSpecularIsInappropriate(false);
    }
    else 
    {
        behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
        levelsMapper->SetSpecularIsInappropriate(true);
        glyphMapper->SetSpecularIsInappropriate(true);
    }

    glyphMapper->SetScale(atts.GetPointSize());
    if (atts.GetPointSizeVarEnabled() &&
        atts.GetPointSizeVar() != "default" &&
        atts.GetPointSizeVar() != "" &&
        atts.GetPointSizeVar() != "\0")
    {
        glyphMapper->ScaleByVar(atts.GetPointSizeVar());
    }
    else
    {
        glyphMapper->DataScalingOff();
    }

    glyphMapper->SetGlyphType(atts.GetPointType());

    SetPointGlyphSize();
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

avtMapperBase *
avtFilledBoundaryPlot::GetMapper(void)
{
    if (topologicalDim != 0)
    {
        return levelsMapper;
    }
    else
    {
        return glyphMapper;
    }
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
//    Jeremy Meredith, Tue Oct 14 16:02:06 EDT 2008
//    The ghost zone/facelist filter must create polydata.  This wasn't
//    previously a problem because the MIR would always output ugrids, but
//    I'm adding optimizations to allow even rgrids to pass through in 
//    some cases.
//
//    Kathleen Biagas, Tue Dec 20 14:20:48 PST 2016
//    Removed logic in support of non-material subset types.
//
// ****************************************************************************

avtDataObject_p
avtFilledBoundaryPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    gzfl->SetMustCreatePolyData(true);

    if (!atts.GetWireframe())
    {
        gzfl->SetUseFaceFilter(true);

        // Set the amount of smoothing required
        smooth->SetSmoothingLevel(atts.GetSmoothingLevel());
    
        //
        // Apply the needed filters
        //
        // We're doing non-wireframe boundary plot:
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
    else
    {
        // We're doing wireframe boundary plot:
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
//    Brad Whitlock, Thu Jul 21 15:36:35 PST 2005
//    Set the point glyph size.
//
// ****************************************************************************

void
avtFilledBoundaryPlot::CustomizeBehavior(void)
{
    SortLabels();
    SetColors();
    SetPointGlyphSize();
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
// Method: avtFilledBoundaryPlot::SetPointGlyphSize
//
// Purpose: 
//   Sets the point glyph size into the mapper.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 21 15:24:25 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 10:12:51 PDT 2005
//   Added sphere points.
//
// ****************************************************************************

void
avtFilledBoundaryPlot::SetPointGlyphSize()
{
    // Size used for points when using a point glyph.
    if(atts.GetPointType() == Point)
        glyphMapper->SetPointSize(atts.GetPointSizePixels());
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
//    Kathleen Bonnell, Mon Jan 17 18:07:08 MST 2011
//    Retrieve invertColorTable flag and pass it to ColorTable.
//
//    Kathleen Biagas, Thu Oct 16 09:12:03 PDT 2014
//    Send 'needsRecalculation' flag to levelsMapper when setting colors.
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
        cal.AddColors(ca);

        avtLUT->SetLUTColorsWithOpacity(ca.GetColor(), 1);
        levelsMapper->SetColors(cal, needsRecalculation);
        glyphMapper->SetColors(cal, needsRecalculation);
        // 
        //  Send an empty color map, rather than one where all
        //  entries map to same value. 
        //
        levelsLegend->SetLabelColorMap(levelColorMap);
        levelsLegend->SetLevels(labels);
    }
    else if (atts.GetColorType() == FilledBoundaryAttributes::ColorByMultipleColors)
    {
        ColorAttributeList cal(atts.GetMultiColor());

        //
        // If we are doing clean-zones-only, we will need a mixed color
        //
        allLabels.push_back("mixed");
        cal.AddColors(atts.GetMixedColor());

        int numColors = cal.GetNumColors();

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
        levelsMapper->SetColors(cal, needsRecalculation);
        glyphMapper->SetColors(cal, needsRecalculation);
        levelsLegend->SetLevels(labels);

        levelsMapper->SetLabelColorMap(levelColorMap);
        glyphMapper->SetLabelColorMap(levelColorMap);
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
        cal.AddColors(atts.GetMixedColor());

        //
        // It is a litte more complicated to handle c.z.o. here relative
        // to when using MultiColor, because we want to interpolate the
        // color table using only the size of array *before* adding the
        // mixed color.  We use two "numColors" variables to handle this.
        //
        int numColorsFull = cal.GetNumColors();
        int numColors     = numColorsFull - 1;
        unsigned char *colors = new unsigned char[numColorsFull * 4];
        unsigned char *cptr = colors;
        avtColorTables *ct = avtColorTables::Instance();
        int opacity = int((float)atts.GetOpacity()*255.f);

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

        bool invert = atts.GetInvertColorTable();

        // 
        // Add a color for each boundary name.
        //
        if(ct->IsDiscrete(ctName.c_str()))
        {
            // The CT is discrete, get its color color control points.
            for(int i = 0; i < numColors; ++i)
            {
                unsigned char rgb[3] = {0,0,0};
                ct->GetControlPointColor(ctName.c_str(), i, rgb, invert);
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
            unsigned char *rgb = ct->GetSampledColors(ctName.c_str(), numColors, invert);
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
        levelsMapper->SetColors(cal, needsRecalculation);
        glyphMapper->SetColors(cal, needsRecalculation);
        levelsLegend->SetLevels(labels);

        levelsMapper->SetLabelColorMap(levelColorMap);
        glyphMapper->SetLabelColorMap(levelColorMap);
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
    size_t   i;

    vector < string > originalLabels = atts.GetBoundaryNames();

    //
    // Construct pairs of (label, index-into-ordered-list)
    //
    vector < pair < string, int > > originalLabelPairs;
    for (i = 0 ; i < originalLabels.size() ; i++)
    {
        originalLabelPairs.push_back(pair<string, int>(originalLabels[i], (int)i));
    }
    sort(originalLabelPairs.begin(), originalLabelPairs.end());
    
    vector < string > usedLabels;
    behavior->GetInfo().GetAttributes().GetLabels(usedLabels);
    sort(usedLabels.begin(), usedLabels.end());
    size_t origLabelIndex = 0;
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


