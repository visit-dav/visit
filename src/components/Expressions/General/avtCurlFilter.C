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
// ****************************************************************************

void
avtCurlFilter::GetMacro(std::vector<std::string> &args, std::string &ne, 
                        Expression::ExprType &type)
{
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        if (atts.GetTopologicalDimension() < 3)
        {
            EXCEPTION1(ExpressionException, "You can only take the curl of 3D"
                              " data sets.  The input is not a 3D data set.");
        }
    }

    char new_expr[1024];
    sprintf(new_expr, "{gradient(%s[2])[1]-gradient(%s[1])[2],"
                      "gradient(%s[0])[2]-gradient(%s[2])[0],"
                      "gradient(%s[1])[0]-gradient(%s[0])[1]}",
                       args[0].c_str(), args[0].c_str(), args[0].c_str(),
                       args[0].c_str(), args[0].c_str(), args[0].c_str());
    ne = new_expr;
    type = Expression::VectorMeshVar;
}


