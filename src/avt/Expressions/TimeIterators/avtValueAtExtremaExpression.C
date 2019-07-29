// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtValueAtExtremaExpression.C                     //
// ************************************************************************* //

#include <avtValueAtExtremaExpression.h>

#include <float.h>

#include <vtkDataArray.h>


// ****************************************************************************
//  Method: avtValueAtExtremaExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtValueAtExtremaExpression::avtValueAtExtremaExpression()
{
    outputType = VE_OUTPUT_TIME;
    atMaximum  = true;
}


// ****************************************************************************
//  Method: avtValueAtExtremaExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

avtValueAtExtremaExpression::~avtValueAtExtremaExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtValueAtExtremaExpression::ExecuteDataset
//
//  Purpose:
//      Finds the time/cycle/time index/variable at the min/max.
//
//  Programmer:   Hank Childs
//  Creation:     February 16, 2009
//
// ****************************************************************************

void
avtValueAtExtremaExpression::ExecuteDataset(std::vector<vtkDataArray *> &inVars,
                                            vtkDataArray *outVar, int ts)
{
    vtkDataArray *ex_var  = inVars[0];
    vtkDataArray *value_var = NULL;
    if (outputType == VE_OUTPUT_VARIABLE)
        value_var   = inVars[1];

    bool exVarIsConstant   = (ex_var->GetNumberOfTuples()==1);
    bool valueVarIsConstant= (value_var != NULL && value_var->GetNumberOfTuples()==1);

    vtkIdType nvars = outVar->GetNumberOfTuples();
    for (vtkIdType i = 0 ; i < nvars ; i++)
    {
        int exVarIndex   = (exVarIsConstant ? 0 : i);
        int valueVarIndex= (valueVarIsConstant ? 0 : i);
        bool reset = false;
        if (ts == 0)
        {
            outVar->SetComponent(i, 0, (atMaximum ? -FLT_MAX : FLT_MAX));
            reset = true; // set outVar(i,1) below
        }

        if (atMaximum)
        {
            if (ex_var->GetTuple1(exVarIndex) > outVar->GetComponent(i, 0))
                reset = true;
        }
        else
        {
            if (ex_var->GetTuple1(exVarIndex) < outVar->GetComponent(i, 0))
                reset = true;
        }
              
        if (reset)
        {
            double val = 0.;
            switch (outputType)
            {
              case VE_OUTPUT_TIME:
                val = currentTime;
                break;
              case VE_OUTPUT_CYCLE:
                val = (double) currentCycle;
                break;
              case VE_OUTPUT_TIME_INDEX:
                val = (double) ts;
                break;
              case VE_OUTPUT_VARIABLE:
                val = value_var->GetTuple1(valueVarIndex);
                break;
            }
            outVar->SetComponent(i, 0, ex_var->GetTuple1(exVarIndex));
            outVar->SetComponent(i, 1, val);
        }
    }
}


// ****************************************************************************
//  Method: avtValueAtExtremaExpression::ConvertIntermediateArrayToFinalArray
//
//  Purpose:
//      We have an array that contains both the max (or min) and the value
//      associated with that.  Now convert that to an array with just the value
//      since the max (or min) was just for bookkeeping.
//
//  Programmer:   Hank Childs
//  Creation:     February 16, 2009
//
// ****************************************************************************


vtkDataArray *
avtValueAtExtremaExpression::ConvertIntermediateArrayToFinalArray(vtkDataArray *d)
{
    vtkDataArray *rv = d->NewInstance();
    rv->SetNumberOfTuples(d->GetNumberOfTuples());
    for (vtkIdType i = 0 ; i < d->GetNumberOfTuples() ; i++)
    {
        rv->SetTuple1(i, d->GetComponent(i,1));
    }

    rv->SetName(d->GetName());

    return rv;
}

