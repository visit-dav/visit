// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCoshExpression.h                              //
// ************************************************************************* //

#ifndef AVT_COSH_FILTER_H
#define AVT_COSH_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtCoshExpression
//
//  Purpose:
//      A filter that calculates the hyperbolic cosine of its input.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 27, 2009 
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtCoshExpression : public avtUnaryMathExpression
{
  public:
                              avtCoshExpression();
    virtual                  ~avtCoshExpression();

    virtual const char       *GetType(void)   
                                  { return "avtCoshExpression"; }
    virtual const char       *GetDescription(void) 
                                  { return "Calculating hyperbolic cosine"; }

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


