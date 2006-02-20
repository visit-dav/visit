// ************************************************************************* //
//                           avtApplyDDFExpression.C                         //
// ************************************************************************* //

#include <avtApplyDDFExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <avtExprNode.h>
#include <ExprToken.h>

#include <avtDDF.h>
#include <avtDDFFunctionInfo.h>

#include <ExpressionException.h>


GetDDFCallback     avtApplyDDFExpression::getDDFCallback = NULL;
void              *avtApplyDDFExpression::getDDFCallbackArgs = NULL;


// ****************************************************************************
//  Method: avtApplyDDFExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

avtApplyDDFExpression::avtApplyDDFExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtApplyDDFExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

avtApplyDDFExpression::~avtApplyDDFExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtApplyDDFExpression::RegisterGetDDFCallback
//
//  Purpose:
//      Registers a callback that allows us to get DDFs.
//
//  Programmer:  Hank Childs
//  Creation:    February 18, 2006
//
// ****************************************************************************

void
avtApplyDDFExpression::RegisterGetDDFCallback(GetDDFCallback gdc, void *args)
{
    getDDFCallback     = gdc;
    getDDFCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtApplyDDFExpression::ProcessArguments
//
//  Purpose:
//      Gets the DDF to use.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

void
avtApplyDDFExpression::ProcessArguments(ArgsExpr *args, 
                                        ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs != 2)
    {
        EXCEPTION1(ExpressionException, "the syntax for the apply_ddf "
                    "expression were incorrect.  Arguments should be: "
                    "<meshname>, \"ddf_name\"");
    }

    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    ArgExpr *secondarg = (*arguments)[1];
    ExprParseTreeNode *secondTree = secondarg->GetExpr();
    string type = secondTree->GetTypeName();
    if (type != "Var")
    {
        EXCEPTION1(ExpressionException, "the syntax for the apply_ddf "
                    "expression were incorrect.  Arguments should be: "
                    "<meshname>, \"ddf_name\"");
    }

    ddfName = dynamic_cast<VarExpr*>(secondTree)->GetVar()->GetFullpath();
    if (getDDFCallback == NULL)
    {
        // No one ever registered the callback.  The NetworkManager should
        // do this.
        EXCEPTION1(ExpressionException, "An internal error occurred when "
                        "trying to locate the DDF.");
    }

    theDDF = getDDFCallback(getDDFCallbackArgs, ddfName.c_str());
    if (theDDF == NULL)
    {
        EXCEPTION1(ExpressionException, "The DDF name you have specified is "
                        "not recognized.  VisIt is only aware of the DDFs that"
                        " have been calculated this session.  In addition, if "
                        "the engine crashes, you must have VisIt regenerate "
                        "the DDFs again.");
    }
}


// ****************************************************************************
//  Method: avtApplyDDFExpression::PerformRestriction
//
//  Purpose:
//      Tells the pipeline contract that we need additional variables so that
//      we can evaluate the DDF.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

avtPipelineSpecification_p
avtApplyDDFExpression::PerformRestriction(avtPipelineSpecification_p spec)
{
    if (theDDF == NULL)
    {
        // We should have failed before getting to this point...
        EXCEPTION1(ExpressionException, "Could not locate the DDF.");
    }

    avtDataSpecification_p ds = spec->GetDataSpecification();
    avtDataSpecification_p new_ds = new avtDataSpecification(ds);
    avtDDFFunctionInfo *info = theDDF->GetFunctionInfo();
    int nVars = info->GetDomainNumberOfTuples();
    for (int i = 0 ; i < nVars ; i++)
        new_ds->AddSecondaryVariable(info->GetDomainTupleName(i).c_str());
    new_ds->AddSecondaryVariable(info->GetCodomainName().c_str());

    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec, new_ds);
    rv = avtSingleInputExpressionFilter::PerformRestriction(rv);
    return rv;
}


// ****************************************************************************
//  Method: avtApplyDDFExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     February 18, 2006
//
// ****************************************************************************

vtkDataArray *
avtApplyDDFExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (theDDF == NULL)
    {
        // We should have failed before getting to this point...
        EXCEPTION1(ExpressionException, "Could not locate the DDF.");
    }

    return theDDF->ApplyFunction(in_ds);
}


