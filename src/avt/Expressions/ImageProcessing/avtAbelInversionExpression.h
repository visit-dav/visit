// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtAbelInversionExpression.h                     //
// ************************************************************************* //

#ifndef AVT_ABEL_INVERSION_EXPRESSION_H
#define AVT_ABEL_INVERSION_EXPRESSION_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtAbelInversionExpression
//
//  Purpose:
//      A filter that calculates the inverse of the Abel transform for
//      the projection of axially symmetric data.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

class EXPRESSION_API avtAbelInversionExpression : public avtUnaryMathExpression
{
  public:
                              avtAbelInversionExpression();
    virtual                  ~avtAbelInversionExpression();

    virtual const char       *GetType(void) 
                                 { return "avtAbelInversionExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating inverse of Abel transform"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                                 int ncomps, int ntuples, vtkDataSet *in_ds);
};


#endif


