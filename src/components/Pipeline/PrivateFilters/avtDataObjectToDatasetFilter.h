// ************************************************************************* //
//                       avtDataObjectToDatasetFilter.h                      //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_TO_DATASET_FILTER_H
#define AVT_DATA_OBJECT_TO_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtFilter.h>
#include <avtDatasetSource.h>


// ****************************************************************************
//  Class: avtDataObjectToDatasetFilter
//
//  Purpose:
//      A filter that takes in a data object as input and has a dataset as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
// ****************************************************************************

class PIPELINE_API avtDataObjectToDatasetFilter
    : virtual public avtFilter, virtual public avtDatasetSource
{
  public:
                       avtDataObjectToDatasetFilter() {;};
    virtual           ~avtDataObjectToDatasetFilter() {;};

    void               OutputSetActiveVariable(const char *);
};


#endif


