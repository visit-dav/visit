// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtVarianceReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_VARIANCE_REDUCE_FILTER_H
#define AVT_VARIANCE_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVarianceReductionExpression
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtVarianceReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtVarianceReductionExpression();
    virtual                  ~avtVarianceReductionExpression();

    virtual const char       *GetType(void)   { return "avtVarianceReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating variance across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


