// ************************************************************************* //
//                         avtImageToDataObjectFilter.h                      //
// ************************************************************************* //

#ifndef AVT_IMAGE_TO_DATA_OBJECT_FILTER_H
#define AVT_IMAGE_TO_DATA_OBJECT_FILTER_H
#include <pipeline_exports.h>


#include <avtFilter.h>
#include <avtImageSink.h>


// ****************************************************************************
//  Class: avtImageToDataObjectFilter
//
//  Purpose:
//      A filter that takes in an image as input and has a data object as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
// ****************************************************************************

class PIPELINE_API avtImageToDataObjectFilter
    : virtual public avtFilter, virtual public avtImageSink
{
  public:
                       avtImageToDataObjectFilter() {;};
    virtual           ~avtImageToDataObjectFilter() {;};
};


#endif


