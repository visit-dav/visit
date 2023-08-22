// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//     avtVolumeResampleFilter.h
// ****************************************************************************

#ifndef AVT_VOLUME_RESAMPLE_FILTER_H
#define AVT_VOLUME_RESAMPLE_FILTER_H

#include <avtDatasetToDatasetFilter.h>

#include <VolumeAttributes.h>

class avtResampleFilter;

// ****************************************************************************
//  Class: avtVolumeResampleFilter
//
//  Purpose:
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 11, 2023 
//
//  Modifications:
//
// ****************************************************************************

class avtVolumeResampleFilter : public avtDatasetToDatasetFilter
{
  public:
                             avtVolumeResampleFilter(const VolumeAttributes &);
    virtual                 ~avtVolumeResampleFilter();

    virtual const char      *GetType(void) { return "avtVolumeResampleFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Volume resampling"; };

  protected:
    VolumeAttributes         atts;
    avtResampleFilter       *resampleFilter;

    void                     Execute(void) override;
    avtContract_p            ModifyContract(avtContract_p) override;
    void                     VerifyInput(void) override;
    bool                     FilterUnderstandsTransformedRectMesh() override
                                 { return true; }
    int                      DataMustBeResampled(avtDataObject_p input);
};

#endif
