// ************************************************************************* //
//                           avtConditionalFilter.C                          //
// ************************************************************************* //

#include <avtConditionalFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtConditionalFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConditionalFilter::avtConditionalFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtConditionalFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConditionalFilter::~avtConditionalFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtConditionalFilter::DeriveVariable
//
//  Purpose:
//      Selects between two arrays based on a condition.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     August 20, 2003
//
// ****************************************************************************

vtkDataArray *
avtConditionalFilter::DeriveVariable(vtkDataSet *in_ds)
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

    if (data1 == NULL)
    {
        EXCEPTION1(ExpressionException, 
                   "Could not identify centering of first variable");
    }
    if (data1->GetNumberOfComponents() != 1)
    {
        EXCEPTION1(ExpressionException, 
                   "Cannot interpret conditional with vector dimensionality");
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

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int nvals  = data2->GetNumberOfTuples();
    vtkDataArray *dv = data2->NewInstance();
    dv->SetNumberOfComponents(data2->GetNumberOfComponents());
    dv->SetNumberOfTuples(nvals);

    if (data1->GetDataType() == VTK_FLOAT)
    {
        for (int i = 0 ; i < nvals ; i++)
        {
            float val = data1->GetTuple1(i);
            if (val != 0.)
            {
                dv->SetTuple(i, data2->GetTuple(i));
            }
            else
            {
                dv->SetTuple(i, data3->GetTuple(i));
            }
        }
    }
    else if (data1->GetDataType() == VTK_UNSIGNED_CHAR)
    {
        vtkUnsignedCharArray *uca = (vtkUnsignedCharArray *) data1;
        for (int i = 0 ; i < nvals ; i++)
        {
            unsigned char val = uca->GetValue(i);
            if (val != '\0')
            {
                dv->SetTuple(i, data2->GetTuple(i));
            }
            else
            {
                dv->SetTuple(i, data3->GetTuple(i));
            }
        }
    }
      
    return dv;
}


