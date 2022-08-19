// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Mark C. Miller, Wed Aug 22 09:30:01 PDT 2012
//    Fixed leak of 'pdrpf' on early return due to EXCEPTION.
//
//    David Camp, Thu May 23 12:52:53 PDT 2013
//    Change the avtFacelistFilter::FindFaces from a static function to a 
//    normal function for the thread code to work.
//
//    Eric Brugger, Mon Jul 21 12:09:02 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

vtkDataArray *
avtFindExternalExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
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

    avtFacelistFilter *flf = new avtFacelistFilter();
    avtDataRepresentation new_dr(new_ds, -1, "");
    avtDataTree_p tree = flf->FindFaces(&new_dr,
                                  GetInput()->GetInfo(), false, false,
                                  true, true, NULL);
    delete flf;
    vtkDataSet *ds = tree->GetSingleLeaf();

    vtkPolyDataRelevantPointsFilter *pdrpf = NULL;
    if (!doCells && ds->GetNumberOfPoints() > 0)
    {
        // If we have an unstructured grid, then the facelist filter sent
        // back the same point list.  Reduce the point list to just those
        // that are on the boundary.
        if (ds->GetDataObjectType() == VTK_POLY_DATA)
        {
            pdrpf = vtkPolyDataRelevantPointsFilter::New();
            pdrpf->SetInputData((vtkPolyData *) ds);
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
            if (pdrpf != NULL)
                pdrpf->Delete();
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


