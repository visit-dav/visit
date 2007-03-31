// ************************************************************************* //
//                         avtExpressionEvaluatorFilter.h                    //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_EVALUATOR_FILTER_H
#define AVT_EXPRESSION_EVALUATOR_FILTER_H


#include <avtDatasetToDatasetFilter.h>
#include <avtQueryableSource.h>

#include <ExprPipelineState.h>


// ****************************************************************************
//  Class: avtExpressionEvaluatorFilter
//
//  Purpose:
//      This filter parses out expressions and turns them into executable
//      VTK networks.  It encapsulates the code that used to be in the
//      NetworkManager and the Viewer.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
// ****************************************************************************

class PIPELINE_API avtExpressionEvaluatorFilter 
    : virtual public avtDatasetToDatasetFilter,
      virtual public avtQueryableSource
{
public:
                             avtExpressionEvaluatorFilter() {}
    virtual                 ~avtExpressionEvaluatorFilter() {}
    virtual const char*      GetType(void)
                                     { return "avtExpressionEvaluatorFilter";};

    virtual void             Query(PickAttributes *);
    virtual avtQueryableSource *
                             GetQueryableSource(void) { return this; };

protected:
    virtual void             PreExecute(void) {}
    virtual void             PostExecute(void) {}
    virtual void             Execute(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual int              AdditionalPipelineFilters(void);

protected:
    ExprPipelineState        pipelineState;
};


#endif


