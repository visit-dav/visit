// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************** //
//                         avtNaturalLogExpression.h                          //
// ************************************************************************** //

#ifndef AVT_NATURALLOG_FILTER_H
#define AVT_NATURALLOG_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;
class ArgsExpr;
class ExprPipelineState;

// ****************************************************************************
//  Class: avtNaturalLogExpression
//
//  Purpose:
//      A filter that calculates the natural log of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Fri May  8 13:21:18 PDT 2009
//    Added defaultErrorValue, useDefaultOnError, and ProcessArguments.
//
// ****************************************************************************

class EXPRESSION_API avtNaturalLogExpression : public avtUnaryMathExpression
{
  public:
                              avtNaturalLogExpression();
    virtual                  ~avtNaturalLogExpression();

    virtual const char       *GetType(void)  { return "avtNaturalLogExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating natural log"; };
    virtual void              ProcessArguments(ArgsExpr *, ExprPipelineState *);

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    double                    defaultErrorValue;
    bool                      useDefaultOnError;
};


#endif


