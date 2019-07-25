// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtDO_REPL_STRINGToDataObjectFilter.h                  //
// ************************************************************************* //

#ifndef AVT_DO_REPL_STRING_TO_DATA_OBJECT_FILTER_H
#define AVT_DO_REPL_STRING_TO_DATA_OBJECT_FILTER_H

#include <pipeline_exports.h>

#include <avtFilter.h>
#include <avtDO_REPL_STRINGSink.h>


// ****************************************************************************
//  Class: avtDO_REPL_STRINGToDataObjectFilter
//
//  Purpose:
//      A filter that takes in an DO_REPL_STRING as input and has a 
//      data object as output.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDO_REPL_STRINGToDataObjectFilter
    : virtual public avtFilter, virtual public avtDO_REPL_STRINGSink
{
  public:
                       avtDO_REPL_STRINGToDataObjectFilter();
    virtual           ~avtDO_REPL_STRINGToDataObjectFilter();
};


#endif


