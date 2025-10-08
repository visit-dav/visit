// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtGlobalAvgExpression.h>
#include <avtGlobalMaxExpression.h>
#include <avtGlobalMinExpression.h>
#include <avtGlobalRMSExpression.h>
#include <avtGlobalStdDevExpression.h>
#include <avtGlobalSumExpression.h>
#include <avtGlobalVarianceExpression.h>

#include <DebugStream.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateGlobalConstantFilters
//
// Purpose: 
//   Creates global constant filters.
//
// Arguments:
//   functionName : The name of the expression filter to create.
//
// Returns:    An expression filter or 0 if one could not be created.
//
// Note:       
//
// Programmer: Justin Privitera
// Creation:   Thu Oct  2 16:10:10 PDT 2025
//
// Modifications:
//   
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateGlobalConstantFilters(const std::string &functionName) const
{
    debug5 << "Entering avtFunctionExpr::CreateGlobalConstantFilters(const string&)" << std::endl;
    avtExpressionFilter *f = 0;

    if (functionName == "global_avg")
        f = new avtGlobalAvgExpression();
    else if (functionName == "global_max")
        f = new avtGlobalMaxExpression();
    else if (functionName == "global_min")
        f = new avtGlobalMinExpression();
    else if (functionName == "global_rms")
        f = new avtGlobalRMSExpression();
    else if (functionName == "global_std")
        f = new avtGlobalStdDevExpression();
    else if (functionName == "global_sum")
        f = new avtGlobalSumExpression();
    else if (functionName == "global_var")
        f = new avtGlobalVarianceExpression();

    debug5 << "Exiting  avtFunctionExpr::CreateGlobalConstantFilters(const string&)" << std::endl;
    return f;
}
