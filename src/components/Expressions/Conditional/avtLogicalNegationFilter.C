// ************************************************************************* //
//                          avtLogicalNegationFilter.C                       //
// ************************************************************************* //

#include <avtLogicalNegationFilter.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtLogicalNegationFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtLogicalNegationFilter::avtLogicalNegationFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtLogicalNegationFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtLogicalNegationFilter::~avtLogicalNegationFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtLogicalNegationFilter::DoOperation
//
//  Purpose:
//      Takes the logical negation of an array.
//
//  Arguments:
//      in1           The input data array.
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
avtLogicalNegationFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                      int ncomponents, int ntuples)
{
    int inncomps = in->GetNumberOfComponents();
    if (inncomps != 1)
    {
        EXCEPTION1(ExpressionException, "Cannot logically negate a vector "
                                        "variable.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        bool val1;

        if (in->GetDataType() == VTK_UNSIGNED_CHAR)
        {
            val1 = (unsigned char) in->GetTuple1(i);
        }
        else
        {
            val1 = (in->GetTuple1(i) != 0. ? true : false);
        }

        unsigned char outval = !val1;
        out->SetTuple1(i, outval);
    }
}


