/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

    // Going the ptr arithmetic route, because I'm having buggy behavior with
    // Get/SetTup requiring double * and I cannot use GetSetTuple1.
    float *out_var_ptr = (float *) outVar->GetVoidPointer(0);

    int nvars = outVar->GetNumberOfTuples();
    for (int i = 0 ; i < nvars ; i++)
    {
        int exVarIndex   = (exVarIsConstant ? 0 : i);
        int valueVarIndex= (valueVarIsConstant ? 0 : i);
        bool reset = false;
        if (ts == 0)
        {
            out_var_ptr[2*i]   = (atMaximum ? -FLT_MAX : FLT_MAX);
            reset = true; // set out_var_ptr[2*i+1] below
        }

        if (atMaximum)
        {
            if (ex_var->GetTuple1(exVarIndex) > out_var_ptr[2*i])
                reset = true;
        }
        else
        {
            if (ex_var->GetTuple1(exVarIndex) < out_var_ptr[2*i])
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
            out_var_ptr[2*i+0] = ex_var->GetTuple1(exVarIndex);
            out_var_ptr[2*i+1] = val;
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
    // Going the ptr arithmetic route, because I'm having buggy behavior with
    // Get/SetTup requiring double * and I cannot use GetSetTuple1.
    float *d_ptr = (float *) d->GetVoidPointer(0);

    vtkDataArray *rv = d->NewInstance();
    rv->SetNumberOfTuples(d->GetNumberOfTuples());
    for (int i = 0 ; i < d->GetNumberOfTuples() ; i++)
    {
        rv->SetTuple1(i, d_ptr[2*i+1]);
    }

    rv->SetName(d->GetName());

    return rv;
}

