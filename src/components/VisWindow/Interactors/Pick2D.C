// ************************************************************************* //
//                                  Pick2D.C                                 //
// ************************************************************************* //

#include <Pick2D.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderWindowInteractor.h>


// ****************************************************************************
//  Method: Pick2D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
// ****************************************************************************

Pick2D::Pick2D(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ;
}


// ****************************************************************************
//  Method: Pick2D::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Pick2D, this means
//      performing a pick.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jun 29 16:08:05 PDT 200
//    Added downcast to picker type from abstract picker from the render window
//    to account for VTK changes.
//
//    Hank Childs, Wed Jul  5 16:15:09 PDT 2000
//    Removed picker argument to VisWindow's pick.
//
//    Kathleen Bonnell, Wed Nov 12 10:54:55 PST 2001
//    Uncomment proxy.Pick.
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Removed arguments to match vtk's new interactor api, retrieve them
//    directly from the RenderWindowInteractor.
//
// ****************************************************************************

void
Pick2D::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    //
    // Just call the VisWindow's routine to do this.
    //
    proxy.Pick(x, y);
}


