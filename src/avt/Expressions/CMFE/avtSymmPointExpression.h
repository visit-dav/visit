// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtSymmPointExpression.h                        //
// ************************************************************************* //

#ifndef AVT_SYMM_POINT_EXPRESSION_H
#define AVT_SYMM_POINT_EXPRESSION_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtSymmPointExpression
//
//  Purpose:
//      Uses the EvalPointExpression to calculate differences.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtSymmPointExpression : public avtMacroExpressionFilter
{
  public:
                              avtSymmPointExpression();
    virtual                  ~avtSymmPointExpression();

    virtual const char       *GetType(void) 
                               { return "avtSymmPointExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating the SymmPoint"; };

  protected:
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

