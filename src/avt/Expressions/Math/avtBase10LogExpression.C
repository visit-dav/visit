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
//                         avtBase10LogExpression.C                          //
// ************************************************************************* //

#include <avtBase10LogExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBase10LogExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri May  8 13:21:52 PDT 2009
//    Initialize defaultErrorValue, useDefaultOnError.
//
// ****************************************************************************

avtBase10LogExpression::avtBase10LogExpression()
{
    defaultErrorValue = 0.;
    useDefaultOnError = false;
}


// ****************************************************************************
//  Method: avtBase10LogExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBase10LogExpression::~avtBase10LogExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBase10LogExpression::DoOperation
//
//  Purpose:
//      Performs the base-10 logarithm to each component,tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 15, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:25:26 PST 2002
//    Added support for vectors and arbitrary data types.
//
//    Kathleen Bonnell, Fri May  8 13:01:58 PDT 2009
//    Added support for a default value to be used as a return value for
//    non-positive values. Upated exception message.
//
// ****************************************************************************

void
avtBase10LogExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float f = in->GetComponent(i, j);
            if (f <= 0)
            {
                if (useDefaultOnError)
                    out->SetComponent(i, j, defaultErrorValue);
                else
                {
                    string msg = "you cannot take the logarithm of values";
                    msg += "<=0.  You might want to try log10(var, ";
                    msg +=  "some-default-numeric-value).";
                    EXCEPTION2(ExpressionException, outputVariableName,
                            msg.c_str()); 
                }
            }
            else
            {
                out->SetComponent(i, j, log10(f));
            }
        }
    }
}



// ****************************************************************************
//  Method: avtBase10LogExpression::ProcessArguments
//
//  Purpose:
//      Parses optional arguments. 
//      Allows the user to pass a default value to be used in error cases.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     May 8, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtBase10LogExpression::ProcessArguments(ArgsExpr *args, 
                                         ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "avtBase10LogExpression: No arguments given.");
    }

    // First arg should be an expression, let it gen is filters. 
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    // If we have two arguments, we expect a numerical const for the second.
    if (nargs == 2)
    {
        ArgExpr *sec = (*arguments)[1];
        avtExprNode *second_tree = dynamic_cast<avtExprNode*>(sec->GetExpr());
        double v = 0;
        if (GetNumericVal(second_tree, v))
        {
            useDefaultOnError = true;
            defaultErrorValue = v;
            debug4 << "avtBase10LogExpression:" << "Using " << v 
                   << " as default value in error conditions" << endl;
        }
        else
        {
            string error_msg = "avtBase10LogExpression: "
                               "Invalid second argument."
                               "Should be float or int";
            debug5 << error_msg << endl;
            EXCEPTION2(ExpressionException, outputVariableName, 
                       error_msg.c_str());
        }
    }
}




