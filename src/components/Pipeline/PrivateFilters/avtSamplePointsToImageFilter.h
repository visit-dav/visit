// ************************************************************************* //
//                       avtSamplePointsToImageFilter.h                      //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_TO_IMAGE_FILTER_H
#define AVT_SAMPLE_POINTS_TO_IMAGE_FILTER_H
#include <pipeline_exports.h>


#include <avtSamplePointsToDataObjectFilter.h>
#include <avtDataObjectToImageFilter.h>


// ****************************************************************************
//  Class: avtSamplePointsToImageFilter
//
//  Purpose:
//      A filter that takes in sample points as input and has an image as 
//      output.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsToImageFilter
    : virtual public avtSamplePointsToDataObjectFilter,
      virtual public avtDataObjectToImageFilter
{
  public:
                       avtSamplePointsToImageFilter() {;};
    virtual           ~avtSamplePointsToImageFilter() {;};
};


#endif


