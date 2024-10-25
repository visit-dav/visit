// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalMinExpression.h                    //
// ************************************************************************* //

#ifndef AVT_MIN_REDUCE_FILTER_H
#define AVT_MIN_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalMinExpression
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

class EXPRESSION_API avtGlobalMinExpression : public avtUnaryMathExpression
{
  public:
                              avtGlobalMinExpression();
    virtual                  ~avtGlobalMinExpression();

    virtual const char       *GetType(void)   { return "avtGlobalMinExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating min across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


