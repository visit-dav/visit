/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                      avtLocalThresholdExpression.C                        //
// ************************************************************************* //

#include <avtLocalThresholdExpression.h>

#include <vector>
#include <algorithm>
#include <map>
#include <stack>

#include "vtkDataSet.h"
#include "vtkDataArray.h"
#include "vtkTypeUInt32Array.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkMergeTree.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>



// human readable transform name table
const char* avtLocalThresholdExpression::TransformationName[] = 
{
    "relevance",
};

// ****************************************************************************
//  Method: avtLocalThresholdExpression constructor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtLocalThresholdExpression::avtLocalThresholdExpression()
{
}

// ****************************************************************************
//  Method: avtLocalThresholdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtLocalThresholdExpression::~avtLocalThresholdExpression()
{
  ; // For now ... Later we need to delete the tree
}

// ****************************************************************************
//  Method: avtLocalThresholdExpression::DeriveVariable
//
//  Purpose:
//      Called to calc the expression results.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************

vtkDataArray *
avtLocalThresholdExpression::DeriveVariable(vtkDataSet *in_ds,
                                            int currentDomainsIndex)
{
    vtkIdType i;

    // Get the original function values
    vtkDataArray* function;

    // Just as in the mergetree expression we assume the data is node centered
    function = in_ds->GetPointData()->GetArray(varnames[0]);

    // IF we cannot find node-centered data
    if (function == NULL)
    {
        // We try with cell-centered
        function = in_ds->GetCellData()->GetArray(varnames[0]);
    }


    // If we still can't find anything we abort
    if (function == NULL)
    {
        EXCEPTION2(ExpressionException,
                   outputVariableName,
                   "avtLocalThresholdExpression: Could not find function.");
        return NULL;
    }


    vtkTypeUInt32Array* labels;

    // Similarly, find either node- or cell-centered labels
    labels = vtkTypeUInt32Array::SafeDownCast(in_ds->GetPointData()->GetArray(varnames[1]));

    // If we cannot find node-centered labels
    if (labels == NULL)
    {
        // We try our luck with cell centered data
        labels = vtkTypeUInt32Array::SafeDownCast(in_ds->GetCellData()->GetArray(varnames[1]));
    }

    // If we still can't find anything we abort
    if (labels == NULL)
    {
        EXCEPTION2(ExpressionException,
                   outputVariableName,
                   "avtLocalThresholdExpression: Could not find labels.");
                   return NULL;
    }

    // Create an array for the local thresholds
    vtkFloatArray* local = vtkFloatArray::New();
    local->SetNumberOfComponents(1);
    local->SetName("Local Thresholds");

    // For now we will create a full sized array
    local->SetNumberOfTuples(function->GetNumberOfTuples());

    Transformation* trans;
    switch (transformation)
    {
        case RELEVANCE:
            trans = new Relevance();
            break;
        case UNDEFINED:
            trans = NULL;
            return NULL;
        default:
            trans = NULL;
            return NULL;
    }

    // For now we pass this as a static variable
    vtkMergeTree* tree = avtMergeTreeExpression::tree;

    for (i=0;i<local->GetNumberOfTuples();i++)
    {
        if (labels->GetTuple1(i) != avtMergeTreeExpression::LNULL)
        {
            local->SetTuple1(i,trans->eval(tree,function,labels,i));
        }
        else
        {
            local->SetTuple1(i,trans->fillValue());
        }
    }

    return local;
}

// ****************************************************************************
//  Method: avtLocalThresholdExpression::ProcessArguments
//
//  Purpose:
//      Processes args passed to the expression.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************

void
avtLocalThresholdExpression::ProcessArguments(ArgsExpr* args,
                                              ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    if (nargs < 2)
    {
        EXCEPTION2(ExpressionException,
                   outputVariableName,
                   "avtMergeTreeExpression: Too few arguments given.");
    }
    
    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Tell the second argument to create its filters.
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    secondTree->CreateFilters(state);


    // Check if there is no third argument.
    if (nargs == 2)
    {
        this->transformation = RELEVANCE;
    }
    else
    {
        // Pull off the second argument and check whether we can convert that into a transformation
        ArgExpr *thirdarg = (*arguments)[2];
        this->transformation = DetermineTransformationType(thirdarg->GetText());
    }

    debug1 << "Parsed arguments in LocalThreshold " 
           << TransformationName[this->transformation]
           << std::endl; 
}


// ****************************************************************************
//  Method: avtLocalThresholdExpression::DetermineTransformationType
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtLocalThresholdExpression::TransformationType
avtLocalThresholdExpression::DetermineTransformationType(const std::string& name)
{
    for (uint8_t i=0; i<UNDEFINED; i++)
    {
        if (name == TransformationName[i])
            return (TransformationType)i;
    }

    return UNDEFINED;
}

// ****************************************************************************
//  Method: avtLocalThresholdExpression::Relevance::eval
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
double
avtLocalThresholdExpression::Relevance::eval(vtkMergeTree* tree,
                                             vtkDataArray* function,
                                             vtkDataArray* labels,
                                             vtkIdType index)
{
    vtkTypeUInt32 label = labels->GetTuple1(index);

    if (label == avtMergeTreeExpression::LNULL)
        return fillValue();

    double local_max = function->GetTuple1(tree->GetId(tree->GetRep(label)));

    return 1 - (local_max - function->GetTuple1(index)) / (local_max - tree->GetMinimum());
}



