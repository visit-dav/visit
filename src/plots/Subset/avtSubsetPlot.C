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
//                              avtSubsetPlot.C                              //
// ************************************************************************* //

#include <avtSubsetPlot.h>

#include <algorithm>

#include <SubsetAttributes.h>

#include <avtColorTables.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtFacelistFilter.h>
#include <avtGhostZoneFilter.h>
#include <avtLevelsLegend.h>
#include <avtLevelsPointGlyphMapper.h>
#include <avtLookupTable.h>
#include <avtSubsetFilter.h>
#include <avtFeatureEdgesFilter.h>
#include <avtSmoothPolyDataFilter.h>

#include <DebugStream.h>
#include <InvalidColortableException.h>
#include <LineAttributes.h>
#include <maptypes.h>


using std::sort;
using std::pair;


// ****************************************************************************
//  Method: avtSubsetPlot constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Sep 12 13:44:13 PDT 2002 
//    Keep track of all of our filters to prevent memory leaks.
//
//    Jeremy Meredith, Tue Dec 10 10:00:09 PST 2002
//    Added poly data smooth filter.
//
//    Kathleen Bonnell, Mon May 19 13:46:55 PDT 2003 
//    Tell the legend to draw in top-to-bottom order (ReverseOrder). 
//
//    Eric Brugger, Wed Jul 16 11:17:28 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Hank Childs, Wed Oct 15 20:30:10 PDT 2003
//    Tell facelist filter to consolidate faces.
//
//    Kathleen Bonnell, Fri Nov 12 11:47:49 PST 2004 
//    Changed mapper type to avtLevelsPointGlyphMapper. 
//
//    Hank Childs, Wed Dec 20 09:25:42 PST 2006
//    Make new method calls in response to changing behavior from facelist
//    filters and ghost zone creation.
//
// ****************************************************************************

avtSubsetPlot::avtSubsetPlot()
{
    levelsMapper = new avtLevelsPointGlyphMapper();
    levelsLegend = new avtLevelsLegend();
    levelsLegend->SetTitle("Subset");
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
    gzfl->SetForceFaceConsolidation(true);
    gzfl->GhostDataMustBeRemoved();
    gzfl->MustCreatePolyData();
    gz    = new avtGhostZoneFilter();
    gz->GhostDataMustBeRemoved();
    fl    = new avtFacelistFilter();
    fl->SetForceFaceConsolidation(true);
    fl->MustCreatePolyData();
    sub   = new avtSubsetFilter();
    smooth= new avtSmoothPolyDataFilter();
}


// ****************************************************************************
//  Method: avtLevelsMapper destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Sep 12 13:44:13 PDT 2002 
//    Keep track of all of our filters to prevent memory leaks.
//
//    Jeremy Meredith, Tue Dec 10 10:00:09 PST 2002
//    Added poly data smooth filter.
//
// ****************************************************************************

avtSubsetPlot::~avtSubsetPlot()
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
//  Method:  avtSubsetPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
// ****************************************************************************

avtPlot*
avtSubsetPlot::Create()
{
    return new avtSubsetPlot;
}


// ****************************************************************************
//  Method: avtSubsetPlot::SetAtts
//
//  Purpose:
//      Sets the plot with the attributes.
//
//  Arguments:
//      atts    The attributes for this subset plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
//  Modifications:
//    Brad Whitlock, Fri Feb 1 16:13:14 PST 2002
//    Added overall opacity.
//
//    Jeremy Meredith, Wed Mar 13 11:08:38 PST 2002
//    Added line style and line width.
//
//    Kathleen Bonnell, Wed Apr 10 09:45:43 PDT 2002  
//    Moved color-related code to new SetColors method. 
//
//    Kathleen Bonnell, Mon Sep 29 12:31:18 PDT 2003 
//    Set AntialiasedRenderOrder depending upon wireframe mode. 
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004 
//    Ensure that specular properties aren't used in wireframe mode.
//
//    Kathleen Bonnell, Fri Nov 12 11:47:49 PST 2004 
//    Incorporate pointSize, pointtype and pointSizeVar. 
//
//    Brad Whitlock, Wed Jul 20 13:26:13 PST 2005
//    I made the pointSize in the atts be used for to set the point size for
//    points, which is not the same as what's used for Box, Axis, Icosahedra.
//
// ****************************************************************************

void
avtSubsetPlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const SubsetAttributes*)a);
    atts = *(const SubsetAttributes*)a;

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
    SetPointGlyphSize();
}

// ****************************************************************************
// Method: avtSubsetPlot::SetColorTable
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
// Creation:   Tue Nov 26 11:07:03 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtSubsetPlot::SetColorTable(const char *ctName)
{
    bool retval = false;

    if(atts.GetColorType() == SubsetAttributes::ColorByColorTable)
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
//  Method: avtSubsetPlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
// ****************************************************************************

void
avtSubsetPlot::SetLegend(bool legendOn)
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
//  Method: avtSubsetPlot::SetLineStyle
//
//  Purpose:
//      Sets the line style.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
avtSubsetPlot::SetLineStyle(int ls)
{
    levelsMapper->SetLineStyle(Int2LineStyle(ls));
}


// ****************************************************************************
//  Method: avtSubsetPlot::SetLineWidth
//
//  Purpose:
//      Sets the line width.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
// ****************************************************************************

void
avtSubsetPlot::SetLineWidth(int lw)
{
    levelsMapper->SetLineWidth(Int2LineWidth(lw));
}


// ****************************************************************************
//  Method: avtSubsetPlot::GetMapper
//
//  Purpose:
//      Gets the levels mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
// ****************************************************************************

avtMapper *
avtSubsetPlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtSubsetPlot::ApplyOperators
//
//  Purpose:
//      Does the implied operators for a subset plot, that is none.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the subset plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
//  Modifications:
//    Eric Brugger, Fri Dec 14 13:05:27 PST 2001
//    I modified the routine to first apply the ghost zone and facelist
//    filter before applying the subset plot.
//
//    Hank Childs, Wed Feb 27 11:12:54 PST 2002
//    Re-enabled facelists.  Removed meaningless comment about disabling them.
//
//    Jeremy Meredith, Wed Mar 13 11:07:53 PST 2002
//    Added wireframe mode.
//
//    Jeremy Meredith, Fri Mar 15 12:56:28 PST 2002
//    Added support for all combinations of wireframe mode and requested
//    drawing of internal boundaries for all subset plot types.
//
//    Kathleen Bonnell, Wed Sep  4 16:14:12 PDT 2002  
//    Removed NeedDomainLabels, functionality replace by new attribute 
//    subsetType. 
//    
//    Hank Childs, Thu Sep 12 13:44:13 PDT 2002 
//    Clean up memory leaks.
//
//    Kathleen Bonnell, Tue Oct 22 08:41:29 PDT 2002  
//    Moved entire method to ApplyRenderingTransformation, so that the output
//    of this method could serve as accurate input to a query. 
//
//    Hank Childs, Wed Mar 19 10:03:28 PST 2003
//    Move the wireframe portion into this routine (from
//    ApplyRenderingTransformation) because it is serving as our unfilled
//    boundary for now.
//
//    Jeremy Meredith, Thu Jun 12 09:23:06 PDT 2003
//    Removed the wireframe code now that we have a real unfilled boundary
//    plot.  It's all back in ApplyRenderingTransformation again.
//
// ****************************************************************************

avtDataObject_p
avtSubsetPlot::ApplyOperators(avtDataObject_p input)
{
    return input;
}

// ****************************************************************************
//  Method: avtSubsetPlot::RenderingTransformation
//
//  Purpose:
//      Does the rendering transformation for a subset plot, namely, the
//      subset, ghost-zone and facelist filters. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the subset plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
//  Modifications:
//    Jeremy Meredith, Tue Dec 10 10:00:09 PST 2002
//    Added smoothing of output.
//
//    Jeremy Meredith, Fri Jan 24 13:33:08 PST 2003
//    Only add the smoothing filter in the chain if it is to be executed.
//
//    Hank Childs, Wed Mar 19 10:03:28 PST 2003
//    Removed code related to wireframes, since that is our current
//    approximation of an unfilled boundary.
//
//    Jeremy Meredith, Thu Jun 12 10:03:53 PDT 2003
//    Reverted back to old version now that we have an unfilled boundary.
//
//    Hank Childs, Thu Jan 26 11:38:15 PST 2006
//    No longer do face consolidation with feature edges because ghost nodes
//    can screw it up.
//
// ****************************************************************************

avtDataObject_p
avtSubsetPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    int type = atts.GetSubsetType();

    if (!atts.GetWireframe())
    {
        gzfl->SetForceFaceConsolidation(true);
        fl->SetForceFaceConsolidation(true);
        if ((type == SubsetAttributes::Domain ||
              type == SubsetAttributes::Group)
            && atts.GetDrawInternal())
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
        if ((type==SubsetAttributes::Domain || type==SubsetAttributes::Group)
             && atts.GetDrawInternal())
        {
            // We're doing a non-wireframe domain subset plot
            // where we require internal faces:
            //   - strip ghost zones first to keep domain boundaries
            //   - find the external faces of every domain
            //   - do the subset (smoothing if needed)
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
            // We're doing any other non-wireframe subset plot:
            //   - do the facelist and ghost zones in the needed order
            //   - do the subset (smoothing if needed)
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
        gzfl->SetForceFaceConsolidation(false);
        fl->SetForceFaceConsolidation(false);
        if (type==SubsetAttributes::Domain || type==SubsetAttributes::Group)
        {
            // We're doing a wireframe domain subset plot:
            //   - strip ghost zones first to keep domain boundaries
            //   - find the external faces of every domain
            //   - do the subset (smoothing if needed)
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
            // We're doing any other wireframe subset plot:
            //   - find the external faces first
            //   - do the subset (smoothing if needed)
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
//  Method: avtSubsetPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.  Since we do not yet have a 
//      levels mapper, this is only satisfying the requirement that the hook
//      must be defined so the type can be concrete.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 17, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr 10 09:45:43 PDT 2002  
//    Added call to SetColors, so that legend will correspond to current
//    subselection.
//
//    Hank Childs, Wed Oct 23 13:57:55 PDT 2002
//    Shift the plot to the front if we are in wireframe mode.
//
//    Kathleen Bonnell, Thu Dec 19 12:27:09 PST 2002  
//    Added call to SortLabels. 
//
//    Kathleen Bonnell, Mon Sep 29 12:31:18 PDT 2003 
//    Set AntialiasedRenderOrder depending upon wireframe mode. 
//
//    Brad Whitlock, Thu Jul 21 15:39:12 PST 2005
//    Set the point glyph size.
//
// ****************************************************************************

void
avtSubsetPlot::CustomizeBehavior(void)
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
// Method: avtSubsetPlot::SetPointGlyphSize
//
// Purpose: 
//   Sets the point glyph size into the mapper.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 21 15:24:25 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 10:17:39 PDT 2005
//   Added sphere points.
//
// ****************************************************************************

void
avtSubsetPlot::SetPointGlyphSize()
{
    // Size used for points when using a point glyph.
    if(atts.GetPointType() == SubsetAttributes::Point ||
       atts.GetPointType() == SubsetAttributes::Sphere)
        levelsMapper->SetPointSize(atts.GetPointSizePixels());
}

// ****************************************************************************
//  Method: avtSubsetPlot::SetColors
//
//  Purpose:
//    Sets up colors for the mapper and legend to use.   Either a single
//    color based on the flag set in atts, or colors corrsponding to the
//    current subselection.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 9, 2002 
//
//  Modifications:
//    Eric Brugger, Mon Apr 22 15:36:34 PDT 2002
//    I corrected the setting of colors when a subselection had occured to
//    account for the fact that the subsetted list might have its labels in
//    a different order from the non-subsetted list when run in parallel.
//
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002  
//    Turn off color bar and send message to legend if no subsets are present.  
//    
//    Kathleen Bonnell, Tue Sep 10 14:14:18 PDT 2002 
//    Create a label-to-color-index map for use by mapper and legend.  Provides
//    consistency in color mapping, regardless of the order of labels,
//    or whether run in parallel. 
//
//    Brad Whitlock, Fri Nov 15 10:10:47 PDT 2002
//    I changed the map type.
//
//    Kathleen Bonnell, Mon Nov 25 17:36:27 PST 2002 
//    Allow the levels to be set in the legend, even if doing single-color. 
//
//    Brad Whitlock, Tue Nov 26 10:42:25 PDT 2002
//    I added a color table coloring mode.
//
//    Brad Whitlock, Fri Apr 25 12:44:29 PDT 2003
//    I made it throw an InvalidColortableException.
//
//    Eric Brugger, Wed Jul 16 11:17:28 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Brad Whitlock, Thu Oct 23 14:57:44 PST 2003
//    I fixed it so the global plot opacity affects the plot when the plot
//    is colored using a single color.
//
// ****************************************************************************

void 
avtSubsetPlot::SetColors()
{
    vector < string > allLabels = atts.GetSubsetNames();
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

    if (atts.GetColorType() == SubsetAttributes::ColorBySingleColor)
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
    else if (atts.GetColorType() == SubsetAttributes::ColorByMultipleColors)
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
        // Add a color for each subset name.
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
//  Method: avtSubsetPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Nov 17 13:19:34 PST 2003
//    Release data with smooth filter.
//
// ****************************************************************************
 
void
avtSubsetPlot::ReleaseData(void)
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
//  Method: avtSubsetPlot::SortLabels
//
//  Purpose:
//    Ensure that the labels used by the legend are in the same order
//    as listed in plot atts.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 19, 2002
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
avtSubsetPlot::SortLabels()
{
    int   i;

    vector < string > originalLabels = atts.GetSubsetNames();

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


