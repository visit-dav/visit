// ************************************************************************* //
//                          avtDatasetToImageFilter.h                        //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_IMAGE_FILTER_H
#define AVT_DATASET_TO_IMAGE_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDataObjectFilter.h>
#include <avtDataObjectToImageFilter.h>


// ****************************************************************************
//  Class: avtDatasetToImageFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has an image as output.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Feb 10 11:49:06 PDT 2003
//    The MSVC compiler does not seem to generate virtual tables for a class
//    unless it has some implementation in a C file. I moved the bodies of the
//    ctor and dtor, even though they do nothing, to a C file to avoid the
//    problem.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToImageFilter
    : virtual public avtDatasetToDataObjectFilter,
      virtual public avtDataObjectToImageFilter
{
  public:
                       avtDatasetToImageFilter();
    virtual           ~avtDatasetToImageFilter();
};


#endif


