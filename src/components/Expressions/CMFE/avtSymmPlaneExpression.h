// ************************************************************************* //
//                           avtSymmPlaneExpression.h                        //
// ************************************************************************* //

#ifndef AVT_SYMM_PLANE_EXPRESSION_H
#define AVT_SYMM_PLANE_EXPRESSION_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtSymmPlaneExpression
//
//  Purpose:
//      Uses the EvalPlaneExpression to calculate differences.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtSymmPlaneExpression : public avtMacroExpressionFilter
{
  public:
                              avtSymmPlaneExpression();
    virtual                  ~avtSymmPlaneExpression();

    virtual const char       *GetType(void) 
                               { return "avtSymmPlaneExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating the SymmPlane"; };

  protected:
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

