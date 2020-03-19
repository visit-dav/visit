// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                 avtDO_REPL_STRINGToDO_REPL_STRINGFilter.h                 //
// ************************************************************************* //

#ifndef AVT_DO_REPL_STRING_TO_DO_REPL_STRING_FILTER_H
#define AVT_DO_REPL_STRING_TO_DO_REPL_STRING_FILTER_H

#include <pipeline_exports.h>

#include <avtDO_REPL_STRINGToDataObjectFilter.h>
#include <avtDataObjectToDO_REPL_STRINGFilter.h>


// ****************************************************************************
//  Class: avtDO_REPL_STRINGToDO_REPL_STRINGFilter
//
//  Purpose:
//      A filter that takes in an DO_REPL_STRING as input and has an 
//      DO_REPL_STRING as output.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDO_REPL_STRINGToDO_REPL_STRINGFilter
    : virtual public avtDO_REPL_STRINGToDataObjectFilter,
      virtual public avtDataObjectToDO_REPL_STRINGFilter
{
  public:
                       avtDO_REPL_STRINGToDO_REPL_STRINGFilter();
    virtual           ~avtDO_REPL_STRINGToDO_REPL_STRINGFilter();
};


#endif


