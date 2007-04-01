#include <VisitHotPointInteractor.h>

#include <BadInteractorException.h>
#include <InteractorAttributes.h>
#include <VisWindowInteractorProxy.h>
#include <VisitInteractiveTool.h>
#include <VisWindow.h>
#include <VisWindowTypes.h>
#include <vtkRenderWindowInteractor.h>

#include <Dolly3D.h>
#include <FlyThrough.h>
#include <Lineout2D.h>
#include <Navigate2D.h>
#include <Navigate3D.h>
#include <NavigateCurve.h>
#include <NullInteractor.h>
#include <Pick2D.h>
#include <Pick3D.h>
#include <Zoom2D.h>
#include <Zoom3D.h>
#include <ZoomCurve.h>


// ****************************************************************************
// Method: VisitHotPointInteractor::VisitHotPointInteractor
//
// Purpose: 
//   This is the constructor for the VisitHotPointInteractor class.
//
// Arguments:
//   v : A VisWindowInteractorProxy reference.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:53:27 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Wed May  8 10:34:52 PDT 2002  
//   Added Lineout2D, ZoomCurve.
//   
//   Eric Brugger, Thu Oct 23 15:12:04 PDT 2003
//   Added navigateCurve.
//
//   Hank Childs, Wed Jul 21 08:16:10 PDT 2004
//   Initialize currentInteractor.
//
//   Eric Brugger, Thu Oct 28 15:56:10 PDT 2004
//   Added flyThrough.
//
//   Eric Brugger, Tue Dec 28 07:52:34 PST 2004
//   Added dolly3D.
//
// ****************************************************************************

VisitHotPointInteractor::VisitHotPointInteractor(VisWindowInteractorProxy &v) :
    VisitInteractor(v), currentHotPoint()
{
    dolly3D           = NULL;
    flyThrough        = NULL;
    lineout2D         = NULL;
    navigate2D        = NULL;
    navigate3D        = NULL;
    navigateCurve     = NULL;
    pick2D            = NULL;
    pick3D            = NULL;
    zoom2D            = NULL;
    zoom3D            = NULL;
    zoomCurve         = NULL;
    nullInteractor    = new NullInteractor(proxy);
    currentInteractor = NULL;
    SetInteractor(nullInteractor);

    hotPointMode = false;
}

// ****************************************************************************
// Method: VisitHotPointInteractor::~VisitHotPointInteractor
//
// Purpose: 
//   This is the destructor for the VisitHotPointInteractor class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:54:17 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Wed May  8 10:34:52 PDT 2002  
//   Added lineout2D, zoomCurve.
// 
//   Eric Brugger, Wed Oct 15 17:36:06 PDT 2003
//   Added navigateCurve.
//
//   Eric Brugger, Thu Oct 28 15:56:10 PDT 2004
//   Added flyThrough.
//
//   Eric Brugger, Tue Dec 28 07:52:34 PST 2004
//   Added dolly3D.
//
// ****************************************************************************

VisitHotPointInteractor::~VisitHotPointInteractor()
{
    if(dolly3D != NULL)
    {
        dolly3D->Delete();
        dolly3D = NULL;
    }
    if(flyThrough != NULL)
    {
        flyThrough->Delete();
        flyThrough = NULL;
    }
    if(lineout2D != NULL)
    {
        lineout2D->Delete();
        lineout2D = NULL;
    }
    if(navigate2D != NULL)
    {
        navigate2D->Delete();
        navigate2D = NULL;
    }
    if(navigate3D != NULL)
    {
        navigate3D->Delete();
        navigate3D = NULL;
    }
    if(navigateCurve != NULL)
    {
        navigateCurve->Delete();
        navigateCurve = NULL;
    }
    if(nullInteractor != NULL)
    {
        nullInteractor->Delete();
        nullInteractor = NULL;
    }
    if(pick2D != NULL)
    {
        pick2D->Delete();
        pick2D = NULL;
    }
    if(pick3D != NULL)
    {
        pick3D->Delete();
        pick3D = NULL;
    }
    if(zoom2D != NULL)
    {
        zoom2D->Delete();
        zoom2D = NULL;
    }
    if(zoom3D != NULL)
    {
        zoom3D->Delete();
        zoom3D = NULL;
    }
    if(zoomCurve != NULL)
    {
        zoomCurve->Delete();
        zoomCurve = NULL;
    }
}

// ****************************************************************************
// Method: VisitHotPointInteractor::SetInteractor
//
// Purpose: 
//   This method sets the current interactor that is called when no hot points
//   are dragged.
//
// Arguments:
//   newInteractor : The new default interactor.
//
// Note:       The Interactor pointer for the hot point interactor is shared
//             with the new default interactor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:55:14 PDT 2001
//
// Modifications:
//   Hank Childs, Tue Jul 20 10:54:45 PDT 2004
//   If the current interactor has buttons depressed, allow it to finish
//   gracefully.
//
// ****************************************************************************

void
VisitHotPointInteractor::SetInteractor(VisitInteractor *newInteractor)
{
    if (currentInteractor != NULL)
    {
        if (currentInteractor->LeftButtonIsDown())
            currentInteractor->EndLeftButtonAction();
        if (currentInteractor->RightButtonIsDown())
            currentInteractor->EndRightButtonAction();
        if (currentInteractor->MiddleButtonIsDown())
            currentInteractor->EndMiddleButtonAction();
    }

    newInteractor->SetInteractor(Interactor);
    currentInteractor = newInteractor;
    hotPointMode = false;
}

// ****************************************************************************
// Method: VisitHotPointInteractor::SetNullInteractor
//
// Purpose: 
//   Sets the NullInteractor as the default interactor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:56:43 PDT 2001
//
// ****************************************************************************

void
VisitHotPointInteractor::SetNullInteractor()
{
    SetInteractor(nullInteractor);
}

// ****************************************************************************
// Method: VisitHotPointInteractor::Start2DMode
//
// Purpose: 
//   Starts 2D interaction mode.
//
// Arguments:
//   mode : The new interaction mode.
//
// Note:       This code was adapted from Hanks Interaction colleague.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:57:25 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 14 11:54:54 PST 2001
//   Added LINEOUT.
//   
//   Kathleen Bonnell, Fri Jun 27 16:34:31 PDT 2003  
//   Handle NodePick, ZonePick. 
//   
// ****************************************************************************

void
VisitHotPointInteractor::Start2DMode(INTERACTION_MODE mode)
{
    if(!proxy.HasPlots())
    {
        return;
    }

    VisitInteractor  *newInteractor  = NULL;
    switch(mode)
    {
    case NAVIGATE:
        if(navigate2D == NULL)
        {
            navigate2D = new Navigate2D(proxy);
        }
        newInteractor = navigate2D;
        break;
    case NODE_PICK:
    case ZONE_PICK:
        if(pick2D == NULL)
        {
            pick2D = new Pick2D(proxy);
        }
        newInteractor = pick2D;
        break;
    case ZOOM:
        if(zoom2D == NULL)
        {
            zoom2D = new Zoom2D(proxy);
        }
        newInteractor = zoom2D;
        break;
    case LINEOUT:
        if(lineout2D == NULL)
        {
            lineout2D = new Lineout2D(proxy);
        }
        newInteractor = lineout2D;
        break;
    }

    if(newInteractor == NULL)
    {
        //
        // We have an invalid navigation mode or an invalid window mode.
        //
        EXCEPTION1(BadInteractorException, mode);
    }

    //
    // No reason to set the interactor again if it is the same one.
    //
    if(newInteractor != currentInteractor)
        SetInteractor(newInteractor);
}

// ****************************************************************************
// Method: VisitHotPointInteractor::Start3DMode
//
// Purpose: 
//   Starts 3D interaction mode.
//
// Arguments:
//   mode : The new interaction mode.
//
// Note:       This code was adapted from Hanks Interaction colleague.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:57:25 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Mon Dec 23 11:16:59 PST 2002 
//   Handle LINEOUT mode.  There is no true 3d Lineout mode yet, but 3d
//   lineout queries need to set the interactor, so use Navigate.
//   
//   Kathleen Bonnell, Fri Jun 27 16:34:31 PDT 2003  
//   Handle NodePick, ZonePick. 
//   
//   Eric Brugger, Thu Oct 28 15:56:10 PDT 2004
//   Modified to use the FlyThrough interactor when the navigation mode
//   is Flythrough and we are in navigate mode.
//
//   Eric Brugger, Tue Dec 28 07:52:34 PST 2004
//   Modified to use the Dolly3D interactor when the navigation mode is
//   Dolly and we are in navigate mode.
//
// ****************************************************************************

void
VisitHotPointInteractor::Start3DMode(INTERACTION_MODE mode)
{
    if(!proxy.HasPlots())
    {
        return;
    }

    VisWindow *vw = proxy;
    const InteractorAttributes *atts=vw->GetInteractorAtts();

    VisitInteractor  *newInteractor  = NULL;
    switch(mode)
    {
    case LINEOUT:  // use Navigate until a 3d lineout mode can be implemented.
    case NAVIGATE:
        if (atts->GetNavigationMode() == InteractorAttributes::Trackball)
        {
            if(navigate3D == NULL)
            {
                navigate3D = new Navigate3D(proxy);
            }
            newInteractor = navigate3D;
        }
        else if (atts->GetNavigationMode() == InteractorAttributes::Dolly)
        {
            if(dolly3D == NULL)
            {
                dolly3D = new Dolly3D(proxy);
            }
            newInteractor = dolly3D;
        }
        else
        {
            if(flyThrough == NULL)
            {
                flyThrough = new FlyThrough(proxy);
            }
            newInteractor = flyThrough;
        }
        break;
    case ZONE_PICK: // fall-through
    case NODE_PICK:
        if(pick3D == NULL)
        {
            pick3D = new Pick3D(proxy);
        }
        newInteractor = pick3D;
        break;
    case ZOOM:
        if(zoom3D == NULL)
        {
            zoom3D = new Zoom3D(proxy);
        }
        newInteractor = zoom3D;
        break;
    }

    if(newInteractor == NULL)
    {
        //
        // We have an invalid navigation mode or an invalid window mode.
        //
        EXCEPTION1(BadInteractorException, mode);
    }

    //
    // No reason to set the interactor again if it is the same one.
    //
    if(newInteractor != currentInteractor)
        SetInteractor(newInteractor);
}

// ****************************************************************************
// Method: VisitHotPointInteractor::StartCurveMode
//
// Purpose: 
//   Starts Curve interaction mode.
//
// Arguments:
//   mode : The new interaction mode.
//
// Note:       
//   Only two interaction modes are valid in CurveMode:  Navigate and Zoom.
//
// Programmer: Kathleen Bonnell 
// Creation:   May 8, 2002 
//
// Modifications:
//   Kathleen Bonnell, Fri Jun 27 16:34:31 PDT 2003  
//   Handle NodePick, ZonePick. 
//   
//   Eric Brugger, Wed Oct 15 17:36:06 PDT 2003
//   Change navigate mode to use the NavigateCurve interactor.
//
//   Kathleen Bonnell, Tue Dec  2 17:41:31 PST 2003 
//   Allow Pick interactions (use Pick2d). 
//
// ****************************************************************************

void
VisitHotPointInteractor::StartCurveMode(INTERACTION_MODE mode)
{
    if(!proxy.HasPlots())
    {
        return;
    }

    VisitInteractor  *newInteractor  = NULL;
    switch(mode)
    {
    case NAVIGATE:
        if(navigateCurve == NULL)
        {
            navigateCurve = new NavigateCurve(proxy);
        }
        newInteractor = navigateCurve;
        break;
    case ZOOM:
        if(zoomCurve == NULL)
        {
            zoomCurve = new ZoomCurve(proxy);
        }
        newInteractor = zoomCurve;
        break;
    case ZONE_PICK:
    case NODE_PICK:
        if(pick2D == NULL)
        {
            pick2D = new Pick2D(proxy);
        }
        newInteractor = pick2D;
        break;
    case LINEOUT:
        // These are only here to prevent compiler warning. 
        break;
    }

    if(newInteractor == NULL)
    {
        //
        // We have an invalid navigation mode or an invalid window mode.
        //
        EXCEPTION1(BadInteractorException, mode);
    }

    //
    // No reason to set the interactor again if it is the same one.
    //
    if(newInteractor != currentInteractor)
        SetInteractor(newInteractor);
}

// ****************************************************************************
// Method: VisitHotPointInteractor::Stop2DMode
//
// Purpose: 
//   Ends 2D interaction mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:59:20 PDT 2001
//
// ****************************************************************************

void
VisitHotPointInteractor::Stop2DMode()
{
    SetNullInteractor();
}

// ****************************************************************************
// Method: VisitHotPointInteractor::Stop3DMode
//
// Purpose: 
//   Ends 3D interaction mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 11:59:43 PDT 2001
//
// ****************************************************************************

void
VisitHotPointInteractor::Stop3DMode()
{
    SetNullInteractor();
}

// ****************************************************************************
// Method: VisitHotPointInteractor::StopCurveMode
//
// Purpose: 
//   Ends Curve interaction mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   May 8, 2002 
//
// ****************************************************************************

void
VisitHotPointInteractor::StopCurveMode()
{
    SetNullInteractor();
}

// ****************************************************************************
// Method: VisitHotPointInteractor::StartLeftButtonAction
//
// Purpose: 
//   This method is called when the left mouse button is clicked.
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Note:       We search the list of hot points and if a hot point is found,
//             we let the hot point handle the event. Otherwise, give the
//             event to the interactor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 12:00:05 PDT 2001
//
// Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Brad Whitlock, Mon Sep 23 11:20:04 PDT 2002
//    I moved MotionEnd from the base class to here so it is not called
//    twice for each button down event.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  The values are
//    now accessed directly from the RenderWindowInteractor. 
//    
// ****************************************************************************

void
VisitHotPointInteractor::StartLeftButtonAction()
{
    int x, y, ctrl, shift;
    Interactor->GetEventPosition(x, y);
    ctrl = Interactor->GetControlKey();
    shift = Interactor->GetShiftKey();
    //
    // See if we clicked on a hotpoint.
    //
    hotPointMode = proxy.GetHotPoint(x, y, currentHotPoint);

    //
    // Hide the hotpoint highlights.
    //
    proxy.SetHighlightEnabled(false);

    //
    // If we have a hot point, call its handler.
    //
    if(hotPointMode)
    {
        StartRotate();

        if(currentHotPoint.callback != NULL && currentHotPoint.tool != NULL)
        {
            currentHotPoint.tool->SetLastLocation(x, y);
            (*currentHotPoint.callback)(currentHotPoint.tool,
                                         CB_START, ctrl, shift, x, y);
        }
    }
    else
        currentInteractor->OnLeftButtonDown();

    // Tell the other colleagues that motion is starting.
    MotionBegin();

    // Save the x,y location as the last coordinates.
    lastX = x;
    lastY = y;
}

// ****************************************************************************
// Method: VisitHotPointInteractor::EndLeftButtonAction
//
// Purpose: 
//   This method is called when the left mouse button is released. 
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Note:       This method calls the active hotpoint if there is one.
//             Otherwise, the interactor is called.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:12:52 PST 2001
//
// Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Brad Whitlock, Mon Sep 23 11:20:04 PDT 2002
//    I moved MotionEnd from the base class to here so it is not called
//    twice for each button up event.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  The values are
//    now accessed directly from the RenderWindowInteractor. 
//
// ****************************************************************************

void
VisitHotPointInteractor::EndLeftButtonAction()
{
    int x, y, ctrl, shift;
    Interactor->GetEventPosition(x, y);
    ctrl = Interactor->GetControlKey();
    shift = Interactor->GetShiftKey();

    //
    // Show the hotpoint highlights.
    //
    proxy.SetHighlightEnabled(true);

    //
    // If we have a hot point, call it one more time before clearing it.
    //
    if(hotPointMode)
    {
        EndRotate();

        if(currentHotPoint.callback != NULL && currentHotPoint.tool != NULL)
        {
            (*currentHotPoint.callback)(currentHotPoint.tool, CB_END,
                                         ctrl, shift, x, y);
            currentHotPoint.tool->SetLastLocation(-1, -1);
        }

        hotPointMode = false;
    }
    else
    {
        currentInteractor->OnLeftButtonUp();
    }

    // Tell the other colleagues that motion is ending.
    MotionEnd();

    // Clear the last coordinates
    lastX = lastY = -1;
}

// ****************************************************************************
// Method: VisitHotPointInteractor::AbortLeftButtonAction
//
// Purpose:
//   This method is called when inconsistent button clicks are received.
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Programmer: Hank Childs
// Creation:   March 18, 2002
//
// Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  
//
// ****************************************************************************

void
VisitHotPointInteractor::AbortLeftButtonAction()
{
    if (hotPointMode)
    {
        VisitInteractor::AbortLeftButtonAction();
    }
    else
    {
        currentInteractor->AbortLeftButtonAction();
    }
}

// ****************************************************************************
// Method: VisitHotPointInteractor::StartMiddleButtonAction
//
// Purpose: 
//   This method is called when the middle mouse button is pressed.
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Note:       The event is passed to the current interactor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:16:12 PST 2001
//
// Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonDown.
//
//    Brad Whitlock, Mon Sep 23 11:20:04 PDT 2002
//    I moved MotionEnd from the base class to here so it is not called
//    twice for each button down event.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  
//
// ****************************************************************************

void
VisitHotPointInteractor::StartMiddleButtonAction()
{
    //
    // Hide the hotpoint highlights.
    //
    proxy.SetHighlightEnabled(false);

    currentInteractor->OnMiddleButtonDown();

    // Tell the other colleagues that motion is starting.
    MotionBegin();

    // Save the x,y location as the last coordinates.
    Interactor->GetEventPosition(lastX, lastY);
}

// ****************************************************************************
// Method: VisitHotPointInteractor::EndMiddleButtonAction
//
// Purpose: 
//   This method is called when the middle mouse button is released.
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Note:       The event is passed to the current interactor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:16:12 PST 2001
//
// Modifications:
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonUp.
//
//    Brad Whitlock, Mon Sep 23 11:20:04 PDT 2002
//    I moved MotionEnd from the base class to here so it is not called
//    twice for each button up event.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  
//
// ****************************************************************************

void
VisitHotPointInteractor::EndMiddleButtonAction()
{
    //
    // Show the hotpoint highlights.
    //
    proxy.SetHighlightEnabled(true);

    currentInteractor->OnMiddleButtonUp();

    // Tell the other colleagues that motion is ending.
    MotionEnd();

    // Clear the last coordinates
    lastX = lastY = -1;
}

// ****************************************************************************
// Method: VisitHotPointInteractor::AbortMiddleButtonAction
//
// Purpose:
//   This method is called when inconsistent button clicks are received.
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Programmer: Hank Childs
// Creation:   March 18, 2002
//
// Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  
//
// ****************************************************************************

void
VisitHotPointInteractor::AbortMiddleButtonAction()
{
    if (hotPointMode)
    {
        VisitInteractor::AbortMiddleButtonAction();
    }
    else
    {
        currentInteractor->AbortMiddleButtonAction();
    }
}

// ****************************************************************************
// Method: VisitHotPointInteractor::OnMouseMove
//
// Purpose: 
//   This method is called when the mouse is moved.
//
// Arguments:
//   ctrl  : The state of the Ctrl key.
//   shift : The state of the Shift key.
//   x     : The x location of the pointer.
//   y     : The y location of the mouse.
//
// Note:       If there is a current hotpoint, it gets the event. Otherwise
//             the event goes to the current interactor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:17:26 PST 2001
//
// Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Removed arguments to match vtk's new interactor api.  They are now
//    accessed directly from the RenderWindowInteractor.
//   
// ****************************************************************************

void
VisitHotPointInteractor::OnMouseMove()
{
    int x, y, ctrl, shift;
    Interactor->GetEventPosition(x, y);
    if(lastX != x || lastY != y)
    {
        //
        // Call the hot point if we have one.
        //
        if(hotPointMode)
        {
            ctrl = Interactor->GetControlKey();
            shift = Interactor->GetShiftKey();
            PrepTrackball();
            if(currentHotPoint.callback != NULL && currentHotPoint.tool != NULL)
            {
                (*currentHotPoint.callback)(currentHotPoint.tool,
                                         CB_MIDDLE, ctrl, shift, x, y);
                currentHotPoint.tool->SetLastLocation(x, y);
            }
        }
        else
            currentInteractor->OnMouseMove();

        // Save the x,y location as the last coordinates.
        lastX = x;
        lastY = y;  
    }
}

// ****************************************************************************
// Method: VisitHotPointInteractor::OnTimer
//
// Purpose: 
//   This method is called with a timer.
//
// Note:       If there is no current hotpoint, the current interactor's
//             OnTimer method is called.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:18:22 PST 2001
//
// ****************************************************************************

void
VisitHotPointInteractor::OnTimer()
{
    // If we have no current hotpoint, call the interactor's OnTimer method.
    if(!hotPointMode)
    {
        currentInteractor->OnTimer();
    }
}

// ****************************************************************************
//  Method: VisitHotPointInteractor::SetInteractor
//
//  Purpose:
//      Associate this 'style' with the passed render window interactor. 
//
//  Arguments:
//      rwi     The render window interactor.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   November 25, 2002 
//
// ****************************************************************************

void
VisitHotPointInteractor::SetInteractor(vtkRenderWindowInteractor *rwi)
{
    //
    //  We want this class to be the 'controller', so it should observe the
    //  actions taken by the RenderWindowInteractor.  The base class adds 
    //  the observers, so call it.
    //
    vtkInteractorStyle::SetInteractor(rwi);
}
