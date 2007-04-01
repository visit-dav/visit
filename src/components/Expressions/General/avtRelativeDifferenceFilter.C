// ************************************************************************* //
//                          avtRelativeDifferenceFilter.C                    //
// ************************************************************************* //

#include <avtRelativeDifferenceFilter.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtRelativeDifferenceFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

avtRelativeDifferenceFilter::avtRelativeDifferenceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtRelativeDifferenceFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

avtRelativeDifferenceFilter::~avtRelativeDifferenceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtRelativeDifferenceFilter::DoOperation
//
//  Purpose:
//      Finds the relative difference.  Relative difference is defined as:
//      if (A == B && A == 0) then 0.
//      else (A-B) / (abs(A) + abs(B))
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
//  Creation:   December 28, 2004
//
// ****************************************************************************

void
avtRelativeDifferenceFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                vtkDataArray *out, int ncomponents,int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps != 1 || in2ncomps != 1)
    {
        EXCEPTION1(ExpressionException, "Can only take relative difference of "
                                        "scalars.");
    }

    int ntups = out->GetNumberOfTuples();
    for (int i = 0 ; i < ntups ; i++)
    {
        float val1 = in1->GetComponent(i, 0);
        float val2 = in2->GetComponent(i, 0);
        float outval = 0.;
        if (val1 != 0. || val2 != 0.)
            outval = (val1-val2) / (fabs(val1) + fabs(val2));
        out->SetComponent(i, 0, outval);
    }
}


