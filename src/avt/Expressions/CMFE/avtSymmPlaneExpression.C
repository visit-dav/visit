// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSymmPlaneExpression.h                         //
// ************************************************************************* //

#include <avtSymmPlaneExpression.h>

#include <stdio.h>

#include <Expression.h>
#include <avtExpressionTypeConversions.h>


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
//      plane symmetry.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 10:31:22 PST 2007
//    Do not assume that the expression created is a scalar.
//
//    Brad Whitlock, Tue Jan 20 16:24:40 PST 2009
//    I changed a conversion function name.
//
// ****************************************************************************

void
avtSymmPlaneExpression::GetMacro(std::vector<std::string> &args, 
                                 std::string &ne, Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "%s - eval_plane(%s, %s, %s)",
                 args[0].c_str(), args[0].c_str(), args[0].c_str(),
                 args[1].c_str());
    ne = new_expr;
    avtVarType et = DetermineVariableType(args[0]);
    type = avtVarType_To_ExprType(et);
    if (type == Expression::Unknown)
        type = Expression::ScalarMeshVar;
}


