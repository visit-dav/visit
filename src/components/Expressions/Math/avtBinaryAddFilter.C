// ************************************************************************* //
//                              avtBinaryAddFilter.C                         //
// ************************************************************************* //

#include <avtBinaryAddFilter.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryAddFilter::DoOperation
//
//  Purpose:
//      Sums two arrays into a third array.
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
//    Hank Childs, Thu Aug 14 11:40:23 PDT 2003
//    Added support for mixing scalars and vectors.
//
// ****************************************************************************

void
avtBinaryAddFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                vtkDataArray *out, int ncomponents,int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps == in2ncomps)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                float val1 = in1->GetComponent(i, j);
                float val2 = in2->GetComponent(i, j);
                out->SetComponent(i, j, val1 + val2);
            }
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float val2 = in2->GetTuple1(i);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                float val1 = in1->GetComponent(i, j);
                out->SetComponent(i, j, val1 + val2);
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float val1 = in1->GetTuple1(i);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                float val2 = in2->GetComponent(i, j);
                out->SetComponent(i, j, val1 + val2);
            }
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Don't know how to add vectors of "
                                        "differing dimensions.");
    }
}


