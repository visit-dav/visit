// ************************************************************************* //
//                                  Pick.C                                   //
// ************************************************************************* //

#include <Pick.h>
#include <VisWindowInteractorProxy.h>
#include <vtkRenderWindowInteractor.h>

using std::queue;

// ****************************************************************************
//  Method: Pick constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 12 14:03:39 PST 2006
//    Initialize picking and handlingCache.
//
// ****************************************************************************

Pick::Pick(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    picking = false;
    handlingCache = false;
}


// ****************************************************************************
//  Method: Pick::StartLeftButtonAction
//
//  Purpose:
//      Handles the left button being pushed down.  For Pick, this means
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
//    Kathleen Bonnell, Thu Jan 12 14:03:39 PST 2006 
//    Added logic to prevent a call to proxy->Pick when the current Pick has
//    not yet completed (can cause hang).  Added logic to cache picks and
//    handle them.
//
//    Kathleen Bonnell, Mon Mar  6 14:44:19 PST 2006 
//    Moved execution of the actual pick to EndLeftButtonAction. As pick
//    can reset InteractionMode, and it isn't good for that to happen 
//    in-between a Start and End action.
//
// ****************************************************************************

void
Pick::StartLeftButtonAction()
{
    int x, y;
    Interactor->GetEventPosition(x, y);
    pickCache.push(x);
    pickCache.push(y);
}


// ****************************************************************************
//  Method: Pick::DoPick
//
//  Purpose:
//    Call's the VisWindow's routine to handle pick. 
//
//  Arguments:
//    x         The x screen position that was picked.
//    y         The y screen position that was picked.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 12, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
Pick::DoPick(int x, int y)
{
    picking = true;
    //
    // Just call the VisWindow's routine to do this.
    //
    proxy.Pick(x, y);
    picking = false;
}


// ****************************************************************************
//  Method: Pick::HandlePickCache
//
//  Purpose:
//    Performs pick for all the x,y values stored in the pick cache.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 12, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
Pick::HandlePickCache()
{
    handlingCache = true;
    while (!pickCache.empty())
    {
        int x, y;
        x = pickCache.front();
        pickCache.pop();
        y = pickCache.front();
        pickCache.pop();
        DoPick(x, y);
    }
    handlingCache = false;
}


// ****************************************************************************
//  Method: Pick::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released, meaning it is time to perform
//    the pick.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 28 10:29:03 PDT 2006
//    Set leftButtonDown to false before handling pick cache.  (Normally set
//    to false by VisitInteractor after EndLeftButtonAction is complete.
//    If pick requires a re-execution, then SetInteractor will be called in the
//    course of the re-execution, and a second pick will be cached, 
//    unnecessarily, unless leftButtonDown is false.
//
// ****************************************************************************

void
Pick::EndLeftButtonAction()
{
    leftButtonDown = false;
    if (!picking && !handlingCache)
        HandlePickCache();
}


