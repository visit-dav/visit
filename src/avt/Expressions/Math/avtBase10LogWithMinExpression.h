// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtBase10LogWithMinExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BASE10LOG_WITH_MIN_FILTER_H
#define AVT_BASE10LOG_WITH_MIN_FILTER_H


#include <avtBinaryMathExpression.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;


// ****************************************************************************
//  Class: avtBase10LogWithMinExpression
//
//  Purpose:
//      A filter that calculates the base 10 logarithm of its input and also
//      take a minimum value.  This expression can be done with other 
//      expressions, but is more efficiently implemented.
//
//  Programmer: Hank Childs
//  Creation:   May 20, 2010
//
//  Modifications:
//
//    Hank Childs, Thu Sep 23 14:10:45 PDT 2010
//    Indicate this expression can deal with singletons (improves performance).
//
// ****************************************************************************

class EXPRESSION_API avtBase10LogWithMinExpression 
                                               : public avtBinaryMathExpression
{
  public:
                           avtBase10LogWithMinExpression();
    virtual               ~avtBase10LogWithMinExpression();

    virtual const char    *GetType(void)   { return "avtBase10LogWithMinExpression"; };
    virtual const char    *GetDescription(void) 
                                { return "Calculating base 10 logarithm"; };

  protected:
    virtual void           DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                       vtkDataArray *out, int ncomps,int ntups);
    virtual bool           CanHandleSingleton(void) { return true; };
};


#endif


