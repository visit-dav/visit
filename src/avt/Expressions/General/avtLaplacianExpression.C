// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtLaplacianExpression.C                        //
// ************************************************************************* //

#include <avtLaplacianExpression.h>

#include <snprintf.h>
#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtLaplacianExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtLaplacianExpression::avtLaplacianExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtLaplacianExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtLaplacianExpression::~avtLaplacianExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtLaplacianExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      the Laplacian.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
//  Modifications:
//
//    Cyrus Harrison, Sat Aug 11 18:45:53 PDT 2007
//    Add second argument for gradient algorithm selection
//
//    Hank Childs, Wed May  7 16:20:59 PDT 2008
//    Add support for rectilinear Laplacian, which only requires one layer
//    of data.
//
// ****************************************************************************

void
avtLaplacianExpression::GetMacro(std::vector<std::string> &args, 
                                 std::string &ne, Expression::ExprType &type)
{
    char new_expr[2048];
    size_t nargs = args.size();
    if(nargs == 1)
    {
        avtMeshType mt = GetInput()->GetInfo().GetAttributes().GetMeshType();
        if (mt == AVT_RECTILINEAR_MESH || mt == AVT_AMR_MESH)
        {
            SNPRINTF(new_expr, 2048, "rectilinear_laplacian(%s)", 
                                     args[0].c_str());
        }
        else
        {
            SNPRINTF(new_expr, 2048, "divergence(gradient(%s))", 
                                     args[0].c_str());
        }
    }
    else if (nargs == 2)
    {
        SNPRINTF(new_expr, 2048,
                "divergence(gradient(%s,%s))", 
                args[0].c_str(),args[1].c_str());
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                    " invalid laplacian syntax. "
                    "Expected arguments: "
                    "var, gradient_algorithm\n"
                    "[gradient_algorithm is optional]");
    }
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


