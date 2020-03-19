// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtConstantFunctionExpression.h                          //
// ************************************************************************* //

#ifndef AVT_CONSTANT_FUNCTION_EXPRESSION_H
#define AVT_CONSTANT_FUNCTION_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtConstantFunctionExpression
//
//  Purpose:
//      Create a constant-valued nodal variable on a mesh.  Useful if you
//      want to create a constant on a mesh using only a single function.
//          
//  Programmer: Jeremy Meredith
//  Creation:   February 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 19 16:04:11 EST 2008
//    By having this function specify it only had one argument, it was
//    possible to clobber other constant creations.  I upped it to 2 and
//    had the second arg push a textual representation on the state.
//
//    Jeremy Meredith, Wed Feb 20 10:00:31 EST 2008
//    Support either nodal or zonal values.
//
//    Kathleen Biagas, Thu May  1 17:18:30 PDT 2014
//    Changed value to double.
//
// ****************************************************************************

class EXPRESSION_API avtConstantFunctionExpression
    : public avtMultipleInputExpressionFilter
{
  public:
                          avtConstantFunctionExpression(bool nodal);
    virtual              ~avtConstantFunctionExpression();

    virtual const char   *GetType() { return "avtConstantFunctionExpression"; }
    virtual const char   *GetDescription() { return "Assigning constant."; }
    virtual void          ProcessArguments(ArgsExpr*, ExprPipelineState *);
  protected:
    virtual vtkDataArray *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool          IsPointVariable()  { return nodal; }
    virtual int           GetVariableDimension() { return 1; }
    virtual int           NumVariableArguments(void) { return 2; }
  private:
    bool    nodal;
    double  value;
};


#endif


