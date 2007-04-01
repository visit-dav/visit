// ************************************************************************* //
//                      avtDatasetToSamplePointsFilter.h                     //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_SAMPLE_POINTS_FILTER_H
#define AVT_DATASET_TO_SAMPLE_POINTS_FILTER_H

#include <pipeline_exports.h>

#include <avtDatasetToDataObjectFilter.h>
#include <avtDataObjectToSamplePointsFilter.h>


// ****************************************************************************
//  Class: avtDatasetToSamplePointsFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has sample points as 
//      output.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 15 15:29:18 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToSamplePointsFilter
    : virtual public avtDatasetToDataObjectFilter,
      virtual public avtDataObjectToSamplePointsFilter
{
  public:
                       avtDatasetToSamplePointsFilter();
    virtual           ~avtDatasetToSamplePointsFilter();

  protected:
    virtual void       PreExecute(void);
};


#endif


