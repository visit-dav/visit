// ************************************************************************* //
//                             avtConstantCreatorFilter.C                    //
// ************************************************************************* //

#include <avtConstantCreatorFilter.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

// ****************************************************************************
//  Method: avtConstantCreatorFilter::DoOperation
//
//  Purpose:
//      Generates new constants.
//
//  Arguments:
//      in            The input data array (ignored).
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern
//  Creation:   Sat Feb 22 00:42:35 America/Los_Angeles 2003
//
//  Modifications:
//
// ****************************************************************************
 
void
avtConstantCreatorFilter::DoOperation(vtkDataArray *, vtkDataArray *out,
                                      int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
        out->SetTuple1(i, value);
}
