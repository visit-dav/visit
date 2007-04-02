// ************************************************************************* //
//                       avtExpressionComponentMacro.h                       //
// ************************************************************************* //

#include <avtExpressionComponentMacro.h>

#include <stdio.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtExpressionComponentMacro constructor
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtExpressionComponentMacro::avtExpressionComponentMacro()
{
    expr = "<NotSet>";
}


// ****************************************************************************
//  Method: avtExpressionComponentMacro destructor
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtExpressionComponentMacro::~avtExpressionComponentMacro()
{
    ;
}


// ****************************************************************************
//  Method: avtExpressionComponentMacro::SetMacro
//
//  Purpose:
//      Sets the arguments to use in the macro.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

void
avtExpressionComponentMacro::SetMacro(const std::string &e, int c)
{
    expr = e;
    comp = c;
}


// ****************************************************************************
//  Method: avtExpressionComponentMacro::GetMacro
//
//  Purpose:
//      Uses macros to apply an expression and then take a component of the
//      result.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

void
avtExpressionComponentMacro::GetMacro(std::vector<std::string> &args, 
                                  std::string &ne, Expression::ExprType &type)
{
    if (expr == "<NotSet>")
        EXCEPTION0(ImproperUseException);

    char new_expr[1024];
    sprintf(new_expr, "%s(%s)[%d]", expr.c_str(), args[0].c_str(), comp);
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


