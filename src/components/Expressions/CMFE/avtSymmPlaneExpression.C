// ************************************************************************* //
//                        avtSymmPlaneExpression.h                       //
// ************************************************************************* //

#include <avtSymmPlaneExpression.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtSymmPlaneExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmPlaneExpression::avtSymmPlaneExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmPlaneExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmPlaneExpression::~avtSymmPlaneExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmPlaneExpression::GetMacro
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
avtSymmPlaneExpression::GetMacro(std::vector<std::string> &args, std::string &ne,
                             Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "%s - eval_plane(%s, %s, %s)",
                 args[0].c_str(), args[0].c_str(), args[0].c_str(),
                 args[1].c_str());
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


