// ************************************************************************* //
//                               avtTraceFilter.C                            //
// ************************************************************************* //

#include <avtTraceFilter.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTraceFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTraceFilter::avtTraceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTraceFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTraceFilter::~avtTraceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTraceFilter::DoOperation
//
//  Purpose:
//      Finds the trace (sum of diagonal components) of a tensor.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2003
//
// ****************************************************************************
 
void
avtTraceFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float val1 = in->GetComponent(i, 0);
            float val2 = in->GetComponent(i, 4);
            float val3 = in->GetComponent(i, 8);
            out->SetTuple1(i, val1+val2+val3);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot take trace of non-tensor.");
    }
}


