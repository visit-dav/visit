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
//  Modifications:
//
//    Hank Childs, Mon Jun  6 11:21:23 PDT 2005
//    Add support for 2D.
//
// ****************************************************************************

void
avtDivergenceFilter::GetMacro(std::vector<std::string> &args, std::string &ne,
                              Expression::ExprType &type)
{
    bool do3D = true;

    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        do3D   = (atts.GetTopologicalDimension() == 3);
    }

    char new_expr[1024];
    if (do3D)
        sprintf(new_expr, "gradient(%s[0])[0]+gradient(%s[1])[1]+"
                          "gradient(%s[2])[2]",
                           args[0].c_str(), args[0].c_str(), args[0].c_str());
    else
        sprintf(new_expr, "gradient(%s[0])[0]+gradient(%s[1])[1]",
                           args[0].c_str(), args[0].c_str());
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


