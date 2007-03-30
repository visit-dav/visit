// ************************************************************************* //
//                             avtVolumeFilter.h                             //
// ************************************************************************* //

#ifndef AVT_VOLUME_FILTER_H
#define AVT_VOLUME_FILTER_H

#include <avtDatasetToDataObjectFilter.h>

#include <avtImage.h>
#include <VolumeAttributes.h>


class     avtRayTracer;
class     avtResampleFilter;


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
// ****************************************************************************

class avtVolumeFilter : public avtDatasetToDataObjectFilter
{
  public:
                             avtVolumeFilter();
    virtual                 ~avtVolumeFilter();

    void                     SetAttributes(const VolumeAttributes &);
    virtual const char      *GetType(void) { return "avtVolumeFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Volume rendering"; };

    virtual void             ReleaseData(void);

    virtual avtDataObject_p  GetOutput(void);

  protected:
    VolumeAttributes         atts;
    avtDataset_p             dataset;
    avtImage_p               image;
    avtRayTracer            *software;
    avtResampleFilter       *resampler;
    char                    *primaryVariable;

    virtual void             Execute(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual void             VerifyInput(void);
    virtual int              AdditionalPipelineFilters(void);
    bool                     DoSoftwareRender(void);
};


#endif


