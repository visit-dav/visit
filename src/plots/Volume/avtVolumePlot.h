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

class avtLookupTable;
class avtShiftCenteringFilter;
class avtUserDefinedMapper;
class avtVolumeFilter;
class avtResampleFilter;


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
    virtual bool        Equivalent(const AttributeGroup *);

    virtual bool        CanCacheWriterExternally(void) { return false; }

  protected:
    VolumeAttributes         atts;
    avtVolumeFilter         *volumeFilter;
    avtResampleFilter       *resampleFilter;
    avtShiftCenteringFilter *shiftCentering;
    avtVolumeRenderer_p      renderer;
    avtUserDefinedMapper    *mapper;
    avtLookupTable          *avtLUT;

    avtVolumeVariableLegend *varLegend;
    avtLegend_p              varLegendRefPtr;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
    void                     SetLegendOpacities();
    virtual avtPipelineSpecification_p
                             EnhanceSpecification(avtPipelineSpecification_p);
};


#endif


