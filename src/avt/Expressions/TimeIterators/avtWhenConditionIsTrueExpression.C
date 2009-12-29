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


