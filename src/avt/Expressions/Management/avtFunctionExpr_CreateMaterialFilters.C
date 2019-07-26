// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtDominantMaterialExpression.h>
#include <avtMIRvfExpression.h>
#include <avtMatErrorExpression.h>
#include <avtMatvfExpression.h>
#include <avtNMatsExpression.h>
#include <avtPerMaterialValueExpression.h>
#include <avtSpecMFExpression.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateMaterialFilters
//
// Purpose: 
//   Creates material filters.
//
// Arguments:
//   functionName : The name of the expression filter to create.
//
// Returns:    An expression filter or 0 if one could not be created.
//
// Note:       
//
// Programmer: 
// Creation:   Thu May 21 08:55:58 PDT 2009
//
// Modifications:
//   
//   Hank Childs, Mon Nov  9 13:50:43 PST 2009
//   Added dominant_mat.
//
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateMaterialFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "matvf")
        f = new avtMatvfExpression();
    else if (functionName == "mirvf")
        f = new avtMIRvfExpression();
    else if (functionName == "materror")
        f = new avtMatErrorExpression();
    else if (functionName == "value_for_material")
        f = new avtPerMaterialValueExpression();     
    else if (functionName == "val4mat")
        f = new avtPerMaterialValueExpression();
    else if (functionName == "specmf")
        f = new avtSpecMFExpression();
    else if (functionName == "nmats")
        f = new avtNMatsExpression();
    else if (functionName == "dominant_mat")
        f = new avtDominantMaterialExpression();

    return f;
}
