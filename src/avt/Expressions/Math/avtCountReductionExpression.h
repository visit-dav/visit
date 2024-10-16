// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtCountReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_COUNT_REDUCE_FILTER_H
#define AVT_COUNT_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtCountReductionExpression
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

class EXPRESSION_API avtCountReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtCountReductionExpression();
    virtual                  ~avtCountReductionExpression();

    virtual const char       *GetType(void)   { return "avtCountReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating count across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


