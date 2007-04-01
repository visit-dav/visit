// ************************************************************************* //
//                    avtSamplePointsToSamplePointsFilter.h                  //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_TO_SAMPLE_POINTS_FILTER_H
#define AVT_SAMPLE_POINTS_TO_SAMPLE_POINTS_FILTER_H
#include <pipeline_exports.h>


#include <avtSamplePointsToDataObjectFilter.h>
#include <avtDataObjectToSamplePointsFilter.h>


// ****************************************************************************
//  Class: avtSamplePointsToSamplePointsFilter
//
//  Purpose:
//      A filter that takes in sample points as input and has sample points
//      as output.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 28 12:46:49 PST 2001 
//    Added support for multiple variables.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsToSamplePointsFilter
    : virtual public avtSamplePointsToDataObjectFilter,
      virtual public avtDataObjectToSamplePointsFilter
{
  public:
                       avtSamplePointsToSamplePointsFilter();
    virtual           ~avtSamplePointsToSamplePointsFilter();

  protected:
    virtual void       PreExecute(void);
};


#endif


