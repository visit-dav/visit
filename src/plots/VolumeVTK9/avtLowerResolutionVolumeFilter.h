// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtLowerResolutionVolumeFilter.h              //
// ************************************************************************* //

#ifndef AVT_LOWER_RESOLUTION_VOLUME_FILTER_H
#define AVT_LOWER_RESOLUTION_VOLUME_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <VolumeAttributes.h>


// ****************************************************************************
//  Class: avtLowerResolutionVolumeFilter
//
//  Purpose:
//      Calculates scaled data and histograms for the resampled volume plots.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Dec 15 16:19:01 PST 2008
//
//  Modifications:
//    Eric Brugger, Tue Aug 19 14:01:15 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtLowerResolutionVolumeFilter : public avtPluginDataTreeIterator
{
  public:
                             avtLowerResolutionVolumeFilter();
    virtual                 ~avtLowerResolutionVolumeFilter();

    virtual void             SetAtts(const AttributeGroup *);
    virtual const char      *GetType(void) { return "avtLowerResolutionVolumeFilter"; };
    virtual const char      *GetDescription(void)
                                  { return "Scaling data, creating histograms"; };
  protected:
    VolumeAttributes         atts;
    float                   *hist{nullptr};
    int                      hist_size{256};

    void                     LogTransform(vtkDataArray *linear,
                                          vtkDataArray *log);
    void                     SkewTransform(vtkDataArray *linear,
                                           vtkDataArray *log);
    void                     CalculateHistograms(vtkDataSet *ds);

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void             PostExecute();
    virtual bool             FilterUnderstandsTransformedRectMesh();
};

#endif


