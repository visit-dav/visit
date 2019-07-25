// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtMatErrorExpression.h                           //
// ************************************************************************* //

#ifndef AVT_MAT_ERROR_FILTER_H
#define AVT_MAT_ERROR_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtMatErrorExpression
//
//  Purpose:
//      A filter that calculates the relative difference in volume fraction
//      between what was specified in the material object and what our MIR
//      algorithm produced.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

class EXPRESSION_API avtMatErrorExpression : public avtMacroExpressionFilter
{
  public:
                              avtMatErrorExpression();
    virtual                  ~avtMatErrorExpression();

    virtual const char       *GetType(void)  { return "avtMatErrorExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating divergence"; };

  protected:
    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif


