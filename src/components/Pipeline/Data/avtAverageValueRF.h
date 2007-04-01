// ************************************************************************* //
//                              avtAverageValueRF.h                          //
// ************************************************************************* //

#ifndef AVT_AVERAGE_VALUE_RF_H
#define AVT_AVERAGE_VALUE_RF_H
#include <pipeline_exports.h>


#include <avtRayFunction.h>

class     avtVariablePixelizer;


// ****************************************************************************
//  Class: avtAverageValueRF
//
//  Purpose:
//      The derived type of ray function that will give the average value along
//      a ray.  A value must be set for what contribution a sample point makes
//      if it is not valid (typically the minimum for the dataset).
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:49:43 PST 2001
//    Made ray function return a color instead of a value.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtAverageValueRF : public avtRayFunction
{
  public:
                        avtAverageValueRF(avtLightingModel *,
                                          avtVariablePixelizer *, bool,
                                          double = 0.);
    virtual            ~avtAverageValueRF();

    virtual void        GetRayValue(const avtRay *, const avtGradients *,
                                    unsigned char rgb[3], float);

  protected:
    double              noSampleValue;
    bool                useNoSampleValue;
    avtVariablePixelizer *pix;

    virtual bool        NeedsGradientsForFunction(void) { return false; };
};


#endif


