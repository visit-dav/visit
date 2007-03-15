/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtBoundaryPlot.C                              //
// ************************************************************************* //

#include <avtBoundaryPlot.h>

#include <algorithm>

#include <BoundaryAttributes.h>

#include <avtColorTables.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtFacelistFilter.h>
#include <avtGhostZoneFilter.h>
#include <avtLevelsLegend.h>
#include <avtLevelsPointGlyphMapper.h>
#include <avtLookupTable.h>
#include <avtBoundaryFilter.h>
#include <avtFeatureEdgesFilter.h>
#include <avtSmoothPolyDataFilter.h>

#include <DebugStream.h>
#include <InvalidColortableException.h>
#include <LineAttributes.h>
#include <maptypes.h>

using std::sort;
using std::pair;


// ****************************************************************************
//  Method: avtBoundaryPlot constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 10:19:00 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Kathleen Bonnell, Fri Nov 12 10:23:09 PST 2004 
//    Changed mapper type to avtLevelsPointGlyphMapper.
//
// ****************************************************************************

avtBoundaryPlot::avtBoundaryPlot()
{
    levelsMapper = new avtLevelsPointGlyphMapper();
    levelsLegend = new avtLevelsLegend();
    levelsLegend->SetTitle("Boundary");
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
    gz    = new avtGhostZoneFilter();
    sub   = new avtBoundaryFilter();
    smooth= new avtSmoothPolyDataFilter();
}


// ****************************************************************************
//  Method: avtLevelsMapper destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Modifications:
//
// ****************************************************************************

avtBoundaryPlot::~avtBoundaryPlot()
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
    if (gz != NULL)
    {
        delete gz;
        gz = NULL;
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
//  Method:  avtBoundaryPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

avtPlot*
avtBoundaryPlot::Create()
{
    return new avtBoundaryPlot;
}


// ****************************************************************************
//  Method: avtBoundaryPlot::SetAtts
//
//  Purpose:
//      Sets the plot with the attributes.
//
//  Arguments:
//      atts    The attributes for this boundary plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 29 13:15:20 PDT 2003
//    Set AntialisedRenderOrder dependent upon wireframe mode.
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004 
//    Ensure that specular properties aren't used in wireframe mode. 
//
//    Kathleen Bonnell, Fri Nov 12 10:23:09 PST 2004 
//    Incorporate point controls (pointSize, pointType, pointSizeVar).
//
//    Brad Whitlock, Wed Jul 20 13:26:13 PST 2005
//    I made the pointSize in the atts be used for to set the point size for
//    points, which is not the same as what's used for Box, Axis, Icosahedra.
//
// ****************************************************************************

void
avtBoundaryPlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const BoundaryAttributes*)a);
    atts = *(const BoundaryAttributes*)a;

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

    //
    // Setup point controls
    //
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
    SetPointGlyphSize();
}


// ****************************************************************************
// Method: avtBoundaryPlot::SetColorTable
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
// Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// Modifications:
//   
// ****************************************************************************

bool
avtBoundaryPlot::SetColorTable(const char *ctName)
{
    bool retval = false;

    if(atts.GetColorType() == BoundaryAttributes::ColorByColorTable)
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
//  Method: avtBoundaryPlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtBoundaryPlot::SetLegend(bool legendOn)
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
//  Method: avtBoundaryPlot::SetLineStyle
//
//  Purpose:
//      Sets the line style.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtBoundaryPlot::SetLineStyle(int ls)
{
    levelsMapper->SetLineStyle(Int2LineStyle(ls));
}


// ****************************************************************************
//  Method: avtBoundaryPlot::SetLineWidth
//
//  Purpose:
//      Sets the line width.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtBoundaryPlot::SetLineWidth(int lw)
{
    levelsMapper->SetLineWidth(Int2LineWidth(lw));
}


// ****************************************************************************
//  Method: avtBoundaryPlot::GetMapper
//
//  Purpose:
//      Gets the levels mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

avtMapper *
avtBoundaryPlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtBoundaryPlot::ApplyOperators
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
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtBoundaryPlot::ApplyOperators(avtDataObject_p input)
{
    return input;
}

// ****************************************************************************
//  Method: avtBoundaryPlot::RenderingTransformation
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
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtBoundaryPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    // Set the amount of smoothing required
    smooth->SetSmoothingLevel(atts.GetSmoothingLevel());
    
    if (atts.GetSmoothingLevel() > 0)
    {
        smooth->SetInput(input);
        sub->SetInput(smooth->GetOutput());
    }
    else
    {
        sub->SetInput(input);
    }

    if (atts.GetWireframe())
    {
        wf->SetInput(sub->GetOutput());
        gz->SetInput(wf->GetOutput());
    }
    else
    {
        gz->SetInput(sub->GetOutput());
    }

    return gz->GetOutput();
}


// ****************************************************************************
//  Method: avtBoundaryPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.  Since we do not yet have a 
//      levels mapper, this is only satisfying the requirement that the hook
//      must be defined so the type can be concrete.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 29 13:15:20 PDT 2003
//    Set AntialisedRenderOrder dependent upon wireframe mode.
//
//    Brad Whitlock, Thu Jul 21 15:34:01 PST 2005
//    Set point glyph size.
// ****************************************************************************

void
avtBoundaryPlot::CustomizeBehavior(void)
{
    SortLabels();
    SetColors();
    SetPointGlyphSize();
    levelsLegend->SetLookupTable(avtLUT->GetLookupTable());

    behavior->SetLegend(levLegendRefPtr);
    if (atts.GetWireframe() ||
        behavior->GetInfo().GetAttributes().GetTopologicalDimension()==1)
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
// Method: avtBoundaryPlot::SetPointGlyphSize
//
// Purpose: 
//   Sets the point glyph size into the mapper.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 21 15:24:25 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 10:12:07 PDT 2005
//   Added sphere points.
//
// ****************************************************************************

void
avtBoundaryPlot::SetPointGlyphSize()
{
    // Size used for points when using a point glyph.
    if(atts.GetPointType() == BoundaryAttributes::Point ||
       atts.GetPointType() == BoundaryAttributes::Sphere)
        levelsMapper->SetPointSize(atts.GetPointSizePixels());
}

// ****************************************************************************
//  Method: avtBoundaryPlot::SetColors
//
//  Purpose:
//    Sets up colors for the mapper and legend to use.   Either a single
//    color based on the flag set in atts, or colors corrsponding to the
//    current subselection.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 10:19:00 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Brad Whitlock, Thu Oct 23 14:57:44 PST 2003
//    I fixed it so the global plot opacity affects the plot when the plot
//    is colored using a single color.
//
// ****************************************************************************

void 
avtBoundaryPlot::SetColors()
{
    vector < string > allLabels = atts.GetBoundaryNames();
    vector < string > labels; 
    LevelColorMap levelColorMap;

    behavior->GetInfo().GetAttributes().GetLabels(labels);
   
    if (labels.size() == 0)
    {
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage("No boundaries present");
    }  
    else 
    {
        levelsLegend->SetColorBarVisibility(1);
        levelsLegend->SetMessage(NULL);
    }  

    if (atts.GetColorType() == BoundaryAttributes::ColorBySingleColor)
    {
        ColorAttribute ca(atts.GetSingleColor());
        ca.SetAlpha((unsigned char)(float(ca.Alpha()) * atts.GetOpacity()));
        ColorAttributeList cal;
        cal.AddColors(ca);

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
    else if (atts.GetColorType() == BoundaryAttributes::ColorByMultipleColors)
    {
        ColorAttributeList cal(atts.GetMultiColor());
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
        levelsMapper->SetColors(cal);
        levelsLegend->SetLevels(labels);

        levelsMapper->SetLabelColorMap(levelColorMap);
        levelsLegend->SetLabelColorMap(levelColorMap);

        delete [] colors;
    }
    else // ColorByColorTable
    {
        ColorAttributeList cal(atts.GetMultiColor());
        int numColors = cal.GetNumColors();
        unsigned char *colors = new unsigned char[numColors * 4];
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
        for(int i = 0; i < numColors; ++i)
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

        avtLUT->SetLUTColorsWithOpacity(colors, numColors);
        levelsMapper->SetColors(cal);
        levelsLegend->SetLevels(labels);

        levelsMapper->SetLabelColorMap(levelColorMap);
        levelsLegend->SetLabelColorMap(levelColorMap);

        delete [] colors;
    }
}


// ****************************************************************************
//  Method: avtBoundaryPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Nov 17 13:19:34 PST 2003
//    Release data associated with smooth filter.
//
// ****************************************************************************
 
void
avtBoundaryPlot::ReleaseData(void)
{
    avtVolumeDataPlot::ReleaseData();
 
    if (wf != NULL)
    {
        wf->ReleaseData();
    }
    if (gz != NULL)
    {
        gz->ReleaseData();
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
//  Method: avtBoundaryPlot::SortLabels
//
//  Purpose:
//    Ensure that the labels used by the legend are in the same order
//    as listed in plot atts.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
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
avtBoundaryPlot::SortLabels()
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


