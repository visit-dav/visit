// ************************************************************************* //
//                             avtConstantCreatorFilter.C                    //
// ************************************************************************* //

#include <avtConstantCreatorFilter.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Method: avtConstantCreatorFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConstantCreatorFilter::avtConstantCreatorFilter()
{
    value = 0;
}


// ****************************************************************************
//  Method: avtConstantCreatorFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConstantCreatorFilter::~avtConstantCreatorFilter()
{
    ;
}


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


// ****************************************************************************
//  Method: avtConstantCreatorFilter::CreateArray
//
//  Purpose:
//      Creates an array of constant value.  Note that this routine will not
//      return uchar arrays.  The motivation for this is that most uchar arrays
//      come from the conditional expression (if), which uses a uchar as the
//      conditional, but really wants floats as the then- and else- variables.
//      (ie if(gt(X, 2.), 1., 0.) -- the constant 1. should be a float
//      (or whatever X is, not a uchar).
//
//  Programmer: Hank Childs
//  Creation:   November 3, 2003
//
// ****************************************************************************

vtkDataArray *
avtConstantCreatorFilter::CreateArray(vtkDataArray *in)
{
    if (in->GetDataType() == VTK_UNSIGNED_CHAR)
    {
        return vtkFloatArray::New();
    }

    return in->NewInstance();
}


