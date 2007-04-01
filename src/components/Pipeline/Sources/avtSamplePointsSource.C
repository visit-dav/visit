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
//  Method: avtSamplePointsSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSamplePointsSource::~avtSamplePointsSource()
{
    ;
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


