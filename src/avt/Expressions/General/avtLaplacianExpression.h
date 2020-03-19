// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtLaplacianExpression.h                          //
// ************************************************************************* //

#ifndef AVT_LAPLACIAN_FILTER_H
#define AVT_LAPLACIAN_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtLaplacianExpression
//
//  Purpose:
//      A filter that calculates the Laplacian.  The Laplacian takes in a 
//      scalar and produces a scalar.  The Laplacian is the divergence of
//      a gradient, which is why we need to be a derived type of the macro
//      expression filter.
//
//      Laplacian of scalar S  = divergence(gradient(S))
//
//      If F describes temperature, Laplacian(F) gives information about
//      the gain or loss of heat in a region.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

class EXPRESSION_API avtLaplacianExpression : public avtMacroExpressionFilter
{
  public:
                              avtLaplacianExpression();
    virtual                  ~avtLaplacianExpression();

    virtual const char       *GetType(void)  { return "avtLaplacianExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating the Laplacian"; };

  protected:
    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

