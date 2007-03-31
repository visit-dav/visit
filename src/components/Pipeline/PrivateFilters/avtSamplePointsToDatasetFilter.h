// ************************************************************************* //
//                      avtSamplePointsToDatasetFilter.h                     //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_TO_DATASET_FILTER_H
#define AVT_SAMPLE_POINTS_TO_DATASET_FILTER_H

#include <pipeline_exports.h>

#include <avtSamplePointsToDataObjectFilter.h>
#include <avtDataObjectToDatasetFilter.h>


// ****************************************************************************
//  Class: avtSamplePointsToDatasetFilter
//
//  Purpose:
//      A filter that takes in sample points as input and has a dataset as 
//      output.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsToDatasetFilter
    : virtual public avtSamplePointsToDataObjectFilter,
      virtual public avtDataObjectToDatasetFilter
{
  public:
                       avtSamplePointsToDatasetFilter() {;};
    virtual           ~avtSamplePointsToDatasetFilter() {;};
};


#endif


