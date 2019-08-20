// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtArctan2Expression.h                             //
// ************************************************************************* //

#ifndef AVT_ARCTAN2_FILTER_H
#define AVT_ARCTAN2_FILTER_H


#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtArctan2Expression
//
//  Purpose:
//      A filter that calculates the atan2 of its two inputs.
//
//  Programmer: Sean Ahern
//  Creation:   May 8, 2007
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtArctan2Expression : public avtBinaryMathExpression
{
  public:
                              avtArctan2Expression();
    virtual                  ~avtArctan2Expression();

    virtual const char       *GetType(void)   { return "avtArctan2Expression"; };
    virtual const char       *GetDescription(void) 
                                          { return "Calculating atan2"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in1,
                                          vtkDataArray *in2,
                                          vtkDataArray *out, int
                                          ncomponents, int ntuples);
};


#endif


