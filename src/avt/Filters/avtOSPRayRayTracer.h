// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9: add
//    SetBackgroundMode and SetGradientBackgroundColors from avtRayTracerBase.
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

    // avtRayTracerBase
    void SetBackgroundMode(int mode) override {};
    void SetGradientBackgroundColors(const double [3],
                                     const double [3]) override {};


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
