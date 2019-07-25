// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtDatasetToDO_REPL_STRINGFilter.h                    //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_DO_REPL_STRING_FILTER_H
#define AVT_DATASET_TO_DO_REPL_STRING_FILTER_H

#include <pipeline_exports.h>


#include <avtDatasetToDataObjectFilter.h>
#include <avtDataObjectToDO_REPL_STRINGFilter.h>


// ****************************************************************************
//  Class: avtDatasetToDO_REPL_STRINGFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has an 
//      DO_REPL_STRING as output.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDatasetToDO_REPL_STRINGFilter
    : virtual public avtDatasetToDataObjectFilter,
      virtual public avtDataObjectToDO_REPL_STRINGFilter
{
  public:
                       avtDatasetToDO_REPL_STRINGFilter();
    virtual           ~avtDatasetToDO_REPL_STRINGFilter();
};


#endif


