/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <avtExprNode.h>

#include <avtAverageOverTimeExpression.h>
#include <avtMaxOverTimeExpression.h>
#include <avtMinOverTimeExpression.h>
#include <avtSumOverTimeExpression.h>
#include <avtTimeExpression.h>
#include <avtValueAtExtremaExpression.h>
#include <avtWhenConditionIsTrueExpression.h>

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
avtFunctionExpr::CreateTimeAndValueFilters(const string &functionName) const
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
