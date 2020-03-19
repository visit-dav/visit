// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
// ************************************************************************* //
//                          avtBinaryAndExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_AND_EXPRESSION_H
#define AVT_BINARY_AND_EXPRESSION_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryAndExpression
//
//  Purpose:
//      A filter that calculates the sum of its two inputs.
//
//  Programmer: Mark C. Miller 
//  Creation:   Mon Mar 31 19:39:25 PDT 2008
//
// ****************************************************************************

class EXPRESSION_API avtBinaryAndExpression : public avtBinaryMathExpression
{
  public:
                              avtBinaryAndExpression();
    virtual                  ~avtBinaryAndExpression();

    virtual const char       *GetType(void)   { return "avtBinaryAndExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating binary bitwise and"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


