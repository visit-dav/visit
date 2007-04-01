// ************************************************************************* //
//                            avtAverageValueRF.C                            //
// ************************************************************************* //

#include <avtAverageValueRF.h>

#include <avtGradients.h>
#include <avtLightingModel.h>
#include <avtRay.h>
#include <avtVariablePixelizer.h>


// ****************************************************************************
//  Method: avtAverageValueRF constructor
//
//  Arguments:
//      l        The lighting model for the ray.
//      p        The pixelizer.
//      unsv     A boolean indicating if a dummy value should be inserted when
//               there is no sample point.
//      nsv      The value to use if there is no sample.
//
//  Programmer:  Hank Childs
//  Creation:    December 1, 2000
//
// ****************************************************************************

avtAverageValueRF::avtAverageValueRF(avtLightingModel *l,
                                     avtVariablePixelizer *p, bool unsv,
                                     double nsv)
    : avtRayFunction(l)
{
    useNoSampleValue = unsv;
    noSampleValue    = nsv;
    pix              = p;
}


// ****************************************************************************
//  Method: avtAverageValueRF destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtAverageValueRF::~avtAverageValueRF()
{
    ;
}


// ****************************************************************************
//  Method: avtAverageValueRF::GetRayValue
//
//  Purpose:
//      Gets the value for the pixel that the ray coincides with.  Does this
//      by examining the ray profile and determining what the average value
//      is along the ray.  Then maps that value to a color and returns the
//      color.
//
//  Arguments:
//      ray         The ray to use.
//      <unnamed>   The gradients along the ray.
//      rgb         A place to store the color.
//      depth       The depth of the zbuffer for this pixel.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:49:43 PST 2001
//    Modified function to return a color instead of a value.
//
//    Hank Childs, Sat Feb 17 11:52:34 PST 2001
//    Account for geometry in the window as well.
//
//    Hank Childs, Wed Nov 14 14:51:34 PST 2001
//    Added support for multiple variables.
//
// ****************************************************************************

void
avtAverageValueRF::GetRayValue(const avtRay *ray, const avtGradients *,
                               unsigned char rgb[3], float depth)
{
    //
    // Some compilers do very poor optimizations, so make sure that we don't
    // do an offset from the pointer at each step by creating automatic
    // variables and using those.
    //
    const int     numSamples  = ray->numSamples;
    const bool   *validSample = ray->validSample;

    // Only handle one variable, even if there is more.
    const float  *sample      = ray->sample[0];

    int depthIndex = IndexOfDepth(depth, numSamples);

    double average    = 0.;
    int    numValidSamples = 0;
    for (int i = 0 ; i < depthIndex ; i++)
    {
        if (validSample[i])
        {
            average += sample[i];
            numValidSamples++;
        }
        else
        {
            if (useNoSampleValue)
            {
                average += noSampleValue;
                numValidSamples++;
            }
        }
    }

    if (numValidSamples != 0)
    {
        double value = average / numValidSamples;

        //
        // This is a case where shading does not make a lot of sense.
        //
        double intensity = 1.;

        pix->GetColor(value, intensity, rgb);
    }
}


