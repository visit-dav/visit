// ************************************************************************* //
//                      avtMultipleInputToDatasetFilter.h                    //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_TO_DATASET_FILTER_H
#define AVT_MULTIPLE_INPUT_TO_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtMultipleInputToDataObjectFilter.h>
#include <avtDataObjectToDatasetFilter.h>


// ****************************************************************************
//  Class: avtMultipleInputToDatasetFilter
//
//  Purpose:
//      A filter that takes in multiple inputs and has a dataset as output.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

class PIPELINE_API avtMultipleInputToDatasetFilter
    : virtual public avtMultipleInputToDataObjectFilter,
      virtual public avtDataObjectToDatasetFilter
{
  public:
                       avtMultipleInputToDatasetFilter() {;};
    virtual           ~avtMultipleInputToDatasetFilter() {;};
};


#endif


