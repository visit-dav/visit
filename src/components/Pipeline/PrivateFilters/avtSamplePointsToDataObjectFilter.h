// ************************************************************************* //
//                     avtSamplePointsToDataObjectFilter.h                   //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_TO_DATA_OBJECT_FILTER_H
#define AVT_SAMPLE_POINTS_TO_DATA_OBJECT_FILTER_H
#include <pipeline_exports.h>


#include <avtFilter.h>
#include <avtSamplePointsSink.h>


// ****************************************************************************
//  Class: avtSamplePointsToDataObjectFilter
//
//  Purpose:
//      A filter that takes in sample points as input and has a data object as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsToDataObjectFilter
    : virtual public avtFilter, virtual public avtSamplePointsSink
{
  public:
                       avtSamplePointsToDataObjectFilter() {;};
    virtual           ~avtSamplePointsToDataObjectFilter() {;};
};


#endif


