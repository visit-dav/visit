// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtAverageOverTimeExpression.h>
#include <avtMaxOverTimeExpression.h>
#include <avtMinOverTimeExpression.h>
#include <avtSumOverTimeExpression.h>
#include <avtTimeExpression.h>
#include <avtValueAtExtremaExpression.h>
#include <avtWhenConditionIsTrueExpression.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateTimeAndValueFilters
//
// Purpose: 
//   Creates time and value expression filters.
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
avtFunctionExpr::CreateTimeAndValueFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "time")
        f = new avtTimeExpression(avtTimeExpression::MODE_TIME);
    else if (functionName == "cycle")
        f = new avtTimeExpression(avtTimeExpression::MODE_CYCLE);
    else if (functionName == "timestep")
        f = new avtTimeExpression(avtTimeExpression::MODE_INDEX);
    else if (functionName == "average_over_time")
        f = new avtAverageOverTimeExpression();
    else if (functionName == "min_over_time")
        f = new avtMinOverTimeExpression();
    else if (functionName == "max_over_time")
        f = new avtMaxOverTimeExpression();
    else if (functionName == "sum_over_time")
        f = new avtSumOverTimeExpression();
    else if (functionName == "first_time_when_condition_is_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(true);
        e->SetOutputType(WCT_OUTPUT_TIME);
        f = e;
    }
    else if (functionName == "last_time_when_condition_is_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(false);
        e->SetOutputType(WCT_OUTPUT_TIME);
        f = e;
    }
    else if (functionName == "first_cycle_when_condition_is_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(true);
        e->SetOutputType(WCT_OUTPUT_CYCLE);
        f = e;
    }
    else if (functionName == "last_cycle_when_condition_is_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(false);
        e->SetOutputType(WCT_OUTPUT_CYCLE);
        f = e;
    }
    else if (functionName == "first_time_index_when_condition_is_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(true);
        e->SetOutputType(WCT_OUTPUT_TIME_INDEX);
        f = e;
    }
    else if (functionName == "last_time_index_when_condition_is_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(false);
        e->SetOutputType(WCT_OUTPUT_TIME_INDEX);
        f = e;
    }
    else if (functionName == "var_when_condition_is_first_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(true);
        e->SetOutputType(WCT_OUTPUT_VARIABLE);
        f = e;
    }
    else if (functionName == "var_when_condition_is_last_true")
    {
        avtWhenConditionIsTrueExpression *e = 
                                          new avtWhenConditionIsTrueExpression;
        e->SetWhenConditionIsFirstTrue(false);
        e->SetOutputType(WCT_OUTPUT_VARIABLE);
        f = e;
    }
    else if (functionName == "time_at_minimum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(false);
        e->SetOutputType(VE_OUTPUT_TIME);
        f = e;
    }
    else if (functionName == "cycle_at_minimum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(false);
        e->SetOutputType(VE_OUTPUT_CYCLE);
        f = e;
    }
    else if (functionName == "time_index_at_minimum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(false);
        e->SetOutputType(VE_OUTPUT_TIME_INDEX);
        f = e;
    }
    else if (functionName == "value_at_minimum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(false);
        e->SetOutputType(VE_OUTPUT_VARIABLE);
        f = e;
    }
    else if (functionName == "time_at_maximum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(true);
        e->SetOutputType(VE_OUTPUT_TIME);
        f = e;
    }
    else if (functionName == "cycle_at_maximum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(true);
        e->SetOutputType(VE_OUTPUT_CYCLE);
        f = e;
    }
    else if (functionName == "time_index_at_maximum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(true);
        e->SetOutputType(VE_OUTPUT_TIME_INDEX);
        f = e;
    }
    else if (functionName == "value_at_maximum")
    {
        avtValueAtExtremaExpression *e = new avtValueAtExtremaExpression;
        e->SetAtMaximum(true);
        e->SetOutputType(VE_OUTPUT_VARIABLE);
        f = e;
    }

    return f;
}
