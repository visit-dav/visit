// ************************************************************************* //
//                          avtDistanceToValueRF.C                           //
// ************************************************************************* //

#include <avtDistanceToValueRF.h>

#include <avtDistancePixelizer.h>
#include <avtGradients.h>
#include <avtLightingModel.h>
#include <avtRay.h>


// ****************************************************************************
//  Method: avtDistanceToValueRF constructor
//
//  Arguments:
//      l         The lighting model to use.
//      cv        The critical value to find the distance to.
//      p         The pixelizer for this ray function.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

avtDistanceToValueRF::avtDistanceToValueRF(avtLightingModel *l, double cv,
                                           avtDistancePixelizer *p)
    : avtRayFunction(l)
{
    criticalValue = cv;
    pix = p;
}


// ****************************************************************************
//  Method: avtDistanceToValueRF destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDistanceToValueRF::~avtDistanceToValueRF()
{
    ;
}


// ****************************************************************************
//  Method: avtDistanceToValueRF::GetRayValue
//
//  Purpose:
//      Gets the distance to the critical value along the ray.
//
//  Arguments:
//      ray         The ray to use.
//      gradients   The gradients along the ray.
//      rgb         A place to put the color.
//      depth       The z-buffer value to restrict to.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
// 
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:49:43 PST 2001
//    Re-wrote to incorporate pixelizers.
//
//    Hank Childs, Tue Feb 13 16:36:55 PST 2001
//    Added depth for opaque images.
//
//    Hank Childs, Wed Nov 14 14:51:34 PST 2001
//    Added support for multiple variables.
//
// ****************************************************************************

void
avtDistanceToValueRF::GetRayValue(const avtRay *ray,
                                  const avtGradients *gradients,
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

    double distance;
    double intensity;

    int     location = -1;
    for (int i = 0 ; i < depthIndex ; i++)
    {
        if (validSample[i])
        {
            if (sample[i] == criticalValue)
            {
                location = i;
                distance = (double) i;
                break;
            }
            if (validSample[i+1])
            {
                if (sample[i] < criticalValue && criticalValue < sample[i+1])
                {
                    double in_between = (criticalValue - sample[i]) /
                                        (sample[i+1] - sample[i]);
                    distance = ((double) i) + in_between;
                    location = (in_between > 0.5 ? i+1 : i);
                    break;
                }
                if (sample[i] > criticalValue && criticalValue > sample[i+1])
                {
                    double in_between = (sample[i] - criticalValue) /
                                        (sample[i] - sample[i+1]);
                    distance = ((double) i) + in_between;
                    location = (in_between > 0.5 ? i+1 : i);
                    break;
                }
            }
        }
    }
            
    if (location != -1)
    {
        //
        // We have already found the distance, so let's shade it and find an
        // intensity.
        //
        // There is a leap of faith here that the gradients were not sent
        // in (if the test is false) because the lighting does not need them.
        //
        double grad[3] = { 0., 0., 0. };
        if (gradients)
        {
            gradients->GetGradient(location, grad);
        }
        double dist = ((double) (location+1)) / ((double) numSamples);
        intensity = lighting->GetShading(dist, grad);
        pix->GetColor(distance, intensity, rgb);
    }
}


