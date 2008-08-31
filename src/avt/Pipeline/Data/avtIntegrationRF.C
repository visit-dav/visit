/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                             avtIntegrationRF.C                            //
// ************************************************************************* //

#include <avtIntegrationRF.h>

#include <snprintf.h>

#include <avtCallback.h>
#include <avtLightingModel.h>
#include <avtParallel.h>
#include <avtRay.h>
#include <avtVariablePixelizer.h>


int       avtIntegrationRF::windowSize[2];
double   *avtIntegrationRF::vals = NULL;
bool      avtIntegrationRF::issuedWarning = false;


// ****************************************************************************
//  Method: avtIntegrationRF constructor
//
//  Arguments:
//      l        The lighting model for the ray.
//
//  Programmer:  Hank Childs
//  Creation:    September 11, 2006
//
// ****************************************************************************

avtIntegrationRF::avtIntegrationRF(avtLightingModel *l)
    : avtRayFunction(l)
{
    distance = 1.;
}


// ****************************************************************************
//  Method: avtIntegrationRF destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2006
//
// ****************************************************************************

avtIntegrationRF::~avtIntegrationRF()
{
    ;
}


// ****************************************************************************
//  Method: avtIntegrationRF::GetRayValue
//
//  Purpose:
//      Gets the value for the pixel that the ray coincides with.  Does this
//      by examining the ray profile and determining the integral of all values
//      along the ray.  Then maps that value to a greyscale intensity and 
//      returns that intensity.
//
//  Arguments:
//      ray         The ray to use.
//      rgb         A place to store the color.
//      depth       The depth of the zbuffer for this pixel.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Mar 13 16:16:42 PDT 2007
//    Incorporate distances so that intensities will be absolute.
//
//    Hank Childs, Sun Aug 31 08:42:03 PDT 2008
//    Adapt to the new way that lighting is done.
//
// ****************************************************************************

void
avtIntegrationRF::GetRayValue(const avtRay *ray,
                               unsigned char rgb[3], float depth)
{
    //
    // Some compilers do very poor optimizations, so make sure that we don't
    // do an offset from the pointer at each step by creating automatic
    // variables and using those.
    //
    const int     numSamples  = ray->numSamples;
    const bool   *validSample = ray->validSample;

    const float  *sample      = ray->sample[primaryVariableIndex];

    double sum    = 0.;
    double divisor = distance/numSamples;

    int firstValid = -1;
    for (int i = 0 ; i < numSamples ; i++)
    {
        if (validSample[i])
        {
            if (firstValid < 0)
                firstValid = i;
            sum += sample[i] * divisor;
        }
    }

    //
    // Store off the raw values into a buffer for use later.
    //
    int index = pixelIndexJ * windowSize[0] + pixelIndexI;
    vals[index] = sum;

    //
    // Now map the sum to a grey scale intensity
    //
    double rel_val = (sum - min*distance) / (max*distance - min*distance);
    if (rel_val >= 1.)
        rel_val = 1.;
    if (rel_val <= 0.)
        rel_val = 0.;
    unsigned char intensity = (unsigned char) (rel_val*255);

    int maxSample = IndexOfDepth(depth, numSamples);
    if (maxSample >= firstValid)
    {
        rgb[0] = intensity;
        rgb[1] = intensity;
        rgb[2] = intensity;
    }
    else
    {
        // Leave the color as is...
        ;
    }
}


// ****************************************************************************
//  Function: SetWindowSize
//
//  Purpose:
//      A static method to set the window size.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2006
//
// ****************************************************************************

void
avtIntegrationRF::SetWindowSize(int s0, int s1)
{
    windowSize[0] = s0;
    windowSize[1] = s1;
    if (vals != NULL)
        delete [] vals;
    int nvals = windowSize[0]*windowSize[1];
    vals = new double[nvals];
    for (int i = 0 ; i < nvals ; i++)
        vals[i] = 0.;
}


// ****************************************************************************
//  Function: OutputRawValues
//
//  Purpose:
//      A static method to output the raw values from the integration.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Feb 20 11:09:49 PST 2007
//    Output the window size as well.
//
//    Hank Childs, Tue Mar 13 16:26:55 PDT 2007
//    Issue a warning to tell the user where the file was outputted.
//
// ****************************************************************************

void
avtIntegrationRF::OutputRawValues(const char *filename)
{
    int nvals = windowSize[0]*windowSize[1];
    double *out_vals = new double[nvals];
    SumDoubleArrayAcrossAllProcessors(vals, out_vals, nvals);

    if (PAR_Rank() == 0)
    {
        ofstream ofile(filename);
        if (ofile.fail())
        {
            avtCallback::IssueWarning("The integration option for the ray"
                    " casted volume rendering could not output a file, because"
                    " it could not open the file.");
        }

        ofile << windowSize[0] << " " << windowSize[1] << endl;
        for (int i = 0 ; i < nvals ; i++)
            ofile << out_vals[i] << endl;
    }

    if (!issuedWarning)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "The integration was outputted to the file \"%s\".  "
                 "If you are running client/server, the file is located where "
                 "the remote server is running.  This message will only be issued"
                 " once per VisIt session.  Further renderings will overwrite"
                 " previous versions of this file.", filename);
        avtCallback::IssueWarning(msg);
        issuedWarning = true;
    }

    delete [] out_vals;
}


