/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400152
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
//                 avtTimeIteratorDataTreeIteratorExpression.C               //
// ************************************************************************* //

#include <avtTimeIteratorDataTreeIteratorExpression.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <DebugStream.h>
#include <ExpressionException.h>

#include <snprintf.h>


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2009
//
// ****************************************************************************

avtTimeIteratorDataTreeIteratorExpression::avtTimeIteratorDataTreeIteratorExpression()
{
    arrayIndex = 0;
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2009
//
// ****************************************************************************

avtTimeIteratorDataTreeIteratorExpression::~avtTimeIteratorDataTreeIteratorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression::ProcessDataTree
//
//  Purpose:
//      Processes a data tree.
//
//  Programmer:   Hank Childs
//  Creation:     February 15, 2009
//
// ****************************************************************************

void
avtTimeIteratorDataTreeIteratorExpression::ProcessDataTree(avtDataTree_p tree,
                                                           int ts)
{
    arrayIndex = 0;
    InternalProcessDataTree(tree, ts);
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression::InternalProcessDataTree
//
//  Purpose:
//      Processes a data tree.
//
//  Programmer:   Hank Childs
//  Creation:     February 15, 2009
//
// ****************************************************************************

void
avtTimeIteratorDataTreeIteratorExpression::InternalProcessDataTree(
                                                    avtDataTree_p tree, int ts)
{
    if (*tree == NULL)
    {
        return;
    }

    int nc = tree->GetNChildren();

    if (nc <= 0 && !tree->HasData())
    {
        return;
    }

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = tree->GetDataRepresentation().GetDataVTK();
        PrepareAndExecuteDataset(in_ds, ts);
    }
    else
    {
        for (int j = 0; j < nc; j++)
            if (tree->ChildIsPresent(j))
                InternalProcessDataTree(tree->GetChild(j), ts);
    }
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression::PrepareAndExecuteDataset
//
//  Purpose:
//      Gets the proper data arrays and calls ExecuteDataset
//
//  Programmer:   Hank Childs
//  Creation:     February 15, 2009
//
// ****************************************************************************

void
avtTimeIteratorDataTreeIteratorExpression::PrepareAndExecuteDataset(
                                                        vtkDataSet *ds, int ts)
{
    int  i;

    std::vector<vtkDataArray *> ds_vars;
    std::vector<vtkDataArray *> delete_vars;

    bool haveZonal = false;
    bool haveNodal = false;

    int nvars = varnames.size();
    if (cmfeType == POS_CMFE)
        nvars--;
    for (i = 0 ; i < nvars ; i++)
    {
        std::string vname = GetInternalVarname(i);
        vtkDataArray *cell_data1 = ds->GetCellData()->GetArray(vname.c_str());
        vtkDataArray *point_data1 = ds->GetPointData()->GetArray(vname.c_str());

        if (cell_data1 != NULL)
            haveZonal = true;
        else if (point_data1 != NULL)
            haveNodal = true;
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                       "An internal error occurred when calculating an expression."
                       "  Please contact a VisIt developer.");
        }
    }

    bool doZonal = false;
    if (haveZonal)
        doZonal = true;  // mixed centering -> zonal

    for (i = 0 ; i < nvars ; i++)
    {
        std::string vname = GetInternalVarname(i);
        vtkDataArray *cell_data1 = ds->GetCellData()->GetArray(vname.c_str());
        vtkDataArray *point_data1 = ds->GetPointData()->GetArray(vname.c_str());

        if (doZonal)
        { 
            if (cell_data1 != NULL)
                ds_vars.push_back(cell_data1);
            else
            {
                vtkDataArray *tmp = Recenter(ds, point_data1, AVT_NODECENT, 
                                             varnames[i], AVT_ZONECENT);
                ds_vars.push_back(tmp);
                delete_vars.push_back(tmp);
            }
        }
        else
        {
            if (point_data1 != NULL)
                ds_vars.push_back(point_data1);
            else
            {
                vtkDataArray *tmp = Recenter(ds, cell_data1, AVT_ZONECENT, 
                                             varnames[i], AVT_NODECENT);
                ds_vars.push_back(tmp);
                delete_vars.push_back(tmp);
            }
        }
    }

    vtkDataArray *out_arr = vars[arrayIndex++];
    ExecuteDataset(ds_vars, out_arr, ts);

    for (i = 0 ; i < delete_vars.size() ; i++)
        delete_vars[i]->Delete();
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression::InitializeOutput
//
//  Purpose:
//      Sets up the data arrays we will use later.
//
//  Programmer:   Hank Childs
//  Creation:     February 15, 2009
//
// ****************************************************************************

void
avtTimeIteratorDataTreeIteratorExpression::InitializeOutput(void)
{
    avtDataTree_p topTree = GetInputDataTree();
    std::vector<avtDataTree_p> treesToProcess;
    treesToProcess.push_back(topTree);
    int curIndex = 0;
    while (curIndex < treesToProcess.size())
    {
        avtDataTree_p tree = treesToProcess[curIndex];
        curIndex++;
        if (*tree == NULL)
            continue;

        int nc = tree->GetNChildren();
        if (nc <= 0 && !tree->HasData())
            continue;

        if (nc == 0)
        {
            vtkDataSet *in_ds = tree->GetDataRepresentation().GetDataVTK();
            vtkFloatArray *arr = vtkFloatArray::New();
            arr->SetNumberOfComponents(GetIntermediateSize());
            if (IsPointVariable())
                arr->SetNumberOfTuples(in_ds->GetNumberOfPoints());
            else
                arr->SetNumberOfTuples(in_ds->GetNumberOfCells());
            arr->SetName(outputVariableName);
            vars.push_back(arr);
        }
        else
        {
            for (int j = 0; j < nc; j++)
                if (tree->ChildIsPresent(j))
                    treesToProcess.push_back(tree->GetChild(j));
        }
    }
    
    // Set up this counter variable that we use during "ProcessDataTree".
    arrayIndex = 0;
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression::FinalizeOutput
//
//  Purpose:
//      Take the data array we have created and add it to the output.
//
//  Programmer:   Hank Childs
//  Creation:     February 15, 2009
//
// ****************************************************************************

void
avtTimeIteratorDataTreeIteratorExpression::FinalizeOutput(void)
{
    avtDataTree_p tree = GetInputDataTree();
    arrayIndex = 0;
    avtDataTree_p rv = ConstructOutput(tree);
    SetOutputDataTree(rv);
}


// ****************************************************************************
//  Method: avtTimeIteratorDataTreeIteratorExpression::ConstructOutput
//
//  Purpose:
//      A helper method for FinalizeOutput that iterates over a tree.
//
//  Programmer:   Hank Childs
//  Creation:     February 15, 2009
//
// ****************************************************************************

avtDataTree_p
avtTimeIteratorDataTreeIteratorExpression::ConstructOutput(avtDataTree_p t)
{
    if (*t == NULL)
    {
        return NULL;
    }

    int nc = t->GetNChildren();

    if (nc <= 0 && !t->HasData())
    {
        return NULL;
    }

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = t->GetDataRepresentation().GetDataVTK();
        vtkDataSet *new_ds = in_ds->NewInstance();
        new_ds->ShallowCopy(in_ds);
        vtkDataArray *final_arr = 
                        ConvertIntermediateArrayToFinalArray(vars[arrayIndex]);
        vars[arrayIndex]->Delete();
        arrayIndex++;
        if (IsPointVariable())
            new_ds->GetPointData()->AddArray(final_arr);
        else
            new_ds->GetCellData()->AddArray(final_arr);
        final_arr->Delete();
        avtDataTree_p rv = new avtDataTree(new_ds,
                                   t->GetDataRepresentation().GetDomain(),
                                   t->GetDataRepresentation().GetLabel());
        new_ds->Delete();
        return rv;
    }
    else
    {
        // there is more than one input dataset to process
        // and we need an output datatree for each
        //
        avtDataTree_p *outDT = new avtDataTree_p[nc];
        for (int j = 0; j < nc; j++)
        {
            if (t->ChildIsPresent(j))
            {
                outDT[j] = ConstructOutput(t->GetChild(j));
            }
            else
            {
                outDT[j] = NULL;
            }
        }
        avtDataTree_p rv = new avtDataTree(nc, outDT);
        delete [] outDT;
        return (rv);
    }

    return NULL;
}


// ****************************************************************************
//  avtTimeIteratorDataTreeIteratorExpression::
//                                         ConvertIntermediateArrayToFinalArray
//
//  Purpose:
//      Some derived types may use a data array when doing its processing that
//      is bigger than the output.  If that is the case, we need to process
//      the output before exiting.  It is up to the derived type to do this.
//      So this method is a hook ... basically a no-op ... for those derived
//      types.
//
//  Note:   It is assumed that the return value of this method should have its
//          reference count decremented.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2009
//
// ****************************************************************************

vtkDataArray *
avtTimeIteratorDataTreeIteratorExpression::ConvertIntermediateArrayToFinalArray
                                             (vtkDataArray *intermediateArray)
{
    if (GetIntermediateSize() != GetVariableDimension())
    {
        // If the intermediate size is redefined, then this method must be
        // redefined as well.
        EXCEPTION0(ImproperUseException);
    }

    intermediateArray->Register(NULL);
    return intermediateArray;
}


// ****************************************************************************
//  Method:  avtTimeIteratorDataTreeIteratorExpression::GetVariableType
//
//  Purpose:
//    Try to do better than unknown type for the output type.
//    Specifically, the time iteration stuff typically makes the output
//    num components the same as the input, so we try to re-use the
//    input data type.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 18, 2009
//
// ****************************************************************************
avtVarType
avtTimeIteratorDataTreeIteratorExpression::GetVariableType()
{
    if (varnames.size() != 1)
        return AVT_UNKNOWN_TYPE;

    avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
    if (!inatts.ValidVariable(varnames[0]))
        return AVT_UNKNOWN_TYPE;

    return inatts.GetVariableType(varnames[0].c_str());
}
