// ************************************************************************* //
//                             avtMatErrorFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MAT_ERROR_FILTER_H
#define AVT_MAT_ERROR_FILTER_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtMatErrorFilter
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

class EXPRESSION_API avtMatErrorFilter : public avtMacroExpressionFilter
{
  public:
                              avtMatErrorFilter();
    virtual                  ~avtMatErrorFilter();

    virtual const char       *GetType(void)  { return "avtMatErrorFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating divergence"; };

  protected:
    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                  std::string &ne, Expression::ExprType &type);
};


#endif


