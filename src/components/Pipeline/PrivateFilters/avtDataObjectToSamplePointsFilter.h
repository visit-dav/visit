// ************************************************************************* //
//                     avtDataObjectToSamplePointsFilter.h                   //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_TO_SAMPLE_POINTS_FILTER_H
#define AVT_DATA_OBJECT_TO_SAMPLE_POINTS_FILTER_H

#include <pipeline_exports.h>

#include <avtFilter.h>
#include <avtSamplePointsSource.h>


// ****************************************************************************
//  Class: avtDataObjectToSamplePointsFilter
//
//  Purpose:
//      A filter that takes in a data object as input and has sample points as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectToSamplePointsFilter
    : virtual public avtFilter, virtual public avtSamplePointsSource
{
  public:
                       avtDataObjectToSamplePointsFilter();
    virtual           ~avtDataObjectToSamplePointsFilter();
};


#endif


