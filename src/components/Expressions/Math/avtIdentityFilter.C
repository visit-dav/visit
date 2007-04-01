// ************************************************************************* //
//                               avtIdentityFilter.C                         //
// ************************************************************************* //

#include <avtIdentityFilter.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtIdentityFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtIdentityFilter::avtIdentityFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtIdentityFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtIdentityFilter::~avtIdentityFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtIdentityFilter::DoOperation
//
//  Purpose:
//      Copies each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   December 9, 2003
//
// ****************************************************************************
 
void
avtIdentityFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                 int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val = in->GetComponent(i, j);
            out->SetComponent(i, j, val);
        }
    }
}


