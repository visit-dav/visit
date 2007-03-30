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
    virtual void       ExamineSpecification(avtPipelineSpecification_p);

    void               SetActiveVariable(const char *);
};


#endif


