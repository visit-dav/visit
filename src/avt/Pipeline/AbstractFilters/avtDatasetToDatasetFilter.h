// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDatasetToDatasetFilter.h                       //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_DATASET_FILTER_H
#define AVT_DATASET_TO_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDataObjectFilter.h>
#include <avtDataObjectToDatasetFilter.h>


// ****************************************************************************
//  Class: avtDatasetToDatasetFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has a dataset as output.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec 22 00:57:47 PST 2010
//    Add a method that controls whether the pipeline variable should be
//    automatically set as the active variable.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToDatasetFilter
    : virtual public avtDatasetToDataObjectFilter,
      virtual public avtDataObjectToDatasetFilter
{
  public:
                       avtDatasetToDatasetFilter();
    virtual           ~avtDatasetToDatasetFilter();
    void               AddSecondaryVariable(const char *var);

  protected:
    char              *pipelineVariable;
    char              *activeVariable;
    bool               switchVariables;
    bool               removeActiveVariableWhenDone;
    
    std::vector<const char *> secondaryVarList;
    std::vector<bool>  removeSecondaryVariable;

    virtual void       PreExecute(void);
    virtual void       PostExecute(void);
    virtual void       ExamineContract(avtContract_p);

    void               SetActiveVariable(const char *);
    virtual bool       AutomaticallyMakePipelineVariableActive(void) 
                                      { return true; };
};


#endif


