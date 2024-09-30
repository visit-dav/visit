// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSumReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_SUM_REDUCE_FILTER_H
#define AVT_SUM_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtSumReductionExpression
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

class EXPRESSION_API avtSumReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtSumReductionExpression();
    virtual                  ~avtSumReductionExpression();

    virtual const char       *GetType(void)   { return "avtSumReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating average across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


