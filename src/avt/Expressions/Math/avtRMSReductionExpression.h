// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtRMSReductionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_RMS_REDUCE_FILTER_H
#define AVT_RMS_REDUCE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRMSReductionExpression
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

class EXPRESSION_API avtRMSReductionExpression : public avtUnaryMathExpression
{
  public:
                              avtRMSReductionExpression();
    virtual                  ~avtRMSReductionExpression();

    virtual const char       *GetType(void)   { return "avtRMSReductionExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating root mean square across mesh"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
};


#endif


