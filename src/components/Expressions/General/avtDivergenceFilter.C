// ************************************************************************* //
//                            avtDivergenceFilter.h                          //
// ************************************************************************* //

#include <avtDivergenceFilter.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtDivergenceFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtDivergenceFilter::avtDivergenceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtDivergenceFilter::~avtDivergenceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceFilter::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      divergence.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

void
avtDivergenceFilter::GetMacro(std::vector<std::string> &args, std::string &ne,
                              Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "gradient(%s[0])[0]+gradient(%s[1])[1]+"
                      "gradient(%s[2])[2]",
                       args[0].c_str(), args[0].c_str(), args[0].c_str());
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


