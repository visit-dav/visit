// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtAbelInversionExpression.h>
#include <avtConservativeSmoothingExpression.h>
#include <avtMeanFilterExpression.h>
#include <avtMedianFilterExpression.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateImageProcessingFilters
//
// Purpose: 
//   Creates image processing filters.
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
avtFunctionExpr::CreateImageProcessingFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "conservative_smoothing")
        f = new avtConservativeSmoothingExpression;
    else if (functionName == "mean_filter")
        f = new avtMeanFilterExpression;
    else if (functionName == "median_filter")
        f = new avtMedianFilterExpression;
    else if (functionName == "abel_inversion")
        f = new avtAbelInversionExpression;

    return f;
}
