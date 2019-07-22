// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtArrayComponentwiseDivisionExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ARRAY_COMPONENTWISE_DIVISION_FILTER_H
#define AVT_ARRAY_COMPONENTWISE_DIVISION_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtArrayComponentwiseDivisionExpression
//
//  Purpose:
//      A filter that computes the componentwise division of two arrays.
//
//  Programmer: Gunther H. Weber (based on avtBinaryAddExpression by Sean Ahern)
//  Creation:   March 2, 2015
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtArrayComponentwiseDivisionExpression : public avtBinaryMathExpression
{
  public:
                              avtArrayComponentwiseDivisionExpression();
    virtual                  ~avtArrayComponentwiseDivisionExpression();

    virtual const char       *GetType(void)   { return "avtArrayComponentwiseDivisionExpression"; };
    virtual const char       *GetDescription(void)
                                             { return "Calculating componentwise array division"; };

  protected:
    virtual avtVarType        GetVariableType();
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual bool     CanHandleSingletonConstants(void) { return true; };
};

#endif
