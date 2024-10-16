// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtExpExpression.h                          //
// ************************************************************************* //

#ifndef AVT_EXP_FILTER_H
#define AVT_EXP_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtExpExpression
//
//  Purpose:
//      A filter that calculates the e^x expression.
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 27 15:13:05 EDT 2008
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtExpExpression : public avtUnaryMathExpression
{
  public:
                              avtExpExpression();
    virtual                  ~avtExpExpression();

    virtual const char       *GetType(void)   { return "avtExpExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating exp function"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};

#endif
