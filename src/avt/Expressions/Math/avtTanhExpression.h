// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTanhExpression.h                             //
// ************************************************************************* //

#ifndef AVT_TANH_FILTER_H
#define AVT_TANH_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTanhExpression
//
//  Purpose:
//      A filter that calculates the hyperbolic tangent of its input.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 27, 2009
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtTanhExpression : public avtUnaryMathExpression
{
  public:
                              avtTanhExpression();
    virtual                  ~avtTanhExpression();

    virtual const char       *GetType(void)  
                                  { return "avtTanhExpression"; }
    virtual const char       *GetDescription(void) 
                                  { return "Calculating hyperbolic tangent"; }

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


