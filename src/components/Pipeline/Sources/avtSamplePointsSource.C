// ************************************************************************* //
//                          avtSamplePointsSource.C                          //
// ************************************************************************* //

#include <avtSamplePointsSource.h>

#include <avtSamplePoints.h>
#include <avtVolume.h>


// ****************************************************************************
//  Method: avtSamplePointsSource constructor
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
// ****************************************************************************

avtSamplePointsSource::avtSamplePointsSource()
{
    samples = new avtSamplePoints(this);
}


// ****************************************************************************
//  Method: avtSamplePointsSource::SetTypedOutput
//
//  Purpose:
//      Sets the output.
//
//  Arguments:
//      out     The new output.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
// ****************************************************************************

void
avtSamplePointsSource::SetTypedOutput(avtSamplePoints_p out)
{
    samples = out;
}


// ****************************************************************************
//  Method: avtSamplePointsSource::GetOutput
//
//  Purpose:
//      Gets the output avtSamplePoints.
//
//  Returns:     The output of the source as avtSamplePoints.
//
//  Programmer:  Hank Childs
//  Creation:    December 4, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 15:28:55 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtSamplePointsSource::GetOutput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, samples);

    return rv;
}


