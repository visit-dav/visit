// ************************************************************************* //
//                                 avtRayTracer.h                            //
// ************************************************************************* //

#ifndef AVT_RAY_TRACER_H
#define AVT_RAY_TRACER_H

#include <pipeline_exports.h>

#include <avtDatasetToImageFilter.h>
#include <avtViewInfo.h>

class   avtRayFunction;


// ****************************************************************************
//  Class: avtRayTracer
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
// ****************************************************************************

class PIPELINE_API avtRayTracer : public avtDatasetToImageFilter
{
  public:
                          avtRayTracer();
    virtual              ~avtRayTracer() {;};

    virtual const char   *GetType(void) { return "avtRayTracer"; };
    virtual const char   *GetDescription(void) { return "Ray tracing"; };
    virtual void          ReleaseData(void);

    void                  SetView(const avtViewInfo &);

    void                  InsertOpaqueImage(avtImage_p);

    void                  SetRayFunction(avtRayFunction *);
    void                  SetScreen(int, int);
    void                  SetSamplesPerRay(int);
    void                  SetBackgroundColor(const unsigned char [3]);
    void                  SetBackgroundMode(int mode);
    void                  SetGradientBackgroundColors(const float [3],
                                                      const float [3]);
    int                   GetSamplesPerRay(void)  { return samplesPerRay; };
    const int            *GetScreen(void)         { return screen; };

  protected:
    avtViewInfo           view;

    int                   screen[2];
    int                   samplesPerRay;
    int                   backgroundMode;
    unsigned char         background[3];
    float                 gradBG1[3];
    float                 gradBG2[3];
    avtRayFunction       *rayfoo;
    
    avtImage_p            opaqueImage;

    virtual void          Execute(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
};


#endif


