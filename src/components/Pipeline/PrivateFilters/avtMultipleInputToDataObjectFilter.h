// ************************************************************************* //
//                    avtMultipleInputToDataObjectFilter.h                   //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_TO_DATA_OBJECT_FILTER_H
#define AVT_MULTIPLE_INPUT_TO_DATA_OBJECT_FILTER_H
#include <pipeline_exports.h>


#include <avtFilter.h>
#include <avtMultipleInputSink.h>


// ****************************************************************************
//  Class: avtMultipleInputToDataObjectFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has a data object as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

class PIPELINE_API avtMultipleInputToDataObjectFilter
    : virtual public avtFilter, virtual public avtMultipleInputSink
{
  public:
                       avtMultipleInputToDataObjectFilter() {;};
    virtual           ~avtMultipleInputToDataObjectFilter() {;};
};


#endif


