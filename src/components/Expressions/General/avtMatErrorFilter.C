// ************************************************************************* //
//                            avtMatErrorFilter.h                          //
// ************************************************************************* //

#include <avtMatErrorFilter.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtMatErrorFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

avtMatErrorFilter::avtMatErrorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMatErrorFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

avtMatErrorFilter::~avtMatErrorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMatErrorFilter::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      material error.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

void
avtMatErrorFilter::GetMacro(std::vector<std::string> &args, std::string &ne,
                            Expression::ExprType &type)
{
    const char *matname  = args[0].c_str();
    const char *mats     = args[1].c_str();

    //
    // So this is a bit unfortunate.  GetMacro can be called before the input
    // is set and we can determine what the topological dimension is.  No
    // matter -- we really just want to get it right when we do the actual
    // execute (when we will have a valid input).  So, if the input is NULL,
    // claim we want "volume".  If the input is not NULL, choose between
    // "volume" and "area".
    //
    // A similar problem happens with the meshname.
    //
    const char *volume = "volume";
    const char *meshname = "";
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        volume   = (atts.GetTopologicalDimension() == 3 ? "volume" : "area");
        meshname = atts.GetMeshname().c_str();
    }
    char new_expr[1024];
    sprintf(new_expr, "relative_difference(matvf(%s, %s), "
                                      "mirvf(%s, zoneid(%s), %s(%s), %s))",
                           matname, mats, matname, meshname, volume,
                           meshname, mats);
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


