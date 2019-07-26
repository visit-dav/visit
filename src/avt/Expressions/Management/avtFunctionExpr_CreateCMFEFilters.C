// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtConnCMFEExpression.h>
#include <avtCurveCMFEExpression.h>
#include <avtEvalPlaneExpression.h>
#include <avtEvalPointExpression.h>
#include <avtEvalTransformExpression.h>
#include <avtPosCMFEExpression.h>
#include <avtSymmPlaneExpression.h>
#include <avtSymmPointExpression.h>
#include <avtSymmTransformExpression.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateCMFEFilters
//
// Purpose: 
//   Creates CMFE expression filters.
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
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateCMFEFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "conn_cmfe")
        f = new avtConnCMFEExpression;
    else if (functionName == "curve_cmfe")
        f = new avtCurveCMFEExpression;
    else if (functionName == "pos_cmfe")
        f = new avtPosCMFEExpression;
    else if (functionName == "eval_transform")
        f = new avtEvalTransformExpression;
    else if (functionName == "symm_transform")
        f = new avtSymmTransformExpression;
    else if (functionName == "eval_plane")
        f = new avtEvalPlaneExpression;
    else if (functionName == "symm_plane")
        f = new avtSymmPlaneExpression;
    else if (functionName == "eval_point")
        f = new avtEvalPointExpression;
    else if (functionName == "symm_point")
        f = new avtSymmPointExpression;

    return f;
}
