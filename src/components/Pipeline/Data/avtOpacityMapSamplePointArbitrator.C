// ************************************************************************* //
//                   avtOpacityMapSamplePointArbitrator.C                    //
// ************************************************************************* //

#include <avtOpacityMapSamplePointArbitrator.h>

#include <avtOpacityMap.h>


// ****************************************************************************
//  Method: avtOpacityMapSamplePointArbitrator constructor
//
//  Arguments:
//      om      The opacity map.
//      av      The arbitration variable.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

avtOpacityMapSamplePointArbitrator::avtOpacityMapSamplePointArbitrator(
                                                     avtOpacityMap *om, int av)
   : avtSamplePointArbitrator(av)
{
    omap                = om;
}


// ****************************************************************************
//  Method: avtOpacityMapSamplePointArbitrator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

avtOpacityMapSamplePointArbitrator::~avtOpacityMapSamplePointArbitrator()
{
    ;
}


// ****************************************************************************
//  Method: avtOpacityMapSamplePointArbitrator::ShouldOverwrite
//
//  Purpose:
//      Decides whether the new sample point has a more important contribution
//      to the picture than the old sample point.  "More important contribution"
//      can be loosely defined as higher opacity, since higher opacity creatures
//      are more noticable when they are missing.
//
//  Arguments:
//     whatIgot        The current value.
//     whatIcouldHave  The potential new value.
//
//  Returns:    true if the new value is better than the old.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

bool
avtOpacityMapSamplePointArbitrator::ShouldOverwrite(float whatIgot,
                                                    float whatIcouldHave)
{
    RGBA val1 = omap->GetOpacity(whatIgot);
    RGBA val2 = omap->GetOpacity(whatIcouldHave);
 
    if (val2.A == val1.A)
    {
        //
        // They got mapped to the same opacity.  Return the higher value, so
        // we don't end up with weirdness on the color map.
        //
        return (whatIcouldHave > whatIgot ? true : false);
    }
 
    return (val2.A > val1.A ? true : false);
}


