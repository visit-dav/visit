// ************************************************************************* //
//                            NullInteractor.C                               //
// ************************************************************************* //

#include <NullInteractor.h>
#include <VisWindowInteractorProxy.h>


// ****************************************************************************
//  Method: NullInteractor constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

NullInteractor::NullInteractor(VisWindowInteractorProxy &v) 
    : VisitInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: NullInteractor::OnMouseMove
//
//  Purpose:
//      Handles the mouse movement.  For NullInteractor, this means do nothing.
//
//  Arguments:
//      ctrl    Non-zero if the ctrl button is being held down.
//      shift   Non-zero if the shift button is being held down.
//      x       The x coordinate of the mouse in display coordinates.
//      y       The y coordinate of the mouse in display coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
NullInteractor::OnMouseMove()
{
    PrepTrackball();
}


