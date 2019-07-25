// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtWhenConditionIsTrueExpression.C                   //
// ************************************************************************* //

#include <avtWhenConditionIsTrueExpression.h>

#include <vtkDataArray.h>


// ****************************************************************************
//  Method: avtWhenConditionIsTrueExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtWhenConditionIsTrueExpression::avtWhenConditionIsTrueExpression()
{
    outputType = WCT_OUTPUT_TIME;
    firstTrue  = true;
}


// ****************************************************************************
//  Method: avtWhenConditionIsTrueExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtWhenConditionIsTrueExpression::~avtWhenConditionIsTrueExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtWhenConditionIsTrueExpression::ExecuteDataset
//
//  Purpose:
//      Finds the maximum over time.  This is the method that looks examines a
//      single VTK data set.
//
//  Programmer:   Hank Childs
//  Creation:     February 16, 2009
//
// ****************************************************************************

void
avtWhenConditionIsTrueExpression::ExecuteDataset(
                                         std::vector<vtkDataArray *> &inVars,
                                         vtkDataArray *outVar, int ts)
{
    vtkDataArray *cond_var = inVars[0];
    vtkDataArray *var_var = NULL;
    vtkDataArray *default_var = NULL;
    if (outputType == WCT_OUTPUT_VARIABLE)
    {
        var_var     = inVars[1];
        default_var = inVars[2];
    }
    else
        default_var = inVars[1];

    bool defaultIsConstant = (default_var->GetNumberOfTuples()==1);
    bool condIsConstant    = (cond_var->GetNumberOfTuples()==1);
    bool varIsConstant     = (var_var != NULL && var_var->GetNumberOfTuples()==1);

    int nvars = outVar->GetNumberOfTuples();
    for (int i = 0 ; i < nvars ; i++)
    {
        int defaultIndex = (defaultIsConstant ? 0 : i);
        int condIndex    = (condIsConstant ? 0 : i);
        int varIndex     = (varIsConstant ? 0 : i);
        if (ts == 0)
            outVar->SetTuple1(i, default_var->GetTuple1(defaultIndex));

        if (cond_var->GetTuple1(condIndex) != 0.)
        {
            double val = 0.;
            switch (outputType)
            {
              case WCT_OUTPUT_TIME:
                val = currentTime;
                break;
              case WCT_OUTPUT_CYCLE:
                val = (double) currentCycle;
                break;
              case WCT_OUTPUT_TIME_INDEX:
                val = (double) ts;
                break;
              case WCT_OUTPUT_VARIABLE:
                val = var_var->GetTuple1(varIndex);
                break;
            }
            if (firstTrue)
            {
               if (outVar->GetTuple1(i) == default_var->GetTuple1(defaultIndex))
                   outVar->SetTuple1(i, val);
            }
            else
            {
                outVar->SetTuple1(i, val);
            }
        }
    }
}


