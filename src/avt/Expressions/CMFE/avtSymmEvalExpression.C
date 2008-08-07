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
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:33:34 EDT 2008
//    Use %ld for long integers.
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
        EXCEPTION2(ExpressionException, outputVariableName, badSyntaxMsg);

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
        EXCEPTION2(ExpressionException, outputVariableName, badSyntaxMsg);

    // It's a list.  Process all of them.
    inputParameters.clear();
    ListExpr *list = dynamic_cast<ListExpr*>(thirdTree);
    std::vector<ListElemExpr*> *elems = list->GetElems();
    if (elems->size() != GetNumberOfInputParameters())
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "%s.  You're list had %ld numbers, where %d were "
                  "expected.", badSyntaxMsg, elems->size(), 
                 GetNumberOfInputParameters());
        EXCEPTION2(ExpressionException, outputVariableName, msg);
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
            EXCEPTION2(ExpressionException, outputVariableName, msg);
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
            EXCEPTION2(ExpressionException, outputVariableName, badSyntaxMsg);
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
        EXCEPTION2(ExpressionException, outputVariableName, "Symmetry evaluation expressions "
                      "must take the form: <var> <default-var> [parameters]");
    }

    avtDataTree_p output = avtPosCMFEAlgorithm::PerformCMFE(in1, in2,
                                 varnames[0], varnames[1], outputVariableName);
    SetOutputDataTree(output);
}


