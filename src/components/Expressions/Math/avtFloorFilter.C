// ************************************************************************* //
//                               avtFloorFilter.C                            //
// ************************************************************************* //

#include <avtFloorFilter.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtFloorFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtFloorFilter::avtFloorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFloorFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtFloorFilter::~avtFloorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFloorFilter::DoOperation
//
//  Purpose:
//      Takes the floor of each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
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
avtFloorFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                 int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val = in->GetComponent(i, j);
            out->SetComponent(i, j, floor(val));
        }
    }
}


