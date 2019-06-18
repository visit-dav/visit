/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtCompositeRF.C                            //
// ************************************************************************* //

#include <avtCompositeRF.h>

#include <float.h>

#include <avtLightingModel.h>
#include <avtOpacityMap.h>
#include <avtPointExtractor.h>
#include <avtRay.h>

#include <cmath>

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
//    Hank Childs, Sat Jan  7 17:50:22 PST 2006
//    Add support for kernel based sampling.
//
//    Alister Maguire, Tue Jun 11 11:08:52 PDT 2019
//    Added initialization of viewDistance. 
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
    weightVariableIndex  = -1;
    trilinearSampling = false;

    int entries = secondaryMap->GetNumberOfTableEntries();
    double *opacities = new double[entries];
    const RGBA *table = secondaryMap->GetTable();
    for (int i = 0 ; i < entries ; i++)
    {
        opacities[i] = table[i].A;
    }
    rangeMaxTable.SetTable(entries, opacities);
    // No need to free "opacities", since the rangeMaxTable now owns it.

    // set some reasonable defaults for the material parameters
    matProperties[0] = 0.4;     // ambient
    matProperties[1] = 0.75;    // diffuse
    matProperties[2] = 0.0;     // specular
    matProperties[3] = 15;      // shininess

    viewDistance = 0.0;
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
//    Hank Childs, Fri Dec  3 14:34:59 PST 2004
//    Allow a fully opaque sample to terminate a ray.  Also make a correction
//    so low opacity samples don't have a huge impact. ['1735]
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Add support for shading that actually works.
//
//    Hank Childs, Mon Dec 29 09:22:47 PST 2008
//    Fix composite to be a true integration.
//
//    Alister Maguire, Mon Jun  3 15:40:31 PDT 2019
//    Replaced oneSamplesContribution with standard opacity correction
//    method. This was to resolve bug #3082. Also renamed local rgb 
//    variables to sampleRGB to avoid confusion with the input rgb variable. 
//
// ****************************************************************************

void
avtCompositeRF::GetRayValue(const avtRay *ray, 
                            unsigned char rgb[3], double depth)
{
    //
    // Some compilers do very poor optimizations, so make sure that we don't
    // do an offset from the pointer at each step by creating automatic
    // variables and using those.
    //
    const int     numSamples  = ray->numSamples;
    const bool   *validSample = ray->validSample;

    // For right now, only work with one variable.
    const double  *sample      = ray->sample[colorVariableIndex];
    const double  *sample2     = ray->sample[opacityVariableIndex];
    const double  *weight      = NULL;
    double        min_weight  = 0.;
    double        min_weight_denom = 0.;
    if (weightVariableIndex >= 0)
    {
        weight = ray->sample[weightVariableIndex];
        min_weight = avtPointExtractor::GetMinimumWeightCutoff();
        if (min_weight > 0.)
            min_weight_denom = 1./min_weight;
    }

    int maxSample = IndexOfDepth(depth, numSamples);

    static float threshold = 254.f/255.f;

    float opacity = 0.;
    float trgb[3] = {0.f, 0.f, 0.f};
    int z = 0;
    double sampleDist = viewDistance/double(ray->numSamples);

    if(trilinearSampling)
    {
        for (z = 0 ; z < maxSample ; z++)
        {
            if (validSample[z])
            {
                float value = map->QuantizeValF(sample[z]);
                float diffRGB = value - ((int)value);
                RGBA colorLow = table[(int)value];
                RGBA colorHigh = table[(int)value+1];

                value = secondaryMap->QuantizeValF(sample2[z]);
                float diffAlpha = value - ((int)value);
                RGBA opacLow = secondaryTable[(int)value];
                RGBA opacHigh = secondaryTable[(int)value+1];

                const RGBA &color = table[map->Quantize(sample[z])];
                const RGBA &opac = secondaryTable[secondaryMap->Quantize(sample2[z])];

                float opacityValue = (1.0-diffAlpha)*opacLow.A + diffAlpha*opacHigh.A;

                //
                // Only calculate further when we get non-zero opacity.
                //
                if (opacityValue > 0)
                {
                    double tableOpac = opac.A;
                    if (weight != NULL)
                    {
                        if (weight[z] < min_weight)
                            tableOpac *= weight[z]*min_weight_denom;
                    }
                    float samplesOpacity = static_cast<float>(tableOpac);
                    unsigned char sampleRGB[3] = { color.R, color.G, color.B };
                    unsigned char rgbLow[3] = { colorLow.R, colorLow.G, colorLow.B };
                    unsigned char rgbHigh[3] = { colorHigh.R, colorHigh.G, colorHigh.B };
                    sampleRGB[0] = (1.f-diffRGB)*rgbLow[0] + diffRGB*rgbHigh[0];
                    sampleRGB[1] = (1.f-diffRGB)*rgbLow[1] + diffRGB*rgbHigh[1];
                    sampleRGB[2] = (1.f-diffRGB)*rgbLow[2] + diffRGB*rgbHigh[2];
                    lighting->AddLightingHeadlight(z, ray, sampleRGB, 1.0, 
                        matProperties);

                    float ff = (1.f-opacity)*samplesOpacity;
                    trgb[0] = trgb[0] + ff*sampleRGB[0];
                    trgb[1] = trgb[1] + ff*sampleRGB[1];
                    trgb[2] = trgb[2] + ff*sampleRGB[2];

                    opacity = opacity + ff;
                }
                if (opacity > threshold)
                {
                    break;
                }
            }
        }
    }
    else
    {
        if(weight != NULL)
        {
            // Compute samples, weighting the opacity by a variable.
            for (z = 0 ; z < maxSample ; z++)
            {
                if (validSample[z])
                {
                    const RGBA &color = table[map->Quantize(sample[z])];
                    float opacityValue = secondaryMap->QueryAlpha(sample2[z]);

                    //
                    // Only calculate further when we get non-zero opacity.
                    //
                    if (opacityValue > 0)
                    {
                        double tableOpac = static_cast<double>(opacityValue);
                        if (weight[z] < min_weight)
                            tableOpac *= weight[z]*min_weight_denom;
                        float samplesOpacity = static_cast<float>(tableOpac);

                        if (samplesOpacity < 1.f)
                        {
                            samplesOpacity = (1.f - std::pow(
                                (1.f - samplesOpacity), sampleDist));
                        }

                        unsigned char sampleRGB[3] = { color.R, color.G, color.B };
                        lighting->AddLighting(z, ray, sampleRGB);

                        float ff = (1.f-opacity)*samplesOpacity;
                        trgb[0] = trgb[0] + ff*sampleRGB[0];
                        trgb[1] = trgb[1] + ff*sampleRGB[1];
                        trgb[2] = trgb[2] + ff*sampleRGB[2];

                        opacity = opacity + ff;

                        if (opacity > threshold)
                        {
                            break; // early terminate
                        }
                    }
                }
            }
        }
        else
        {
            // No opacity weighting.
            for (z = 0 ; z < maxSample ; z++)
            {
                if (validSample[z])
                {
                    float opacityValue = secondaryMap->QueryAlpha(sample2[z]);

                    //
                    // Only calculate further when we get non-zero opacity.
                    //
                    if (opacityValue > 0)
                    {
                        const RGBA &color = table[map->Quantize(sample[z])];
                        unsigned char sampleRGB[3] = { color.R, color.G, color.B };
                        lighting->AddLighting(z, ray, sampleRGB);
                      
                        float samplesOpacity = 1.f;
                        if (opacityValue < 1.f)
                        {
                            samplesOpacity = (1.f - std::pow((1.f - opacityValue), 
                                sampleDist));
                        }

                        float ff = (1.f-opacity)*samplesOpacity;
                        trgb[0] = trgb[0] + ff*sampleRGB[0];
                        trgb[1] = trgb[1] + ff*sampleRGB[1];
                        trgb[2] = trgb[2] + ff*sampleRGB[2];
                        opacity = opacity + ff;

                        if (opacity > threshold)
                        {
                            break; // early terminate
                        }
                    }
                }
            }
        }
    }

    if (z >= maxSample)
    {
        //
        // The pixel is not completely opaque, so incorporate the background.
        //
        trgb[0] = trgb[0] + (unsigned char)((1.f-opacity)*rgb[0]);
        trgb[1] = trgb[1] + (unsigned char)((1.f-opacity)*rgb[1]);
        trgb[2] = trgb[2] + (unsigned char)((1.f-opacity)*rgb[2]);
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
                                       const double (*vals)[AVT_VARIABLE_LIMIT])
{
    double min = +FLT_MAX;
    double max = -FLT_MIN;
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
    double opacMax = rangeMaxTable.GetMaximumOverRange(minIndex, maxIndex);

    return (opacMax > 0. ? true : false);
}


