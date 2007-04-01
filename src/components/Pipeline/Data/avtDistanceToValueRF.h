// ************************************************************************* //
//                           avtDistanceToValueRF.h                          //
// ************************************************************************* //

#ifndef AVT_DISTANCE_TO_VALUE_RF_H
#define AVT_DISTANCE_TO_VALUE_RF_H
#include <pipeline_exports.h>


#include <avtRayFunction.h>

class     avtDistancePixelizer;


// ****************************************************************************
//  Class: avtDistanceToValueRF
//
//  Purpose:
//      A derived type of ray function, this will take a ray profile and
//      determine what the distance is to some value.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDistanceToValueRF : public avtRayFunction
{
  public:
                     avtDistanceToValueRF(avtLightingModel *, double,
                                          avtDistancePixelizer *);
    virtual         ~avtDistanceToValueRF();

    virtual void     GetRayValue(const avtRay *, const avtGradients *,
                                 unsigned char rgb[3], float);

  protected:
    double                  criticalValue;
    avtDistancePixelizer   *pix;

    virtual bool            NeedsGradientsForFunction(void)
                                      { return false; };   
};


#endif


