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
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the syntax for the apply_ddf "
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
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the syntax for the apply_ddf "
                    "expression were incorrect.  Arguments should be: "
                    "<meshname>, \"ddf_name\"");
    }

    ddfName = dynamic_cast<VarExpr*>(secondTree)->GetVar()->GetFullpath();
    if (getDDFCallback == NULL)
    {
        // No one ever registered the callback.  The NetworkManager should
        // do this.
        EXCEPTION2(ExpressionException, outputVariableName,
                   "An internal error occurred when "
                        "trying to locate the DDF.");
    }

    theDDF = getDDFCallback(getDDFCallbackArgs, ddfName.c_str());
    if (theDDF == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The DDF name you have specified is "
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
        EXCEPTION2(ExpressionException, outputVariableName, "Could not locate the DDF.");
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
//  Modifications:
//    Cyrus Harrison, Tue Jul  3 08:04:17 PDT 2007
//    Added error message if unable to create a ddf.
//
// ****************************************************************************

vtkDataArray *
avtApplyDDFExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (theDDF == NULL)
    {
        // We should have failed before getting to this point...
        EXCEPTION2(ExpressionException, outputVariableName, "Could not locate the DDF.");
    }

    vtkDataArray *res = theDDF->ApplyFunction(in_ds);

    if ( res == NULL)
    {
        // ddf was unsuccessful, this is probably due to a centering problem
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Could not apply the ddf. Please check that all variables "
                   " are valid and have the same centering.");
    }

    return res;
}


