// ************************************************************************* //
//                          avtBinarySubtractFilter.C                        //
// ************************************************************************* //

#include <avtBinarySubtractFilter.h>

#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtBinarySubtractFilter::DoOperation
//
//  Purpose:
//      Differences two arrays and puts the results into a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 18, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 07:35:07 PST 2002
//    Added support for vectors and arbitrary data types.
//
// ****************************************************************************
 
void
avtBinarySubtractFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                     vtkDataArray *out, int ncomponents,
                                     int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val1 = in1->GetComponent(i, j);
            float val2 = in2->GetComponent(i, j);
            out->SetComponent(i, j, val1-val2);
        }
    }
}


