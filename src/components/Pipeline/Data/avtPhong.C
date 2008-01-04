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
//                                 avtPhong.C                                //
// ************************************************************************* //

#include <avtPhong.h>

#include <math.h>
#include <visitstream.h>


// ****************************************************************************
//  Method: avtPhong constructor
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:34:07 PST 2002
//    Changed default light.
//
// ****************************************************************************

avtPhong::avtPhong()
{
    //
    // Default light is coming in from side.
    //
    double l[3] = { -0.5, -0.5, -1. };
    SetLightDirection(l);

    SetAmbient(0.5);
    SetDiffuse(1.);
    SetSpecular(1.);
    SetGlossiness(1);
}


// ****************************************************************************
//  Method: avtPhong destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPhong::~avtPhong()
{
    ;
}


// ****************************************************************************
//  Method: avtPhong::SetLightDirection
//
//  Purpose:
//      Sets the vector of the light coming from the light source towards the
//      object (focal point).
//
//  Arguments:
//      l     The new light direction.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:34:07 PST 2002
//    Normalize the light direction.
//
// ****************************************************************************

void
avtPhong::SetLightDirection(double l[3])
{
    double mag = sqrt(l[0]*l[0] + l[1]*l[1] + l[2]*l[2]);
    if (mag == 0.)
    {
        mag = 1.;
    }
    lightDirection[0] = l[0] / mag;
    lightDirection[1] = l[1] / mag;
    lightDirection[2] = l[2] / mag;

    //
    // This is the half-way between the light and the view.  The view is 0,0,-1
    //
    half[0] = lightDirection[0] + 0.;
    half[1] = lightDirection[1] + 0.;
    half[2] = lightDirection[2] + -1.;

    if (half[2] == 0.)
    {
        //
        // The light is on the opposite side of the object from the camera.
        // Put it in the camera's location.
        //
        half[2] = -1.;
    }
    else
    {
        double norm = (half[0]*half[0])+(half[1]*half[1])+(half[2]*half[2]);
        norm = sqrt(norm);
        half[0] /= norm;
        half[1] /= norm;
        half[2] /= norm;
    }
}


// ****************************************************************************
//  Method: avtPhong::GetShading
//
//  Purpose:
//      Modifies the opacity to account for shading.
//
//  Arguments:
//      distance   The distance from the sample point to the plane.  This does
//                 not need to be in specific units, because it is the same
//                 units as the gradient.
//      gradient   The gradient around a point (assumed to be the normal of
//                 an underlying surface).
//
//  Note:       We are assuming a parallel light from far away and we are doing
//              an orthographic projection (in camera space), so the location
//              of the sample does not matter.
//
//  Note:       This version of the Phong lighting model is from Marc Levoy's
//              paper on "Display of Surfaces from Volume Data".  It cites
//              P. Bui-Tuong, "Illumination for Computer-Generated Pictures",
//              CACM, June 1975.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:34:07 PST 2002
//    Do not use specular highlighting.
//
// ****************************************************************************

double
avtPhong::GetShading(double distance, const double gradient[3]) const
{
    double mag = sqrt(gradient[0]*gradient[0] + gradient[1]*gradient[1]
                      + gradient[2]*gradient[2]);
    if (mag == 0)
    {
        return 1.;
    }  

    double norm[3];
    norm[0] = gradient[0] / mag;
    norm[1] = gradient[1] / mag;
    norm[2] = gradient[2] / mag;

    double dot = lightDirection[0]*norm[0] + lightDirection[1]*norm[1] 
                 + lightDirection[2]*norm[2];
    if (dot < 0)
    {
        dot = -dot;
    }
    return ambient + (1. - ambient)*dot;
}


