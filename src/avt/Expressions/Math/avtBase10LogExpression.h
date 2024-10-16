// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtBase10LogExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BASE10LOG_FILTER_H
#define AVT_BASE10LOG_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;


// ****************************************************************************
//  Class: avtBase10LogExpression
//
//  Purpose:
//      A filter that calculates the base 10 logarithm of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:23:54 PST 2002
//    Added support for vectors and non-float arrays.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Fri May  8 13:21:18 PDT 2009
//    Added defaultErrorValue, useDefaultOnError, and ProcessArguments.
//
// ****************************************************************************

class EXPRESSION_API avtBase10LogExpression : public avtUnaryMathExpression
{
  public:
                           avtBase10LogExpression();
    virtual               ~avtBase10LogExpression();

    virtual const char    *GetType(void)   { return "avtBase10LogExpression"; };
    virtual const char    *GetDescription(void) 
                                { return "Calculating base 10 logarithm"; };

    virtual void           ProcessArguments(ArgsExpr *, ExprPipelineState *);

  protected:
    virtual void           DoOperation(vtkDataArray *in, vtkDataArray *out,
                                       int ncomponents, int ntuples, vtkDataSet *in_ds);

    double                 defaultErrorValue;
    bool                   useDefaultOnError;
};


#endif


