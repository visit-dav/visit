// ************************************************************************* //
//                    avtSamplePointsToSamplePointsFilter.C                  //
// ************************************************************************* //

#include <avtSamplePointsToSamplePointsFilter.h>


// ****************************************************************************
//  Method: avtSamplePointsToSamplePointsFilter::PreExecute
//
//  Purpose:
//      Executes before the avtFilter calls Execute.  The sample points need
//      to know how many variables they will be sampling over before they get
//      going.  This is a chance to set that up.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
// ****************************************************************************

void
avtSamplePointsToSamplePointsFilter::PreExecute(void)
{
    avtSamplePoints_p in  = GetTypedInput();
    avtSamplePoints_p out = GetTypedOutput();
    out->SetNumberOfVariables(in->GetNumberOfVariables());
}


