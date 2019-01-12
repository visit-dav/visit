/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
//                            avtSLIVRRayTracer.h                            //
// ************************************************************************* //

#ifndef AVT_SLIVR_RAY_TRACER_H
#define AVT_SLIVR_RAY_TRACER_H

#include <filters_exports.h>

#include <avtRayTracerBase.h>
#include <avtSLIVRImageCompositor.h>

class   vtkMatrix4x4;

// ****************************************************************************
//  Class: avtSLIVRRayTracer
//
//  Purpose:
//      Performs ray tracing, taking in a dataset as a source and has an
//      image as an output.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 16:52:26 PST 2001
//    Added "Get" functions.
//
//    Hank Childs, Sat Feb  3 20:37:01 PST 2001
//    Removed pixelizer and added mechanism to change background color.
//
//    Hank Childs, Tue Feb 13 15:15:50 PST 2001
//    Added ability to insert an opaque image into the rendering.
//
//    Brad Whitlock, Wed Dec 5 11:13:18 PDT 2001
//    Added gradient backgrounds.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Sun Dec  4 18:00:55 PST 2005
//    Add method that estimates number of stages.
//
//    Hank Childs, Mon Jan 16 11:11:47 PST 2006
//    Add support for kernel based sampling.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Wed Dec 24 14:17:03 PST 2008
//    Add method TightenClippingPlanes.
//
//    Pascal Grosset, Fri Sep 20 2013
//    Added ray casting slivr & trilinear interpolation
//
// ****************************************************************************

class AVTFILTERS_API avtSLIVRRayTracer : public avtRayTracerBase
{
  public:
                          avtSLIVRRayTracer();
    virtual              ~avtSLIVRRayTracer();

    virtual const char   *GetType(void) { return "avtSLIVRRayTracer"; };
    virtual const char   *GetDescription(void) { return "SLIVR Ray tracing"; };


    void                  blendImages(float *src, int dimsSrc[2], int posSrc[2], float *dst, int dimsDst[2], int posDst[2]);
    void                  SetLighting(bool l) {lighting = l; };
    void                  SetLightPosition(double _lightPos[4]) { for (int i=0;i<4;i++) lightPosition[i]=_lightPos[i]; }
    void                  SetLightDirection(double _lightDir[3]) { for (int i=0;i<3;i++) lightDirection[i]=_lightDir[i]; }
    void                  SetMatProperties(double _matProp[4]) { for (int i=0;i<4;i++) materialProperties[i]=_matProp[i]; }
    void                  SetViewDirection(double *vd){ for (int i=0; i<3; i++) view_direction[i] = vd[i]; }

  protected:

    virtual void          Execute(void);

    avtSLIVRImageCompositor    imgComm;
    bool                  lighting;
    double                lightPosition[4];
    double                lightDirection[3];
    double                materialProperties[4];
    double                view_direction[3];
    double                panPercentage[2];

    void project3Dto2D(double _3Dextents[6], int width, int height,
                       vtkMatrix4x4 *_mvp, int _2DExtents[4],
                       double depthExtents[2]);
    double project(double _worldCoordinates[3], int pos2D[2], 
                   int _width, int _height, vtkMatrix4x4 *_mvp);
    void unProject(int _x, int _y, float _z, double _worldCoordinates[3],
                   int _width, int _height, vtkMatrix4x4 *invModelViewProj);
    bool checkInBounds(double volBounds[6], double coord[3]);
    void computeRay(double camera[3], double position[3], double ray[3]);
    bool intersect(double bounds[6], double ray[3], double cameraPos[3],
                   double &tMin, double &tMax);
};


#endif


