// ************************************************************************* //
//                                Navigate2D.C                               //
// ************************************************************************* //

#include <Navigate2D.h>
#include <VisWindowInteractorProxy.h>


// ****************************************************************************
//  Method: Navigate2D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

Navigate2D::Navigate2D(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: Navigate2D::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Navigate2D, this means
//      panning.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
Navigate2D::StartLeftButtonAction()
{
    StartPan();
}


// ****************************************************************************
//  Method: Navigate2D::EndLeftButtonAction
//
//  Purpose:
//      Handles the left button being released.  For Navigate2D, this means
//      panning.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonUp.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
Navigate2D::EndLeftButtonAction()
{
    EndPan();
}


// ****************************************************************************
//  Method: Navigate2D::StartMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being pushed down.  For Navigate2D, this 
//      means zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.  Zoom no longer 
//    implemented by parent class, Dolly performs same function. 
//
// ****************************************************************************

void
Navigate2D::StartMiddleButtonAction()
{
    StartDolly();
}


// ****************************************************************************
//  Method: Navigate2D::EndMiddleButtonAction
//
//  Purpose:
//      Handles the middle button being released.  For Navigate2D, this means
//      zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonUp.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.  Zoom no longer 
//    implemented by parent class, Dolly performs same function. 
//
// ****************************************************************************

void
Navigate2D::EndMiddleButtonAction()
{
    EndDolly();
}


