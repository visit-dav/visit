// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtArrayDecomposeExpression.h                         //
// ************************************************************************* //

#ifndef AVT_ARRAY_DECOMPOSE_FILTER_H
#define AVT_ARRAY_DECOMPOSE_FILTER_H

#include <string>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtArrayDecomposeExpression
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

class EXPRESSION_API avtArrayDecomposeExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtArrayDecomposeExpression();
    virtual                  ~avtArrayDecomposeExpression();

    virtual const char       *GetType(void) 
                                     { return "avtArrayDecomposeExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Decomposing an array"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    bool issuedWarning;
    int  index;
    std::string indexStr;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual void              PreExecute(void);
    virtual int               GetVariableDimension(void) { return 1; };
    virtual avtVarType        GetVariableType(void) { return AVT_SCALAR_VAR; };
};


#endif


