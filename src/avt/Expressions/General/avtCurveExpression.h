// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurveExpression.h                             //
// ************************************************************************* //

#ifndef AVT_CURVE_EXPRESSION_H
#define AVT_CURVE_EXPRESSION_H


#include <avtMacroExpressionFilter.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtCurveExpression
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 5, 2009
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtCurveExpression : public avtMacroExpressionFilter
{
  public:
                              avtCurveExpression();
    virtual                  ~avtCurveExpression();

    virtual const char       *GetType(void)   { return "avtCurveExpression"; }
    virtual const char       *GetDescription(void)
                               { return "Calculating Curve"; }

  protected:
    virtual int               GetVariableDimension() { return 1;}
    virtual void              GetMacro(std::vector<std::string> &, 
                                       std::string &, Expression::ExprType &);

    virtual void              ProcessArguments(ArgsExpr *args, 
                                               ExprPipelineState *state);
  private:
    int                       IsYFunc(const char *var) const;
    int                       IsXFunc(const char *var) const;
    bool                      ValidFunctionName(const char *func);
    int                       xvar;
    std::string               function;
};


#endif

