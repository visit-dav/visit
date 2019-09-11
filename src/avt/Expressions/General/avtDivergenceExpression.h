// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtDivergenceExpression.h                          //
// ************************************************************************* //

#ifndef AVT_DIVERGENCE_FILTER_H
#define AVT_DIVERGENCE_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtDivergenceExpression
//
//  Purpose:
//      A filter that calculates the divergence.  Divergence takes in a vector 
//      and produces a scalar.  This depends on several partial derivatives, 
//      which are accomplished using the gradient expression.
//
//      Because we need to use other expressions, this is a derived type of
//      the macro expression filter.
//
//      divergence of vector {u,v,w} = grad(u)[0]+grad(v)[1]+grad(w)[2]
//
//      Divergence is the tendency of a fluid to accumulate or spread out at
//      any given point.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

class EXPRESSION_API avtDivergenceExpression : public avtMacroExpressionFilter
{
  public:
                              avtDivergenceExpression();
    virtual                  ~avtDivergenceExpression();

    virtual const char       *GetType(void)  { return "avtDivergenceExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating divergence"; };

  protected:
    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

