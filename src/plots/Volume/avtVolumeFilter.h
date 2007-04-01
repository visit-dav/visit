// ************************************************************************* //
//                             avtVolumeFilter.h                             //
// ************************************************************************* //

#ifndef AVT_VOLUME_FILTER_H
#define AVT_VOLUME_FILTER_H

#include <avtDatasetToDatasetFilter.h>

#include <VolumeAttributes.h>

#include <avtImage.h>

class     WindowAttributes;


// ****************************************************************************
//  Class: avtVolumeFilter
//
//  Purpose:
//      Decides if a volume renderer should output a software rendered image
//      or if it should output a resampled volume plot.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 24 16:21:39 PST 2004
//    Changed inheritance hierarchy.  This filter now simply does software
//    volume rendering and is used by the volume plot.  It is the interface
//    from the volume plot to the ray tracer.  Also removed many support 
//    methods that are no longer necessary since this filter doesn't switch
//    between multiple modes.
//
// ****************************************************************************

class avtVolumeFilter : public avtDatasetToDatasetFilter
{
  public:
                             avtVolumeFilter();
    virtual                 ~avtVolumeFilter();

    void                     SetAttributes(const VolumeAttributes &);
    virtual const char      *GetType(void) { return "avtVolumeFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Volume rendering"; };

    avtImage_p               RenderImage(avtImage_p, const WindowAttributes &);

  protected:
    VolumeAttributes         atts;
    char                    *primaryVariable;

    virtual void             Execute(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual void             VerifyInput(void);
};


#endif


