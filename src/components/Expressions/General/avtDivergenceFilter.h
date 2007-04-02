// ************************************************************************* //
//                            avtDivergenceFilter.h                          //
// ************************************************************************* //

#ifndef AVT_DIVERGENCE_FILTER_H
#define AVT_DIVERGENCE_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtDivergenceFilter
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

class EXPRESSION_API avtDivergenceFilter : public avtMacroExpressionFilter
{
  public:
                              avtDivergenceFilter();
    virtual                  ~avtDivergenceFilter();

    virtual const char       *GetType(void)  { return "avtDivergenceFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating divergence"; };

  protected:
    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

