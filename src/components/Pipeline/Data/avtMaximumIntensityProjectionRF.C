// ************************************************************************* //
//                       avtMaximumIntensityProjectionRF.C                   //
// ************************************************************************* //

#include <avtMaximumIntensityProjectionRF.h>

#include <float.h>

#include <avtGradients.h>
#include <avtLightingModel.h>
#include <avtRay.h>
#include <avtVariablePixelizer.h>


// ****************************************************************************
//  Method: avtMaximumIntensityProjectionRF constructor
//
//  Arguments:
//      l      The lighting model.
//      p      The pixelizer with this ray function.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

avtMaximumIntensityProjectionRF::avtMaximumIntensityProjectionRF
    (avtLightingModel *l, avtVariablePixelizer *p)  : avtRayFunction(l)
{
    pix = p;
}


// ****************************************************************************
//  Method: avtMaximumIntensityProjectionRF::GetRayValue
//
//  Purpose:
//      Get the pixel value of the ray using a maximum intensity projection.
//      The pixel value is the maximum value along the ray.
//
//  Arguments:
//      ray         The ray to use.
//      gradients   The gradients along the ray.
//      rgb         Where the pixel is stored.
//      depth       The depth that the ray should be shot to.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:49:43 PST 2001
//    Re-wrote to return a color instead of a value.
//
//    Hank Childs, Tue Feb 13 18:30:07 PST 2001
//    Added support for having opaque images in the rendering.
//
//    Hank Childs, Wed Nov 14 14:51:34 PST 2001
//    Added support for multiple variables.
//
// ****************************************************************************

void
avtMaximumIntensityProjectionRF::GetRayValue(const avtRay *ray, 
                                             const avtGradients *gradients,
                                             unsigned char rgb[3], float depth)
{
    double curMax = -1. * DBL_MAX;
    int    maxInd = -1;

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

    for (int i = 0 ; i < depthIndex ; i++)
    {
        if (validSample[i])
        {
            if (curMax < sample[i])
            {
                curMax = sample[i];
                maxInd = i;
            }
        }
    }

    if (maxInd != -1)
    {
        double value = curMax;

        //
        // There is a leap of faith here that the gradients were not sent
        // in (if the test is false) because the lighting does not need them.
        //
        double grad[3] = { 0., 0., 0. };
        if (gradients)
        {
            gradients->GetGradient(maxInd, grad);
        }
        double distance = ((double) (maxInd+1)) / ((double) numSamples);
        double intensity = lighting->GetShading(distance, grad);
        pix->GetColor(value, intensity, rgb);
    }
}


