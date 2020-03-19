// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtModuloExpression.h                       //
// ************************************************************************* //

#ifndef AVT_MODULO_FILTER_H
#define AVT_MODULO_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtModuloExpression
//
//  Purpose:
//      A filter that calculates the first variable module the second variable.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtModuloExpression : public avtBinaryMathExpression
{
  public:
                              avtModuloExpression();
    virtual                  ~avtModuloExpression();

    virtual const char       *GetType(void) { return "avtModuloExpression"; };
    virtual const char       *GetDescription(void)
                                            { return "Calculating modulo"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


