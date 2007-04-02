// ************************************************************************* //
//                        avtSymmTransformExpression.h                       //
// ************************************************************************* //

#include <avtSymmTransformExpression.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtSymmTransformExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmTransformExpression::avtSymmTransformExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmTransformExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmTransformExpression::~avtSymmTransformExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmTransformExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      the Laplacian.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

void
avtSymmTransformExpression::GetMacro(std::vector<std::string> &args, std::string &ne,
                             Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "%s - eval_transform(%s, %s, %s)",
                 args[0].c_str(), args[0].c_str(), args[0].c_str(),
                 args[1].c_str());
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


