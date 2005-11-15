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
        Expression &expr = new_list.GetExpression(i);
        if (expr.GetName() == outputVariableName)
        {
            new_list.RemoveExpression(i);
        }
    }
    
    //
    // Now add our expression to the list.
    //
    Expression exp;
    exp.SetName(outputVariableName);
    exp.SetDefinition(replacement_expr);
    exp.SetType(replacement_type);
    new_list.AddExpression(exp);

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
    avtPipelineSpecification_p pspec = GetGeneralPipelineSpecification();
    avtPipelineSpecification_p new_pspec = new avtPipelineSpecification(pspec,
                                                                    last_spec);

    //
    // The expression evaluator filter to execute will create
    // the correct variable using our macro.
    //
    term_src->ResetTree(GetInputDataTree());
    term_src->GetOutput()->GetInfo().Copy(GetInput()->GetInfo());
    eef.SetInput(term_src->GetOutput());
    eef.GetOutput()->Update(new_pspec);
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
//  Method: avtMacroExpressionFilter::PerformRestriction
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
// ****************************************************************************

avtPipelineSpecification_p
avtMacroExpressionFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    ReplaceMacroInExpressionList();
    if (term_src == NULL)
        term_src = new avtSourceFromAVTDataset(GetTypedInput());
    eef.SetInput(term_src->GetOutput());

    //
    // If we use the default variable, the EEF will try to create it, so tell
    // it that the default variable is the one we are calculating.
    //
    avtDataSpecification_p old_dspec = spec->GetDataSpecification();
    const char *v = outputVariableName;
    avtDataSpecification_p new_dspec = new avtDataSpecification(old_dspec, v);
    new_dspec->RemoveSecondaryVariable(v);

    //
    // If the primary variable is not the macro variable, then we may have just
    // clobbered it.  Add the primary variable back as a secondary variable.
    //
    if (strcmp(old_dspec->GetVariable(), v) != 0)
        new_dspec->AddSecondaryVariable(old_dspec->GetVariable());

    avtPipelineSpecification_p new_pspec = new avtPipelineSpecification(spec,
                                                                    new_dspec);
    last_spec = new_dspec;

    avtPipelineSpecification_p rv = eef.PerformRestriction(new_pspec);
    RestoreExpressionList();
    return rv;
}


