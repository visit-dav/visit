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
//    Sean Ahern, Tue Mar  4 18:43:51 America/Los_Angeles 2003
//    If we don't have an active variable name, just use array 0.  There
//    *has* to be one in the dataset.  Unfortunately, we don't know that
//    it's array 0, but let's go with it for now.
//
//    Hank Childs, Thu Aug 21 23:49:59 PDT 2003
//    Avoid choosing an array that is 'vtkGhostLevels', etc.
//
// ****************************************************************************

vtkDataArray *
avtUnaryMathFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int  i;

    vtkDataArray *cell_data = NULL;
    vtkDataArray *point_data = NULL;
    vtkDataArray *data = NULL;

    if (activeVariable == NULL)
    {
        // HACK: We don't know what the default variable is, so just go for
        // array 0.  It's probably right, but we don't know.  XXXX
        int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
        for (i = 0 ; i < ncellArray ; i++)
        {
            cell_data = in_ds->GetCellData()->GetArray(i);
            if (strstr(cell_data->GetName(), "vtk") != NULL)
            {
                cell_data = NULL;
                continue;
            }
            if (strstr(cell_data->GetName(), "avt") != NULL)
            {
                cell_data = NULL;
                continue;
            }
            if (cell_data != NULL) // We found a winner
                break;
        }
        int npointArray = in_ds->GetPointData()->GetNumberOfArrays();
        for (i = 0 ; i < npointArray ; i++)
        {
            point_data = in_ds->GetPointData()->GetArray(i);
            if (strstr(point_data->GetName(), "vtk") != NULL)
            {
                point_data = NULL;
                continue;
            }
            if (strstr(point_data->GetName(), "avt") != NULL)
            {
                point_data = NULL;
                continue;
            }
            if (point_data != NULL) // We found a winner
                break;
        }
    } else
    {
        cell_data = in_ds->GetCellData()->GetArray(activeVariable);
        point_data = in_ds->GetPointData()->GetArray(activeVariable);
    }

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


// ****************************************************************************
//  Method: avtUnaryMathFilter::CreateArray
//
//  Purpose:
//      Creates an array to write the output into.  Most derived types want
//      the array to be of the same form as the input.  Some (like logical
//      operators) always want them to be a specific type (like uchar).
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

vtkDataArray *
avtUnaryMathFilter::CreateArray(vtkDataArray *in1)
{
    return in1->NewInstance();
}


