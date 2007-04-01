// ************************************************************************* //
//                   avtRelativeValueSamplePointArbitrator.C                 //
// ************************************************************************* //

#include <avtRelativeValueSamplePointArbitrator.h>


// ****************************************************************************
//  Method: avtRelativeValueSamplePointArbitrator constructor
//
//  Arguments:
//      om      The opacity map.
//      av      The arbitration variable.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

avtRelativeValueSamplePointArbitrator::avtRelativeValueSamplePointArbitrator(
                                                     bool lt, int av)
   : avtSamplePointArbitrator(av)
{
    lessThan = lt;
}


// ****************************************************************************
//  Method: avtRelativeValueSamplePointArbitrator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

avtRelativeValueSamplePointArbitrator::~avtRelativeValueSamplePointArbitrator()
{
    ;
}


// ****************************************************************************
//  Method: avtRelativeValueSamplePointArbitrator::ShouldOverwrite
//
//  Purpose:
//      Decides whether the new sample point has a more important contribution
//      to the picture than the old sample point.  In this case, we define
//      important based on relative value.
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
avtRelativeValueSamplePointArbitrator::ShouldOverwrite(float whatIgot,
                                                       float whatIcouldHave)
{
    if (lessThan)
        return (whatIgot < whatIcouldHave);
    else
        return (whatIgot > whatIcouldHave);
}


