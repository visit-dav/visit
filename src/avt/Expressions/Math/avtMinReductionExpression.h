// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMinReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_MIN_REDUCE_FILTER_H
#define AVT_MIN_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMinReductionExpression
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

class EXPRESSION_API avtMinReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtMinReductionExpression();
    virtual                  ~avtMinReductionExpression();

    virtual const char       *GetType(void)   { return "avtMinReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating max across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


