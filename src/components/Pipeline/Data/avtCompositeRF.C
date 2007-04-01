// ************************************************************************* //
//                               avtCompositeRF.C                            //
// ************************************************************************* //

#include <avtCompositeRF.h>

#include <float.h>

#include <avtGradients.h>
#include <avtLightingModel.h>
#include <avtOpacityMap.h>
#include <avtRay.h>


// ****************************************************************************
//  Method: avtCompositeRF constructor
//
//  Arguments:
//      l       The lighting model to use.
//      m       The opacity map.
//      sm      The opacity map for the secondary variable.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Dec  2 15:55:28 PST 2001
//    Add support for secondary variables.
//
// ****************************************************************************

avtCompositeRF::avtCompositeRF(avtLightingModel *l, avtOpacityMap *m,
                               avtOpacityMap *sm)
    : avtRayFunction(l)
{
    map = m;
    table = map->GetTable();
    secondaryMap = sm;
    secondaryTable = secondaryMap->GetTable();
    colorVariableIndex   = 0;
    opacityVariableIndex = 0;

    int entries = secondaryMap->GetNumberOfTableEntries();
    float *opacities = new float[entries];
    const RGBA *table = secondaryMap->GetTable();
    for (int i = 0 ; i < entries ; i++)
    {
        opacities[i] = table[i].A;
    }
    rangeMaxTable.SetTable(entries, opacities);
    // No need to free "opacities", since the rangeMaxTable now owns it.
}


// ****************************************************************************
//  Method: avtCompositeRF destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCompositeRF::~avtCompositeRF()
{
    ;
}


// ****************************************************************************
//  Method: avtCompositeRF::GetRayValue
//
//  Purpose:
//      Gets the value of the ray based on the sample points and the opacity
//      map.
//
//  Arguments:
//      ray         The ray to composite.
//      gradients   The gradients at each sample point.
//      rgb         The value of the pixel.
//      depth       The depth of rgb.
//
//  Programmer:     Hank Childs
//  Creation:       December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 21:03:40 PST 2001
//    Modified routine to return a color instead of a value.
//
//    Hank Childs, Mon Feb 12 17:24:19 PST 2001
//    Made shading happen as ray attenuation.
//
//    Hank Childs, Tue Feb 13 16:27:48 PST 2001
//    Made compositing work with opaque images.
//
//    Hank Chlids, Mon Feb 19 09:01:31 PST 2001
//    Opacity is a float instead of an unsigned char.
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Cast for compiler warning.
//
//    Hank Childs, Wed Nov 14 14:51:34 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Sun Dec  2 15:55:28 PST 2001
//    Full support for multiple variables.
//
// ****************************************************************************

void
avtCompositeRF::GetRayValue(const avtRay *ray, const avtGradients *gradients,
                            unsigned char rgb[3], float depth)
{
    //
    // Some compilers do very poor optimizations, so make sure that we don't
    // do an offset from the pointer at each step by creating automatic
    // variables and using those.
    //
    const int     numSamples  = ray->numSamples;
    const bool   *validSample = ray->validSample;

    // For right now, only work with one variable.
    const float  *sample      = ray->sample[colorVariableIndex];
    const float  *sample2     = ray->sample[opacityVariableIndex];

    int maxSample = IndexOfDepth(depth, numSamples);

    static double threshold = 254./255.;

    double opacity = 0.;
    double trgb[3];
    trgb[0] = 0.;
    trgb[1] = 0.;
    trgb[2] = 0.;
    int z;
    for (z = 0 ; z < maxSample ; z++)
    {
        if (validSample[z])
        {
            const RGBA &color = table[map->Quantize(sample[z])];
            const RGBA &opac  
                          = secondaryTable[secondaryMap->Quantize(sample2[z])];

            //
            // Only calculate further when we get non-zero opacity.
            //
            if (opac.A > 0)
            {
                double numberOfSamplesToReachFullOpacity = numSamples * 0.02;
                double samplesOpacity = opac.A / 
                                             numberOfSamplesToReachFullOpacity;

                //
                // There is a leap of faith here that the gradients were not
                // sent in (if the test is false) because the lighting and the
                // opacity map do not need them.
                //
                double grad[3] = { 0., 0., 0. };
                if (gradients)
                {
                    gradients->GetGradient(z, grad);
                }

                double distance = ((double) (z+1)) / ((double) numSamples);
                double shading = lighting->GetShading(distance, grad);

                // Only make the shading be the attenuation if it has opacity.
                if (opac.A < 0.2)
                {
                    shading = 1.;
                }

                double ff = (1-opacity)*samplesOpacity*shading;
                trgb[0] = trgb[0] + ff*color.R;
                trgb[1] = trgb[1] + ff*color.G;
                trgb[2] = trgb[2] + ff*color.B;

                opacity = opacity + (1-opacity)*samplesOpacity;
            }
            if (opacity > threshold)
            {
                break;
            }
        }
    }

    if (z >= maxSample)
    {
        //
        // The pixel is not completely opaque, so incorporate the background.
        //
        trgb[0] = trgb[0] + (unsigned char)((1-opacity)*rgb[0]);
        trgb[1] = trgb[1] + (unsigned char)((1-opacity)*rgb[1]);
        trgb[2] = trgb[2] + (unsigned char)((1-opacity)*rgb[2]);
    }

    //
    // Copy the temporary rgb into the output rgb.
    //
    rgb[0] = (unsigned char) trgb[0];
    rgb[1] = (unsigned char) trgb[1];
    rgb[2] = (unsigned char) trgb[2];
}


// ****************************************************************************
//  Method: avtCompositeRF::CanContributeToPicture
//
//  Purpose:
//      Determines if a cell can contribute to the final picture.  Cells with
//      zero opacity all the way through are the only cells that cannot
//      contribute to the final picture.
//
//  Arguments:
//      nVerts  The number of vertices.
//      vals    The variable values.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 2001
//
// ****************************************************************************

bool
avtCompositeRF::CanContributeToPicture(int nVerts,
                                       const float (*vals)[AVT_VARIABLE_LIMIT])
{
    float min = +FLT_MAX;
    float max = -FLT_MIN;
    for (int i = 0 ; i < nVerts ; i++)
    {
        if (vals[i][opacityVariableIndex] < min)
        {
            min = vals[i][opacityVariableIndex];
        }
        if (vals[i][opacityVariableIndex] > max)
        {
            max = vals[i][opacityVariableIndex];
        }
    }

    int  minIndex = secondaryMap->Quantize(min);
    int  maxIndex = secondaryMap->Quantize(max);
    float opacMax = rangeMaxTable.GetMaximumOverRange(minIndex, maxIndex);

    return (opacMax > 0. ? true : false);
}


// ****************************************************************************
//  Method: avtCompositeRF::NeedsGradientsForFunction
//
//  Purpose:
//      Determines whether gradients are needed for this function, typically
//      for lighting.
//
//  Returns:    true if we need gradients, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   January 3, 2002
//
// ****************************************************************************

bool
avtCompositeRF::NeedsGradientsForFunction(void)
{
    return lighting->NeedsGradients();
}


// ****************************************************************************
//  Method: avtCompositeRF::ClassifyForShading
//
//  Purpose:
//      Allows the ray function to do the appropriate classification for
//      shading.  This is because the underlying opacity value (its
//      classification) tells a more compelling story than its variable
//      value.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2002
//
// ****************************************************************************

float
avtCompositeRF::ClassifyForShading(float x)
{
    const RGBA &opac = secondaryTable[secondaryMap->Quantize(x)];
    return opac.A;
}


