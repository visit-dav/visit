// ************************************************************************* //
//                              avtRayFunction.h                             //
// ************************************************************************* //

#ifndef AVT_RAY_FUNCTION_H
#define AVT_RAY_FUNCTION_H
#include <pipeline_exports.h>


#include <avtCellTypes.h>


class     avtGradients;
class     avtLightingModel;
class     avtRay;


// ****************************************************************************
//  Class: avtRayFunction
//
//  Purpose:
//      This is the base type for any ray function.  A ray function is a
//      routine that takes a series of sample points along a ray (avtRay) and
//      a lighting model and determines what the shading for the pixel should
//      be.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:14:19 PST 2001
//    Made the output of GetRayValue be a pixel instead of a value.
//
// ****************************************************************************

class PIPELINE_API avtRayFunction
{
  public:
                         avtRayFunction(avtLightingModel *);
    virtual             ~avtRayFunction() {;};

    bool                 NeedsGradients(void);

    virtual void         GetRayValue(const avtRay *, const avtGradients *,
                                     unsigned char rgb[3], float) = 0;
    virtual bool         CanContributeToPicture(int,
                                          const float (*)[AVT_VARIABLE_LIMIT]);
    virtual float        ClassifyForShading(float x) { return x; };

  protected:
    avtLightingModel    *lighting;

    virtual bool         NeedsGradientsForFunction(void) = 0;
    inline int           IndexOfDepth(const float &, const int &);
};


// ****************************************************************************
//  Method: avtRayFunction::IndexOfDepth
//
//  Purpose:
//      Determines the index of a depth in the z-buffer.  Assumes 0. is the
//      near plane, 1. is the far plane.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
// ****************************************************************************

inline int
avtRayFunction::IndexOfDepth(const float &depth, const int &numSamples)
{
    int rv = (int) (depth*numSamples);
    if (rv >= numSamples)
    {
        rv = numSamples-1;
    }
    else if (rv < 0)
    {
        rv = 0;
    }

    return rv;
}


#endif


