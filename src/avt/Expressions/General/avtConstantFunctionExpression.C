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

// ************************************************************************* //
//                        avtConstantFunctionExpression.C                        //
// ************************************************************************* //

#include <avtConstantFunctionExpression.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <ExpressionException.h>
#include <avtExprNode.h>
#include <ExprPipelineState.h>
#include <snprintf.h>

// ****************************************************************************
//  Method: avtConstantFunctionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 19, 2008
//
// ****************************************************************************

avtConstantFunctionExpression::avtConstantFunctionExpression()
{
    value = 0;
}


// ****************************************************************************
//  Method: avtConstantFunctionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 19, 2008
//
// ****************************************************************************

avtConstantFunctionExpression::~avtConstantFunctionExpression()
{
}


// ****************************************************************************
//  Method: avtConstantFunctionExpression::DeriveVariable
//
//  Purpose:
//      Assigns a constant value to the variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Jeremy Meredith
//  Creation:     February 19, 2008
//
// ****************************************************************************

vtkDataArray *
avtConstantFunctionExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int npts   = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
        rv->SetTuple1(i, value);
    }

    return rv;
}

// ****************************************************************************
//  Method:  avtConstantFunctionExpression::ProcessArguments
//
//  Purpose:
//    Process the arguments for this function.
//
//  Arguments:
//    args       the arguments to the function
//    state      the pipeline state
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 19 16:04:11 EST 2008
//    By having this function specify it only had one argument, it was
//    possible to clobber other constant creations.  I upped it to 2 and
//    had the second arg push a textual representation on the state.
//
// ****************************************************************************
void
avtConstantFunctionExpression::ProcessArguments(ArgsExpr *args,
                                                ExprPipelineState *state)
{
    if (!args || args->GetArgs()->size() != 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the function expected two arguments");
    }

    std::vector<ArgExpr*> *arguments = args->GetArgs();
    ArgExpr *arg0 = (*arguments)[0];
    ArgExpr *arg1 = (*arguments)[1];
    ExprParseTreeNode *node0 = arg0->GetExpr();
    ExprParseTreeNode *node1 = arg1->GetExpr();
    avtExprNode *expr0 = dynamic_cast<avtExprNode*>(node0);
    avtExprNode *expr1 = dynamic_cast<avtExprNode*>(node1);

    if (!expr0)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the first argument was not an expression");
    }
    if (!expr1 ||
        (expr1->GetTypeName() != "IntegerConst" &&
         expr1->GetTypeName() != "FloatConst"))
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the second argument was not an int or float constant");
    }

    expr0->CreateFilters(state);

    if (expr1->GetTypeName() == "IntegerConst")
        value = dynamic_cast<avtIntegerConstExpr*>(expr1)->GetValue();
    else
        value = dynamic_cast<avtFloatConstExpr*>(expr1)->GetValue();

    char strrep[30];
    SNPRINTF(strrep, 30, "'%e'", value);
    state->PushName(string(strrep));    
}

