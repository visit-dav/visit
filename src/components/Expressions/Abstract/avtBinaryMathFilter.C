// ************************************************************************* //
//                             avtBinaryMathFilter.C                         //
// ************************************************************************* //

#include <avtBinaryMathFilter.h>

#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryMathFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on two variables in the input dataset.
//
//  Arguments:
//      in_ds     The first input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Tue Jun 25 19:42:13 PDT 2002
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
avtBinaryMathFilter::DeriveVariable(vtkDataSet *in_ds)
{
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL;

    if (cell_data1 != NULL)
    {
        data1 = cell_data1;
        centering = AVT_ZONECENT;
    }
    else
    {
        data1 = point_data1;
        centering = AVT_NODECENT;
    }

    // Get the second variable.
    if (centering == AVT_ZONECENT)
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
    else
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);

    if (data2 == NULL)
    {
        EXCEPTION1(ExpressionException, 
                   "the two variables have different centering.");
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps = data1->GetNumberOfComponents();
    int nvals  = data1->GetNumberOfTuples();

    vtkDataArray *dv = data1->NewInstance();
    dv->SetNumberOfComponents(ncomps);
    dv->SetNumberOfTuples(nvals);

    DoOperation(data1, data2, dv, ncomps, nvals);

    return dv;
}
