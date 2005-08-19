// ************************************************************************* //
//                               avtCurlFilter.h                             //
// ************************************************************************* //

#include <avtCurlFilter.h>

#include <stdio.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtCurlFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtCurlFilter::avtCurlFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtCurlFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtCurlFilter::~avtCurlFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceFilter::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to curl.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
//  Modifications:
//
//    Hank Childs, Mon Jun  6 11:25:51 PDT 2005
//    Add error checking for 2D.
//
//    Hank Childs, Fri Aug 12 13:48:51 PDT 2005
//    Add support for 2D curl (which is a scalar).
//
//    Hank Childs, Fri Aug 19 09:20:24 PDT 2005
//    If we are creating a scalar, then make sure the expression type is a 
//    scalar as well.
//
// ****************************************************************************

void
avtCurlFilter::GetMacro(std::vector<std::string> &args, std::string &ne, 
                        Expression::ExprType &type)
{
    bool do3D = true;
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        if (atts.GetTopologicalDimension() < 3)
        {
            do3D = false;
        }
    }

    char new_expr[1024];
    if (do3D)
    {
        sprintf(new_expr, "{gradient(%s[2])[1]-gradient(%s[1])[2],"
                          "gradient(%s[0])[2]-gradient(%s[2])[0],"
                          "gradient(%s[1])[0]-gradient(%s[0])[1]}",
                           args[0].c_str(), args[0].c_str(), args[0].c_str(),
                           args[0].c_str(), args[0].c_str(), args[0].c_str());
        type = Expression::VectorMeshVar;
    }
    else
    {
        sprintf(new_expr, "gradient(%s[1])[0]-gradient(%s[0])[1]",
                           args[0].c_str(), args[0].c_str());
        type = Expression::ScalarMeshVar;
    }
    ne = new_expr;
}


// ****************************************************************************
//  Method: avtDivergenceFilter::GetVariableDimension
//
//  Purpose:
//      Gets the variable dimension -- scalar for 2D datasets, vector for 3D.
//
//  Programmer: Hank Childs
//  Creation:   August 19, 2005
//
// ****************************************************************************

int
avtCurlFilter::GetVariableDimension(void)
{
    bool do3D = true;
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        if (atts.GetTopologicalDimension() < 3)
        {
            do3D = false;
        }
    }

    return (do3D ? 3 : 1);
}


