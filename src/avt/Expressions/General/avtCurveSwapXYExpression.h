// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtCurveSwapXYExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CURVESWAPXY_EXPRESSION_H
#define AVT_CURVESWAPXY_EXPRESSION_H

#include <expression_exports.h>

#include <string>

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtCurveSwapXYExpression
//
//  Purpose:
//      Swap the x and y coordinates of a curve.
//
//  Programmer: Eric Brugger
//  Creation:   August 27, 2012
//
// ****************************************************************************

class EXPRESSION_API avtCurveSwapXYExpression 
    : virtual public avtSingleInputExpressionFilter
{
  public:
                             avtCurveSwapXYExpression();
    virtual                 ~avtCurveSwapXYExpression();

    virtual const char      *GetType() { return "avtCurveSwapXYExpression"; }
    virtual const char      *GetDescription() 
                                 { return "Swap the x and y coordinates of a curve"; }

  protected:

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *, int currentDomainsIndex) { return NULL;}
    virtual avtVarType       GetVariableType(void) { return AVT_CURVE; };
};


#endif
