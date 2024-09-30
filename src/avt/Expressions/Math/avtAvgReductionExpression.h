// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtAvgReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_AVG_REDUCE_FILTER_H
#define AVT_AVG_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtAvgReductionExpression
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

class EXPRESSION_API avtAvgReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtAvgReductionExpression();
    virtual                  ~avtAvgReductionExpression();

    virtual const char       *GetType(void)   { return "avtAvgReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating average across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


