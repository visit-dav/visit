// ************************************************************************* //
//                          avtVectorCrossProductFilter.C                    //
// ************************************************************************* //

#include <avtVectorCrossProductFilter.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorCrossProductFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorCrossProductFilter::avtVectorCrossProductFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorCrossProductFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorCrossProductFilter::~avtVectorCrossProductFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorCrossProductFilter::DoOperation
//
//  Purpose:
//      Takes the cross product of the two inputs
//
//  Arguments:
//      in1           The first input vector
//      in2           The second input vector
//      out           The output vector.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)  Must be 3 for this operation.
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Notes: a x b = (a2 b3 - a3 b2)i + (a3 b1 - a1 b3)j + (a1 b2 - a2 b1)k
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 11 13:32:55 PDT 2003
//
//  Modifications:
//
// ****************************************************************************
void
avtVectorCrossProductFilter::DoOperation(vtkDataArray *in1,
                                vtkDataArray *in2, vtkDataArray *out,
                                int ncomponents, int ntuples)
{
    if (ncomponents != 3)
    {
        EXCEPTION1(ExpressionException,
                   "you cannot take the cross product of data which are not 3-component vectors.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        float a1 = in1->GetComponent(i, 0);
        float a2 = in1->GetComponent(i, 1);
        float a3 = in1->GetComponent(i, 2);
        float b1 = in2->GetComponent(i, 0);
        float b2 = in2->GetComponent(i, 1);
        float b3 = in2->GetComponent(i, 2);

        out->SetComponent(i, 0, a2*b3 - a3*b2);
        out->SetComponent(i, 1, a3*b1 - a1*b3);
        out->SetComponent(i, 2, a1*b2 - a2*b1);
    }
}
