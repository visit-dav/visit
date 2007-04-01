// ************************************************************************* //
//                             avtNaturalLogFilter.C                         //
// ************************************************************************* //

#include <avtNaturalLogFilter.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtNaturalLogFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNaturalLogFilter::avtNaturalLogFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNaturalLogFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNaturalLogFilter::~avtNaturalLogFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNaturalLogFilter::DoOperation
//
//  Purpose:
//      Performs the natural logarithm on each component,tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 15, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:25:26 PST 2002
//    Added support for vectors and arbitrary data types.
//
// ****************************************************************************
 
void
avtNaturalLogFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                 int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val = in->GetComponent(i, j);
            if (val <= 0)
            {
                EXCEPTION1(ExpressionException,
                           "you cannot take the natural log of values <= 0");
            }
            out->SetComponent(i, j, log(val));
        }
    }
}


