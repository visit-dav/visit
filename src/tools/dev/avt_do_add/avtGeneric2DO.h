// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtDataObjectToDO_REPL_STRINGFilter.h                  //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_TO_DO_REPL_STRING_FILTER_H
#define AVT_DATA_OBJECT_TO_DO_REPL_STRING_FILTER_H

#include <pipeline_exports.h>

#include <avtFilter.h>
#include <avtDO_REPL_STRINGSource.h>


// ****************************************************************************
//  Class: avtDataObjectToDO_REPL_STRINGFilter
//
//  Purpose:
//      A filter that takes in a data object as input and has an DO_REPL_STRING
//      as output.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDataObjectToDO_REPL_STRINGFilter
    : virtual public avtFilter, virtual public avtDO_REPL_STRINGSource
{
  public:
                       avtDataObjectToDO_REPL_STRINGFilter();
    virtual           ~avtDataObjectToDO_REPL_STRINGFilter();
};


#endif


