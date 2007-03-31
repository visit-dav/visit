// ************************************************************************* //
//                         avtVectorDecomposeFilter.C                       //
// ************************************************************************* //

#include <avtVectorDecomposeFilter.h>

#include <math.h>

#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorDecomposeFilter::DeriveVariable
//
//  Purpose:
//      Pulls out a component of a vector variable.
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
//    Sean Ahern, Thu Mar  6 02:00:32 America/Los_Angeles 2003
//    Merged the vector component filters together.
//
// ****************************************************************************

vtkDataArray *
avtVectorDecomposeFilter::DeriveVariable(vtkDataSet *in_ds)
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
        float val = arr->GetComponent(i, which_comp);
        rv->SetTuple1(i, val);
    }

    return rv;
}


