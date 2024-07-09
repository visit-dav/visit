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
#include <avtResampleFilter.h>


// ****************************************************************************
//  Class: avtVolumeResampleFilter
//
//  Purpose:
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 11, 2023 
//
//  Modifications:
//    Kathleen Biagas, Monday July 8, 2024
//    Inherit from avtResampleFilter. Override it's Execute method.
//    Rename atts to volAtts to distinguish from avtResampleFilter's atts
//    which are InternalResampleAttributes.
//
// ****************************************************************************

class avtVolumeResampleFilter : public avtResampleFilter
{
  public:
                             avtVolumeResampleFilter(
                                 const InternalResampleAttributes *,
                                 const VolumeAttributes &);

    virtual                 ~avtVolumeResampleFilter();

    virtual const char      *GetType(void) { return "avtVolumeResampleFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Volume resampling"; };
  protected:

    void                     Execute(void) override;

  private:
    VolumeAttributes         volAtts;

    int                      DataMustBeResampled(avtDataObject_p input);
};

#endif
