// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtDeferExpressionFilter.h
// ************************************************************************* //

#ifndef AVT_DeferExpressionBase_FILTER_H
#define AVT_DeferExpressionBase_FILTER_H


#include <avtExpressionEvaluatorFilter.h>

#include <ExpressionList.h>

// ****************************************************************************
//  Class: avtDeferExpressionBaseFilter
//
//  Purpose:
//      A base class for filters that want to do defer expression operators.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2010
//
// ****************************************************************************

class EXPRESSION_API avtDeferExpressionBaseFilter : virtual public avtExpressionEvaluatorFilter
{
  public:
                         avtDeferExpressionBaseFilter();
    virtual             ~avtDeferExpressionBaseFilter();

    virtual const char  *GetType(void)  { return "avtDeferExpressionFilter"; };

  protected:
    ExpressionList                 orig_el;

    virtual void                   ExamineContract(avtContract_p);
    virtual void                   InferVariableNameFromContract(avtContract_p) {;};
    virtual void                   GetCustomExpressions(std::vector<Expression> &) {;};
    virtual void                   GetVariablesToEvaluate(std::vector<std::string> &) = 0;
    virtual avtContract_p          ModifyContract(avtContract_p);
    virtual void                   Execute(void);

    void                           SetupExpressionList(void);
    void                           RestoreExpressionList(void);
};


#endif


