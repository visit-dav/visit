// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtDivergenceExpression.C                          //
// ************************************************************************* //

#include <avtDivergenceExpression.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtDivergenceExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtDivergenceExpression::avtDivergenceExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtDivergenceExpression::~avtDivergenceExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      divergence.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
//  Modifications:
//
//    Hank Childs, Mon Jun  6 11:21:23 PDT 2005
//    Add support for 2D.
//
//    Cyrus Harrison, Sat Aug 11 18:34:41 PDT 2007
//    Add second argument for gradient algorithm selection
//
// ****************************************************************************

void
avtDivergenceExpression::GetMacro(std::vector<std::string> &args, 
    std::string &ne, Expression::ExprType &type)
{
    bool do3D = true;

    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        do3D   = (atts.GetTopologicalDimension() == 3);
    }

    size_t nargs = args.size();

    char new_expr[2048];
    if (do3D)
    {
        if(nargs == 1)
        {
            snprintf(new_expr, 2048,
                    "gradient(%s[0])[0]+gradient(%s[1])[1]+"
                    "gradient(%s[2])[2]",
                    args[0].c_str(), args[0].c_str(), args[0].c_str());
        }
        else if(nargs > 1)
        {
            snprintf(new_expr, 2048,
                    "gradient(%s[0],%s)[0]+gradient(%s[1],%s)[1]+"
                    "gradient(%s[2],%s)[2]",
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str());            
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                        " invalid divergence syntax. "
                        "Expected arguments: "
                        " vector_var, gradient_algorithm\n"
                        " gradient_algorithm is optional");
        }
    }
    else
    {
        if(nargs == 1)
        {
            snprintf(new_expr, 2048,
                    "gradient(%s[0])[0]+gradient(%s[1])[1]",
                    args[0].c_str(), args[0].c_str());
        }
        else if(nargs > 1)
        {
            snprintf(new_expr, 2048,
                    "gradient(%s[0],%s)[0]+gradient(%s[1],%s)[1]",
                    args[0].c_str(), args[1].c_str(), 
                    args[0].c_str(), args[1].c_str()); 
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                        " invalid divergence syntax. "
                        "Expected arguments: "
                        " vector_var, gradient_algorithm\n"
                        " gradient_algorithm is optional");
        }
    }
    
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


