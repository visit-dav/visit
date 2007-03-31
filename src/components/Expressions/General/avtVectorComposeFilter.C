// ************************************************************************* //
//                         avtVectorComposeFilter.C                          //
// ************************************************************************* //

#include <avtVectorComposeFilter.h>

#include <math.h>

#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorComposeFilter::DeriveVariable
//
//  Purpose:
//      Creates a vector variable from components.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Thu Mar  6 19:40:32 America/Los_Angeles 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 11:01:34 PDT 2003
//    Add better support for 2D vectors.
//
// ****************************************************************************
vtkDataArray *
avtVectorComposeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL, *data3 = NULL;

    avtCentering centering;
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
                   "The first two variables have different centering.");
    }

    bool twoDVector = 
            (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2);

    if (!twoDVector)
    {
        // Get the third variable.
        if (centering == AVT_ZONECENT)
            data3 = in_ds->GetCellData()->GetArray(varnames[2]);
        else
            data3 = in_ds->GetPointData()->GetArray(varnames[2]);
    
        if (data3 == NULL)
        {
            EXCEPTION1(ExpressionException, 
                   "The first and third variables have different centering.");
        }
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int nvals  = data1->GetNumberOfTuples();
    vtkDataArray *dv = data1->NewInstance();
    dv->SetNumberOfComponents(3);
    dv->SetNumberOfTuples(nvals);

    for (int i = 0 ; i < nvals ; i++)
    {
        float val1 = data1->GetComponent(i, 0);
        float val2 = data2->GetComponent(i, 0);
        if (twoDVector)
            dv->SetTuple3(i, val1, val2, 0.);
        else
        {
            float val3 = data3->GetComponent(i, 0);
            dv->SetTuple3(i, val1, val2, val3);
        }
    }

    return dv;
}
