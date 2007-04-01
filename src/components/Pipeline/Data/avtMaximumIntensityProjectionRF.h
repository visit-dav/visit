// ************************************************************************* //
//                      avtMaximumIntensityProjectionRF.h                    //
// ************************************************************************* //

#ifndef AVT_MAXIMUM_INTENSITY_PROJECTION_H
#define AVT_MAXIMUM_INTENSITY_PROJECTION_H
#include <pipeline_exports.h>


#include <avtRayFunction.h>

class     avtVariablePixelizer;


// ****************************************************************************
//  Class: avtMaximumIntensityProjectionRF
//
//  Purpose:
//      A derived type of ray function, this will take ray profiles and
//      determine the maximum along that ray.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:49:43 PST 2001
//    Re-wrote to return pixels instead of values.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtMaximumIntensityProjectionRF : public avtRayFunction
{
  public:
                        avtMaximumIntensityProjectionRF(avtLightingModel *,
                                                      avtVariablePixelizer *);
    virtual            ~avtMaximumIntensityProjectionRF();

    virtual void        GetRayValue(const avtRay *, const avtGradients *,
                                    unsigned char [3], float);

  protected:
    avtVariablePixelizer  *pix;

    virtual bool        NeedsGradientsForFunction(void)  { return false; };
};


#endif


