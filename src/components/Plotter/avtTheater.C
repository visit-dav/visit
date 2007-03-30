// ************************************************************************* //
//                                avtTheater.C                               //
// ************************************************************************* //

#include <avtTheater.h>

#include <NoInputException.h>


// ****************************************************************************
//  Method: avtTheater constructor
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
// ****************************************************************************

avtTheater::avtTheater()
{
    actor    = new avtActor;
    behavior = new avtBehavior;
    actor->SetBehavior(behavior);
}


// ****************************************************************************
//  Method: avtTheater::SetInput
//
//  Purpose:
//      Sets the input for the theater.
//
//  Arguments:
//      drawable   The drawable for the actor.
//      info       The information from the data.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 16:10:49 PDT 2002 
//    Added another drawable to the input, for actor's decorations.
//
// ****************************************************************************

void
avtTheater::SetInput(avtDrawable_p drawable, avtDataObjectInformation &info,
                     avtDrawable_p decorations)
{
    behavior->SetInfo(info);

    actor->SetDrawable(drawable);
    actor->SetDecorations(decorations);
}


