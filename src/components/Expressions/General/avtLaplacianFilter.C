// ************************************************************************* //
//                             avtLaplacianFilter.h                          //
// ************************************************************************* //

#include <avtLaplacianFilter.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtLaplacianFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtLaplacianFilter::avtLaplacianFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtLaplacianFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtLaplacianFilter::~avtLaplacianFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtLaplacianFilter::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      the Laplacian.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

void
avtLaplacianFilter::GetMacro(std::vector<std::string> &args, std::string &ne,
                             Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "divergence(gradient(%s))", args[0].c_str());
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


