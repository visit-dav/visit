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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsToDataObjectFilter
    : virtual public avtFilter, virtual public avtSamplePointsSink
{
  public:
                       avtSamplePointsToDataObjectFilter();
    virtual           ~avtSamplePointsToDataObjectFilter();
};


#endif


