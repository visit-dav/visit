// ************************************************************************* //
//                           avtProcessorIdFilter.C                          //
// ************************************************************************* //

#include <avtProcessorIdFilter.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtProcessorIdFilter::DeriveVariable
//
//  Purpose:
//      Assigns the processor Id to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 19, 2002
//
// ****************************************************************************

vtkDataArray *
avtProcessorIdFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int procId = PAR_Rank();
    int npts   = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
        rv->SetTuple1(i, procId);
    }

    return rv;
}


