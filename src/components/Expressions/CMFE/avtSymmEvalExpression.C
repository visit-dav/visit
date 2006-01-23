// ************************************************************************* //
//                           avtSymmEvalExpression.C                         //
// ************************************************************************* //

#include <avtSymmEvalExpression.h>

#include <snprintf.h>

#include <avtExprNode.h>
#include <ExprToken.h>
#include <ParsingExprList.h>

#include <avtPosCMFEAlgorithm.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtSymmEvalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmEvalExpression::avtSymmEvalExpression()
{
}


// ****************************************************************************
//  Method: avtSymmEvalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmEvalExpression::~avtSymmEvalExpression()
{
}


// ****************************************************************************
//  Method: avtSymmEvalExpression::AddInputVariableName
//
//  Purpose:
//      Adds an input variable's name.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

void
avtSymmEvalExpression::AddInputVariableName(const char *vname)
{
    avtExpressionFilter::AddInputVariableName(vname);
    varnames.push_back(vname);
}


// ****************************************************************************
//  Method: avtSymmEvalExpression::ProcessArguments
//
//  Purpose:
//      Processes the expression arguments.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

void
avtSymmEvalExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    int  i;

    const char *badSyntaxMsg = "The symmetry evaluation expression expects "
             "three arguments: a variable, a default variable to use when "
             "the mesh is not defined across the symmetry condition, and "
             "a list of input parameters, [#, ..., #]";

    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs != 3)
        EXCEPTION1(ExpressionException, badSyntaxMsg);

    // Tell the variables to create their filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    secondTree->CreateFilters(state);

    // Now go through the third argument, which should be a list.
    ArgExpr *thirdarg = (*arguments)[2];
    ExprParseTreeNode *thirdTree = thirdarg->GetExpr();
    string type = thirdTree->GetTypeName();
    if (type != "List")
        EXCEPTION1(ExpressionException, badSyntaxMsg);

    // It's a list.  Process all of them.
    inputParameters.clear();
    ListExpr *list = dynamic_cast<ListExpr*>(thirdTree);
    std::vector<ListElemExpr*> *elems = list->GetElems();
    if (elems->size() != GetNumberOfInputParameters())
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "%s.  You're list had %d numbers, where %d were "
                  "expected.", badSyntaxMsg, elems->size(), 
                 GetNumberOfInputParameters());
        EXCEPTION1(ExpressionException, msg);
    }

    for (i = 0 ; i < elems->size() ; i++)
    {
        ExprNode *item = (*elems)[i]->GetItem();
        string type = item->GetTypeName();
        bool negate = false;
        if (type == "Unary")
        {
            UnaryExpr *unary = dynamic_cast<UnaryExpr*>(item);
            item = unary->GetExpr();
            type = item->GetTypeName();
            negate = true;
        }
        if (type != "IntegerConst" && type != "FloatConst")
        {
            char msg[1024];
            SNPRINTF(msg, 1024, "%s.  Element #%d in your list was "
                      "not a number.",  badSyntaxMsg, i);
            EXCEPTION1(ExpressionException, msg);
        }
        ConstExpr *c = dynamic_cast<ConstExpr*>(item);
        if (c->GetConstantType() == ConstExpr::Integer)
        {
            int v = dynamic_cast<IntegerConstExpr*>(c)->GetValue();
            if (negate)
                v = -v;
            inputParameters.push_back(v);
        }
        else if (c->GetConstantType() == ConstExpr::Float)
        {
            float v = dynamic_cast<FloatConstExpr*>(c)->GetValue();
            if (negate)
                v = -v;
            inputParameters.push_back(v);
        }
        else
        {
            EXCEPTION1(ExpressionException, badSyntaxMsg);
        }
    }
}


// ****************************************************************************
//  Method: avtSymmEvalExpression::Execute
//
//  Purpose:
//      Executes the symmetry evaluation.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

void
avtSymmEvalExpression::Execute(void)
{
    avtDataObject_p input = GetInput();
    avtDataObject_p transformed = TransformData(input);
    avtDataTree_p in1 = ((avtDataset *) (*input))->GetDataTree();
    avtDataTree_p in2 = ((avtDataset *) (*transformed))->GetDataTree();

    if (varnames.size() != 2)
    {
        EXCEPTION1(ExpressionException, "Symmetry evaluation expressions "
                      "must take the form: <var> <default-var> [parameters]");
    }

    avtDataTree_p output = avtPosCMFEAlgorithm::PerformCMFE(in1, in2,
                                 varnames[0], varnames[1], outputVariableName);
    SetOutputDataTree(output);
}


