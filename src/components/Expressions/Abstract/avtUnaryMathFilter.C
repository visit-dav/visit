// ************************************************************************* //
//                             avtUnaryMathFilter.C                          //
// ************************************************************************* //

#include <avtUnaryMathFilter.h>

#include <vtkDataSet.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtUnaryMathFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Wed Jun 12 16:44:28 PDT 2002
//
//  Notes:
//      Sean Ahern, Fri Jun 14 11:52:33 PDT 2002
//      Since the centering that's stored in
//      GetInput()->GetInfo().GetAttributes().GetCentering() is not on a
//      per-variable basis, we can't rely on it for the centering
//      information.  Instead, get the scalars from the point and cell
//      data.  Whichever one is non-NULL is the one we want.
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 14:30:21 PST 2002
//    Allow for arbitrary data array types.
//
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api. 
//
// ****************************************************************************

vtkDataArray *
avtUnaryMathFilter::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataArray *cell_data = in_ds->GetCellData()->GetArray(activeVariable);
    vtkDataArray *point_data = in_ds->GetPointData()->GetArray(activeVariable);
    vtkDataArray *data = NULL;

    if (cell_data != NULL)
    {
        data = cell_data;
        centering = AVT_ZONECENT;
    }
    else
    {
        data = point_data;
        centering = AVT_NODECENT;
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps = data->GetNumberOfComponents();
    int nvals  = data->GetNumberOfTuples();

    vtkDataArray *dv = data->NewInstance();
    dv->SetNumberOfComponents(ncomps);
    dv->SetNumberOfTuples(nvals);

    DoOperation(data, dv, ncomps, nvals);

    return dv;
}


