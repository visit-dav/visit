// ************************************************************************* //
//                         avtSamplePointArbitrator.h                        //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_ARBITRATOR_H
#define AVT_SAMPLE_POINT_ARBITRATOR_H
#include <pipeline_exports.h>


class    avtOpacityMap;


// ****************************************************************************
//  Class: avtSamplePointArbitrator
//
//  Purpose:
//      Decides which sample point to use when multiple cells map to the same
//      sample.  The algorithm it uses tries to emphasize things of higher
//      opacity, since those will show up in the picture anyway and their
//      exclusion is the most noticable.
//
//  Programmer: Hank Childs
//  Creation:   January 23, 2002
//
// ****************************************************************************

class PIPELINE_API avtSamplePointArbitrator
{
  public:
                             avtSamplePointArbitrator(avtOpacityMap *, int);
    virtual                 ~avtSamplePointArbitrator() {;};

    int                      GetArbitrationVariable(void)
                                               { return arbitrationVariable; };
    bool                     ShouldOverwrite(float, float);

  protected:
    avtOpacityMap           *omap;
    int                      arbitrationVariable;
};


#endif



