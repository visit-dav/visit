// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtBinaryModuloExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_MODULO_FILTER_H
#define AVT_BINARY_MODULO_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtBinaryModuloExpression
//
//  Purpose:
//      A filter that calculates the quotient of its two inputs
//
//  Mark C. Miller, Fri Jul 26 00:06:24 PDT 2024
// ****************************************************************************

class EXPRESSION_API avtBinaryModuloExpression : public avtBinaryMathExpression
{
  public:
                              avtBinaryModuloExpression();
    virtual                  ~avtBinaryModuloExpression();

    virtual const char       *GetType(void) 
                                     { return "avtBinaryModuloExpression"; };
    virtual const char       *GetDescription(void) 
                                     { return "Calculating binary division"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual bool     CanHandleSingletonConstants(void) {return true;};
};

#endif
