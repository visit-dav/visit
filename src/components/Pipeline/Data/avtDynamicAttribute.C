// ************************************************************************* //
//                           avtDynamicAttribute.C                           //
// ************************************************************************* //

#include <avtDynamicAttribute.h>


// ****************************************************************************
//  Method: avtDynamicAttribute::PerformRestriction
//
//  Purpose:
//      This gives derived types an opportunity to affect what is going to be
//      coming down in the data object.  They can add additional variable to
//      be read in, for example.
//      
//  Arguments:
//      spec    The current pipeline specification.
//
//  Returns:    The appropriate pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

avtPipelineSpecification_p
avtDynamicAttribute::PerformRestriction(avtPipelineSpecification_p spec)
{
    return spec;
}


