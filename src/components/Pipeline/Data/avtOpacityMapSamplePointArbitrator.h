// ************************************************************************* //
//                    avtOpacityMapSamplePointArbitrator.h                   //
// ************************************************************************* //

#ifndef AVT_OPACITY_MAP_SAMPLE_POINT_ARBITRATOR_H
#define AVT_OPACITY_MAP_SAMPLE_POINT_ARBITRATOR_H

#include <pipeline_exports.h>

#include <avtSamplePointArbitrator.h>

class    avtOpacityMap;


// ****************************************************************************
//  Class: avtOpacityMapSamplePointArbitrator
//
//  Purpose:
//      Decides which sample point to use when multiple cells map to the same
//      sample.  The algorithm it uses tries to emphasize things of higher
//      opacity, since those will show up in the picture anyway and their
//      exclusion is the most noticable.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

class PIPELINE_API avtOpacityMapSamplePointArbitrator 
    : public avtSamplePointArbitrator
{
  public:
                      avtOpacityMapSamplePointArbitrator(avtOpacityMap *, int);
    virtual          ~avtOpacityMapSamplePointArbitrator();

    virtual bool      ShouldOverwrite(float, float);

  protected:
    avtOpacityMap    *omap;
};


#endif


