// ************************************************************************* //
//                           avtBinaryDivideFilter.C                         //
// ************************************************************************* //

#include <avtBinaryDivideFilter.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryDivideFilter::DoOperation
//
//  Purpose:
//      Divides the contents of the first array by the second array and puts
//      the output in a third array.
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
//    Hank Childs, Thu Aug 14 11:18:07 PDT 2003
//    Add support for vector and scalar types mixing.
//
// ****************************************************************************

void
avtBinaryDivideFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                   vtkDataArray *out, int ncomponents,
                                   int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if ((in1ncomps == 1) && (in2ncomps == 1))
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float val1 = in1->GetTuple1(i);
            float val2 = in2->GetTuple1(i);
            if (val2 == 0.)
            {
                EXCEPTION1(ExpressionException, "You can't divide by zero");
            }
            out->SetTuple1(i, val1 / val2);
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float val2 = in2->GetTuple1(i);
            if (val2 == 0)
            {
                EXCEPTION1(ExpressionException, "You can't divide by zero");
            }
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                float val1 = in1->GetComponent(i, j);
                out->SetComponent(i, j, val1/val2);
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
                if (val2 == 0)
                {
                    EXCEPTION1(ExpressionException,"You can't divide by zero");
                }
                out->SetComponent(i, j, val1/val2);
            }
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Division of vectors in undefined.");
    }
}


