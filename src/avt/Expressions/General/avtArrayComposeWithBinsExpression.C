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
//                       avtArrayComposeWithBinsExpression.C                     //
// ************************************************************************* //

#include <avtArrayComposeWithBinsExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <avtCallback.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtSpecies.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtArrayComposeWithBinsExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 12, 2007
//
// ****************************************************************************

avtArrayComposeWithBinsExpression::avtArrayComposeWithBinsExpression()
{
    nvars = 0;
}


// ****************************************************************************
//  Method: avtArrayComposeWithBinsExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 12, 2007
//
// ****************************************************************************

avtArrayComposeWithBinsExpression::~avtArrayComposeWithBinsExpression()
{
}


// ****************************************************************************
//  Method: avtArrayComposeWithBinsExpression::DeriveVariable
//
//  Purpose:
//      Creates an array.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     January 12, 2007
//
// ****************************************************************************

vtkDataArray *
avtArrayComposeWithBinsExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    if (varnames.size() == 0)
        EXCEPTION0(ImproperUseException);

    int nvars = varnames.size();
    vtkDataArray **vars = new vtkDataArray*[nvars];
    avtCentering  *centering = new avtCentering[nvars];

    for (i = 0 ; i < nvars ; i++)
    {
        vars[i] = in_ds->GetPointData()->GetArray(varnames[i]);
        centering[i] = AVT_NODECENT;
        if (vars[i] == NULL)
        {
            vars[i] = in_ds->GetCellData()->GetArray(varnames[i]);
            centering[i] = AVT_ZONECENT;
        }
    }

    for (i = 0 ; i < nvars ; i++)
    {
        if (vars[i] == NULL)
            EXCEPTION2(ExpressionException, outputVariableName, 
                  "Cannot create array because: cannot locate all variables");
        if (vars[i]->GetNumberOfComponents() != 1)
            EXCEPTION2(ExpressionException, outputVariableName, 
                  "Cannot create array because: all inputs must be scalars");
        if (centering[i] != centering[0])
            EXCEPTION2(ExpressionException, outputVariableName,
                  "Cannot create array because: the centering of the "
                  "variables does not agree.");
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(nvars);
    int nvals = vars[0]->GetNumberOfTuples();
    rv->SetNumberOfTuples(nvals);
    for (i = 0 ; i < nvals ; i++)
        for (j = 0 ; j < nvars ; j++)
            rv->SetComponent(i, j, vars[j]->GetTuple1(i));

    delete [] vars;
    delete [] centering;
    return rv;
}


// ****************************************************************************
//  Method: avtArrayComposeWithBinsExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  Parses the second
//      argument into a list of material names.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     January 12, 2007
//
// ****************************************************************************

void
avtArrayComposeWithBinsExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    nvars = arguments->size()-1;

    int idx_of_list = arguments->size()-1;
    ArgExpr *listarg = (*arguments)[idx_of_list];
    ExprParseTreeNode *listTree = listarg->GetExpr();
    if (listTree->GetTypeName() != "List")
    {
        debug1 << "avtArrayComposeWithBinsExpression: second arg is not a list: "
               << listTree->GetTypeName() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, "the last argument to array_compose_"
                    "with_bins must be a list");
    }

    ListExpr *list = dynamic_cast<ListExpr*>(listTree);
    std::vector<ListElemExpr*> *elems = list->GetElems();
    binRanges.resize(elems->size());
    if (elems->size() != nvars+1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "the list for array_compose_with_bins"
                   " must have one more number than there are variables. "
                   " For two variables (V1 and V2), there should be a list of"
                   " size 3: [L0, L1, L2].  V1's bin goes from L0 to L1, "
                   "and V2's bin goes from L1 to L2.");
    }
    for (int i = 0 ; i < elems->size() ; i++)
    {
        if ((*elems)[i]->GetEnd())
        {
            EXCEPTION2(ExpressionException, outputVariableName, "the list for array_compose_with"
                        "_bins expression cannot contain ranges.");
        }

        ExprNode *item = (*elems)[i]->GetItem();
        if (item->GetTypeName() == "FloatConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(item);
            binRanges[i] = dynamic_cast<FloatConstExpr*>(c)->GetValue();
        }
        else if (item->GetTypeName() == "IntegerConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(item);
            binRanges[i] = dynamic_cast<IntegerConstExpr*>(c)->GetValue();
        }
        else 
        {
            EXCEPTION2(ExpressionException, outputVariableName, "the list for the array_compose"
                          "_with_bins expression may contain only numbers.");
        }
    }

    // Let the base class do this processing.  We only had to over-ride this
    // function to determine the number of arguments.
    avtMultipleInputExpressionFilter::ProcessArguments(args, state);
}


// ****************************************************************************
//  Method: avtArrayComposeWithBinsExpression::UpdateDataObjectInfo
//
//  Purpose:
//      Tell the output what the component names are.
//
//  Programmer:   Hank Childs
//  Creation:     January 12, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 18:01:29 EST 2008
//    This wasn't setting the dimension of the output variable, which
//    was necessary, so I added it.
//
// ****************************************************************************

void
avtArrayComposeWithBinsExpression::UpdateDataObjectInfo(void)
{
    avtMultipleInputExpressionFilter::UpdateDataObjectInfo();

    // If we don't know the name of the variable, we can't set it up in the
    // output.
    if (outputVariableName == NULL)
        return;

    std::vector<std::string> subnames(varnames.size());
    for (int i = 0 ; i < varnames.size() ; i++)
        subnames[i] = varnames[i];

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetVariableDimension(varnames.size(), outputVariableName);
    outAtts.SetVariableSubnames(subnames, outputVariableName);
    outAtts.SetVariableBinRanges(binRanges, outputVariableName);
}

