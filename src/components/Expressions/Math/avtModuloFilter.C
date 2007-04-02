// ************************************************************************* //
//                               avtModuloFilter.C                           //
// ************************************************************************* //

#include <avtModuloFilter.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtModuloFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtModuloFilter::avtModuloFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtModuloFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtModuloFilter::~avtModuloFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtModuloFilter::DoOperation
//
//  Purpose:
//      Performs a modulo operation of the first by the second array and 
//      puts the result into a third array.
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
//  Creation:   June 30, 2005
//
// ****************************************************************************
 
void
avtModuloFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                  vtkDataArray *out, int ncomponents,
                                  int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                float val1 = in1->GetComponent(i, j);
                int v1 = (int) val1;
                float val2 = in2->GetComponent(i, j);
                int v2 = (int) val2;
                int output = (v2 <= 0 ? 1 : v1 % v2);
                out->SetComponent(i, j, output);
            }
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Don't know how to perform modulo "
                                        "operation with a vector variable.");
    }
}


