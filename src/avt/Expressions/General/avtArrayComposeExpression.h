// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtArrayComposeExpression.h                          //
// ************************************************************************* //

#ifndef AVT_ARRAY_COMPOSE_FILTER_H
#define AVT_ARRAY_COMPOSE_FILTER_H

#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtArrayComposeExpression
//
//  Purpose:
//      Composes scalar variables into an array.
//          
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug  5 16:48:24 PDT 2005
//    Add support for types and subnames.
//
// ****************************************************************************

class EXPRESSION_API avtArrayComposeExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtArrayComposeExpression();
    virtual                  ~avtArrayComposeExpression();

    virtual const char       *GetType(void) { return "avtArrayComposeExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Composing an array"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return nargs; };

  protected:
    int                       nargs;

    virtual void              UpdateDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_ARRAY_VAR; };
};


#endif


