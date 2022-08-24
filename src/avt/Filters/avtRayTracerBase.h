// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtRayTracerBase.h                            //
// ************************************************************************* //

#ifndef AVT_RAY_TRACER_BASE_H
#define AVT_RAY_TRACER_BASE_H

#include <filters_exports.h>

#include <visit-config.h> // For LIB_VERSION_GE

#include <avtDatasetToImageFilter.h>
#include <avtViewInfo.h>
#include <avtOpacityMap.h>

#include <map>

class   avtRayFunction;
class   vtkMatrix4x4;

// ****************************************************************************
//  Class: avtRayTracerBase
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
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//
// ****************************************************************************

class AVTFILTERS_API avtRayTracerBase : public avtDatasetToImageFilter
{
  public:
                          avtRayTracerBase();
    virtual              ~avtRayTracerBase();

    virtual const char   *GetType(void) { return "avtRayTracerBase"; };
    virtual const char   *GetDescription(void) { return "Ray tracing"; };
    virtual void          ReleaseData(void);

    void                  SetView(const avtViewInfo &);

    static int            GetNumberOfStages(int, int, int);

    void                  InsertOpaqueImage(avtImage_p);

    void                  SetRayFunction(avtRayFunction *);
    void                  SetScreen(int, int);
    void                  SetSamplesPerRay(int);
    void                  SetBackgroundColor(const unsigned char [3]);
    virtual void          SetBackgroundMode(int mode) = 0;
    virtual void          SetGradientBackgroundColors(const double [3],
                                                      const double [3]) = 0;
    int                   GetSamplesPerRay(void)  { return samplesPerRay; };
    const int            *GetScreen(void)         { return screen; };

    void                  SetKernelBasedSampling(bool v) { kernelBasedSampling = v; };


    void                  SetTransferFn(avtOpacityMap *_transferFn1D) {transferFn1D = _transferFn1D; };
    void                  SetTrilinear(bool t) {trilinearInterpolation = t; };

#if LIB_VERSION_GE(VTK,9,1,0)
    void                  SetActiveVarName( std::string name ) { activeVarName = name; };
    void                  SetOpacityVarName( std::string name ) { opacityVarName = name; };
    void                  SetGradientVarName( std::string name ) { gradientVarName = name; };
#endif

  protected:
    avtViewInfo           viewInfo;

    int                   screen[2] = {400,400};
    int                   samplesPerRay{40};
    bool                  kernelBasedSampling{false};
    bool                  trilinearInterpolation{false};

    unsigned char         background[3] = {255,255,255};

    avtRayFunction       *rayfoo{nullptr};

#if LIB_VERSION_GE(VTK,9,1,0)
    std::string           activeVarName{"default"};
    std::string           opacityVarName{"default"};
    std::string           gradientVarName{"default"};
#endif

    avtOpacityMap        *transferFn1D{nullptr};

    avtImage_p            opaqueImage{nullptr};

    virtual void          Execute(void) = 0;

    virtual avtContract_p ModifyContract(avtContract_p);
    static int            GetNumberOfDivisions(int, int, int);
    virtual bool          FilterUnderstandsTransformedRectMesh();
    void                  TightenClippingPlanes(const avtViewInfo &view,
                                                vtkMatrix4x4 *,
                                                double &, double &);
};


#endif


