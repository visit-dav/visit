// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMaxReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_MAX_REDUCE_FILTER_H
#define AVT_MAX_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMaxReductionExpression
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

class EXPRESSION_API avtMaxReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtMaxReductionExpression();
    virtual                  ~avtMaxReductionExpression();

    virtual const char       *GetType(void)   { return "avtMaxReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating max across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


