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
//                                 avtPhong.C                                //
// ************************************************************************* //

#include <avtPhong.h>

#include <math.h>

#include <visitstream.h>

#include <avtRay.h>


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
//  Method: avtPhong::AddLighting
//
//  Purpose:
//      Modifies the color to account for shading.
//
//  Note:       This version of the Phong lighting model is from Marc Levoy's
//              paper on "Display of Surfaces from Volume Data".  It cites
//              P. Bui-Tuong, "Illumination for Computer-Generated Pictures",
//              CACM, June 1975.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2008
//
//  Modifications:
//      Sean Ahern, Wed Sep  3 11:27:43 EDT 2008
//      Added support for camera lights.
//
//      Hank Childs, Wed Aug 19 19:09:08 PDT 2009
//      If the gradient is 0 magnitude, then don't apply lighting to that 
//      sample.
//
// ****************************************************************************

void
avtPhong::AddLighting(int index, const avtRay *ray, unsigned char *rgb) const
{
    double r = 0., g = 0., b = 0.;

    const int maxNumLights = 8;
    for (int i = 0 ; i < maxNumLights ; i++)
    {
        const LightAttributes &l = lights.GetLight(i);
        if (! l.GetEnabledFlag())
            continue;
  
        double brightness = l.GetBrightness();
        if (l.GetType() == LightAttributes::Ambient)
        {
            r += brightness*rgb[0];
            g += brightness*rgb[1];
            b += brightness*rgb[2];
        }
        else 
        {
            double dir[3];
            if (l.GetType() == LightAttributes::Object)
            {
                dir[0] = l.GetDirection()[0];
                dir[1] = l.GetDirection()[1];
                dir[2] = l.GetDirection()[2];
            }
            else // Camera light.
            {
                double view_right[3];   // The view "right" vector.
                                        // view_direction cross view_up
                view_right[0] = view_direction[1]*view_up[2] - view_direction[2]*view_up[1];
                view_right[1] = view_direction[2]*view_up[0] - view_direction[0]*view_up[2];
                view_right[2] = view_direction[0]*view_up[1] - view_direction[1]*view_up[0];

                // A camera light's components are scaling factors of
                // view_right, view_up, and view_direction.  Scale the
                // components and set to the dir vector.

                double comp1[3];
                comp1[0] = view_right[0] * l.GetDirection()[0];
                comp1[1] = view_right[1] * l.GetDirection()[0];
                comp1[2] = view_right[2] * l.GetDirection()[0];

                double comp2[3];
                comp2[0] = view_up[0] * l.GetDirection()[1];
                comp2[1] = view_up[1] * l.GetDirection()[1];
                comp2[2] = view_up[2] * l.GetDirection()[1];

                double comp3[3];
                comp3[0] = -view_direction[0] * l.GetDirection()[2];
                comp3[1] = -view_direction[1] * l.GetDirection()[2];
                comp3[2] = -view_direction[2] * l.GetDirection()[2];

                dir[0] = comp1[0] + comp2[0] + comp3[0];
                dir[1] = comp1[1] + comp2[1] + comp3[1];
                dir[2] = comp1[2] + comp2[2] + comp3[2];
            }

            double grad[3];
            grad[0] = ray->sample[gradientVariableIndex][index];
            grad[1] = ray->sample[gradientVariableIndex+1][index];
            grad[2] = ray->sample[gradientVariableIndex+2][index];
            double mag = sqrt(grad[0]*grad[0] + grad[1]*grad[1] +
                              grad[2]*grad[2]);
            if (mag == 0.)
            {
                r += brightness*rgb[0];
                g += brightness*rgb[1];
                b += brightness*rgb[2];
                continue;
            }
            grad[0] /= mag;
            grad[1] /= mag;
            grad[2] /= mag;

            double diffuse = grad[0]*dir[0] + grad[1]*dir[1] + grad[2]*dir[2];
            if (diffuse < 0.)
                diffuse *= -1.; // setting to 0 would be one-sided lighting
            r += brightness*diffuse*rgb[0];
            g += brightness*diffuse*rgb[1];
            b += brightness*diffuse*rgb[2];
  
            if (doSpecular)
            {
                // If we have vector n1 bouncing off a wall with normal s 
                // to give a reflecting vector n2, then n2 = n1 + 2s.
                double reflection[3];
                reflection[0] = dir[0] + 2*grad[0];
                reflection[1] = dir[1] + 2*grad[1];
                reflection[2] = dir[2] + 2*grad[2];
                double mag = sqrt(reflection[0]*reflection[0] +
                                  reflection[1]*reflection[1] +
                                  reflection[2]*reflection[2]);
                reflection[0] /= mag;
                reflection[1] /= mag;
                reflection[2] /= mag;
                double dot = view_direction[0]*reflection[0]
                           + view_direction[1]*reflection[1]
                           + view_direction[2]*reflection[2];
                if (dot < 0)
                    dot *= -1.;
                double p = pow(dot, specularPower);
                double distToGoR = (r < 255 ? 255-r : 0.);
                r += distToGoR*brightness*specularCoeff*p;
                double distToGoG = (g < 255 ? 255-g : 0.);
                g += distToGoG*brightness*specularCoeff*p;
                double distToGoB = (b < 255 ? 255-b : 0.);
                b += distToGoB*brightness*specularCoeff*p;
            }
        }
    }

    if (r >= 255.0)
        rgb[0] = 255;
    else
        rgb[0] = (unsigned char) r;
    if (g >= 255.0)
        rgb[1] = 255;
    else
        rgb[1] = (unsigned char) g;
    if (b >= 255.0)
        rgb[2] = 255;
    else
        rgb[2] = (unsigned char) b;
}


