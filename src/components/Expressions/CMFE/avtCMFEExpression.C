// ************************************************************************* //
//                             avtCMFEExpression.C                           //
// ************************************************************************* //

#include <avtCMFEExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedIntArray.h>

#include <avtExprNode.h>
#include <ExprToken.h>

#include <avtCallback.h>
#include <avtMetaData.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


// ****************************************************************************
//  Method: avtCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

avtCMFEExpression::avtCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

avtCMFEExpression::~avtCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCMFEExpression::PreExecute
//
//  Purpose:
//      Reset the "issuedWarning" flag.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

void
avtCMFEExpression::PreExecute(void)
{
    issuedWarning = false;
    avtExpressionFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtCMFEExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  The second argument
//      is the database to compare with.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

void
avtCMFEExpression::ProcessArguments(ArgsExpr *args, 
                                          ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // Check if there's a second argument.
    if (nargs < 2)
    {
        EXCEPTION1(ExpressionException, 
                   "The database comparison expression only expects two "
                   "arguments: the database variable and a mesh to sample "
                   "onto.");
    }

    // See if there are other arguments.
    if (nargs > 2)
    {
        EXCEPTION1(ExpressionException, 
                   "The database comparison expression only expects two "
                   "arguments.  To specify more than one database, please "
                   "use several cmfe expressions.");
    }

    // Tell the second argument (the mesh) to create its filters.
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    secondTree->CreateFilters(state);

    // Pull off the first argument and see if it's a string or a list.
    ArgExpr *firstarg = (*arguments)[0];
    ExprParseTreeNode *firstTree = firstarg->GetExpr();
    if (firstTree->GetTypeName() == "Var")
    {
        VarExpr *var_expr = dynamic_cast<VarExpr *>(firstTree);
        DBExpr *db_expr = var_expr->GetDB();
        if (db_expr == NULL)
        {
            EXCEPTION1(ExpressionException, 
                       "The first argument of the database comparison "
                       "expression must be a database.");
        }
        db = db_expr->GetFile()->GetFullpath();
        var  = var_expr->GetVar()->GetFullpath();
    }
    string type = firstTree->GetTypeName();
}


// ****************************************************************************
//  Method: avtCMFEExpression::Execute
//
//  Purpose:
//      Gets the second mesh to compare onto and calls the virtual method that
//      does the actual cross mesh field evaluation.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

void
avtCMFEExpression::Execute()
{
    // Need to account for time, possibility that variable is an expression.
    ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
    if (*dbp == NULL)
        EXCEPTION1(InvalidFilesException, db.c_str());
    avtDataObject_p dob = dbp->GetOutput(var.c_str(), 0);
    if (*dob == NULL)
        EXCEPTION1(InvalidVariableException, var.c_str());
    if (strcmp(dob->GetType(), "avtDataset") != 0)
        EXCEPTION1(InvalidVariableException, var.c_str());

    // HACK.  This will only work for conn_cmfe.
    avtPipelineSpecification_p spec = 
                new avtPipelineSpecification(
                            dob->GetTerminatingSource()
                               ->GetGeneralPipelineSpecification()
                               ->GetDataSpecification(),
                            1);

    dob->Update(spec);
    avtDataset_p dsp;
    CopyTo(dsp, dob);

    avtDataTree_p output = PerformCMFE(GetInputDataTree(), dsp->GetDataTree(),
                                       var, outputVariableName);
    SetOutputDataTree(output);

    //
    // Reset the variable information to match the newly CMFE'd variable.
    //
    avtDataAttributes &inputAtts = dob->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    SetExpressionAttributes(inputAtts, outAtts);
}


