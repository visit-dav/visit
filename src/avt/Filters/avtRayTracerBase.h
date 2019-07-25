// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtRayTracerBase.h                            //
// ************************************************************************* //

#ifndef AVT_RAY_TRACER_BASE_H
#define AVT_RAY_TRACER_BASE_H

#include <filters_exports.h>

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
    void                  SetBackgroundMode(int mode);
    void                  SetGradientBackgroundColors(const double [3],
                                                      const double [3]);
    int                   GetSamplesPerRay(void)  { return samplesPerRay; };
    const int            *GetScreen(void)         { return screen; };

    void                  SetKernelBasedSampling(bool v) { kernelBasedSampling = v; };


    void                  SetTransferFn(avtOpacityMap *_transferFn1D) {transferFn1D = _transferFn1D; };
    void                  SetTrilinear(bool t) {trilinearInterpolation = t; };


  protected:
    avtViewInfo           view;

    int                   screen[2];
    int                   samplesPerRay;
    bool                  kernelBasedSampling;
    bool                  trilinearInterpolation;
    int                   backgroundMode;
    unsigned char         background[3];
    double                gradBG1[3];
    double                gradBG2[3];
    avtRayFunction       *rayfoo;
    avtOpacityMap        *transferFn1D;

    avtImage_p            opaqueImage;

    virtual void          Execute(void) = 0;

    virtual avtContract_p ModifyContract(avtContract_p);
    static int            GetNumberOfDivisions(int, int, int);
    virtual bool          FilterUnderstandsTransformedRectMesh();
    void                  TightenClippingPlanes(const avtViewInfo &view,
                                                vtkMatrix4x4 *,
                                                double &, double &);
};


#endif


