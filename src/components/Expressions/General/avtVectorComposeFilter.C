// ************************************************************************* //
//                         avtVectorComposeFilter.C                          //
// ************************************************************************* //

#include <avtVectorComposeFilter.h>

#include <math.h>

#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorComposeFilter::GetVariableDimension
//
//  Purpose:
//      Does its best to guess at the variable dimension will be after
//      decomposing a "vector".
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

int
avtVectorComposeFilter::GetVariableDimension(void)
{
    int inDim = GetInput()->GetInfo().GetAttributes().GetVariableDimension();
    if (inDim == 1)
        return 3;
    else if (inDim == 3)
        return 9;

    // Who knows?!?
    return 3;
}


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
//    Hank Childs, Fri Sep 19 13:46:13 PDT 2003
//    Added support for tensors, symmetric tensors.
//
// ****************************************************************************
vtkDataArray *
avtVectorComposeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int numinputs = varnames.size();

    bool twoDVector = 
            (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2);

    //
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    //
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

    if (numinputs == 3)
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

    int nvals  = data1->GetNumberOfTuples();
    vtkDataArray *dv = data1->NewInstance();
    if (twoDVector)
    {
        if (numinputs == 2)
        {
            if (data1->GetNumberOfComponents() == 1 &&
                data2->GetNumberOfComponents() == 1)
            {
                //
                // Standard vector case.
                //
                dv->SetNumberOfComponents(3);  // VTK doesn't like 2.
                dv->SetNumberOfTuples(nvals);

                for (int i = 0 ; i < nvals ; i++)
                {
                    float val1 = data1->GetTuple1(i);
                    float val2 = data2->GetTuple1(i);
                    dv->SetTuple3(i, val1, val2, 0.);
                }
            }
            else if ((data1->GetNumberOfComponents() == 3) &&
                     (data2->GetNumberOfComponents() == 3))
            {
                //
                // 2D tensor.
                //
                dv->SetNumberOfComponents(9); 
                dv->SetNumberOfTuples(nvals);
                
                for (int i = 0 ; i < nvals ; i++)
                {
                    float vals[9];
                    vals[0] = data1->GetComponent(i, 0);
                    vals[1] = data1->GetComponent(i, 1);
                    vals[2] = 0.;
                    vals[3] = data2->GetComponent(i, 0);
                    vals[4] = data2->GetComponent(i, 1);
                    vals[5] = 0.;
                    vals[6] = 0.;
                    vals[7] = 0.;
                    vals[8] = 0.;
                    dv->SetTuple(i, vals);
                }
            }
            else
            {
                char str[1024];
                sprintf(str, "Do not know how to assemble arrays of %d and "
                              "%d into a vector or tensor.", 
                              data1->GetNumberOfComponents(),
                              data2->GetNumberOfComponents());
                EXCEPTION1(ExpressionException, str);
            }
        }
        else if (numinputs == 3)
        {
            EXCEPTION1(ExpressionException, "I don't know how to compose "
                           "3 variables to make a field for a 2D dataset.");
        }
    }
    else
    {
        if (numinputs == 3)
        {
            if (data1->GetNumberOfComponents() == 1 && 
                data2->GetNumberOfComponents() == 1 && 
                data3->GetNumberOfComponents() == 1)
            {
                //
                // This is your everyday 3D vector combination.
                //
                dv->SetNumberOfComponents(3); 
                dv->SetNumberOfTuples(nvals);
                
                for (int i = 0 ; i < nvals ; i++)
                {
                    float val1 = data1->GetTuple1(i);
                    float val2 = data2->GetTuple1(i);
                    float val3 = data3->GetTuple1(i);
                    dv->SetTuple3(i, val1, val2, val3);
                }
            }
            else if (data1->GetNumberOfComponents() == 3 &&
                     data2->GetNumberOfComponents() == 3 &&
                     data3->GetNumberOfComponents() == 3)
            {
                //
                // This is a 3D tensor.  Interpret it as:
                // Data1 = XX, XY, XZ
                // Data2 = YX, YY, YZ
                // Data3 = ZX, ZY, ZZ
                //
                dv->SetNumberOfComponents(9); 
                dv->SetNumberOfTuples(nvals);
                
                for (int i = 0 ; i < nvals ; i++)
                {
                    float entry[9];
                    data1->GetTuple(i, entry);
                    data2->GetTuple(i, entry+3);
                    data3->GetTuple(i, entry+6);
                    dv->SetTuple(i, entry);
                }
            }
            else
            {
                EXCEPTION1(ExpressionException, "The only interpretation "
                           "VisIt can make of 3 variables for a 3D dataset is "
                           "a vector or a tensor.  But these inputs don't have"
                           " the right number of components to make either.");
            }
        }
        else 
        {
            EXCEPTION1(ExpressionException, "You must specify three vectors "
                        "to compose a field for a 3D dataset.");
        }
    }

    return dv;
}


