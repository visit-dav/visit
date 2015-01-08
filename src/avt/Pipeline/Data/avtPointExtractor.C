/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                              avtPointExtractor.C                          //
// ************************************************************************* //

#include <avtPointExtractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>


// ****************************************************************************
//  Method: avtPointExtractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put samples into.
//     cl    The cell list to put cells whose sampling was deferred.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2006
//
// ****************************************************************************

avtPointExtractor::avtPointExtractor(int w, int h, int d,
                                                 avtVolume *vol,
                                                 avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    epsilon = 0.1;
    correction = 1. / (1 + epsilon);

    // We want the smallest R-max to be based on the most degenerate case --
    // where a zone falls perfectly between sample points (this is the furthest
    // away a zone can ever be from samples).  Whatever this distance is, we
    // want R-max to be 50% bigger.  So calculate this by calculating the
    // position of something between the 8 samples ([0|1], [0|1], [0|1]).
    double xS = (0*2. / width) - 1.;   // = -1.
    double yS = (0*2. / height) - 1.;  // = -1.
    double zS = 0.;
    double xZ = (0.5*2. / width) - 1.;
    double yZ = (0.5*2. / height) - 1.;
    double zZ = 1. / depth;
    smallestX = 1.5*(xZ - xS);
    smallestY = 1.5*(yZ - yS);
    smallestZ = 1.5*(zZ - zS);
    if (depth == 1)
        smallestZ = 0.;
}


// ****************************************************************************
//  Method: avtPointExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2006
//
// ****************************************************************************

avtPointExtractor::~avtPointExtractor()
{
    ;
}


// ****************************************************************************
//  Method: avtPointExtractor::Extract
//
//  Purpose:
//      Extracts the sample points from the bounding box using a kernel based
//      technique.
//
//  Programmer:   Hank Childs
//  Creation:     January 24, 2006
//
//  Modifications:
//
//    Hank Childs, Thu May 31 08:26:30 PDT 2007
//    Sharpen up weighting function ... values were being blurred way too much.
//
// ****************************************************************************

void
avtPointExtractor::Extract(const avtPoint &pt)
{
    const double *bbox = pt.bbox;

    double center[3];
    center[0] = (bbox[0] + bbox[1]) / 2.;
    center[1] = (bbox[2] + bbox[3]) / 2.;
    center[2] = (bbox[4] + bbox[5]) / 2.;
    double xDiff = (center[0] - bbox[0]);
    xDiff = (xDiff > smallestX ? xDiff : smallestX);
    double yDiff = (center[1] - bbox[2]);
    yDiff = (yDiff > smallestY ? yDiff : smallestY);
    double zDiff = (center[2] - bbox[4]);
    zDiff = (zDiff > smallestZ ? zDiff : smallestZ);

    double rad = 1.5*sqrt(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);
    int minx = (int) ceil((((center[0] - rad) + 1.) / 2.) * width);
    int maxx = (int) floor((((center[0] + rad) + 1.) / 2.) * width);
    int miny = (int) ceil((((center[1] - rad) + 1.) / 2.) * height);
    int maxy = (int) floor((((center[1] + rad) + 1.) / 2.) * height);
    // Note that X&Y span -1 -> +1, while Z spans 0->+1, so we should
    // correct Z to make all lengths "equal".
    int minz = (int) ceil((center[2] - rad/2) * depth);
    int maxz = (int) floor((center[2] + rad/2) * depth);

    //
    // Make sure we're within the view frustum.
    //
    if (minx > restrictedMaxWidth)
        return;
    if (maxx < restrictedMinWidth)
        return;
    if (miny > restrictedMaxHeight)
        return;
    if (maxy < restrictedMinHeight)
        return;

    minx = (minx < restrictedMinWidth ? restrictedMinWidth : minx);
    maxx = (maxx > restrictedMaxWidth ? restrictedMaxWidth : maxx);
    miny = (miny < restrictedMinHeight ? restrictedMinHeight : miny);
    maxy = (maxy > restrictedMaxHeight ? restrictedMaxHeight : maxy);
    minz = (minz < 0 ? 0 : minz);
    maxz = (maxz >= depth ? depth-1 : maxz);

    int potentialNumSamples = (maxx-minx+1)*(maxy-miny+1)*(maxz-minz+1);
    if (sendCellsMode && potentialNumSamples > 64)
    {
        celllist->Store(pt, minx, maxx, miny, maxy);
        return;
    }

    double cval2[AVT_VARIABLE_LIMIT];
    int nVars = pt.nVars;

    for (int i = minx ; i <= maxx ; i++)
    {
        if (i < 0 || i >= width)
            continue;
        double xf = ((i*2.) / width)  - 1.;
        double xD = xf-center[0];
        for (int j = miny ; j <= maxy ; j++)
        {
            if (j < 0 || j >= height)
                continue;
            avtRay *ray = volume->GetRay(i, j);
            double yf = ((j*2.) / height) - 1.;
            double yD = yf-center[1];
            for (int k = minz ; k <= maxz ; k++)
            {
                if (k < 0 || k >= depth)
                    continue;
                //double zf = ((k*2.) / depth)  - 1.;
                double zf = ((double)k)/((double)depth);
                double zD = 2*(zf-center[2]);
                double dist = xD*xD + yD*yD + zD*zD;
                double weight = 0.;
                if (dist >= rad*rad)
                    continue;
                else if (dist < 1e-6)
                    weight = 1000000.;
                else
                    weight = 1/dist;
                for (int ii = 0 ; ii < nVars ; ii++)
                    cval2[ii] = weight*pt.val[ii];
                cval2[nVars] = weight;
                ray->SetSample(k, cval2);
            }
        }
    }
}


