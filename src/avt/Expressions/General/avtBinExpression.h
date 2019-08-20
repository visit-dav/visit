// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtBinExpression.h                                 //
// ************************************************************************* //

#ifndef AVT_BIN_EXPRESSION_H
#define AVT_BIN_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

#include <vector>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ExprParseTreeNode;
class     ListExpr;

// ****************************************************************************
//  Class: avtBinExpression
//
//  Purpose:
//      Map a set of numeric values specified by user bins to bin ids.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 12 16:49:00 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtBinExpression
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtBinExpression();
    virtual                  ~avtBinExpression();

    virtual const char       *GetType(void) 
                                 { return "avtBinExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Applying bin expression"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return 1; };

  protected:
    virtual vtkDataArray         *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType            GetVariableType(void) { return AVT_SCALAR_VAR;};
  private:
    std::vector<double>           bins;

    void                          ThrowError(const std::string &msg);
};


#endif


