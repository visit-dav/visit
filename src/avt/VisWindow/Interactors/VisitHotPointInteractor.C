// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <NavigateAxisArray.h>
#include <NullInteractor.h>
#include <Pick.h>
#include <Zoom2D.h>
#include <Zoom3D.h>
#include <ZoomAxisArray.h>
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
//   Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//   Replaced Pick3D and Pick2D with Pick.
//
//   Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//   Added navigate interactor for new AxisArray window mode.
//
//   Jeremy Meredith, Mon Feb  4 13:24:08 EST 2008
//   Added zoom interactor for AxisArray mode.
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
    navigateAxisArray = NULL;
    pick              = NULL;
    zoom2D            = NULL;
    zoom3D            = NULL;
    zoomCurve         = NULL;
    zoomAxisArray     = NULL;
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
//   Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//   Replaced Pick3D and Pick2D with Pick.
//
//   Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//   Added navigate interactor for new AxisArray window mode.
//
//   Jeremy Meredith, Mon Feb  4 13:24:08 EST 2008
//   Added zoom interactor for AxisArray mode.
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
    if(navigateAxisArray != NULL)
    {
        navigateAxisArray->Delete();
        navigateAxisArray = NULL;
    }
    if(nullInteractor != NULL)
    {
        nullInteractor->Delete();
        nullInteractor = NULL;
    }
    if(pick != NULL)
    {
        pick->Delete();
        pick = NULL;
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
    if(zoomAxisArray != NULL)
    {
        zoomAxisArray->Delete();
        zoomAxisArray = NULL;
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
//   Jeremy Meredith, Fri Oct 14 12:17:16 PDT 2005
//   Make sure if the mouse button is still down for an interactor getting
//   re-applied, restart the appropriate button actions.  ('6643)
//   Don't set hotPointMode to false.  It's not cleanly ending the hot point
//   mode, and more importantly, we don't need to end it just because the
//   interactor changes out from underneath us.  ('6715)
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

    if (newInteractor != NULL) 
    {
        if (newInteractor->LeftButtonIsDown())
            newInteractor->StartLeftButtonAction();
        if (newInteractor->RightButtonIsDown())
            newInteractor->StartRightButtonAction();
        if (newInteractor->MiddleButtonIsDown())
            newInteractor->StartMiddleButtonAction();
    }

    currentInteractor = newInteractor;
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
//   Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//   Replaced Pick2D with Pick.
//
//   Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//   Added SPREADSHEET_PICK 
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
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
    case SPREADSHEET_PICK:
    case DDT_PICK:
        if(pick == NULL)
        {
            pick = new Pick(proxy);
        }
        newInteractor = pick;
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
//   Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//   Replaced Pick3D with Pick.
//
//   Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//   Added SPREADSHEET_PICK 
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
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
    case SPREADSHEET_PICK:
    case DDT_PICK:
        if(pick == NULL)
        {
            pick = new Pick(proxy);
        }
        newInteractor = pick;
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
//   Brad Whitlock, Wed Apr 27 17:20:09 PST 2005
//   Make it use a navigate interactor for lineout mode so we at least have
//   an interactor in the rare case that we enter lineout mode here.
//
//   Kathleen Bonnell, Thu Jan 12 13:58:25 PST 2006 
//   Replaced Pick3D and Pick2D with Pick.
//
//   Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//   Added SPREADSHEET_PICK 
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
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
    case LINEOUT:
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
    case SPREADSHEET_PICK:
    case DDT_PICK:
        if(pick == NULL)
        {
            pick = new Pick(proxy);
        }
        newInteractor = pick;
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
//  Method:  VisitHotPointInteractor::StartAxisArrayMode
//
//  Purpose:
//    Sets up the interactors for axis array window mode.
//
//  Arguments:
//    mode       the interaction mode
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2008
//
//  Modifications:
//   Jeremy Meredith, Mon Feb  4 13:24:08 EST 2008
//   Added zoom interactor for AxisArray mode.
//
//   Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//   Added SPREADSHEET_PICK 
//
//   Eric Brugger, Tue Dec  9 16:32:45 PST 2008
//   Added code to set the axis orientation for the navigateAxisArray.
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
//   Eric Brugger, Mon Nov  5 15:48:46 PST 2012
//   I added the ability to display the parallel axes either horizontally
//   or vertically.
//
// ****************************************************************************
void
VisitHotPointInteractor::StartAxisArrayMode(INTERACTION_MODE mode)
{
    if (!proxy.HasPlots())
    {
        return;
    }

    VisitInteractor  *newInteractor  = NULL;
    switch(mode)
    {
      case LINEOUT:
      // We don't have a lineout or zoom interaction.
      // Fall through to navigation mode.
      case NAVIGATE:
        if (navigateAxisArray == NULL)
        {
            navigateAxisArray = new NavigateAxisArray(proxy);
        }
        navigateAxisArray->SetAxisOrientation(NavigateAxisArray::Vertical);
        navigateAxisArray->SetDomainOrientation(NavigateAxisArray::Horizontal);
        newInteractor = navigateAxisArray;
        break;
      case ZOOM:
        if(zoomAxisArray == NULL)
        {
            zoomAxisArray = new ZoomAxisArray(proxy);
        }
        newInteractor = zoomAxisArray;
        break;
      case ZONE_PICK:
      case NODE_PICK:
      case SPREADSHEET_PICK:
      case DDT_PICK:
        if (pick == NULL)
        {
            pick = new Pick(proxy);
        }
        newInteractor = pick;
        break;
    }

    if (newInteractor == NULL)
    {
        //
        // We have an invalid navigation mode or an invalid window mode.
        //
        EXCEPTION1(BadInteractorException, mode);
    }

    //
    // No reason to set the interactor again if it is the same one.
    //
    if (newInteractor != currentInteractor)
        SetInteractor(newInteractor);
}

// ****************************************************************************
//  Method:  VisitHotPointInteractor::StartParallelAxesMode
//
//  Purpose:
//    Sets up the interactors for ParallelAxes window mode.
//
//  Arguments:
//    mode       the interaction mode
//
//  Programmer:  Eric Brugger
//  Creation:    December 9, 2008
//
//  Modifications:
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
//   Eric Brugger, Mon Nov  5 15:48:46 PST 2012
//   I added the ability to display the parallel axes either horizontally
//   or vertically.
//
// ****************************************************************************
void
VisitHotPointInteractor::StartParallelAxesMode(INTERACTION_MODE mode)
{
    if (!proxy.HasPlots())
    {
        return;
    }

    VisWindow *vw = proxy;
    VisitInteractor  *newInteractor  = NULL;
    switch(mode)
    {
      case LINEOUT:
      // We don't have a lineout or zoom interaction.
      // Fall through to navigation mode.
      case NAVIGATE:
        if (navigateAxisArray == NULL)
        {
            navigateAxisArray = new NavigateAxisArray(proxy);
        }
        if (vw->GetWindowMode() == WINMODE_PARALLELAXES)
        {
            navigateAxisArray->SetAxisOrientation(NavigateAxisArray::Horizontal);
            navigateAxisArray->SetDomainOrientation(NavigateAxisArray::Horizontal);
        }
        else
        {
            navigateAxisArray->SetAxisOrientation(NavigateAxisArray::Vertical);
            navigateAxisArray->SetDomainOrientation(NavigateAxisArray::Vertical);
        }
        newInteractor = navigateAxisArray;
        break;
      case ZOOM:
        if(zoomAxisArray == NULL)
        {
            zoomAxisArray = new ZoomAxisArray(proxy);
        }
        newInteractor = zoomAxisArray;
        break;
      case ZONE_PICK:
      case NODE_PICK:
      case DDT_PICK:
        if (pick == NULL)
        {
            pick = new Pick(proxy);
        }
        newInteractor = pick;
        break;
      default:
        newInteractor = NULL;
        break;
    }

    if (newInteractor == NULL)
    {
        //
        // We have an invalid navigation mode or an invalid window mode.
        //
        EXCEPTION1(BadInteractorException, mode);
    }

    //
    // No reason to set the interactor again if it is the same one.
    //
    if (newInteractor != currentInteractor)
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
// Method: VisitHotPointInteractor::StopAxisArrayMode
//
// Purpose: 
//   Ends AxisArray interaction mode.
//
// Programmer: Jeremy Meredith
// Creation:   January 29, 2008
//
// ****************************************************************************

void
VisitHotPointInteractor::StopAxisArrayMode()
{
    SetNullInteractor();
}

// ****************************************************************************
// Method: VisitHotPointInteractor::StopParallelAxesMode
//
// Purpose: 
//   Ends ParallelAxes interaction mode.
//
// Programmer: Eric Brugger
// Creation:   December 9, 2008
//
// ****************************************************************************

void
VisitHotPointInteractor::StopParallelAxesMode()
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
//    Jeremy Meredith, Fri Feb  1 18:01:15 EST 2008
//    Added new data value to hotpoints that is passed to callback.
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
                                        CB_START, ctrl, shift, x, y,
                                        currentHotPoint.data);
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
//    Jeremy Meredith, Fri Feb  1 18:01:15 EST 2008
//    Added new data value to hotpoints that is passed to callback.
//
//    Brad Whitlock, Wed Jul 14 14:08:27 PDT 2010
//    I added a render after the tool has executed so its extra actors are
//    guaranteed to be removed from the window.
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
                                        ctrl, shift, x, y,
                                        currentHotPoint.data);
            currentHotPoint.tool->SetLastLocation(-1, -1);

            proxy.Render();
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
//    Eric Brugger, Mon Aug 15 13:32:17 PDT 2005
//    Modified the routine to no longer call AbortLeftButtonAction for
//    the current interactor if it isn't in hot point mode.  The previous
//    behavior caused AbortLeftButtonAction to be erroneously called
//    twice, once for the hot point interactor and once for the current
//    interactor.
//
// ****************************************************************************

void
VisitHotPointInteractor::AbortLeftButtonAction()
{
    if (hotPointMode)
    {
        VisitInteractor::AbortLeftButtonAction();
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
//    Eric Brugger, Mon Aug 15 13:32:17 PDT 2005
//    Modified the routine to no longer call AbortMiddleButtonAction for
//    the current interactor if it isn't in hot point mode.  The previous
//    behavior caused AbortMiddleButtonAction to be erroneously called
//    twice, once for the hot point interactor and once for the current
//    interactor.
//
// ****************************************************************************

void
VisitHotPointInteractor::AbortMiddleButtonAction()
{
    if (hotPointMode)
    {
        VisitInteractor::AbortMiddleButtonAction();
    }
}

// ****************************************************************************
// Method: VisitHotPointInteractor::StartRightButtonAction
//
// Purpose: 
//   This method is called when the right mouse button is pressed.
//
// Programmer: Eric Brugger
// Creation:   June 24, 2013
//
// ****************************************************************************

void
VisitHotPointInteractor::StartRightButtonAction()
{
    currentInteractor->OnRightButtonDown();
}

// ****************************************************************************
// Method: VisitHotPointInteractor::EndRightButtonAction
//
// Purpose: 
//   This method is called when the right mouse button is released.
//
// Programmer: Eric Brugger
// Creation:   June 24, 2013
//
// ****************************************************************************

void
VisitHotPointInteractor::EndRightButtonAction()
{
    currentInteractor->EndRightButtonAction();
}

// ****************************************************************************
// Method: VisitHotPointInteractor::AbortRightButtonAction
//
// Purpose:
//   This method is called when inconsistent button clicks are received.
//
// Programmer: Eric Brugger
// Creation:   June 24, 2013
//
// ****************************************************************************

void
VisitHotPointInteractor::AbortRightButtonAction()
{
    currentInteractor->AbortRightButtonAction();
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
//    Jeremy Meredith, Fri Feb  1 18:01:15 EST 2008
//    Added new data value to hotpoints that is passed to callback.
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
                                            CB_MIDDLE, ctrl, shift, x, y,
                                            currentHotPoint.data);
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
//  Method: VisitHotPointInteractor::OnMouseWheelForward()
//
//  Purpose:
//    Handles the mouse wheel turned backward.
//
//  Arguments:
//
//  Programmer: Gunther H. Weber
//  Creation:   August 07, 2007
//
//  Modifications:
//    Eric Brugger, Mon Jun 24 14:33:29 PDT 2013
//    I modified the interactor to ignore wheel movement while any of the
//    mouse buttons are pressed to avoid strange behavior in some situations.
//
//    Kathleen Biagas, Thu Mar 17, 2022
//    Use currentInteractor to check button status.
//
// ****************************************************************************

void
VisitHotPointInteractor::OnMouseWheelForward()
{
    if (!currentInteractor->LeftButtonIsDown() && 
        !currentInteractor->MiddleButtonIsDown() && 
        !currentInteractor->RightButtonIsDown())
    {
        currentInteractor->OnMouseWheelForward();
    }
}


// ****************************************************************************
//  Method: VisitHotPointInteractor::OnMouseWheelBackward()
//
//  Purpose:
//    Handles the mouse wheel turned forward.  
//
//  Arguments:
//
//  Programmer: Gunther H. Weber
//  Creation:   August 07, 2007
//
//  Modifications:
//    Eric Brugger, Mon Jun 24 14:33:29 PDT 2013
//    I modified the interactor to ignore wheel movement while any of the
//    mouse buttons are pressed to avoid strange behavior in some situations.
//
//    Kathleen Biagas, Thu Mar 17, 2022
//    Use currentInteractor to check button status.
//
// ****************************************************************************

void
VisitHotPointInteractor::OnMouseWheelBackward()
{
    if (!currentInteractor->LeftButtonIsDown() && 
        !currentInteractor->MiddleButtonIsDown() && 
        !currentInteractor->RightButtonIsDown())
    {
        currentInteractor->OnMouseWheelBackward();
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
