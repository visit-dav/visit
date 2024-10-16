// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtCurveIntegrateExpression.h                        //
// ************************************************************************* //

#ifndef AVT_CURVEINTEGRATE_EXPRESSION_H
#define AVT_CURVEINTEGRATE_EXPRESSION_H

#include <expression_exports.h>

#include <string>

#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtCurveIntegrateExpression
//
//  Purpose:
//      Integrate a curve.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2012
//
// ****************************************************************************

class EXPRESSION_API avtCurveIntegrateExpression 
    : virtual public avtUnaryMathExpression
{
  public:
                             avtCurveIntegrateExpression();
    virtual                 ~avtCurveIntegrateExpression();

    virtual const char      *GetType() { return "avtCurveIntegrateExpression"; }
    virtual const char      *GetDescription() 
                                 { return "Integrate a curve"; }

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual avtVarType       GetVariableType(void) { return AVT_CURVE; };
};


#endif
