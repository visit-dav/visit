// ************************************************************************* //
//                        avtSamplePointArbitrator.C                         //
// ************************************************************************* //

#include <avtSamplePointArbitrator.h>

#include <avtOpacityMap.h>


// ****************************************************************************
//  Method: avtSamplePointArbitrator constructor
//
//  Arguments:
//      om      The opacity map.
//      av      The arbitration variable.
//
//  Programmer: Hank Childs
//  Creation:   January 23, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Jan 29 10:22:56 PST 2005
//    Removed opacity map, which is now in a derived type.
//
// ****************************************************************************

avtSamplePointArbitrator::avtSamplePointArbitrator(int av)
{
    arbitrationVariable = av;
}


// ****************************************************************************
//  Method: avtSamplePointArbitrator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSamplePointArbitrator::~avtSamplePointArbitrator()
{
    ;
}


