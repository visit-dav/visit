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
//      sample.  The algorithm it uses is defined by its derived type.
//
//  Programmer: Hank Childs
//  Creation:   January 23, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sat Jan 29 10:22:56 PST 2005
//    Made ShouldOverwrite be pure virtual.  This is now an abstract type.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointArbitrator
{
  public:
                             avtSamplePointArbitrator(int);
    virtual                 ~avtSamplePointArbitrator();

    int                      GetArbitrationVariable(void)
                                               { return arbitrationVariable; };
    virtual bool             ShouldOverwrite(float, float) = 0;

  protected:
    int                      arbitrationVariable;
};


#endif



