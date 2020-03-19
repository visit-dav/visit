// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtArrayComposeWithBinsExpression.h                     //
// ************************************************************************* //

#ifndef AVT_ARRAY_COMPOSE_WITH_BINS_FILTER_H
#define AVT_ARRAY_COMPOSE_WITH_BINS_FILTER_H

#include <avtMultipleInputExpressionFilter.h>

#include <vector>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;


// ****************************************************************************
//  Class: avtArrayComposeWithBinsExpression
//
//  Purpose:
//      ComposeWithBinss scalar variables into an array.
//          
//  Programmer: Hank Childs
//  Creation:   January 12, 2007
//
// ****************************************************************************

class EXPRESSION_API avtArrayComposeWithBinsExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtArrayComposeWithBinsExpression();
    virtual                  ~avtArrayComposeWithBinsExpression();

    virtual const char       *GetType(void) 
                                 { return "avtArrayComposeWithBinsExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Composing an array"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return nvars; };

  protected:
    int                       nvars;
    std::vector<double>       binRanges;

    virtual void              UpdateDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_ARRAY_VAR; };
};


#endif


