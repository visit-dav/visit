// ************************************************************************* //
//                          avtMacroExpressionFilter.h                       //
// ************************************************************************* //

#ifndef AVT_MACRO_EXPRESSION_FILTER_H
#define AVT_MACRO_EXPRESSION_FILTER_H

#include <avtExpressionFilter.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtSourceFromAVTDataset.h>

#include <Expression.h>
#include <ExpressionList.h>


// ****************************************************************************
//  Class: avtMacroExpressionFilter
//
//  Purpose:
//      A filter that instantiates others expression filters.  This is an
//      abstract type that does handles some of the dirty work for dealing with
//      the expression filters.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

class EXPRESSION_API avtMacroExpressionFilter : public avtExpressionFilter
{
  public:
                              avtMacroExpressionFilter();
    virtual                  ~avtMacroExpressionFilter();

    virtual const char       *GetType(void) 
                                  { return "avtMacroExpressionFilter"; };
    virtual int               NumVariableArguments() { return 1; }

  protected:
    avtExpressionEvaluatorFilter  eef;
    avtSourceFromAVTDataset      *term_src;
    std::vector<std::string>      expression_arguments;
    ExpressionList                original_list;
    avtDataSpecification_p        last_spec;

    virtual void              Execute(void);
    virtual void              ProcessArguments(ArgsExpr *,ExprPipelineState *);
    virtual void              GetMacro(std::vector<std::string> &,
                                    std::string &, Expression::ExprType &) = 0;

    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual int              AdditionalPipelineFilters(void);

    void                     ReplaceMacroInExpressionList(void);
    void                     RestoreExpressionList(void);
};


#endif


