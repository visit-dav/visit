/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                              avtResampleExpression.C                      //
// ************************************************************************* //

#include <avtResampleExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <avtResampleFilter.h>
#include <InternalResampleAttributes.h>
#include <avtSourceFromAVTDataset.h>
#include <avtExprNode.h>
#include <avtDatasetExaminer.h>

#include <DebugStream.h>
#include <ExpressionException.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtResampleExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
//  Modifications:
//
// ****************************************************************************

avtResampleExpression::avtResampleExpression()
{
    samplesX = 1;
    samplesY = 1;
    samplesZ = 1;
}


// ****************************************************************************
//  Method: avtResampleExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
// ****************************************************************************

avtResampleExpression::~avtResampleExpression()
{
}


// ****************************************************************************
//  Method: avtResampleExpression::ProcessArguments
//
//  Purpose:
//      Parses optional centering argument.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
// ****************************************************************************

void
avtResampleExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Get the argument list and number of arguments.
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    // Check for a call with no arguments.
    if (nargs != 4)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "resample(): Incorrect syntax.\n"
                   " usage: resample(varname, samplesX, samplesY, samplesZ)\n");
    }

    //Variable.
    ArgExpr *firstArg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstArg->GetExpr());
    firstTree->CreateFilters(state);

    for (int i = 0; i < 3; i++)
    {
        ArgExpr *samp = (*arguments)[i+1];
        ExprParseTreeNode *sampTree = samp->GetExpr();
        std::string secondType = sampTree->GetTypeName();
        if (secondType == "IntegerConst")
        {
            int val = dynamic_cast<IntegerConstExpr*>(sampTree)->GetValue();

            if (val <= 0)
                EXCEPTION2(ExpressionException, outputVariableName,
                           "avtResampleExpression: Invalid resample argument.\n"
                           "Must be > 0.");
            if (i == 0)
                samplesX = val;
            else if (i == 1)
                samplesY = val;
            else
                samplesZ = val;
        }
    }
}


// ****************************************************************************
//  Method: avtResampleExpression::Execute
//
//  Purpose:
//      Execute
//
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
// ****************************************************************************

void
avtResampleExpression::Execute()
{
    InternalResampleAttributes resampleAtts;
    resampleAtts.SetUseBounds(false);
    resampleAtts.SetDistributedResample(true);

    resampleAtts.SetWidth(samplesX);
    resampleAtts.SetHeight(samplesY);
    resampleAtts.SetDepth(samplesZ);

    avtResampleFilter *resample = new avtResampleFilter(&resampleAtts);
    resample->SetInput(GetInput());
    
    avtContract_p spec = GetInput()->GetOriginatingSource()->GetGeneralContract();
    avtDataObject_p dob = resample->GetOutput();
    dob->Update(spec);

    //Need to rename the output variable.
    avtDataTree_p tree = resample->GetTypedOutput()->GetDataTree();
    int nLeaves;
    vtkDataSet **leaves = tree->GetAllLeaves(nLeaves);
    
    std::string inputVarname = GetInput()->GetInfo().GetAttributes().GetVariableName();
    std::string outputVarname = GetOutputVariableName();
    
    avtDataTree_p newTree = NULL;
    for (int i = 0; i < nLeaves; i++)
    {
        vtkDataSet *ds = leaves[i];
        vtkDataArray *oldVar = ds->GetPointData()->GetArray(inputVarname.c_str());

        vtkDataArray *newVar = oldVar->NewInstance();
        newVar->SetNumberOfComponents(oldVar->GetNumberOfComponents());
        newVar->SetNumberOfTuples(oldVar->GetNumberOfTuples());
        for (vtkIdType j = 0; j < newVar->GetNumberOfTuples(); j++)
            newVar->SetTuple(j, oldVar->GetTuple(j));
        newVar->SetName(outputVarname.c_str());
        
        ds->GetPointData()->RemoveArray(inputVarname.c_str());
        ds->GetPointData()->AddArray(newVar);
        newVar->Delete();

        if (i == 0)
            newTree = new avtDataTree(ds, i);
        else
            newTree->Merge(new avtDataTree(ds, i));
    }
    
    delete [] leaves;
    if (*newTree != NULL)
        SetOutputDataTree(newTree);
}

// ****************************************************************************
//  Method: avtResampleExpression::IsPointVariable
//
//  Purpose:
//      Switch the centering of the point variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
//  Modifications:
//
//
// ****************************************************************************

bool
avtResampleExpression::IsPointVariable(void)
{
    bool isPoint  = true;
    bool foundOne = false;
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (activeVariable != NULL)
    {
        if (atts.ValidVariable(activeVariable))
        {
            isPoint = (atts.GetCentering(activeVariable) != AVT_ZONECENT);
            foundOne = true;
        }
    }

    if (!foundOne)
        if (atts.ValidActiveVariable())
            isPoint = (atts.GetCentering() != AVT_ZONECENT);
    
    return (!isPoint);
}


// ****************************************************************************
//  Method: avtResampleExpression::GetVariableDimension
//
//  Purpose:
//      Determines what the variable dimension of the output is.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Jul  2 14:10:45 EDT 2010
//
// ****************************************************************************

int
avtResampleExpression::GetVariableDimension(void)
{
    if (*(GetInput()) == NULL)
        return avtExpressionFilter::GetVariableDimension();

    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;
    if (varname == NULL)
        return avtExpressionFilter::GetVariableDimension();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varname))
        return avtExpressionFilter::GetVariableDimension();
    int ncomp = atts.GetVariableDimension(varname);
    return ncomp;
}
