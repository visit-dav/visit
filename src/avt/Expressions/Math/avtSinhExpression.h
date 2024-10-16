// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSinhExpression.h                              //
// ************************************************************************* //

#ifndef AVT_SINH_FILTER_H
#define AVT_SINH_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtSinhExpression
//
//  Purpose:
//      A filter that calculates the hyperbolic sine of its input.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 27, 2009
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtSinhExpression : public avtUnaryMathExpression
{
  public:
                              avtSinhExpression();
    virtual                  ~avtSinhExpression();

    virtual const char       *GetType(void)   
                                  { return "avtSinhExpression"; }
    virtual const char       *GetDescription(void) 
                                  { return "Calculating hyperbolic sine"; }

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


