// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtSubsetPlot.C
// ****************************************************************************

#include <avtSubsetPlot.h>

#include <SubsetAttributes.h>

#include <avtColorTables.h>
#include <avtGhostZoneAndFacelistFilter.h>
#include <avtFacelistFilter.h>
#include <avtGhostZoneFilter.h>
#include <avtLevelsLegend.h>
#include <avtLookupTable.h>
#include <avtSubsetFilter.h>
#include <avtSubsetMapper.h>
#include <avtFeatureEdgesFilter.h>
#include <avtSmoothPolyDataFilter.h>
#include <avtSubsetBlockMergeFilter.h>
#include <avtVertexExtractor.h>

#include <DebugStream.h>
#include <InvalidColortableException.h>
#include <LineAttributes.h>
#include <StackTimer.h>
#include <maptypes.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#define AVT_SUBSET_LEGEND_MAX_LABELS 100

using std::sort;
using std::string;
using std::pair;
using std::vector;

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
//    Changed subsetMapper type to avtLevelsPointGlyphMapper.
//
//    Hank Childs, Wed Dec 20 09:25:42 PST 2006
//    Make new method calls in response to changing behavior from facelist
//    filters and ghost zone creation.
//
//    Hank Childs, Fri Aug  3 13:46:26 PDT 2007
//    Initialized gz2.
//
//    Jeremy Meredith, Tue Oct 14 14:00:06 EDT 2008
//    Changed interface to SetMustCreatePolyData to allow either setting.
//
//    Kathleen Biagas, Tue Aug 23 11:20:32 PDT 2016
//    Added LevelsMapper as points and surfaces no longer handled by the
//    same mapper.
//
//    Kathleen Biagas, Fri Jun  5 08:18:16 PDT 2020
//    Add avtSurfaceMapper in place of levels and point glyph subsetMapper.
//    Add avtVertexExtractor.
//
// ****************************************************************************

avtSubsetPlot::avtSubsetPlot()
{
    subsetMapper  = new avtSubsetMapper();
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
    gzfl->SetMustCreatePolyData(true);
    gz    = new avtGhostZoneFilter();
    gz->GhostDataMustBeRemoved();
    gz2   = new avtGhostZoneFilter();
    gz2->GhostDataMustBeRemoved();
    fl    = new avtFacelistFilter();
    fl->SetForceFaceConsolidation(true);
    fl->SetMustCreatePolyData(true);
    sub   = new avtSubsetFilter();
    smooth= new avtSmoothPolyDataFilter();
    sbmf = new avtSubsetBlockMergeFilter();

    vertexExtractor = nullptr;
}


// ****************************************************************************
//  Method: avtSubsetPlot destructor
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
//    Hank Childs, Fri Aug  3 13:46:26 PDT 2007
//    Deleted gz2.
//
//    Kathleen Biagas, Fri Jun  5 08:18:16 PDT 2020
//    Add avtSurfaceMapper in place of levels and point glyph subsetMapper.
//    Add avtVertexExtractor.
//
// ****************************************************************************

avtSubsetPlot::~avtSubsetPlot()
{
    if (subsetMapper != NULL)
    {
        delete subsetMapper;
        subsetMapper = NULL;
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
    if (gz2 != NULL)
    {
        delete gz2;
        gz2 = NULL;
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
    if (sbmf != NULL)
    {
        delete sbmf;
        sbmf = NULL;
    }
    if (vertexExtractor != nullptr)
    {
        delete vertexExtractor;
        vertexExtractor = nullptr;
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
//    Brad Whitlock, Tue Jan  8 11:44:18 PST 2013
//    I added some new glyph types.
//
//    Kathleen Biagas, Fri Jun  5 08:18:16 PDT 2020
//    Add avtSurfaceMapper in place of levels and point glyph subsetMapper.
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
    SetLineWidth(atts.GetLineWidth());
    sub->SetPlotAtts(&atts);
    if (!atts.GetWireframe())
    {
        behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
        subsetMapper->SetSpecularIsInappropriate(false);
    }
    else
    {
        behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
        subsetMapper->SetSpecularIsInappropriate(true);
    }

    subsetMapper->SetScale(atts.GetPointSize());
    if (atts.GetPointSizeVarEnabled() &&
        atts.GetPointSizeVar() != "default" &&
        atts.GetPointSizeVar() != "" &&
        atts.GetPointSizeVar() != "\0")
    {
        subsetMapper->ScaleByVar(atts.GetPointSizeVar());
    }
    else
    {
        subsetMapper->DataScalingOff();
    }
    subsetMapper->SetGlyphType(atts.GetPointType());
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
        bool usesCT = (atts.GetColorTableName() == string(ctName));
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
    subsetMapper->SetLineWidth(Int2LineWidth(lw));
}


// ****************************************************************************
//  Method: avtSubsetPlot::GetMapper
//
//  Purpose:
//      Gets the mapper for this class.
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 17, 2001
//
// ****************************************************************************

avtMapperBase *
avtSubsetPlot::GetMapper(void)
{
    return subsetMapper;
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
//    Hank Childs, Fri Aug  3 13:46:26 PDT 2007
//    Add a second ghost zone filter in the case of wireframe rendering.
//    This addresses the coarse-fine boundary issues for AMR meshes.
//
//    Kevin Griffin, Mon Nov 3 13:05:42 PDT 2014
//    Added the block merge filter to the pipeline for the block wireframe
//    option.
//
//    Kevin Griffin, Thu Jan 15 16:42:43 PST 2015
//    Added the drawInternal flag check so that the wireframe behavior of the
//    subset plot of blocks executes like the old (incorrect) wireframe behavior
//    when this flag is true. If the drawInteral flag is false, the wireframe of
//    subset plots of blocks will only outline the blocks.
//
//    Kathleen Biagas, Fri Jun  5 08:29:08 PDT 2020
//    Added avtVertexExtractor, for separating out vertex cells from non-vertex
//    for proper point glyhping and sizing in mixed topology data.
//
// ****************************************************************************

avtDataObject_p
avtSubsetPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    avtDataObject_p dob = input;
    int topoDim = dob->GetInfo().GetAttributes().GetTopologicalDimension();
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
            gz->SetInput(dob);
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
            dob = sub->GetOutput();
        }
        else
        {
            // We're doing any other non-wireframe subset plot:
            //   - do the facelist and ghost zones in the needed order
            //   - do the subset (smoothing if needed)
            gzfl->SetInput(dob);
            unsigned char nodeType = 0;
            nodeType |= (1 << DUPLICATED_NODE);
            nodeType |= (1 << NODE_NOT_APPLICABLE_TO_PROBLEM);
            gzfl->SetGhostNodeTypesToRemove(nodeType);
            if (atts.GetSmoothingLevel() > 0)
            {
                smooth->SetInput(gzfl->GetOutput());
                sub->SetInput(smooth->GetOutput());
            }
            else
            {
                sub->SetInput(gzfl->GetOutput());
            }
            dob = sub->GetOutput();
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
            //   - remove edges that are on coarse-fine boundaries
            //      (these are capture with avtGhostNodes.  The avtGhostNodes
            //       are not removed by the first ghost zone filter, since
            //       they only take effect with poly data.)
            gz->SetInput(dob);
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

            if ((type == SubsetAttributes::Group) && !atts.GetDrawInternal())
            {
                sbmf->SetInput(sub->GetOutput());
                wf->SetInput(sbmf->GetOutput());
            }
            else
            {
                wf->SetInput(sub->GetOutput());
            }

            gz2->SetInput(wf->GetOutput());
            unsigned char nodeType = 0;
            nodeType |= (1 << NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY);
            nodeType |= (1 << NODE_NOT_APPLICABLE_TO_PROBLEM);
            gz2->SetGhostNodeTypesToRemove(nodeType);
            dob = gz2->GetOutput();
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
            dob = gz->GetOutput();
        }
    }
    if (topoDim != 0 && atts.GetPointType() != Point)
    {
        if (vertexExtractor == nullptr)
        {
            vertexExtractor = new avtVertexExtractor();
        }
        vertexExtractor->SetInput(dob);
        vertexExtractor->SetLabelPrefix("subset");
        vertexExtractor->SetConvertAllPoints(false);
        vertexExtractor->SetKeepNonVertex(true);
        vertexExtractor->SetPointType(atts.GetPointType());
        dob = vertexExtractor->GetOutput();
    }

    return dob;
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
//    Kathleen Biagas, Fri Jun  5 08:32:31 PDT 2020
//    Strip off prefixes on labels that may have been added by vertexExtractor
//    before seting them to the legend.
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
    subsetMapper->SetPointSize(atts.GetPointSizePixels());
}


// ****************************************************************************
//  Method: avtSubsetPlot::NeedZBufferToCompositeEvenIn2D
//
//  Purpose:
//      Tells the compositer that it needs zbuffer info to composite correctly,
//      in the case that the subset plot is bleeding over the domain boundary,
//      which means it can spill into other processor's portion of image
//      space.
//
//  Programmer: Hank Childs
//  Creation:   August 13, 2008
//
// ****************************************************************************

bool
avtSubsetPlot::NeedZBufferToCompositeEvenIn2D(void)
{
    if (atts.GetLineWidth() > 0)
        return true;
    if (atts.GetPointSizePixels() > 0)
        return true;

    return false;
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
//    Kathleen Bonnell, Mon Jan 17 18:16:41 MST 2011
//    Retrieve invertColorTable flag and send to color table.
//
//    Kathleen Biagas, Thu Oct 16 09:12:03 PDT 2014
//    Send 'needsRecalculation' flag to levelsMapper when setting colors.
//
// ****************************************************************************

void
avtSubsetPlot::SetColors()
{
    const vector < string > &allLabels = atts.GetSubsetNames();
    LevelColorMap levelColorMap;


    // The labels coming from behavior may have special prefixes used to
    // determine which type of mapper to use.  Dont want them being used
    // by the legend, so strip off prefixes first.
    vector<string> labels = behavior->GetInfo().GetAttributes().GetLabels();
    for(size_t i = 0; i < labels.size(); ++i)
    {
        if (labels[i].compare(0, 12, "subset_surf_") == 0)
           labels[i] = labels[i].substr(12);
        else if (labels[i].compare(0, 14, "subset_points_") == 0)
           labels[i] = labels[i].substr(14);
    }

    bool colorBarVisible = true;
    if (labels.size() == 0)
    {
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage("No subsets present");
        colorBarVisible = false;
    }
    else if(labels.size() > AVT_SUBSET_LEGEND_MAX_LABELS)
    {
        char msg[100];
        snprintf(msg, 100, "%d subsets", static_cast<int>(labels.size()));
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage(msg);
        colorBarVisible = false;
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
        subsetMapper->SetColors(cal, needsRecalculation);
        //
        //  Send an empty color map, rather than one where all
        //  entries map to same value.
        //
        if(colorBarVisible)
        {
            levelsLegend->SetLabelColorMap(levelColorMap);
            levelsLegend->SetLevels(labels);
        }
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
        {
            StackTimer t0("Set up levels subsetMapper");
            subsetMapper->SetColors(cal, needsRecalculation);
            subsetMapper->SetLabelColorMap(levelColorMap);
        }
        if(colorBarVisible)
        {
            StackTimer t1("Set up levels legend");
            levelsLegend->SetLevels(labels);
            levelsLegend->SetLabelColorMap(levelColorMap);
        }

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

        bool invert = atts.GetInvertColorTable();
        //
        // Add a color for each subset name.
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

        avtLUT->SetLUTColorsWithOpacity(colors, numColors);
        {
            StackTimer t0("Set up levels subsetMapper");
            subsetMapper->SetColors(cal, needsRecalculation);
            subsetMapper->SetLabelColorMap(levelColorMap);
        }

        if(colorBarVisible)
        {
            StackTimer t0("Set up levels legend");
            levelsLegend->SetLevels(labels);
            levelsLegend->SetLabelColorMap(levelColorMap);
        }
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
    if (vertexExtractor != nullptr)
    {
        vertexExtractor->ReleaseData();
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
    StackTimer t0("Sort labels");
    size_t   i;

    vector < string > originalLabels = atts.GetSubsetNames();

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


