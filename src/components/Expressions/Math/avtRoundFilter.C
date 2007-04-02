// ************************************************************************* //
//                               avtRoundFilter.C                            //
// ************************************************************************* //

#include <avtRoundFilter.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtRoundFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtRoundFilter::avtRoundFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtRoundFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtRoundFilter::~avtRoundFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtRoundFilter::DoOperation
//
//  Purpose:
//      Rounds each component up or down.
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
avtRoundFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                 int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val = in->GetComponent(i, j);
            out->SetComponent(i, j, floor(val+0.5));
        }
    }
}


