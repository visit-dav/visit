// ************************************************************************* //
//                         avtVectorComponent1Filter.C                       //
// ************************************************************************* //

#include <avtVectorComponent1Filter.h>

#include <math.h>

#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorComponent1Filter::DeriveVariable
//
//  Purpose:
//      Pulls out the first component of a vector variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 18, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api.
//
// ****************************************************************************

vtkDataArray *
avtVectorComponent1Filter::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataArray *arr = NULL;

    if (in_ds->GetPointData()->GetVectors() != NULL)
    {
        arr = in_ds->GetPointData()->GetVectors();
    }
    else
    {
        arr = in_ds->GetCellData()->GetVectors();
    }

    if (arr == NULL)
    {
        EXCEPTION1(ExpressionException, "Cannot locate variable");
    }

    int ntuples = arr->GetNumberOfTuples();

    vtkDataArray *rv = arr->NewInstance();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(ntuples);
    for (int i = 0 ; i < ntuples ; i++)
    {
        float val = arr->GetComponent(i, 0);
        rv->SetTuple1(i, val);
    }

    return rv;
}


