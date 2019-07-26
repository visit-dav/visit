// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtDO_REPL_STRINGToDatasetFilter.h                    //
// ************************************************************************* //

#ifndef AVT_DO_REPL_STRING_TO_DATASET_FILTER_H
#define AVT_DO_REPL_STRING_TO_DATASET_FILTER_H

#include <pipeline_exports.h>

#include <avtDO_REPL_STRINGToDataObjectFilter.h>
#include <avtDataObjectToDatasetFilter.h>


// ****************************************************************************
//  Class: avtDO_REPL_STRINGToDatasetFilter
//
//  Purpose:
//      A filter that takes in DO_REPL_STRING as input and has a dataset as 
//      output.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDO_REPL_STRINGToDatasetFilter
    : virtual public avtDO_REPL_STRINGToDataObjectFilter,
      virtual public avtDataObjectToDatasetFilter
{
  public:
                       avtDO_REPL_STRINGToDatasetFilter();
    virtual           ~avtDO_REPL_STRINGToDatasetFilter();
};


#endif


