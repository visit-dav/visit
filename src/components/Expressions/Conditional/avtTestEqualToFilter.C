// ************************************************************************* //
//                             avtTestEqualToFilter.C                        //
// ************************************************************************* //

#include <avtTestEqualToFilter.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTestEqualToFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTestEqualToFilter::avtTestEqualToFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTestEqualToFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTestEqualToFilter::~avtTestEqualToFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTestEqualToFilter::DoOperation
//
//  Purpose:
//      Test the equality of two arrays.
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
avtTestEqualToFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
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
        unsigned char outval = (in1->GetTuple1(i) == in2->GetTuple1(i)
                                ? '\1' : '\0');
        out->SetTuple1(i, outval);
    }
}


