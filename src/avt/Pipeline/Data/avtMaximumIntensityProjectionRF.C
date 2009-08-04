/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                       avtMaximumIntensityProjectionRF.C                   //
// ************************************************************************* //

#include <avtMaximumIntensityProjectionRF.h>

#include <float.h>

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
//  Method: avtMaximumIntensityProjectionRF destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMaximumIntensityProjectionRF::~avtMaximumIntensityProjectionRF()
{
    ;
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

        pix->GetColor(value, 1., rgb);
        lighting->AddLighting(maxInd, ray, rgb);
    }
}


