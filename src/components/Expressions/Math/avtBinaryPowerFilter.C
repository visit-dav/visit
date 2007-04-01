// ************************************************************************* //
//                            avtBinaryPowerFilter.C                         //
// ************************************************************************* //

#include <avtBinaryPowerFilter.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryPowerFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryPowerFilter::avtBinaryPowerFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryPowerFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryPowerFilter::~avtBinaryPowerFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryPowerFilter::DoOperation
//
//  Purpose:
//      Raises the first array to the power in the second array and puts the
//      result into a third array.
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
//    Hank Childs, Thu Aug 14 13:40:20 PDT 2003
//    Added support for mixing vectors and scalars.
//
// ****************************************************************************
 
void
avtBinaryPowerFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
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
                float val2 = in2->GetComponent(i, j);
                out->SetComponent(i, j, pow(val1, val2));
            }
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Don't know how to raise a variable "
                                        "by a vector variable.");
    }
}


