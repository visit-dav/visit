// ************************************************************************* //
//                         avtExpressionEvaluatorFilter.h                    //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_EVALUATOR_FILTER_H
#define AVT_EXPRESSION_EVALUATOR_FILTER_H
#include <expression_exports.h>

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
// Modifications:
//   Brad Whitlock, Wed Aug 27 14:06:00 PST 2003
//   Made it use the right API.
//
//   Hank Childs, Mon Nov 17 16:47:33 PST 2003
//   Add ReleaseData.
//
//   Kathleen Bonnell, Thu Nov 13 08:39:40 PST 2003 
//   Added 'FindElementForPoint'.
//
// ****************************************************************************

class EXPRESSION_API avtExpressionEvaluatorFilter 
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
    virtual void             ReleaseData(void);

    virtual bool             FindElementForPoint(const char *, const int, 
                                 const int, const char *, float[3], int &);


protected:
    virtual void             PreExecute(void) {}
    virtual void             PostExecute(void) {}
    virtual void             Execute(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual int              AdditionalPipelineFilters(void);

protected:
    ExprPipelineState            pipelineState;
    avtPipelineSpecification_p   lastUsedSpec;
};


#endif


