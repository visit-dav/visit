// ************************************************************************* //
//                          avtTestGreaterThanFilter.C                       //
// ************************************************************************* //

#include <avtTestGreaterThanFilter.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTestGreaterThanFilter::DoOperation
//
//  Purpose:
//      Tests whether one array is greater than another.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************
 
void
avtTestGreaterThanFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                  vtkDataArray *out, int ncomponents,
                                  int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps != 1 || in2ncomps != 1)
    {
        EXCEPTION1(ExpressionException, "Cannot compare vector "
                                        "variables.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        unsigned char outval = (in1->GetTuple1(i) > in2->GetTuple1(i)
                                ? '\1' : '\0');
        out->SetTuple1(i, outval);
    }
}


