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
//                       avtApplyEnumerationExpression.C                     //
// ************************************************************************* //

#include <avtApplyEnumerationExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtApplyEnumerationExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 13, 2008
//
// ****************************************************************************

avtApplyEnumerationExpression::avtApplyEnumerationExpression()
{
}


// ****************************************************************************
//  Method: avtApplyEnumerationExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 13, 2008
//
// ****************************************************************************

avtApplyEnumerationExpression::~avtApplyEnumerationExpression()
{
}


// ****************************************************************************
//  Method: avtApplyEnumerationExpression::DeriveVariable
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
//  Programmer:   Jeremy Meredith
//  Creation:     February 13, 2008
//  
//  Modification:
//    Kathleen Bonnell, Tue Jun  3 08:11:22 PDT 2008
//    Remove unreferenced variable.
//
// ****************************************************************************

vtkDataArray *
avtApplyEnumerationExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int    i;

    if (varnames.size() == 0)
        EXCEPTION0(ImproperUseException);

    vtkDataArray *var = in_ds->GetPointData()->GetArray(varnames[0]);
    if (var == NULL)
        var = in_ds->GetCellData()->GetArray(varnames[0]);
    if (var == NULL)
        EXCEPTION2(ExpressionException, outputVariableName, 
               "Cannot enumerate scalar because: cannot locate all variables");
    if (var->GetNumberOfComponents() != 1)
        EXCEPTION2(ExpressionException, outputVariableName, 
               "Cannot enumerate scalar because: all inputs must be scalars");

    vtkFloatArray *rv = vtkFloatArray::New();
    int nvals = var->GetNumberOfTuples();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(nvals);
    for (i = 0 ; i < nvals ; i++)
    {
        double dval = var->GetTuple1(i);
        int    ival = int(dval);
        if (dval != double(ival))
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                       "Cannot enumerate scalar because: "
                       "encountered a non-integer input value");
        }
        if (ival < 0)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                       "Cannot enumerate scalar because: "
                       "encountered a negative input value");
        }
        if (ival >= enumeratedValues.size())
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                       "Cannot enumerate scalar because: "
                       "encountered an input value larger than "
                       "the number of values in the enumeration list");
        }
        rv->SetComponent(i, 0, enumeratedValues[ival]);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtApplyEnumerationExpression::ProcessArguments
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
//  Programmer:   Jeremy Meredith
//  Creation:     February 13, 2008
//
//  Modifications:
//    Jeremy Meredith, Mon Dec 15 12:52:34 EST 2008
//    Account for negative values in the enumeration list.
//
//    Jeremy Meredith, Mon Dec 15 14:31:42 EST 2008
//    Moved the accounting for unary minus into the parser itself, so
//    I removed the check for it in here.  The new fix/optimization
//    should apply to all expressions now.
//
// ****************************************************************************

void
avtApplyEnumerationExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    if (arguments->size() != 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the enumerate expression accepts only two arguments");
    }

    ArgExpr *listarg = (*arguments)[1];
    ExprParseTreeNode *listTree = listarg->GetExpr();
    if (listTree->GetTypeName() != "List")
    {
        debug1 << "avtApplyEnumerationExpression: second arg is not a list: "
               << listTree->GetTypeName() << endl;
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the last argument to enumerate "
                   "must be a list");
    }

    ListExpr *list = dynamic_cast<ListExpr*>(listTree);
    std::vector<ListElemExpr*> *elems = list->GetElems();
    enumeratedValues.resize(elems->size());
    for (int i = 0 ; i < elems->size() ; i++)
    {
        if ((*elems)[i]->GetEnd())
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                       "the list for the enumerate "
                        "expression cannot contain ranges.");
        }

        ExprNode *item = (*elems)[i]->GetItem();
        if (item->GetTypeName() == "FloatConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(item);
            enumeratedValues[i] = dynamic_cast<FloatConstExpr*>(c)->GetValue();
        }
        else if (item->GetTypeName() == "IntegerConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(item);
            enumeratedValues[i] = dynamic_cast<IntegerConstExpr*>(c)->GetValue();
        }
        else 
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                       "the list for the enumerate "
                       "expression may contain only numbers.");
        }
    }

    // Let the base class do this processing.  We only had to over-ride this
    // function to determine the number of arguments.
    avtMultipleInputExpressionFilter::ProcessArguments(args, state);
}


// ****************************************************************************
//  Method: avtApplyEnumerationExpression::UpdateDataObjectInfo
//
//  Purpose:
//      Tell the output what the component names are.
//
//  Programmer:   Jeremy Meredith
//  Creation:     February 13, 2008
//
//  Modifications:
// ****************************************************************************

void
avtApplyEnumerationExpression::UpdateDataObjectInfo(void)
{
    avtMultipleInputExpressionFilter::UpdateDataObjectInfo();
}
