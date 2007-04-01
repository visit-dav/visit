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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsToDatasetFilter
    : virtual public avtSamplePointsToDataObjectFilter,
      virtual public avtDataObjectToDatasetFilter
{
  public:
                       avtSamplePointsToDatasetFilter();
    virtual           ~avtSamplePointsToDatasetFilter();
};


#endif


