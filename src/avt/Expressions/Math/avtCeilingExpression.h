// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCeilingExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CEILING_FILTER_H
#define AVT_CEILING_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtCeilingExpression
//
//  Purpose:
//      A filter that calculates the ceiling of a floating point quantity.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtCeilingExpression : public avtUnaryMathExpression
{
  public:
                              avtCeilingExpression();
    virtual                  ~avtCeilingExpression();

    virtual const char       *GetType(void)  { return "avtCeilingExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating the ceiling"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


