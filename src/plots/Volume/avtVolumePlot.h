// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtVolumePlot.h                              //
// ************************************************************************* //

#ifndef AVT_VOLUME_PLOT_H
#define AVT_VOLUME_PLOT_H

#include <VolumeAttributes.h>
#include <WindowAttributes.h>

#include <avtVolumeRenderer.h>
#include <avtPlot.h>
#include <avtVolumeVariableLegend.h>

#include <string>

class WindowAttributes;

class avtCompactTreeFilter;
class avtGradientExpression;
class avtLookupTable;
class avtShiftCenteringFilter;
class avtUserDefinedMapper;
class avtVolumeFilter;
class avtVolumeResampleFilter;
class avtLowerResolutionVolumeFilter;



// ****************************************************************************
//  Method: avtVolumePlot
//
//  Purpose:
//      A concrete type of avtPlot for volume-rendered plots.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 19 15:07:38 PST 2001
//    I added a legend to the plot.
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    Added avtLookupTable.
//
//    Hank Childs, Tue Nov 20 12:09:13 PST 2001
//    Added support for software volume rendering as well.
//
//    Hank Childs, Fri Feb  8 19:38:57 PST 2002
//    Allow for data to be smoothed with a shift centering filter.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation.
//
//    Hank Childs, Wed Nov 24 16:44:44 PST 2004
//    Integrated this plot with SR mode, meaning that a lot of infrastructure
//    for delivering images could be removed.
//
//    Hank Childs, Sun Dec  4 17:55:06 PST 2005
//    Added GetNumberOfStagesForImageBasedPlots.
//
//    Kathleen Bonnell, Wed May  9 16:58:50 PDT 2007
//    Set CanDo2DViewScaling to false.
//
//    Hank Childs, Tue Aug 26 14:46:36 PDT 2008
//    Define UtilizeRenderingFilters to state that we don't need them.
//
//    Hank Childs, Tue Sep  2 22:26:09 PDT 2008
//    Remove definition of "CanCacheWriterExternally", as it was leading to
//    unnecessary pipeline re-executions.
//
//    Brad Whitlock, Mon Dec 15 15:58:08 PST 2008
//    I added a new avtLowerResolutionVolumeFilter that works on the
//    resampled data.
//
//    Kathleen Bonnell, Tue Mar  3 13:37:13 PST 2009
//    Removed CanDo2DViewScaling (moved into Viewer PluginInfo)
//
//    Hank Childs, Sun Feb 21 10:33:44 CST 2010
//    Declare that we are managing transparency within the plot for the
//    splatting & 3D texturing case.
//
//    Brad Whitlock, Tue Jan 31 12:11:11 PST 2012
//    I added a compact tree filter.
//
//    Kathleen Biagas, Wed July 12, 2023 
//    Added avtVolumeResampleFilter, removed avtResampleFilter.
//
// ****************************************************************************

class
avtVolumePlot : public avtVolumeDataPlot
{
  public:
                        avtVolumePlot();
    virtual            ~avtVolumePlot();

    static avtPlot     *Create();

    virtual const char *GetName(void) { return "VolumePlot"; };
    virtual void        SetAtts(const AttributeGroup*);
    virtual void        ReleaseData(void);
    void                SetLegend(bool);

    virtual bool        PlotIsImageBased(void);
    virtual avtImage_p  ImageExecute(avtImage_p, const WindowAttributes &);
    virtual int         GetNumberOfStagesForImageBasedPlot(
                                                     const WindowAttributes &) const;
    virtual bool        Equivalent(const AttributeGroup *);

    virtual bool        UtilizeRenderingFilters(void) { return false; };
    virtual bool        ManagesOwnTransparency(void);

  protected:

    VolumeAttributes                atts;

    avtLowerResolutionVolumeFilter *lowResVolumeFilter {nullptr};
    avtVolumeFilter                *volumeFilter {nullptr};
    avtVolumeResampleFilter        *volumeResampleFilter {nullptr};
    avtGradientExpression          *gradientFilter {nullptr};
    avtShiftCenteringFilter        *shiftCentering {nullptr};
    avtCompactTreeFilter           *compactTree {nullptr};
    avtVolumeRenderer_p             renderer {nullptr};
    avtUserDefinedMapper           *mapper {nullptr};
    avtLookupTable                 *avtLUT {nullptr};

    avtVolumeVariableLegend        *varLegend {nullptr};
    avtLegend_p                     varLegendRefPtr {nullptr};

    virtual avtMapperBase   *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
    void                     SetLegendOpacities();
    virtual avtContract_p    EnhanceSpecification(avtContract_p);
};

#endif
