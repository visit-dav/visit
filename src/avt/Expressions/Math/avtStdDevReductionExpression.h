// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtStdDevReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_STD_DEV_REDUCE_FILTER_H
#define AVT_STD_DEV_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtStdDevReductionExpression
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

class EXPRESSION_API avtStdDevReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtStdDevReductionExpression();
    virtual                  ~avtStdDevReductionExpression();

    virtual const char       *GetType(void)   { return "avtStdDevReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating standard deviation across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


