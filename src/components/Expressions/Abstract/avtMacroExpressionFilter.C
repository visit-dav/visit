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
//                           avtMacroExpressionFilter.C                      //
// ************************************************************************* //

#include <avtMacroExpressionFilter.h>

#include <ExprNode.h>
#include <ParsingExprList.h>

#include <avtSourceFromAVTDataset.h>

#include <DebugStream.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtMacroExpressionFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtMacroExpressionFilter::avtMacroExpressionFilter() 
{
    term_src = NULL;
}


// ****************************************************************************
//  Method: avtMacroExpressionFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtMacroExpressionFilter::~avtMacroExpressionFilter()
{
    if (term_src != NULL)
        delete term_src;
}


// ****************************************************************************
//  Method: avtMacroExpressionFilter::ProcessArguments
//
//  Purpose:
//      Gets the string from each argument and then send it down to the
//      derived type to construct the macro.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

void
avtMacroExpressionFilter::ProcessArguments(ArgsExpr *args, 
                                           ExprPipelineState *state)
{
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nArgs = arguments->size();
    for (int i = 0 ; i < nArgs ; i++)
    {
        expression_arguments.push_back((*arguments)[i]->GetText());
    }

    //
    // The base class knows how to process the arguments correctly. 
    //
    avtExpressionFilter::ProcessArguments(args, state);
}


// ****************************************************************************
//  Method: avtMacroExpressionFilter::ReplaceMacroInExpressionList
//
//  Purpose:
//      Takes the master list of expression and removes the macro and replaces
//      it with its subexpressions.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

void
avtMacroExpressionFilter::ReplaceMacroInExpressionList(void)
{
    //
    // Have the derived type define the macro.
    //
    std::string           replacement_expr;
    Expression::ExprType  replacement_type;
    GetMacro(expression_arguments, replacement_expr, replacement_type);

    //
    // We are going to create a new list of expressions that reflects our
    // macro.
    //
    // Our new expression is supposed to be named 'X' (= outputVariableName).
    // If our Macro is M, and there is something of the form X = M(f), then
    // remove it from the original list.
    //
    ParsingExprList *pel = ParsingExprList::Instance();
    original_list = *(pel->GetList());
    ExpressionList new_list = original_list;
    int num_expr = new_list.GetNumExpressions();
    for (int i = num_expr-1 ; i >= 0 ; i--)
    {
        Expression &expr = new_list.GetExpressions(i);
        if (expr.GetName() == outputVariableName)
        {
            new_list.RemoveExpressions(i);
        }
    }
    
    //
    // Now add our expression to the list.
    //
    Expression exp;
    exp.SetName(outputVariableName);
    exp.SetDefinition(replacement_expr);
    exp.SetType(replacement_type);
    new_list.AddExpressions(exp);

    //
    // Change the global expression list to be our new expression list.
    //
    *(pel->GetList()) = new_list;

}


// ****************************************************************************
//  Method: avtMacroExpressionFilter::RestoreExpressionList
//
//  Purpose:
//      Takes the master list of expression and removes the macro and replaces
//      it with its subexpressions.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

void
avtMacroExpressionFilter::RestoreExpressionList(void)
{
    //
    // Swap back the old expression list.
    //
    ParsingExprList *pel = ParsingExprList::Instance();
    *(pel->GetList()) = original_list;
}


// ****************************************************************************
//  Method: avtMacroExpressionFilter::Execute
//
//  Purpose:
//      Creates a sub-pipeline to create the expression.
//
//  Programmer:   Hank Childs
//  Creation:     December 27, 2004
//
// ****************************************************************************

void
avtMacroExpressionFilter::Execute(void)
{
    //
    // Make sure that this expression hasn't already been calculated.  This is
    // especially likely to come up with vectors.  We calculate curl, then
    // magnitude of curl, then curl again.
    //
    if (GetInput()->GetInfo().GetAttributes().
                                             ValidVariable(outputVariableName))
    {
        GetOutput()->Copy(*(GetInput()));
        return;
    }

    ReplaceMacroInExpressionList();

    //
    // Go through the standard hassles to force an Execute.
    //
    // If we use the default variable, the EEF will try to create it, so tell
    // it that the default variable is the one we are calculating.
    //
    avtContract_p contract = GetGeneralContract();
    avtContract_p new_contract = new avtContract(contract,
                                                                    last_spec);

    //
    // The expression evaluator filter to execute will create
    // the correct variable using our macro.
    //
    term_src->ResetTree(GetInputDataTree());
    term_src->GetOutput()->GetInfo().Copy(GetInput()->GetInfo());
    eef.SetInput(term_src->GetOutput());
    eef.GetOutput()->Update(new_contract);
    GetOutput()->Copy(*(eef.GetOutput()));

    RestoreExpressionList();
}


// ****************************************************************************
//  Method: avtMacroExpressionFilter::AdditionalPipelineFilters
//
//  Purpose:
//      Determines how many additional filters we will have in the pipeline
//      due to the expression evaluator filter.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

int
avtMacroExpressionFilter::AdditionalPipelineFilters(void)
{
    return eef.AdditionalPipelineFilters()+1;
}


// ****************************************************************************
//  Method: avtMacroExpressionFilter::ModifyContract
//
//  Purpose:
//      Let the expression evaluator filter have a change to modify the 
//      the pipeline restriction contract.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Nov 15 07:12:53 PST 2005
//    Make sure we don't lose the active variable if it is not a macro
//    variable.
//
//    Hank Childs, Mon Jun 18 09:41:49 PDT 2007
//    Change the list of variables sent into our version of the EEF.
//    Make sure that we are requesting only the variable we want to produce
//    and also the variables that are needed downstream.
//
// ****************************************************************************

avtContract_p
avtMacroExpressionFilter::ModifyContract(avtContract_p spec)
{
    ReplaceMacroInExpressionList();
    if (term_src == NULL)
        term_src = new avtSourceFromAVTDataset(GetTypedInput());
    eef.SetInput(term_src->GetOutput());

    //
    // If we use the default variable, the EEF will try to create it, so tell
    // it that the default variable is the one we are calculating.
    //
    avtDataRequest_p old_dataRequest = spec->GetDataRequest();
    const char *v = outputVariableName;
    avtDataRequest_p new_dataRequest = new avtDataRequest(old_dataRequest, v);
    new_dataRequest->RemoveSecondaryVariable(v);

    last_spec = new avtDataRequest(new_dataRequest);
    avtContract_p new_contract = new avtContract(spec,
                                                                    new_dataRequest);

    avtContract_p rv = eef.ModifyContract(new_contract);
    avtDataRequest_p rv_d = rv->GetDataRequest();
    avtDataRequest_p rv_d2 = new avtDataRequest(rv_d, 
                                                     old_dataRequest->GetVariable());
    rv_d2->AddSecondaryVariable(rv_d->GetVariable());
    avtContract_p rv2 = new avtContract(rv, rv_d2);

    RestoreExpressionList();
    return rv2;
}


