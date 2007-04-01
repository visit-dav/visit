// ************************************************************************* //
//                           avtProcessorIdFilter.C                          //
// ************************************************************************* //

#include <avtProcessorIdFilter.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtProcessorIdFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtProcessorIdFilter::avtProcessorIdFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtProcessorIdFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtProcessorIdFilter::~avtProcessorIdFilter()
{
    ;
}


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


