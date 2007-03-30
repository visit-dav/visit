// ************************************************************************* //
//                              avtRandomFilter.C                            //
// ************************************************************************* //

#include <avtRandomFilter.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtRandomFilter::DeriveVariable
//
//  Purpose:
//      Assigns a random number to each point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     March 7, 2003
//
// ****************************************************************************

vtkDataArray *
avtRandomFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts   = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
        rv->SetTuple1(i, (rand() % 1024) / 1024.);
    }

    return rv;
}


