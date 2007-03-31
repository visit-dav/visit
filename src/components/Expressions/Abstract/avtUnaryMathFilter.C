// ************************************************************************* //
//                             avtUnaryMathFilter.C                          //
// ************************************************************************* //

#include <avtUnaryMathFilter.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

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
//    Hank Childs, Fri Sep 19 16:47:36 PDT 2003
//    Allow derived types to specify how many components there are in the 
//    output.
//
//    Hank Childs, Mon Nov  3 16:02:21 PST 2003
//    Make use of virtual function CreateArray to create VTK arrays.
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
        //
        // This hack is getting more and more refined.  This situation comes up
        // when we don't know what the active variable is (mostly for the
        // constant creation filter).  We probably need more infrastructure
        // to handle this.
        // Iteration 1 of this hack said take any array.
        // Iteration 2 said take any array that isn't vtkGhostLevels, etc.
        // Iteration 3 says take the first scalar array if one is available,
        //             provided that array is not vtkGhostLevels, etc.
        //             This is because most constants we create are scalar.
        //
        int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
        for (i = 0 ; i < ncellArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetCellData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                cell_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                cell_data = candidate;
        }
        int npointArray = in_ds->GetPointData()->GetNumberOfArrays();
        for (i = 0 ; i < npointArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetPointData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                point_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                point_data = candidate;
        }

        if (cell_data != NULL && cell_data->GetNumberOfComponents() == 1)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else if (point_data != NULL && point_data->GetNumberOfComponents()== 1)
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
        else if (cell_data != NULL)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    } 
    else
    {
        cell_data = in_ds->GetCellData()->GetArray(activeVariable);
        point_data = in_ds->GetPointData()->GetArray(activeVariable);

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
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps = data->GetNumberOfComponents();
    int nvals  = data->GetNumberOfTuples();

    vtkDataArray *dv = CreateArray(data);
    int noutcomps = GetNumberOfComponentsInOutput(ncomps);
    dv->SetNumberOfComponents(noutcomps);
    dv->SetNumberOfTuples(nvals);

    //
    // Should we send in ncomps or noutcomps?  They are the same number 
    // unless the derived type re-defined GetNumberOfComponentsInOutput.
    // If it did, it probably doesn't matter.  If not, then it is the same
    // number.  So send in the input.  Really doesn't matter.
    //
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


