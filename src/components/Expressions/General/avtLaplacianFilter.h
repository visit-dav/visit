// ************************************************************************* //
//                            avtLaplacianFilter.h                          //
// ************************************************************************* //

#ifndef AVT_LAPLACIAN_FILTER_H
#define AVT_LAPLACIAN_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtLaplacianFilter
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

class EXPRESSION_API avtLaplacianFilter : public avtMacroExpressionFilter
{
  public:
                              avtLaplacianFilter();
    virtual                  ~avtLaplacianFilter();

    virtual const char       *GetType(void)  { return "avtLaplacianFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating the Laplacian"; };

  protected:
    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif

