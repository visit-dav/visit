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
//                        avtFindExternalExpression.C                        //
// ************************************************************************* //

#include <avtFindExternalExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>

#include <avtFacelistFilter.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtFindExternalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 21, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Feb  4 13:46:18 PST 2011
//    Initialize doCells.
//
// ****************************************************************************

avtFindExternalExpression::avtFindExternalExpression()
{
    doCells = false;
}


// ****************************************************************************
//  Method: avtFindExternalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 21, 2005
//
// ****************************************************************************

avtFindExternalExpression::~avtFindExternalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtFindExternalExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     September 21, 2005
//
//  Modifications:
//
//    Hank Childs, Wed Sep 28 11:07:50 PDT 2005
//    Use correct form of delete operator.
//
//    Hank Childs, Fri Feb  4 13:46:18 PST 2011
//    Extend to cells as well.
//
//    Hank Childs, Mon Feb  7 07:00:30 PST 2011
//    Fix problem with finding external nodes with unstructured meshes.
//
//    Hank Childs, Tue Feb  8 13:24:25 PST 2011
//    Add support for "corner case" where nothing is on the exterior.
//
// ****************************************************************************

vtkDataArray *
avtFindExternalExpression::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataSet *new_ds = in_ds->NewInstance();
    new_ds->ShallowCopy(in_ds);
    vtkIdType nids = (doCells ? new_ds->GetNumberOfCells()
                        : new_ds->GetNumberOfPoints());
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(nids);
    for (vtkIdType i = 0 ; i < nids ; i++)
        arr->SetValue(i, (int)i);
    const char *varname = "_avt_id";
    arr->SetName(varname);
    if (doCells)
        new_ds->GetCellData()->AddArray(arr);
    else
        new_ds->GetPointData()->AddArray(arr);
    arr->Delete();

    avtDataTree_p tree = avtFacelistFilter::FindFaces(new_ds, -1, "",
                                  GetInput()->GetInfo(), false, false,
                                  true, true, NULL);
    vtkDataSet *ds = tree->GetSingleLeaf();

    vtkPolyDataRelevantPointsFilter *pdrpf = NULL;
    if (!doCells)
    {
        // If we have an unstructured grid, then the facelist filter sent
        // back the same point list.  Reduce the point list to just those
        // that are on the boundary.
        if (ds->GetDataObjectType() == VTK_POLY_DATA)
        {
            pdrpf = vtkPolyDataRelevantPointsFilter::New();
            pdrpf->SetInput((vtkPolyData *) ds);
            pdrpf->Update();
            ds = pdrpf->GetOutput();
        }
    }

    vtkDataArray *arr2 = NULL;
    if (doCells)
        arr2 =  ds->GetCellData()->GetArray(varname);
    else
        arr2 =  ds->GetPointData()->GetArray(varname);
    bool haveArray = true;
    if (arr2 == NULL || arr2->GetDataType() != VTK_INT)
    {
        if (ds->GetNumberOfCells() > 0)
        {
            EXCEPTION2(ExpressionException, outputVariableName, "An internal "
                   "error occurred when calculating the external nodes.");
        }
        else
            haveArray = false;
    }
    vtkIntArray *arr3 = (vtkIntArray *) arr2;

    bool *haveId = new bool[nids];
    for (vtkIdType i = 0 ; i < nids ; i++)
        haveId[i] = false;

    if (haveArray)
    {
        vtkIdType nArr = arr3->GetNumberOfTuples();
        for (vtkIdType i = 0 ; i < nArr ; i++)
            haveId[arr3->GetValue(i)] = true;
    }

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(nids);
    for (vtkIdType i = 0 ; i < nids ; i++)
        rv->SetTuple1(i, (haveId[i] ? 1 : 0));

    delete [] haveId;
    new_ds->Delete();
    if (pdrpf != NULL)
        pdrpf->Delete();

    return rv;
}


