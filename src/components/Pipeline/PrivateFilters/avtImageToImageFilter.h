// ************************************************************************* //
//                           avtImageToImageFilter.h                         //
// ************************************************************************* //

#ifndef AVT_IMAGE_TO_IMAGE_FILTER_H
#define AVT_IMAGE_TO_IMAGE_FILTER_H
#include <pipeline_exports.h>


#include <avtImageToDataObjectFilter.h>
#include <avtDataObjectToImageFilter.h>


// ****************************************************************************
//  Class: avtImageToImageFilter
//
//  Purpose:
//      A filter that takes in an image as input and has an image as output.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
// ****************************************************************************

class PIPELINE_API avtImageToImageFilter
    : virtual public avtImageToDataObjectFilter,
      virtual public avtDataObjectToImageFilter
{
  public:
                       avtImageToImageFilter() {;};
    virtual           ~avtImageToImageFilter() {;};
};


#endif


