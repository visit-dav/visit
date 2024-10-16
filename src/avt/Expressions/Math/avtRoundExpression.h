// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtRoundExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ROUND_FILTER_H
#define AVT_ROUND_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRoundExpression
//
//  Purpose:
//      A filter that rounds up or down a floating point quantity.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtRoundExpression : public avtUnaryMathExpression
{
  public:
                              avtRoundExpression();
    virtual                  ~avtRoundExpression();

    virtual const char       *GetType(void)  { return "avtRoundExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Rounding"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


