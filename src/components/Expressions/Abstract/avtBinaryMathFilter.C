// ************************************************************************* //
//                             avtBinaryMathFilter.C                         //
// ************************************************************************* //

#include <avtBinaryMathFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryMathFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMathFilter::avtBinaryMathFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryMathFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryMathFilter::~avtBinaryMathFilter()
{
    ;
}


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
//    Hank Childs, Thu Aug 14 13:40:20 PDT 2003
//    Allow the derived types to specify how many components there will be in
//    the output.
//
//    Hank Childs, Wed Dec 10 11:11:46 PST 2003
//    Do a better job of handling variables with different centerings.
//
//    Hank Childs, Thu Apr 22 11:11:33 PDT 2004
//    When the centerings are different, always use zonal.
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
    bool ownData1 = false;
    bool ownData2 = false;
    if (centering == AVT_ZONECENT)
    {
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
        if (data2 == NULL)
        {
            data2 = in_ds->GetPointData()->GetArray(varnames[1]);
            if (data2 != NULL)
            {
                data2 = Recenter(in_ds, data2, AVT_NODECENT);
                ownData2 = true;
            }
            else
            {
                EXCEPTION1(ExpressionException, "Unable to locate variable");
            }
        }
    }
    else
    {
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);
        if (data2 == NULL)
        {
            data2 = in_ds->GetCellData()->GetArray(varnames[1]);
            if (data2 != NULL)
            {
                // Recenter data1 so it will match data2.  We want both zonal.
                data1 = Recenter(in_ds, data1, AVT_NODECENT);
                centering = AVT_ZONECENT;
                ownData1 = true;
            }
            else
            {
                EXCEPTION1(ExpressionException, "Unable to locate variable");
            }
        }
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps1 = data1->GetNumberOfComponents();
    int ncomps2 = data2->GetNumberOfComponents();
    int ncomps = GetNumberOfComponentsInOutput(ncomps1, ncomps2);
    int nvals  = data1->GetNumberOfTuples();

    vtkDataArray *dv = CreateArray(data1);
    dv->SetNumberOfComponents(ncomps);
    dv->SetNumberOfTuples(nvals);

    DoOperation(data1, data2, dv, ncomps, nvals);

    if (GetOutput()->GetInfo().GetAttributes().GetVariableDimension()
        != ncomps)
    {
        GetOutput()->GetInfo().GetAttributes().SetVariableDimension(ncomps);
    }

    if (ownData1)
    {
        data1->Delete();
    }
    if (ownData2)
    {
        data2->Delete();
    }

    return dv;
}


// ****************************************************************************
//  Method: avtBinaryMathFilter::CreateArray
//
//  Purpose:
//      Creates an array to write the output into.  Most derived types want
//      the array to be of the same form as the input.  Some (like logical
//      operators) always want them to be a specific type (like uchar).
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2003
//
// ****************************************************************************

vtkDataArray *
avtBinaryMathFilter::CreateArray(vtkDataArray *in1)
{
    return in1->NewInstance();
}


