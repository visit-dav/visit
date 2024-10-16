// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtFloorExpression.h                         //
// ************************************************************************* //

#ifndef AVT_FLOOR_FILTER_H
#define AVT_FLOOR_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtFloorExpression
//
//  Purpose:
//      A filter that calculates the floor of a floating point quantity.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtFloorExpression : public avtUnaryMathExpression
{
  public:
                              avtFloorExpression();
    virtual                  ~avtFloorExpression();

    virtual const char       *GetType(void)  { return "avtFloorExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating the floor"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


