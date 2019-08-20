// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSymmTransformExpression.h                      //
// ************************************************************************* //

#ifndef AVT_SYMM_TRANSFORM_EXPRESSION_H
#define AVT_SYMM_TRANSFORM_EXPRESSION_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtSymmTransformExpression
//
//  Purpose:
//      Uses the EvalTransformExpression to calculate differences.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtSymmTransformExpression : public avtMacroExpressionFilter
{
  public:
                              avtSymmTransformExpression();
    virtual                  ~avtSymmTransformExpression();

    virtual const char       *GetType(void) 
                               { return "avtSymmTransformExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating the SymmTransform"; };

  protected:
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

