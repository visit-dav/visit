// ************************************************************************* //
//                                avtMIPRangeRF.h                            //
// ************************************************************************* //

#ifndef AVT_MIP_RANGE_RF_H
#define AVT_MIP_RANGE_RF_H
#include <pipeline_exports.h>


#include <avtRayFunction.h>

class     avtVariablePixelizer;

// ****************************************************************************
//  Class: avtMIPRangeRF
//
//  Purpose:
//      A derived type of ray function, this will take ray profiles and
//      determine the maximum along that ray.  It will throw out any samples
//      that are not within its range.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 21:19:23 PST 2001
//    Modified to return a color instead of a value.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtMIPRangeRF : public avtRayFunction
{
  public:
                        avtMIPRangeRF(avtLightingModel *, double, double,
                                      avtVariablePixelizer *);
    virtual            ~avtMIPRangeRF();

    virtual void        GetRayValue(const avtRay *, const avtGradients *,
                                    unsigned char [3], float);

  protected:
    double              thresholdMin, thresholdMax;
    avtVariablePixelizer *pix;

    virtual bool        NeedsGradientsForFunction(void)  { return false; };
};


#endif


