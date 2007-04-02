// ************************************************************************* //
//                             avtMinMaxExpression.C                         //
// ************************************************************************* //

#include <avtMinMaxExpression.h>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtMinMaxExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
// ****************************************************************************

avtMinMaxExpression::avtMinMaxExpression()
{
    doMin = false;
}


// ****************************************************************************
//  Method: avtMinMaxExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
// ****************************************************************************

avtMinMaxExpression::~avtMinMaxExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMinMaxExpression::DoOperation
//
//  Purpose:
//      Finds the minimum or maximum value.
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
//  Creation:   March 13, 2006
//
// ****************************************************************************

void
avtMinMaxExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
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
                bool val1Bigger = (val1 > val2);
                // Circumflex (^) is the exclusive or.
                // doMin == true  && val1Bigger == true  --> val2
                // doMin == false && val1Bigger == true  --> val1
                // doMin == true  && val1Bigger == false --> val1
                // doMin == false && val1Bigger == false --> val2
                //  --> values same, then val2, values different, then val1
                float outval = (doMin ^ val1Bigger ? val1 : val2);
                out->SetComponent(i, j, outval);
            }
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Don't know how to take minimums or "
                         "maximums with data of differing dimensions.");
    }
}


