// ************************************************************************* //
//                        avtExpressionComponentMacro.h                      //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_COMPONENT_MACRO_H
#define AVT_EXPRESSION_COMPONENT_MACRO_H


#include <avtMacroExpressionFilter.h>


// ****************************************************************************
//  Class: avtExpressionComponentMacro
//
//  Purpose:
//      A macro that performs an expression an then takes a component.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtExpressionComponentMacro 
    : public avtMacroExpressionFilter
{
  public:
                              avtExpressionComponentMacro();
    virtual                  ~avtExpressionComponentMacro();

    virtual const char       *GetType(void) 
                               { return "avtExpressionComponentMacro"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating macro"; };
    void                      SetMacro(const std::string &, int);

  protected:
    std::string               expr;
    int                       comp;

    virtual int               GetVariableDimension() { return 1; }
    virtual void              GetMacro(std::vector<std::string> &,
                                 std::string &ne, Expression::ExprType &type);
};


#endif

