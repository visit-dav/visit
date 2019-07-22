// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtMultipleInputToDataObjectFilter
    : virtual public avtFilter, virtual public avtMultipleInputSink
{
  public:
                       avtMultipleInputToDataObjectFilter();
    virtual           ~avtMultipleInputToDataObjectFilter();
};


#endif


