// ************************************************************************* //
//                              avtRayCompositer.h                           //
// ************************************************************************* //

#ifndef AVT_RAY_COMPOSITER_H
#define AVT_RAY_COMPOSITER_H

#include <filters_exports.h>

#include <avtSamplePointsToImageFilter.h>

#define BACKGROUND_SOLID           0
#define BACKGROUND_GRADIENT_TB     1
#define BACKGROUND_GRADIENT_BT     2
#define BACKGROUND_GRADIENT_LR     3
#define BACKGROUND_GRADIENT_RL     4
#define BACKGROUND_GRADIENT_RADIAL 5

class  avtRayFunction;
class  avtPixelizer;


// ****************************************************************************
//  Class: avtRayCompositer
//
//  Purpose:
//      Composites rays from sample points.  The output is an avtImage.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:17:20 PST 2001
//    Remove dependence on pixelizers.
//
//    Hank Childs, Tue Feb 13 15:15:50 PST 2001
//    Allowed for opaque images to be inserted in to volume rendering.
//
//    Brad Whitlock, Wed Dec 5 10:32:35 PDT 2001
//    Added methods to draw the background.
//
// ****************************************************************************

class AVTFILTERS_API avtRayCompositer : public avtSamplePointsToImageFilter
{
  public:
                          avtRayCompositer(avtRayFunction *);

    virtual const char   *GetType(void) { return "avtRayCompositer"; };
    virtual const char   *GetDescription(void) 
                                             { return "Compositing samples"; };
    void                  SetBackgroundColor(const unsigned char [3]);
    void                  SetBackgroundMode(int mode);
    void                  SetGradientBackgroundColors(const float [3],
                                                      const float [3]);
    void                  InsertOpaqueImage(avtImage_p);
    void                  UpdateCompositeProgress(int, int);

  protected:
    avtRayFunction       *rayfoo;
    int                   backgroundMode;
    unsigned char         background[3];
    float                 gradBG1[3];
    float                 gradBG2[3];
    avtImage_p            opaqueImage;

    virtual void          Execute(void);
    void                  FillBackground(unsigned char *, int, int);
    void                  DrawRadialGradient(unsigned char *, int, int);
};


#endif


