// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtArraySumExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ARRAY_SUM_FILTER_H
#define AVT_ARRAY_SUM_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtArraySumExpression
//
//  Purpose:
//      Extract a scalar variables from an array.
//          
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
//  Modifications:
//
//    Hank Childs, Sun Feb  3 13:43:20 PST 2008
//    Declare the variable dimension.
//
// ****************************************************************************

class EXPRESSION_API avtArraySumExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtArraySumExpression();
    virtual                  ~avtArraySumExpression();

    virtual const char       *GetType(void) 
                                     { return "avtArraySumExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Sum over an an array"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    bool issuedWarning;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual void              PreExecute(void);
    virtual int               GetVariableDimension(void) { return 1; };
    virtual avtVarType        GetVariableType(void) { return AVT_SCALAR_VAR; };
};


#endif


