// ************************************************************************* //
//                               avtRayFunction.C                            //
// ************************************************************************* //

#include <avtRayFunction.h>

#include <avtGradients.h>
#include <avtLightingModel.h>
#include <avtRay.h>


// ****************************************************************************
//  Method: avtRayFunction constructor
//
//  Arguments:
//      l       The lighting model this object should use for shading.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

avtRayFunction::avtRayFunction(avtLightingModel *l)
{
    lighting = l;
}


// ****************************************************************************
//  Method: avtRayFunction destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRayFunction::~avtRayFunction()
{
    ;
}


// ****************************************************************************
//  Method: avtRayFunction::NeedsGradients
//
//  Purpose:
//      Determines if gradients need to be calculated for this ray function
//      to work.
//
//  Returns:    true if the ray function will need gradients to do compositing,
//              false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
// ****************************************************************************

bool
avtRayFunction::NeedsGradients(void)
{
    return (lighting->NeedsGradients() || NeedsGradientsForFunction());
}


// ****************************************************************************
//  Method: avtRayFunction::CanContributeToPicture
//
//  Purpose:
//      Sampling is one of the costliest portions of doing volume rendering.
//      This is an opportunity to the "cheat" and ask the ray function if it
//      thinks that it will needs this cell to make the final picture.  If the
//      answer is no, the cell is not sampled.
//
//  Arguments:
//      <unused>   The number of vertices for the cell.
//      <unused>   The variables at each vertex.
//
//  Returns:       true if the cell can contribute to the picture, false
//                 otherwise.
//
//  Notes:         Derived types should redefine this if appropriate.
//
//  Programmer:    Hank Childs
//  Creation:      December 7, 2001
//
// ****************************************************************************

bool
avtRayFunction::CanContributeToPicture(int,const float (*)[AVT_VARIABLE_LIMIT])
{
    return true;
}


