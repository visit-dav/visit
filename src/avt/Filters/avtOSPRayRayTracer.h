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
//                           avtOSPRayRayTracer.h                            //
// ************************************************************************* //

#ifndef AVT_OSPRAY_RAY_TRACER_H
#define AVT_OSPRAY_RAY_TRACER_H

#include <filters_exports.h>

#include <avtRayTracerBase.h>
#include <avtOSPRayCommon.h>
#include <avtOSPRayImageCompositor.h>
#include <LightList.h>

class   vtkMatrix4x4;

// ****************************************************************************
//  Class: avtOSPRayRayTracer
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
//    Qi Wu, Sun Jul 1 2018
//    Added support for ospray volume rendering.
//
// ****************************************************************************

class AVTFILTERS_API avtOSPRayRayTracer : public avtRayTracerBase
{
public:
    avtOSPRayRayTracer();
    virtual              ~avtOSPRayRayTracer();

    virtual const char   *GetType(void)      { return "avtOSPRayRayTracer"; };
    virtual const char   *GetDescription(void) 
                                             { return "OSPRay Ray tracing"; };

    void SetActiveVariable(const char* s)             { activeVariable = s; };
    void SetLightInfo(const LightList& l)                  { lightList = l; };
    void SetOSPRay(OSPVisItContext *ptr)               { ospray_core = ptr; };

    void SetLighting(bool l)                  { gradientShadingEnabled = l; };
    void SetShadowsEnabled(bool l)                    { shadowsEnabled = l; };
    void SetUseGridAccelerator(bool l)            { useGridAccelerator = l; };
    void SetPreIntegration(bool l)                    { preIntegration = l; };
    void SetSingleShade(bool l)                          { singleShade = l; };
    void SetOneSidedLighting(bool l)                { oneSidedLighting = l; };
    void SetAoTransparencyEnabled(bool l)      { aoTransparencyEnabled = l; };

    void SetAoSamples(int v)                               { aoSamples = v; };
    void SetSpp(int v)                                           { spp = v; };
    
    void SetAoDistance(double v)                          { aoDistance = v; };
    void SetSamplingRate(double v)                      { samplingRate = v; };
    void SetMinContribution(double v)                { minContribution = v; };
    
    void SetMatProperties(double v[4]) 
                    { for (int i=0; i<4; i++) materialProperties[i] = v[i]; };
    void SetViewDirection(double v[3])
                         { for (int i=0; i<3; i++) viewDirection[i] = v[i]; };
protected:
    virtual void             Execute(void);

    const char*              activeVariable;
    LightList                lightList;
    OSPVisItContext         *ospray_core;

    bool                     gradientShadingEnabled;
    bool                     shadowsEnabled;
    bool                     useGridAccelerator;
    bool                     preIntegration;
    bool                     singleShade;
    bool                     oneSidedLighting;
    bool                     aoTransparencyEnabled;
    int                      spp;
    int                      aoSamples;
    double                   aoDistance;
    double                   samplingRate;
    double                   minContribution;
    
    double                   materialProperties[4];
    double                   viewDirection[3];

    avtOSPRayImageCompositor imgComm;
};

#endif
