// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtConditionalExpression.h>
#include <avtLogicalAndExpression.h>
#include <avtLogicalNegationExpression.h>
#include <avtLogicalOrExpression.h>
#include <avtTestEqualToExpression.h>
#include <avtTestGreaterThanExpression.h>
#include <avtTestGreaterThanOrEqualToExpression.h>
#include <avtTestLessThanExpression.h>
#include <avtTestLessThanOrEqualToExpression.h>
#include <avtTestNotEqualToExpression.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateConditionalFilters
//
// Purpose: 
//   Creates conditional and logic expression filters.
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
avtFunctionExpr::CreateConditionalFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "if")
        f = new avtConditionalExpression();
    else if (functionName == "and")
        f = new avtLogicalAndExpression();
    else if (functionName == "or")
        f = new avtLogicalOrExpression();
    else if (functionName == "not")
        f = new avtLogicalNegationExpression();
    else if (functionName == "le" || functionName == "lte")
        f = new avtTestLessThanOrEqualToExpression();
    else if (functionName == "ge" || functionName == "gte")
        f = new avtTestGreaterThanOrEqualToExpression();
    else if (functionName == "lt")
        f = new avtTestLessThanExpression();
    else if (functionName == "gt")
        f = new avtTestGreaterThanExpression();
    else if (functionName == "eq" || functionName == "equal" || 
             functionName == "equals")
        f = new avtTestEqualToExpression();
    else if (functionName == "ne" || functionName == "neq" ||
             functionName == "notequal" || functionName == "notequals")
        f = new avtTestNotEqualToExpression();

    return f;
}
