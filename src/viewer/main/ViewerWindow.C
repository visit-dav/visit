// ************************************************************************* //
//                               ViewerWindow.C                              //
// ************************************************************************* //

#include <ViewerWindow.h> 

#include <float.h>
#include <math.h> 
#include <stdio.h> 
#include <snprintf.h>
#include <string>
#include <map>
using std::string;

#include <avtColorTables.h>
#include <avtDataObjectReader.h>
#include <avtWholeImageCompositer.h>
#include <avtToolInterface.h>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h> 
#include <AttributeSubjectMap.h>
#include <DataNode.h>
#include <LightList.h> 
#include <Line.h>
#include <LineoutInfo.h>
#include <PickAttributes.h>
#include <PickPointInfo.h>
#include <ViewerActionManager.h>
#include <ViewerAnimation.h>
#include <ViewerMessaging.h>
#include <ViewerPlotList.h> 
#include <ViewerPopupMenu.h>
#include <ViewerQueryManager.h>
#include <ViewerToolbar.h>
#include <ViewerSubject.h>
#include <ViewerWindowManager.h> 
#include <VisWindow.h>

#include <DebugStream.h>

//
// Definition of static variables associated with ViewerWindow.
//
bool    ViewerWindow::doNoWinMode = false;

//
// Local macros.
//
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

//
// Global variables.  These should be removed.
//
extern ViewerSubject  *viewerSubject;


// ****************************************************************************
//  Method: ViewerWindow::ViewerWindow
//
//  Purpose: 
//    Constructor for the ViewerWindow class.
//
//  Arguments:
//    windowIndex  The index to associate with the window.  This value is
//                 saved in the popup menu.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:17:17 PDT 2000
//
//  Modifications:
//    Hank Childs, Tue Feb 20 00:00:03 PST 2001
//    Better initialize the view by using SetToDefault.
//
//    Eric Brugger, Tue Feb 20 16:25:43 PST 2001
//    Pass this to AddWindow since it now wants a ViewerWindow instead of a
//    VisWindow.
//
//    Eric Brugger, Tue Apr 24 16:50:32 PDT 2001
//    I modified the class to store seperate 2d and 3d view information.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    I added initialization of annotationAttributes.
//
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
//    Brad Whitlock, Wed Aug 22 11:56:06 PDT 2001
//    I added code to set the close callback in the vis window.
//
//    Brad Whitlock, Thu Aug 30 08:49:19 PDT 2001
//    I removed the annotation attributes and modified the method to set the
//    background/foreground colors for the VisWindow.
//
//    Brad Whitlock, Fri Oct 12 11:01:53 PDT 2001
//    Passed the viswindow pointer to the popup menu.
//
//    Brad Whitlock, Fri Sep 14 15:38:55 PST 2001
//    I added code to set the lights in the new VisWindow.
//
//    Eric Brugger, Wed Nov 21 13:41:28 PST 2001
//    I added code to set the animation attributes in the new VisWindow.
//
//    Kathleen Bonnell, Thu Nov 22 16:03:00 PST 2001 
//    I added code to set the pick callback in the vis window.
//
//    Eric Brugger, Tue Mar 26 16:02:18 PST 2002
//    I increased the size of the default 2d viewport.
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//    Support lineout and avtViewCurve. 
//
//    Kathleen Bonnell, Fri Jul 12 17:28:31 PDT 2002 
//    I increased the size of the default Curve viewport.
//
//    Hank Childs, Mon Jul 15 15:49:50 PDT 2002
//    Initialized plotExtentsType.
//
//    Kathleen Bonell, Tue Aug 13 15:15:37 PDT 2002 
//    Use LightList directly instead of converting to avtLightList. 
//
//    Brad Whitlock, Fri Sep 20 16:32:55 PST 2002
//    I added the render information callback.
//
//    Brad Whitlock, Mon Nov 11 11:18:20 PDT 2002
//    I added support for locking tools.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Removed call to SetPickAttributes. 
//
//    Eric Brugger, Mon Jan  6 14:52:33 PST 2003
//    I added support for keyframing views.
//
//    Brad Whitlock, Wed Jan 29 11:24:24 PDT 2003
//    I added toolbar support.
//
//    Brad Whitlock, Wed Mar 12 09:50:13 PDT 2003
//    I added some callbacks for showing and hiding the window.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I replaced autoCenter with maintainView.  I added viewModified2d and
//    mergeViewLimits.
//
//    Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003  
//    Intialize fullFrame. 
//
// ****************************************************************************

ViewerWindow::ViewerWindow(int windowIndex)
{
    visWindow = new VisWindow(doNoWinMode);
    visWindow->SetCloseCallback(CloseCallback, (void *)this);
    visWindow->SetHideCallback(HideCallback, (void *)this);
    visWindow->SetShowCallback(ShowCallback, (void *)this);
    visWindow->SetExternalRenderCallback(ExternalRenderCallback, (void*)this);
    SetAnnotationAttributes(ViewerWindowManager::GetAnnotationDefaultAtts());
    SetLightList(ViewerWindowManager::GetLightListDefaultAtts());

    // Set some default values.
    cameraView = false;
    maintainView = false;
    viewIsLocked = false;
    viewDimension = 2;
    boundingBoxValid2d = false;
    haveRenderedIn2d = false;
    viewModified2d = false;
    boundingBoxValid3d = false;
    haveRenderedIn3d = false;
    mergeViewLimits = false;
    plotExtentsType = AVT_ORIGINAL_EXTENTS;
    timeLocked = false;
    toolsLocked = false;
    windowId = windowIndex;
    isVisible = false;

    // Create the popup menu and the toolbar.
    popupMenu = new ViewerPopupMenu(this);
    toolbar = new ViewerToolbar(this);

    animation = new ViewerAnimation;
    animation->AddWindow(this);
    SetAnimationAttributes(ViewerWindowManager::GetAnimationClientAtts());

    //
    // Callbacks to show the menu when the right mouse button is
    // clicked in the VisWindow.
    //
    visWindow->SetShowMenu(ShowMenuCallback, this);
    visWindow->SetHideMenu(HideMenuCallback, this);

    //
    // Callback for render information.
    //
    visWindow->SetRenderInfoCallback(ViewerWindowManager::RenderInformationCallback,
        (void *)windowIndex);

    //
    // Callback for pick.
    //
    PICK_POINT_INFO *pdata = new PICK_POINT_INFO;
    pdata->callbackData = this;
    visWindow->SetPickCB(ViewerWindow::PerformPickCallback, pdata);

    //
    // Callback for lineout.
    //
    LINE_OUT_INFO *ldata = new LINE_OUT_INFO;
    ldata->callbackData = this;
    visWindow->SetLineoutCB(ViewerWindow::PerformLineoutCallback, ldata);

    //
    // Initialize the view information.
    //
    avtView2D view2D;

    view2D.viewport[0] = 0.2;
    view2D.viewport[1] = 0.95;
    view2D.viewport[2] = 0.15;
    view2D.viewport[3] = 0.95;
    visWindow->SetView2D(view2D);

    //
    // Initialize the curve view information.
    //
    avtViewCurve viewCurve;

    viewCurve.viewport[0] = 0.2;
    viewCurve.viewport[1] = 0.95;
    viewCurve.viewport[2] = 0.15;
    viewCurve.viewport[3] = 0.95;
    visWindow->SetViewCurve(viewCurve);

    curView = new ViewAttributes;
    view2DAtts = new AttributeSubjectMap;
    view3DAtts = new AttributeSubjectMap;

    fullFrame = false;
    //
    // Create the window's action manager.
    //
    actionMgr = new ViewerActionManager(this);

}

// ****************************************************************************
//  Method: ViewerWindow::~ViewerWindow
//
//  Purpose: 
//    Destructor for the ViewerWindow class.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:18:15 PDT 2000
//
//  Modifications:
//    Eric Brugger, Mon Aug 13 16:55:28 PDT 2001
//    Reverse order of the deletions to eliminate a crash deleting a window.
//
//    Eric Brugger, Mon Jan  6 14:52:33 PST 2003
//    I added support for keyframing views.
//
//    Brad Whitlock, Thu Feb 6 07:26:01 PDT 2003
//    I added the action manager.
//
//    Kathleen Bonnell, Tue Feb 11 12:55:49 PST 2003  
//    Added toolbar. 
//
// ****************************************************************************

ViewerWindow::~ViewerWindow()
{
    delete animation;
    delete popupMenu;
    delete visWindow;
    delete toolbar;

    delete curView;
    delete view2DAtts;
    delete view3DAtts;
    delete actionMgr;
}

// ****************************************************************************
//  Method: ViewerWindow::SetNoWinMode
//
//  Purpose:
//      Controls whether or not VisWindows should be created in nowin mode.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
// ****************************************************************************

void
ViewerWindow::SetNoWinMode(bool newMode)
{
    doNoWinMode = newMode;
}

// ****************************************************************************
// Method: ViewerWindow::GetNoWinMode
//
// Purpose: 
//   Returns the nowin mode.
//
// Returns:    The nowin mode.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 6 08:07:17 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetNoWinMode()
{
    return doNoWinMode;
}

// ****************************************************************************
//  Method: ViewerWindow::GetPopupMenu
//
//  Purpose: 
//    Return a pointer to the window's popup menu.
//
//  Returns:    A pointer to the popup menu.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:19:01 PDT 2000
//
// ****************************************************************************

ViewerPopupMenu *
ViewerWindow::GetPopupMenu() const
{
    return popupMenu;
}

// ****************************************************************************
//  Method: ViewerWindow::GetToolbar
//
//  Purpose: 
//    Return a pointer to the window's toolbar.
//
//  Returns:    A pointer to the toolbar.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 29 11:26:04 PDT 2003
//
// ****************************************************************************

ViewerToolbar *
ViewerWindow::GetToolbar() const
{
    return toolbar;
}

// ****************************************************************************
//  Method: ViewerWindow::GetAnimation
//
//  Purpose: 
//    Return a pointer to the window's animation.
//
//  Returns:    A pointer to the animation.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:19:01 PDT 2000
//
// ****************************************************************************

ViewerAnimation *
ViewerWindow::GetAnimation() const
{
    return animation;
}

// ****************************************************************************
// Method: ViewerWindow::GetActionManager
//
// Purpose: 
//   Returns a pointer to the window's action manager.
//
// Returns:    A pointer to the window's action manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 12:10:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionManager *
ViewerWindow::GetActionManager() const
{
    return actionMgr;
}

// ****************************************************************************
// Method: ViewerWindow::GetWindowId
//
// Purpose: 
//   Returns this window's index within the viewer's window manager.
//
// Returns:    This window's index within the viewer window manager.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 11:27:32 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
ViewerWindow::GetWindowId() const
{
    return windowId;
}

// ****************************************************************************
//  Method: ViewerWindow::SetSize
//
//  Purpose: 
//    Set the size of the window.
//
//  Arguments:
//    width     The width of the window in pixels.
//    height    The height of the window in pixels.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:19:23 PDT 2000
//   
// ****************************************************************************

void
ViewerWindow::SetSize(const int width, const int height)
{
    visWindow->SetSize(width, height);
}

// ****************************************************************************
// Method: ViewerWindow::GetSize
//
// Purpose: 
//   Returns the size of the renderable portion of the window.
//
// Arguments:
//   width  : A reference to an int that is used to return the width.
//   height : A reference to an int that is used to return the height.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 11:04:35 PDT 2001
//   
// ****************************************************************************

void
ViewerWindow::GetSize(int &width, int &height)
{
    visWindow->GetSize(width, height);
}

// ****************************************************************************
// Method: ViewerWindow::GetWindowSize
//
// Purpose: 
//   Returns the size of the window.
//
// Arguments:
//   width  : A reference to an int that is used to return the width.
//   height : A reference to an int that is used to return the height.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 11:04:35 PDT 2001
//   
// ****************************************************************************

void
ViewerWindow::GetWindowSize(int &width, int &height)
{
    visWindow->GetWindowSize(width, height);
}

// ****************************************************************************
//  Method: ViewerWindow::SetLocation
//
//  Purpose: 
//    Set the location of the window.
//
//  Arguments:
//    x         The x coordinate of the upper left corner.
//    y         The y coordinate of the upper left corner.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:20:04 PDT 2000
//   
// ****************************************************************************

void
ViewerWindow::SetLocation(const int x, const int y)
{
    visWindow->SetLocation(x, y);
}

// ****************************************************************************
// Method: ViewerWindow::GetLocation
//
// Purpose: 
//   Returns the location of the window.
//
// Arguments:
//   x : A reference to an int that is used to return the x location.
//   y : A reference to an int that is used to return the y location.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 11:04:35 PDT 2001
//   
// ****************************************************************************

void
ViewerWindow::GetLocation(int &x, int &y)
{
    visWindow->GetLocation(x, y);
}

// ****************************************************************************
//  Method: ViewerWindow::SetTitle
//
//  Purpose: 
//    Set the title of the window.
//
//  Arguments:
//    title     The title of the window.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:20:58 PDT 2000
//   
// ****************************************************************************

void
ViewerWindow::SetTitle(const char *title)
{
    visWindow->SetTitle(title);
}

// ****************************************************************************
//  Method: ViewerWindow::Realize
//
//  Purpose: 
//    Force the window to be created and shown.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:21:43 PDT 2000
//   
// ****************************************************************************

void
ViewerWindow::Realize()
{
    visWindow->Realize();
}

// ****************************************************************************
//  Method: ViewerWindow::SetInterationMode
//
//  Purpose: 
//    Set the interaction mode of the window.
//
//  Arguments:
//    mode      The interaction mode.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 13:33:56 PST 2000
//   
//  Modifications:
//    Kathleen Bonnell, Thu Nov 22 16:03:00 PST 2001 
//    I added calls to Start/Stop pick mode. 
//
//    Kathleen Bonnell, Wed Jun 25 16:50:48 PDT 2003  
//    Modified to handle different Pick modes.  Removed calls to Start/Stop
//    PickMode (moved the logic for those methods here.)
//
// ****************************************************************************

void
ViewerWindow::SetInteractionMode(const INTERACTION_MODE mode)
{
    const INTERACTION_MODE curMode = visWindow->GetInteractionMode();

    bool changingToPickMode  = (ZONE_PICK == mode || NODE_PICK == mode);
    bool currentlyInPickMode = (ZONE_PICK == curMode || NODE_PICK == curMode);

    if  (changingToPickMode)
    {
        //
        // Either starting pick mode or changing from one pick mode to another. 
        //
        bool firstEntry = !currentlyInPickMode;
        bool zonePick = (ZONE_PICK == mode);
        ViewerQueryManager::Instance()->StartPickMode(firstEntry, zonePick);
    }
    else if (!changingToPickMode && currentlyInPickMode)
    {
        //
        // Stopping pick mode
        //
        animation->GetPlotList()->StopPick();
        ViewerQueryManager::Instance()->StopPickMode();
    }

    visWindow->SetInteractionMode(mode);
}

// ****************************************************************************
// Method: ViewerWindow::GetInteractionMode
//
// Purpose: 
//   Returns the window's interaction mode.
//
// Returns:    The window's interaction mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 15:07:40 PST 2002
//
// Modifications:
//   
// ****************************************************************************

INTERACTION_MODE
ViewerWindow::GetInteractionMode() const
{
    return visWindow->GetInteractionMode();
}

// ****************************************************************************
// Method: ViewerWindow::SetToolEnabled
//
// Purpose: 
//   Sets the enabled state of a tool in the VisWindow.
//
// Arguments:
//   toolId  : The id of the tool.
//   enabled : The enabled state of the tool.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 16:19:31 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002  
//   Notify ViewerQueryManager of the tool's enabled state.
//   
// ****************************************************************************

void
ViewerWindow::SetToolEnabled(int toolId, bool enabled)
{
    //
    // If we're turning the tool on, send attributes to it so it can
    // initialize itself if it wants to.
    //
    if(enabled)
    {
        ViewerPlotList *pl = animation->GetPlotList();
        pl->InitializeTool(visWindow->GetToolInterface(toolId));
        ViewerQueryManager::Instance()->
            InitializeTool(this, visWindow->GetToolInterface(toolId));
    }
    else
    {
        ViewerQueryManager::Instance()->
            DisableTool(this, visWindow->GetToolInterface(toolId));
    }

    visWindow->SetToolEnabled(toolId, enabled);
}

// ****************************************************************************
// Method: ViewerWindow::DisableAllTools
//
// Purpose: 
//   Turns all tools off.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 16:19:49 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::DisableAllTools()
{
    for(int i = 0; i < visWindow->GetNumTools(); ++i)
        SetToolEnabled(i, false);
}

// ****************************************************************************
// Method: ViewerWindow::HandleTool
//
// Purpose: 
//   If the tool information is meant for this window, the tool info is
//   applied to the plot list and the window is redrawn with the updated plots.
//
// Arguments:
//   ti : A reference to the tool interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 9 14:59:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Apr 11 17:35:35 PST 2002
//   Added an argument.
//
//   Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002   
//   Allow ViewerQueryManager to also handle a tool. 
//
//   Brad Whitlock, Mon Nov 11 11:21:39 PDT 2002
//   Added support for locking tools.
//
// ****************************************************************************

void
ViewerWindow::HandleTool(const avtToolInterface &ti, bool applyToAll)
{
    //
    // If the tool "event" is not for this window and tools are not locked,
    // return so the tools for this window do not get changed.
    //
    if((visWindow != ti.GetVisWindow()) && !toolsLocked)
        return;

    //
    // Pass the tool info off to the plot list and redraw the window.
    //
    animation->GetPlotList()->HandleTool(ti, applyToAll);
    ViewerQueryManager::Instance()->HandleTool(this, ti);
}

// ****************************************************************************
// Method: ViewerWindow::UpdateTools
//
// Purpose: 
//   Tells all active tools in the window to update themselves using the
//   attributes in the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 09:27:07 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002 
//   ViewerQueryManager may also udpate a tool.
//   
// ****************************************************************************

void
ViewerWindow::UpdateTools()
{
    ViewerPlotList *pl = animation->GetPlotList();
    bool redrawWindow = false;

    //
    // Update any tools that may need to be updated.
    //
    for(int toolId = 0; toolId < visWindow->GetNumTools(); ++toolId)
    {
        if(visWindow->GetToolEnabled(toolId))
        {
            if(pl->InitializeTool(visWindow->GetToolInterface(toolId)))
            {
                redrawWindow = true;
                visWindow->UpdateTool(toolId, false);
            }
            if(ViewerQueryManager::Instance()->
               InitializeTool(this, visWindow->GetToolInterface(toolId)))
            {
                redrawWindow = true;
                visWindow->UpdateTool(toolId, false);
            }
        }
    }

    //
    // If the any of the tools were updated, redraw the window.
    //
    if(redrawWindow)
        SendRedrawMessage();
}

//
// Methods to return information about tools.
//

int
ViewerWindow::GetNumTools() const
{
    return visWindow->GetNumTools();
}

bool
ViewerWindow::GetToolAvailable(int index) const
{
    return visWindow->GetToolAvailable(index);
}

bool
ViewerWindow::GetToolEnabled(int index) const
{
    return visWindow->GetToolEnabled(index);
}

std::string
ViewerWindow::GetToolName(int index) const
{
    return visWindow->GetToolName(index);
}

// ****************************************************************************
//  Method: ViewerWindow::RecenterView
//
//  Purpose: 
//    Recenter the window's view.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2000
//
//  Modifications:
//    Eric Brugger, Tue Apr 24 16:50:32 PDT 2001
//    I modified the class to handle seperate 2d and 3d view information.
//   
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//    Support avtViewCurve. 
//
//    Brad Whitlock, Mon Sep 23 12:56:29 PDT 2002
//    I made it use the new GetExtents method.
//
//    Eric Brugger, Tue Jan 14 07:59:31 PST 2003
//    I added the number of dimensions to the GetExtents call.
//
// ****************************************************************************

void
ViewerWindow::RecenterView()
{
    double limits[6];

    //
    // Get the limits at the current time and recenter the view using those
    // limits.
    //
    GetExtents(viewDimension, limits);

    switch (viewDimension)
    {
      case 2:
        if (!GetTypeIsCurve())
            RecenterView2d(limits);
        else 
            RecenterViewCurve(limits);
        break;
      case 3:
        RecenterView3d(limits);
        break;
    }
}

// ****************************************************************************
//  Method: ViewerWindow::ResetView
//
//  Purpose: 
//    Reset the window's view.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 13:33:56 PST 2000
//
//  Modifications:
//    Eric Brugger, Thu Dec 28 15:17:37 PST 2000
//    I modified the routine to set the view based on the bounding box
//    instead of letting the VisWindow do it.
//   
//    Eric Brugger, Tue Apr 24 13:41:46 PDT 2001
//    I modified the routine to also set the parallel scale and view angle.
//
//    Eric Brugger, Tue Apr 24 16:50:32 PDT 2001
//    I modified the class to handle seperate 2d and 3d view information.
//   
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//    Support avtViewCurve. 
//
// ****************************************************************************

void
ViewerWindow::ResetView()
{
    switch (viewDimension)
    {
      case 2:
        if (!GetTypeIsCurve())
            ResetView2d();
        else 
            ResetViewCurve();
        break;
      case 3:
        ResetView3d();
        break;
    }
}

// ****************************************************************************
//  Method: ViewerWindow::ClearViewKeyframes
//
//  Purpose: 
//    Clear the view keyframes.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::ClearViewKeyframes()
{
    view2DAtts->ClearAtts();
    view3DAtts->ClearAtts();
}

// ****************************************************************************
//  Method: ViewerWindow::DeleteViewKeyframe
//
//  Purpose: 
//    Delete a view keyframe.
//
//  Arguments:
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::DeleteViewKeyframe(const int frame)
{
    int nFrames = animation->GetNFrames();

    //
    // Check that the frame is within range.
    //
    if ((frame < 0) || (frame >= nFrames))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Delete the keyframe at the specified frame.  DeleteAtts
    // returns the range of frames that were invalidated.  The
    // maximum value is clamped to nFrames since DeleteAtts may
    // return INT_MAX to indicate the end of the animation.
    //
    int f0, f1;

    view2DAtts->DeleteAtts(frame, f0, f1);
    view3DAtts->DeleteAtts(frame, f0, f1);
    f1 = f1 < (nFrames - 1) ? f1 : (nFrames - 1);

    //
    // If the current frame is within the range, update the view.
    //
    if (animation->GetFrameIndex() >= f0 || animation->GetFrameIndex() <= f1)
    {
        UpdateCameraView();
        ViewerWindowManager::Instance()->UpdateViewAtts();
    }
}

// ****************************************************************************
//  Method: ViewerWindow::MoveViewKeyframe
//
//  Purpose: 
//    Move the position of a view keyframe.
//
//  Arguments:
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 29, 2003
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::MoveViewKeyframe(int oldFrame, int newFrame)
{
    int nFrames = animation->GetNFrames();

    //
    // Check that the frame numbers are within range.
    //
    if ((oldFrame < 0) || (oldFrame >= nFrames) ||
        (newFrame < 0) || (newFrame >= nFrames))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Move the keyframe at oldFrame to newFrame.  MoveAtts
    // returns the range of frames that were invalidated.  The
    // maximum value is clamped to nFrames since DeleteAtts may
    // return INT_MAX to indicate the end of the animation.
    //
    int f0, f1;

    if (!view2DAtts->MoveAtts(oldFrame, newFrame, f0, f1))
        return;
    if (!view3DAtts->MoveAtts(oldFrame, newFrame, f0, f1))
        return;
    f1 = f1 < (nFrames - 1) ? f1 : (nFrames - 1);

    //
    // If the current frame is within the range, update the view.
    //
    if (animation->GetFrameIndex() >= f0 || animation->GetFrameIndex() <= f1)
    {
        UpdateCameraView();
        ViewerWindowManager::Instance()->UpdateViewAtts();
    }
}

// ****************************************************************************
//  Method: ViewerWindow::SetViewKeyframe
//
//  Purpose: 
//    Set a view keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//    Eric Brugger, Tue Jun 10 13:08:55 PDT 2003
//    I renamed camera to view normal in the view attributes.  I added
//    image pan and image zoom to the 3d view attributes.
//
// ****************************************************************************

void
ViewerWindow::SetViewKeyframe()
{
    //
    // Set a 2d view keyframe.
    //
    const avtView2D &view2d = visWindow->GetView2D();

    curView->SetWindowCoords(view2d.window);
    curView->SetViewportCoords(view2d.viewport);

    view2DAtts->SetAtts(animation->GetFrameIndex(), curView);

    //
    // Set a 3d view keyframe.
    //
    const avtView3D &view3d = visWindow->GetView3D();

    curView->SetViewNormal(view3d.normal);
    curView->SetFocus(view3d.focus);
    curView->SetViewUp(view3d.viewUp);
    curView->SetViewAngle(view3d.viewAngle);
    curView->SetParallelScale(view3d.parallelScale);
    curView->SetSetScale(true);
    curView->SetNearPlane(view3d.nearPlane);
    curView->SetFarPlane(view3d.farPlane);
    curView->SetImagePan(view3d.imagePan);
    curView->SetImageZoom(view3d.imageZoom);
    curView->SetPerspective(view3d.perspective);

    view3DAtts->SetAtts(animation->GetFrameIndex(), curView);
}

// ****************************************************************************
//  Method: ViewerWindow::GetViewKeyframeIndices
//
//  Purpose: 
//    Get the view keyframe indices.
//
//  Programmer: Eric Brugger
//  Creation:   February 4, 2003
//
//  Modifications:
//
// ****************************************************************************

const int *
ViewerWindow::GetViewKeyframeIndices(int &nKeyframes) const
{
    return view2DAtts->GetIndices(nKeyframes);
}

// ****************************************************************************
//  Method: ViewerWindow::SetViewExtentsType
//
//  Purpose: 
//    Set the flavor of view extents to use when determining the view.
//
//  Arguments:
//    viewType  The flavor of view extents.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2002
//
// ****************************************************************************

void
ViewerWindow::SetViewExtentsType(avtExtentType viewType)
{
    visWindow->SetViewExtentsType(viewType);
    if (viewType != plotExtentsType)
    {
        boundingBoxValid2d = false;
        boundingBoxValid3d = false;
        plotExtentsType = viewType;
        animation->GetPlotList()->SetSpatialExtentsType(plotExtentsType);
    }
}

// ****************************************************************************
// Method: ViewerWindow::GetViewExtentsType
//
// Purpose: 
//   Returns the plot extents type.
//
// Returns:    The plot extents type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 15:12:40 PST 2002
//
// Modifications:
//   
// ****************************************************************************

avtExtentType
ViewerWindow::GetViewExtentsType() const
{
    return plotExtentsType;
}

// ****************************************************************************
// Method: ViewerWindow::GetExtents
//
// Purpose: 
//   Returns the plot's outer extents in the specified array.
//
// Arguments:
//   nDimensions : The number of dimensions to which extents corresponds.
//   extents     : The return array for the extents. It must have at least 4
//                 elements for 2d and 6 elements for 3d.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 12:53:23 PDT 2002
//
// Modifications:
//   Eric Brugger, Tue Jan 14 07:56:14 PST 2003 
//   I added an nDimensions argument.
//   
// ****************************************************************************

void
ViewerWindow::GetExtents(int nDimensions, double *extents)
{
    int time = animation->GetFrameIndex();
    animation->GetPlotList()->GetPlotLimits(time, nDimensions, extents);
}

// ****************************************************************************
//  Method: ViewerWindow::SetBoundingBoxMode
//
//  Purpose: 
//    Set the window's bounding box mode.
//
//  Arguments:
//    mode      The bbox mode.  A true value indicates that the window
//              is in navigate bbox mode.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 15:34:23 PST 2000
//
// ****************************************************************************

void
ViewerWindow::SetBoundingBoxMode(const bool mode)
{
    visWindow->SetBoundingBoxMode(mode);
}

// ****************************************************************************
//  Method: ViewerWindow::GetBoundingBoxMode
//
//  Purpose: 
//    Return the window's bbox mode.
//
//  Returns:    The bbox mode.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 15:35:22 PST 2000
//
// ****************************************************************************

bool
ViewerWindow::GetBoundingBoxMode() const
{
    return visWindow->GetBoundingBoxMode();
}

// ****************************************************************************
//  Method: ViewerWindow::SetSpinMode
//
//  Purpose: 
//    Set the window's spin mode.
//
//  Arguments:
//    mode      The spin mode.  A true value indicates that the window
//              should be in spin mode.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

void
ViewerWindow::SetSpinMode(const bool mode)
{
    visWindow->SetSpinMode(mode);
}

// ****************************************************************************
//  Method: ViewerWindow::GetSpinMode
//
//  Purpose: 
//    Return the window's spin mode.
//
//  Returns:    The spin mode.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

bool
ViewerWindow::GetSpinMode() const
{
    return visWindow->GetSpinMode();
}

// ****************************************************************************
//  Method: ViewerWindow::SetCameraViewMode
//
//  Purpose: 
//    Set the window's camera view mode.
//
//  Arguments:
//    mode      The camera view mode.  A true value indicates that the window
//              should be in camera view mode.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
// ****************************************************************************

void
ViewerWindow::SetCameraViewMode(const bool mode)
{
    cameraView = mode;
}

// ****************************************************************************
//  Method: ViewerWindow::GetCameraViewMode
//
//  Purpose: 
//    Return the window's camera view mode.
//
//  Returns:    The camera view mode.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
// ****************************************************************************

bool
ViewerWindow::GetCameraViewMode() const
{
    return cameraView;
}

// ****************************************************************************
//  Method: ViewerWindow::SetMaintainViewMode
//
//  Purpose: 
//    Set the window's maintain view mode.
//
//  Arguments:
//    mode      The maintain view mode.
//
//  Programmer: Eric Brugger
//  Creation:   April 18, 2003
//
// ****************************************************************************

void
ViewerWindow::SetMaintainViewMode(const bool mode)
{
    //
    // If maintain view is toggled to false recenter the view if necessary.
    //
    if (maintainView == true && mode == false)
    {
        if (viewDimension == 3 ||
            (viewDimension == 2 && viewModified2d == false))
        {
            RecenterView();
        }
    }
    maintainView = mode;
}

// ****************************************************************************
//  Method: ViewerWindow::GetMaintainViewMode
//
//  Purpose: 
//    Return the window's maintain view mode.
//
//  Returns:    The maintain view mode.
//
//  Programmer: Eric Brugger
//  Creation:   April 18, 2003
//
// ****************************************************************************

bool
ViewerWindow::GetMaintainViewMode() const
{
    return maintainView;
}


// ****************************************************************************
//  Method: ViewerWindow::SetFullFrameMode
//
//  Purpose: 
//    Set the window's full frame mode.
//
//  Arguments:
//    mode      The full frame mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 13, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed May 28 11:18:29 PDT 2003
//    Reworked so that plots are only scaled and view is only set for 2d,
//    non-curve window with realized plots.  Preserve the current view's zoom 
//    or pan parameters by not calling ResetView.  Instead, simply scale or 
//    reverse the scale of view2D.window as necessary.
//
//    Kathleen Bonnell, Fri Jun  6 16:39:22 PDT 2003
//    Removed call to ScalePlots. Now handled at VisWindow level.  
//
// ****************************************************************************

void
ViewerWindow::SetFullFrameMode(const bool mode)
{
    if (fullFrame == mode)
    {
        return;
    }

    fullFrame = mode;

    if (viewDimension == 2 && !GetTypeIsCurve() && boundingBoxValid2d) 
    {
        avtView2D view2D=visWindow->GetView2D();
        if (fullFrame)
        { 
            //
            // Scale the window.
            //
            Compute2DScaleFactor(view2D.axisScaleFactor, view2D.axisScaleType);
            if (view2D.axisScaleFactor != 0.)
            {
                if (view2D.axisScaleType == 0) // requires x-axis scaling
                {
                    view2D.window[0] *= view2D.axisScaleFactor;
                    view2D.window[1] *= view2D.axisScaleFactor;
                }
                else // requires y-axis scaling
                {
                    view2D.window[2] *= view2D.axisScaleFactor;
                    view2D.window[3] *= view2D.axisScaleFactor;
                }
            }
        }
        else
        {
            //
            // Un-do the previous scaling on the window.
            //
            if (view2D.axisScaleFactor != 0.)
            {
                if (view2D.axisScaleType == 0) // requires x-axis scaling
                {
                    view2D.window[0] /= view2D.axisScaleFactor;
                    view2D.window[1] /= view2D.axisScaleFactor;
                }
                else // requires y-axis scaling
                {
                    view2D.window[2] /= view2D.axisScaleFactor;
                    view2D.window[3] /= view2D.axisScaleFactor;
                }
            }
            view2D.axisScaleFactor = 0.;
        }

        visWindow->SetView2D(view2D);

        //
        // Flag the view as unmodified.
        //
        viewModified2d = false;
    }
}


// ****************************************************************************
//  Method: ViewerWindow::GetFullFrameMode
//
//  Purpose: 
//    Return the window's full frame mode.
//
//  Returns:    The full frame mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 13, 2003
//
// ****************************************************************************

bool
ViewerWindow::GetFullFrameMode() const
{
    return fullFrame;
}


// ****************************************************************************
//  Method: ViewerWindow::SetViewIsLocked
//
//  Purpose:
//    Set the whether or not the window's view is locked to a global view.
//
//  Arguments:
//    val       The new value of viewIsLocked.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2002
//
// ****************************************************************************

void
ViewerWindow::SetViewIsLocked(const bool val)
{
    viewIsLocked = val;
}

// ****************************************************************************
//  Method: ViewerWindow::GeViewIsLocked
//
//  Purpose: 
//    Return the whether or not the window's view is locked to a global view.
//
//  Returns:    True if it is locked, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2002
//
// ****************************************************************************

bool
ViewerWindow::GetViewIsLocked() const
{
    return viewIsLocked;
}

// ****************************************************************************
// Method: ViewerWindow::SetTimeLock
//
// Purpose: 
//   Sets the window's timelocked flag.
//
// Arguments:
//   val : The new timelocked value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:11:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetTimeLock(const bool val)
{
    timeLocked = val;
}

// ****************************************************************************
// Method: ViewerWindow::GetTimeLock
//
// Purpose: 
//   Returns the window's time lock flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:11:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetTimeLock() const
{ 
    return timeLocked;
}

// ****************************************************************************
// Method: ViewerWindow::SetToolLock
//
// Purpose: 
//   Sets the window's flag for tool locking.
//
// Arguments:
//   val : The new locking state.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:16:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetToolLock(const bool val)
{
    toolsLocked = val;
}

// ****************************************************************************
// Method: ViewerWindow::GetToolLock
//
// Purpose: 
//   Returns whether or not the window's tools are locked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:17:13 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetToolLock() const
{ 
    return toolsLocked;
}

// ****************************************************************************
//  Method: ViewerWindow::SetPerspectiveProjection
//
//  Purpose:
//    Set the perspective projection mode.
//
//  Arguments:
//    mode      The perspective projection mode.
//
//  Programmer: Hank Childs
//  Creation:   November 10, 2000
//
//  Modifications:
//    Eric Brugger, Mon Mar 12 14:37:31 PST 2001
//    I modified the routine to set the perspective through the avtViewInfo.
//
//    Eric Brugger, Tue Apr 24 14:51:24 PDT 2001
//    I replaced some code which updated the viewer window's view information
//    with a method invocation.
//
//    Eric Brugger, Tue Apr 24 16:50:32 PDT 2001
//    I modified the class to store seperate 2d and 3d view information.
//
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
// ****************************************************************************

void
ViewerWindow::SetPerspectiveProjection(const bool mode)
{
    //
    // Set the perspective mode and update the view.
    //
    avtView3D view3D=visWindow->GetView3D();

    view3D.perspective = mode;
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::GetPerspectiveProjection
//
//  Purpose:
//    Return the window's perspective projection mode.
//
//  Returns:    The perspective projection mode.
//
//  Programmer: Hank Childs
//  Creation:   November 10, 2000
//
//  Modifications:
//    Eric Brugger, Mon Mar 12 14:37:31 PST 2001
//    I modified the routine to get the perspective through the avtViewInfo.
//
//    Eric Brugger, Tue Apr 24 16:50:32 PDT 2001
//    I modified the class to store seperate 2d and 3d view information.
//
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
// ****************************************************************************

bool
ViewerWindow::GetPerspectiveProjection() const
{
    return visWindow->GetView3D().perspective;
}


// ****************************************************************************
// Method: ViewerWindow::UpdateColorTable
//
// Purpose: 
//   Tells the plot list for the window to update the color tables of its plots.
//
// Arguments:
//   ctName : The name of the color table to update.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:31:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::UpdateColorTable(const char *ctName)
{
    // If any plots in the plot list had to be updated, redraw the window.
    if(animation->GetPlotList()->UpdateColorTable(ctName))
    {
        SendRedrawMessage();
    }
}

// ****************************************************************************
// Method: ViewerWindow::RedrawWindow
//
// Purpose: 
//   Enables updates and tells the VisWindow to redraw itself.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 13:23:56 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::RedrawWindow()
{
    visWindow->EnableUpdates();
    visWindow->Render();
}

// ****************************************************************************
// Method: ViewerWindow::UpdatesEnabled
//
// Purpose: 
//   Returns whether or not the VisWindow is allowing updates.
//
// Returns:    Whether or not the VisWindow is allowing updates.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 16:14:27 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::UpdatesEnabled() const
{
    return visWindow->UpdatesEnabled();
}

// ****************************************************************************
//  Method: ViewerWindow::ClearWindow
//
//  Purpose: 
//    Clear the window's plots.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 12:21:34 PDT 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Nov 27 16:03:00 PST 2001 
//    I added call to ClearPickPoints. 
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//    Added ClearRefLines. 
//
// ****************************************************************************

void
ViewerWindow::ClearWindow()
{
    ClearRefLines();
    ClearPickPoints();
    animation->GetPlotList()->ClearPlots();
}

// ****************************************************************************
//  Method: ViewerWindow::ScreenCapture
//
//  Purpose: 
//    Tells the VisWindow to do a screen capture.
//
//  Returns:    The image from the screen capture.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
//
// ****************************************************************************

avtImage_p
ViewerWindow::ScreenCapture(void)
{
    return visWindow->ScreenCapture();
}

// ****************************************************************************
//  Method: ViewerWindow::GetAllDatasets
//
//  Purpose: 
//    Tells the VisWindow to create one actor of all the datasets.
//
//  Returns:    One large poly data mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

avtDataset_p
ViewerWindow::GetAllDatasets(void)
{
    return visWindow->GetAllDatasets();
}

// ****************************************************************************
//  Method: ViewerWindow::InvertBackgroundColor
//
//  Purpose: 
//    Reverses the background and foreground colors and redraws the image.
//
//  Programmer: Brad Whitlock
//  Creation:   August 27, 2001 
//   
// ****************************************************************************

void
ViewerWindow::InvertBackgroundColor()
{
    visWindow->InvertBackgroundColor();
}

// ****************************************************************************
// Method: ViewerWindow::CopyGeneralAttributes
//
// Purpose: 
//   Copies the general attributes (rendering attributes, view lock mode, etc.)
//   from the source window to this window.
//
// Programmer: Eric Brugger
// Creation:   November 2, 2001
//
// Modifications:
//   Hank Childs, Thu May  2 13:43:57 PDT 2002
//   Do not copy over the database name if it is NULL.
//
//   Brad Whitlock, Wed Feb 12 11:11:21 PDT 2003
//   I rewrote the method so it does not copy animation attributes but instead
//   copies attributes like the window's camera view mode, rendering attributes,
//   etc.
//
//   Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//   I replaced autoCenter with maintainView.
//
//   Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003  
//   Copy over full frame mode. 
//
// ****************************************************************************

void
ViewerWindow::CopyGeneralAttributes(const ViewerWindow *source)
{
    //
    // Copy the rendering attributes.
    //
    SetAntialiasing(source->GetAntialiasing());
    SetStereoRendering(source->GetStereo(), source->GetStereoType());
    SetImmediateModeRendering(source->GetImmediateModeRendering());
    SetSurfaceRepresentation(source->GetSurfaceRepresentation());
    SetNotifyForEachRender(source->GetNotifyForEachRender());
    SetScalableRendering(source->GetScalableRendering());
    SetScalableThreshold(source->GetScalableThreshold());

    //
    // Set window mode flags.
    //
    SetBoundingBoxMode(source->GetBoundingBoxMode());
    SetSpinMode(source->GetSpinMode());
    SetCameraViewMode(source->GetCameraViewMode());
    SetMaintainViewMode(source->GetMaintainViewMode());
    SetViewIsLocked(source->GetViewIsLocked());
    SetTimeLock(source->GetTimeLock());
    SetToolLock(source->GetToolLock());
    SetFullFrameMode(source->GetFullFrameMode());
}

// ****************************************************************************
//  Method: ViewerWindow::AddPlot
//
//  Purpose:
//    Add a plot to the window.
//
//  Arguments:
//    actor     The actor to add to the window.  
//
//  Programmer: Eric Brugger
//  Creation:   February 20, 2001
//
// ****************************************************************************

void
ViewerWindow::AddPlot(avtActor_p &actor)
{
    visWindow->AddPlot(actor);
}

// ****************************************************************************
// Method: ViewerWindow::CopyAnimation
//
// Purpose: 
//   Copies the animation from one window to this window.
//
// Arguments:
//   w         : The viewer window from which we're copying the animation.
//   copyPlots : Whether or not we're copying the animation's plot list too.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:54:36 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::CopyAnimation(const ViewerWindow *w, bool copyplots)
{
    animation->CopyFrom(w->GetAnimation(), copyplots);
}

// ****************************************************************************
//  Method: ViewerWindow::ClearPlots
//
//  Purpose:
//    Clear the plots associated with the window.
//
//  Programmer: Eric Brugger
//  Creation:   February 20, 2001
//
// ****************************************************************************

void
ViewerWindow::ClearPlots()
{
    visWindow->ClearPlots();
}

// ****************************************************************************
//  Method: ViewerWindow::EnableUpdates
//
//  Purpose:
//    Enable automatically updating the window as plots are added and deleted
//    from the window.
//
//  Programmer: Eric Brugger
//  Creation:   February 20, 2001
//
// ****************************************************************************

void
ViewerWindow::EnableUpdates()
{
    visWindow->EnableUpdates();
}

// ****************************************************************************
//  Method: ViewerWindow::DisableUpdates
//
//  Purpose:
//    Disable automatically updating the window as plots are added and deleted
//    from the window.
//
//  Programmer: Eric Brugger
//  Creation:   February 20, 2001
//
// ****************************************************************************

void
ViewerWindow::DisableUpdates()
{
    visWindow->DisableUpdates();
}

// ****************************************************************************
// Method: ViewerWindow::SendRedrawMessage
//
// Purpose: 
//   Sends a redrawWindow message to the rendering thread that causes the
//   thread to redraw the contents of the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 09:34:08 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SendRedrawMessage()
{
    char msg[256];

    SNPRINTF(msg, 256, "redrawWindow 0x%p;", this);
    viewerSubject->MessageRendererThread(msg);
}

// ****************************************************************************
// Method: ViewerWindow::SendUpdateMessage
//
// Purpose: 
//   Sends an updateWindow message to the rendering thread that tells it to
//   allow updates to the VisWindow.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 13:26:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SendUpdateMessage()
{
    char msg[256];

    SNPRINTF(msg, 256, "updateWindow 0x%p;", this);
    viewerSubject->MessageRendererThread(msg);
}

// ****************************************************************************
// Method: ViewerWindow::SendDeleteMessage
//
// Purpose: 
//   Sends a deleteWindow message to the rendering thread that tells it to
//   delete the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 11:50:22 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SendDeleteMessage()
{
    char msg[256];

    SNPRINTF(msg, 256, "deleteWindow 0x%p;", this);
    viewerSubject->MessageRendererThread(msg);
}

// ****************************************************************************
//  Method: ViewerWindow::UpdateView
//
//  Purpose: 
//    Update the view for the window.
//
//  Arguments:
//    dimension The dimension of the plots.
//    limits    The limits of all the plots.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2000
//
//  Modifications:
//    Eric Brugger, Tue Apr 24 16:50:32 PDT 2001
//    I added a dimension argument to keep seperate track of 2d and 3d views.
//   
//    Hank Childs, Wed Mar  6 17:18:16 PST 2002
//    Turn off the zoom interactor when we are in 3D because of current
//    limitations.
//
//    Eric Brugger, Thu Apr 11 16:57:31 PDT 2002
//    Enable zoom mode in 3d.
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//    Support avtViewCurve. 

// ****************************************************************************

void
ViewerWindow::UpdateView(const int dimension, const double *limits)
{
    if (viewDimension != dimension)
        ViewerQueryManager::Instance()->ViewDimChanged(this);

    viewDimension = dimension;
    switch (dimension)
    {
      case 2:
        if (!GetTypeIsCurve())
        {
            UpdateView2d(limits);
        }
        else 
        {
            UpdateViewCurve(limits);
        }
        break;
      case 3:
        UpdateView3d(limits);
        break;
    }
}

// ****************************************************************************
//  Method: ViewerWindow::SetView2D
//
//  Purpose: 
//    Sets the 2D view for the window and updates the window if the
//    window is 2d.
//
//  Arguments:
//    v         The new view info.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 20 14:58:02 PST 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//   
//    Eric Brugger, Wed Jan  8 11:48:23 PST 2003
//    I modified the routine to set the haveRenderedIn2d flag so that the
//    view will not get clobbered when the first image is rendered.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I modified the routine to set the viewModified2d flag.
//
// ****************************************************************************

void
ViewerWindow::SetView2D(const avtView2D &v)
{
    visWindow->SetView2D(v);

    haveRenderedIn2d = true;

    viewModified2d = true;
}

// ****************************************************************************
//  Method: ViewerWindow::SetView3D
//
//  Purpose: 
//    Sets the 3D view for the window and updates the window if the
//    window is 3d.
//
//  Arguments:
//    v         The new view info.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 20 14:58:02 PST 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//   
//    Eric Brugger, Wed Jan  8 11:48:23 PST 2003
//    I modified the routine to set the haveRenderedIn3d flag so that the
//    view will not get clobbered when the first image is rendered.
//
// ****************************************************************************

void
ViewerWindow::SetView3D(const avtView3D &v)
{
    visWindow->SetView3D(v);

    haveRenderedIn3d = true;
}


// ****************************************************************************
//  Method: ViewerWindow::SetViewCurve
//
//  Purpose: 
//    Sets the Curve view for the window and updates the window if the
//    window is Curve.
//
//  Arguments:
//    v         The new view info.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002 
//
// ****************************************************************************

void
ViewerWindow::SetViewCurve(const avtViewCurve &v)
{
    visWindow->SetViewCurve(v);
}


// ****************************************************************************
//  Method: ViewerWindow::GetView2D
//
//  Purpose: 
//    Returns a constant reference to the 2d view.
//
//  Returns:    A constant reference to the 2d view.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 27 09:32:28 PDT 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//   
// ****************************************************************************

const avtView2D &
ViewerWindow::GetView2D() const
{
    return visWindow->GetView2D();
}

// ****************************************************************************
//  Method: ViewerWindow::GetView3D
//
//  Purpose: 
//    Returns a constant reference to the 3d view.
//
//  Returns:    A constant reference to the 3d view.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 27 09:32:28 PDT 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//   
// ****************************************************************************

const avtView3D &
ViewerWindow::GetView3D() const
{
    return visWindow->GetView3D();
}

// ****************************************************************************
//  Method: ViewerWindow::GetViewCurve
//
//  Purpose: 
//    Returns a constant reference to the Curve view.
//
//  Returns:    A constant reference to the Curve view.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002  
//
// ****************************************************************************

const avtViewCurve &
ViewerWindow::GetViewCurve() const
{
    return visWindow->GetViewCurve();
}


// ****************************************************************************
// Method: ViewerWindow::CopyViewAttributes
//
// Purpose: 
//   Copies the view attributes from the source window to this window.
//
// Programmer: Eric Brugger
// Creation:   November 2, 2001
//
// Modifications:
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//    Support avtViewCurve. 
//
//    Kathleen Bonnell, Wed May 15 16:40:35 PDT 2002 
//    Remove avtViewCurve. It doesn't really make sense for a curve view to
//    be copied.
//
//    Hank Childs, Thu Jul 18 15:04:20 PDT 2002
//    Copy over haveRendered... data members.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I removed autoCenter and added viewModified2d.
//
// ****************************************************************************

void
ViewerWindow::CopyViewAttributes(const ViewerWindow *source)
{
    //
    // Copy the 2d and 3d view.
    //
    visWindow->SetView2D(source->visWindow->GetView2D());
    visWindow->SetView3D(source->visWindow->GetView3D());

    //
    // Copy the 2d and 3d bounding boxes.
    //
    boundingBoxValid2d = source->boundingBoxValid2d;
    centeringValid2d   = source->centeringValid2d;
    haveRenderedIn2d   = source->haveRenderedIn2d;
    viewModified2d     = source->viewModified2d;
    boundingBoxValid3d = source->boundingBoxValid3d;
    centeringValid3d   = source->centeringValid3d;
    haveRenderedIn3d   = source->haveRenderedIn3d;
    int       i;

    for (i = 0; i < 4; i++)
        boundingBox2d[i] = source->boundingBox2d[i];
    for (i = 0; i < 6; i++)
        boundingBox3d[i] = source->boundingBox3d[i];
}

// ****************************************************************************
// Method: ViewerWindow::UpdateCameraView
//
// Purpose: 
//   Update the view if in camera view mode.
//
// Programmer: Eric Brugger
// Creation:   January 7, 2003
//
// Modifications:
//   Eric Brugger, Tue Jun 10 13:08:55 PDT 2003
//   I renamed camera to view normal in the view attributes.  I added
//   image pan and image zoom to the 3d view attributes.
//
// ****************************************************************************

void
ViewerWindow::UpdateCameraView()
{
    //
    // If we are in camera view mode and at least one view keyframe has
    // been defined then set the view based on the view keyframes.
    //
    if (cameraView && view2DAtts->GetNIndices() > 0)
    {
        if (viewDimension == 2)
        {
            view2DAtts->GetAtts(animation->GetFrameIndex(), curView);
 
            avtView2D view2d;
 
            const double *viewport=curView->GetViewportCoords();
            const double *window=curView->GetWindowCoords();
 
            for (int i = 0; i < 4; i++)
            {
                view2d.viewport[i] = viewport[i];
                view2d.window[i]   = window[i];
            }
 
            visWindow->SetView2D(view2d);
        }
        else
        {
            view3DAtts->GetAtts(animation->GetFrameIndex(), curView);

            avtView3D view3d;

            view3d.normal[0] = curView->GetViewNormal()[0];
            view3d.normal[1] = curView->GetViewNormal()[1];
            view3d.normal[2] = curView->GetViewNormal()[2];
            view3d.focus[0] = curView->GetFocus()[0];
            view3d.focus[1] = curView->GetFocus()[1];
            view3d.focus[2] = curView->GetFocus()[2];
            view3d.viewUp[0] = curView->GetViewUp()[0];
            view3d.viewUp[1] = curView->GetViewUp()[1];
            view3d.viewUp[2] = curView->GetViewUp()[2];
            view3d.viewAngle = curView->GetViewAngle();
            view3d.parallelScale = curView->GetParallelScale();
            view3d.nearPlane = curView->GetNearPlane();
            view3d.farPlane = curView->GetFarPlane();
            view3d.imagePan[0] = curView->GetImagePan()[0];
            view3d.imagePan[1] = curView->GetImagePan()[1];
            view3d.imageZoom = curView->GetImageZoom();
            view3d.perspective = curView->GetPerspective();

            visWindow->SetView3D(view3d);
        }
    } 
}

// ****************************************************************************
//  Method: ViewerWindow::SetAnimationAttributes
//
//  Purpose: 
//    Set the animation attributes of the window.
//
//  Arguments:
//    atts      The animation attributes for this window. 
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001 
//   
// ****************************************************************************

void
ViewerWindow::SetAnimationAttributes(const AnimationAttributes *atts)
{
    animation->SetPipelineCaching(atts->GetPipelineCachingMode());
}

// ****************************************************************************
// Method: ViewerWindow::GetAnimationAttributes
//
// Purpose: 
//   Returns a pointer to the VisWindow's animation attributes.
//
// Note:       Note that the pointer returned by this method cannot be used
//             to set attributes of the annotation attributes.
//
// Programmer: Eric Brugger
// Creation:   November 21, 2001
//
// ****************************************************************************

static AnimationAttributes animationAttributes;

const AnimationAttributes *
ViewerWindow::GetAnimationAttributes() const
{
    animationAttributes.SetPipelineCachingMode(
        animation->GetPipelineCaching());

    return &animationAttributes;
}

// ****************************************************************************
//  Method: ViewerWindow::SetAnnotationAttributes
//
//  Purpose: 
//    Set the annotation attributes of the window.
//
//  Arguments:
//    atts      The annotation attributes for this window. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001 
//   
//  Modifications:
//    Eric Brugger, Fri Nov  2 14:59:33 PST 2001
//    I added a const qualifier for atts.
//
// ****************************************************************************

void
ViewerWindow::SetAnnotationAttributes(const AnnotationAttributes *atts)
{
    visWindow->SetAnnotationAtts(atts);
}

// ****************************************************************************
// Method: ViewerWindow::GetAnnotationAttributes
//
// Purpose: 
//   Returns a pointer to the VisWindow's annotation attributes.
//
// Note:       Note that the pointer returned by this method cannot be used
//             to set attributes of the annotation attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 08:52:46 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const AnnotationAttributes *
ViewerWindow::GetAnnotationAttributes() const
{
    return (const AnnotationAttributes *)visWindow->GetAnnotationAtts();
}

// ****************************************************************************
// Method: ViewerWindow::CopyAnnotationAttributes
//
// Purpose: 
//   Copies the annotation attributes from the source window to this window.
//
// Programmer: Eric Brugger
// Creation:   November 2, 2001
//
// ****************************************************************************

void
ViewerWindow::CopyAnnotationAttributes(const ViewerWindow *source)
{
    SetAnnotationAttributes(source->GetAnnotationAttributes());
}

// ****************************************************************************
// Method: ViewerWindow::SetLightList
//
// Purpose: 
//   Sets the light list in the VisWindow.
//
// Arguments:
//   ll : The new light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 15:32:48 PST 2001
//
// Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Changed argument to LightList from avtLightList. 
//   
// ****************************************************************************

void
ViewerWindow::SetLightList(const LightList *lightList)
{
    visWindow->SetLightList(lightList);
}

// ****************************************************************************
// Method: ViewerWindow::GetLightList
//
// Purpose: 
//   Returns the VisWindow's light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 15:33:33 PST 2001
//
// Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Changed return to LightList from avtLightList. 
//   
// ****************************************************************************

const LightList *
ViewerWindow::GetLightList() const
{
    return (const LightList*)visWindow->GetLightList();
}

// ****************************************************************************
// Method: ViewerWindow::CopyLightList
//
// Purpose: 
//   Copies the light source list from the source window to this window.
//
// Programmer: Eric Brugger
// Creation:   November 2, 2001
//
// ****************************************************************************

void
ViewerWindow::CopyLightList(const ViewerWindow *source)
{
    SetLightList(source->GetLightList());
}

// ****************************************************************************
//  Method: ViewerWindow::ShowMenu
//
//  Purpose: 
//    Activates the window's popup menu.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:22:05 PDT 2000
//
//  Modifications:
//    Brad Whitlock, Tue Feb 4 15:41:51 PST 2003
//    I removed UpdateMenu.
//
// ****************************************************************************

void
ViewerWindow::ShowMenu()
{
    popupMenu->ShowMenu();
}

// ****************************************************************************
//  Method: ViewerWindow::HideMenu
//
//  Purpose: 
//    Hide the window's popup menu.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:22:30 PDT 2000
//
// ****************************************************************************

void
ViewerWindow::HideMenu()
{
    popupMenu->HideMenu();
}

// ****************************************************************************
// Method: ViewerWindow::Iconify
//
// Purpose: 
//   Iconfies the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:19:06 PDT 2001
//
// ****************************************************************************

void
ViewerWindow::Iconify()
{
    visWindow->Iconify();
}

// ****************************************************************************
// Method: ViewerWindow::DeIconify
//
// Purpose: 
//   Deiconifies the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:19:35 PDT 2001
//
// ****************************************************************************

void
ViewerWindow::DeIconify()
{
    visWindow->DeIconify();
}

// ****************************************************************************
// Method: ViewerWindow::Show
//
// Purpose: 
//   Shows the window.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:42:08 PDT 2002
//
// ****************************************************************************

void
ViewerWindow::Show()
{
    isVisible = true;
    visWindow->Show();
}

// ****************************************************************************
// Method: ViewerWindow::Raise
//
// Purpose: 
//   Raises the window.
//
// Programmer: Sean Ahern
// Creation:   Mon May 20 13:27:10 PDT 2002
//
// ****************************************************************************

void
ViewerWindow::Raise()
{
    visWindow->Raise();
}

// ****************************************************************************
// Method: ViewerWindow::Raise
//
// Purpose: 
//   Raises the window.
//
// Programmer: Sean Ahern
// Creation:   Mon May 20 13:27:10 PDT 2002
//
// ****************************************************************************

void
ViewerWindow::Lower()
{
    visWindow->Lower();
}

// ****************************************************************************
// Method: ViewerWindow::Hide
//
// Purpose: 
//   Hides the window.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:42:27 PDT 2002
//
// ****************************************************************************

void
ViewerWindow::Hide()
{
    isVisible = false;
    visWindow->Hide();
}

// ****************************************************************************
// Method: ViewerWindow::GetRealized
//
// Purpose: 
//   Returns the realized state of the window.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:42:27 PDT 2002
//
// ****************************************************************************

bool
ViewerWindow::GetRealized()
{
    return visWindow->GetRealized();
}

// ****************************************************************************
// Method: ViewerWindow::IsVisible
//
// Purpose: 
//   Returns whether the window is visible.
//
// Returns:    Whether the window is visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 09:38:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::IsVisible() const
{
    return visWindow->IsVisible();
}

void
ViewerWindow::SetVisible(bool val)
{
    isVisible = val;
}

// ****************************************************************************
//  Method: ViewerWindow::SetPlotColors
//
//  Purpose: 
//    Sets the foreground & background colors for use by plots. 
//    Sends redraw message if any plots indicate they need it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 26, 2001 
//   
// ****************************************************************************

void
ViewerWindow::SetPlotColors(const double *bg, const double *fg)
{
    bool redraw = false;
    redraw |= animation->GetPlotList()->SetBackgroundColor(bg);
    redraw |= animation->GetPlotList()->SetForegroundColor(fg);
    if (redraw) 
    {
        SendRedrawMessage();
    }
}

// ****************************************************************************
//  Method: ViewerWindow::RecenterView2d
//
//  Purpose: 
//    Recenter the window's 2d view using the specified limits.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I modified the routine to set the viewModified2d flag.
//
//    Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003   
//    Added code to compute axis scale factor when in fullFrame mode. 
//
// ****************************************************************************

void
ViewerWindow::RecenterView2d(const double *limits)
{
    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (limits[0] == DBL_MAX && limits[1] == -DBL_MAX)
    {
        centeringValid2d = false;
        return;
    }

    //
    // Set the new window.
    //
    int       i;

    for (i = 0; i < 4; i++)
    {
        boundingBox2d[i] = limits[i];
    }

    //
    // Update the view.
    //
    avtView2D view2D=visWindow->GetView2D();

    for (i = 0; i < 4; i++)
    {
        view2D.window[i] = limits[i];
    }

    if (fullFrame)
    { 
        Compute2DScaleFactor(view2D.axisScaleFactor, view2D.axisScaleType);
        if (view2D.axisScaleType == 0) // requires x-axis scaling
        {
            view2D.window[0] *= view2D.axisScaleFactor;
            view2D.window[1] *= view2D.axisScaleFactor;
        }
        else // requires y-axis scaling
        {
            view2D.window[2] *= view2D.axisScaleFactor;
            view2D.window[3] *= view2D.axisScaleFactor;
        }
    }
    else
    {
        view2D.axisScaleFactor = 0.;
    }
    visWindow->SetView2D(view2D);

    //
    // Flag the view as unmodified.
    //
    viewModified2d = false;
}

// ****************************************************************************
//  Method: ViewerWindow::Compute2DScaleFactor
//
//  Purpose: 
//    Computes an axis scale factor that will create a square plot, based
//    on the current 2d bounding box. 
//
//  Arguments:
//    s         The computed scale factor. 
//    t         The axis that should be scaled (0 == x, 1 == y) 
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 13, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::Compute2DScaleFactor(double &s, int & t)
{
    // assumes a valid boundingBox2d.
    double width = boundingBox2d[1] - boundingBox2d[0];
    double height = boundingBox2d[3] - boundingBox2d[2];
    if (width > height && height > 0.)
    {
        s =  width / height;
        t = 1; // scale the y_axis
    }
    else if (height > width && width > 0.)
    {
        s =  height / width;
        t = 0; // scale the x_axis
    }
    else 
    {
        s = 0.;  // no scaling needs to take place
        t = 1;   // scale the y_axis (default)
    }
}


// ****************************************************************************
//  Method: ViewerWindow::GetScaleFactorAndType
//
//  Purpose: 
//    Retrieves the axis scale factor from view2d.
//
//  Arguments:
//    s         The scale factor. 
//    t         The axis that should be scaled (0 == x, 1 == y) 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::GetScaleFactorAndType(double &s, int &t)
{
    if (fullFrame && viewDimension == 2 && !GetTypeIsCurve())
    {
        avtView2D view2D=visWindow->GetView2D();
        s = view2D.axisScaleFactor;
        t = view2D.axisScaleType;
    }
    else 
    {
        s = 0.;
        t = 1;
    }
}


// ****************************************************************************
//  Method: ViewerWindow::RecenterView3d
//
//  Purpose: 
//    Recenter the window's 3d view using the specified limits.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
//    Eric Brugger, Tue Apr  2 11:35:29 PST 2002
//    I modified the setting of the clip planes since the definition of the
//    clip plane distance was now from the focal point instead of the camera.
//    I also increased the volume between the clip planes so that the view
//    was less likely to be clipped.  I also corrected a bug where the
//    routine was not compensating properly for zooming.
//
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
// ****************************************************************************

void
ViewerWindow::RecenterView3d(const double *limits)
{
    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (limits[0] == DBL_MAX && limits[1] == -DBL_MAX)
    {
        centeringValid3d = false;
        return;
    }

    //
    // Get the current view.
    //
    avtView3D view3D=visWindow->GetView3D();

    //
    // Determine the zoom factor.
    //
    double    width;
    double    zoomFactor;

    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));
    zoomFactor = width / view3D.parallelScale;

    //
    // Set the new window.
    //
    int       i;

    for (i = 0; i < 6; i++)
    {
        boundingBox3d[i] = limits[i];
    }

    //
    // Calculate the new focal point.
    //
    view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2.;
    view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2.;
    view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2.;

    //
    // Calculate the new parallel scale.
    //
    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));

    view3D.parallelScale = width / zoomFactor;

    //
    // Calculate the near and far clipping planes.
    //
    view3D.nearPlane = - 2.0 * width;
    view3D.farPlane  =   2.0 * width;

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::RecenterViewCurve
//
//  Purpose: 
//    Recenter the window's Curve view using the specified limits.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 17:28:31 PDT 2002
//    Use the scale factor when setting the window.
//
//    Kathleen Bonnell, Thu Apr 24 17:17:21 PDT 2003 
//    Make the bbox square when it is reported as 1D.
//
// ****************************************************************************

void
ViewerWindow::RecenterViewCurve(const double *limits)
{
    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (limits[0] == DBL_MAX && limits[1] == -DBL_MAX)
    {
        centeringValid2d = false;
        return;
    }

    //
    // Set the new window.
    //
    int       i;

    for (i = 0; i < 4; i++)
    {
        boundingBox2d[i] = limits[i];
    }

    if (boundingBox2d[2] == boundingBox2d[3])
    {
        double fudge = (boundingBox2d[1] - boundingBox2d[0] ) * 0.5;
        boundingBox2d[2] -= fudge;
        boundingBox2d[3] += fudge;
    }
    else if (boundingBox2d[0] == boundingBox2d[1])
    {
        double fudge = (boundingBox2d[3] - boundingBox2d[2] ) * 0.5;
        boundingBox2d[0] -= fudge;
        boundingBox2d[1] += fudge;
    }

    //
    // Update the view.
    //
    avtViewCurve viewCurve=visWindow->GetViewCurve();
    viewCurve.window[0] = boundingBox2d[0];
    viewCurve.window[1] = boundingBox2d[1];
    viewCurve.window[2] = boundingBox2d[2]*viewCurve.yScale;
    viewCurve.window[3] = boundingBox2d[3]*viewCurve.yScale;

    visWindow->SetViewCurve(viewCurve);
}

// ****************************************************************************
//  Method: ViewerWindow::ResetView2d
//
//  Purpose: 
//    Reset the window's 2d view.
//
//  Notes:
//    We do not reset the window.  That would be an annoying feature.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
//    Eric Brugger, Thu Oct 25 13:51:53 PDT 2001
//    I modified the routine to also reset the bounding box based on the
//    plot limits.
//
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
//    Brad Whitlock, Mon Sep 23 12:52:17 PDT 2002
//    I made it use the new GetExtents method.
//
//    Eric Brugger, Tue Jan 14 07:59:31 PST 2003
//    I added the number of dimensions to the GetExtents call.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I modified the routine to set the viewModified2d flag.
//
//    Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003   
//    Create a square window by scaling, if in fullFrame mode. 
//
// ****************************************************************************

void
ViewerWindow::ResetView2d()
{
    avtView2D view2D=visWindow->GetView2D();

    //
    // Set the bounding box based on the plot limits.
    //
    GetExtents(2, boundingBox2d);

    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (boundingBox2d[0] == DBL_MAX && boundingBox2d[1] == -DBL_MAX)
    {
        boundingBoxValid2d = false;
        return;
    }

    //
    // Set the window.
    //
    view2D.window[0]   = boundingBox2d[0];
    view2D.window[1]   = boundingBox2d[1];
    view2D.window[2]   = boundingBox2d[2];
    view2D.window[3]   = boundingBox2d[3];
    if (fullFrame)
    { 
        Compute2DScaleFactor(view2D.axisScaleFactor, view2D.axisScaleType);
        if (view2D.axisScaleFactor != 0.)
        {
            if (view2D.axisScaleType == 0) // requires x-axis scaling
            {
                view2D.window[0] *= view2D.axisScaleFactor;
                view2D.window[1] *= view2D.axisScaleFactor;
            }
            else // requires y-axis scaling
            {
                view2D.window[2] *= view2D.axisScaleFactor;
                view2D.window[3] *= view2D.axisScaleFactor;
            }
        }
    }
    else
    {
        view2D.axisScaleFactor = 0.;
    }

    visWindow->SetView2D(view2D);

    //
    // Flag the view as unmodified.
    //
    viewModified2d = false;
}

// ****************************************************************************
//  Method: ViewerWindow::ResetView3d
//
//  Purpose: 
//    Reset the window's 3d view.
//
//  Notes:
//    We do not reset the perspective.  That would be an annoying feature.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Wed Aug  1 07:55:14 PDT 2001
//    I fixed a lot of miscellaneous problems with the setting of the view.
//    These problems came about from matching what VTK was setting for its
//    initial view.
//
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//
//    Eric Brugger, Thu Oct 25 13:51:53 PDT 2001
//    I modified the routine to also reset the bounding box based on the
//    plot limits.
//
//    Eric Brugger, Tue Apr  2 11:35:29 PST 2002
//    I modified the setting of the clip planes since the definition of the
//    clip plane distance was now from the focal point instead of the camera.
//    I also increased the volume between the clip planes so that the view
//    was less likely to be clipped.
//
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
//    Brad Whitlock, Mon Sep 23 12:55:03 PDT 2002
//    I made it use the new GetExtents method.
//
//    Eric Brugger, Tue Jan 14 07:59:31 PST 2003
//    I added the number of dimensions to the GetExtents call.
//
//    Eric Brugger, Tue Jun 10 13:08:55 PDT 2003
//    I added image pan and image zoom to the 3d view attributes.
//
// ****************************************************************************

void
ViewerWindow::ResetView3d()
{
    avtView3D view3D=visWindow->GetView3D();

    //
    // Set the bounding box based on the plot limits.
    //
    GetExtents(3, boundingBox3d);

    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (boundingBox3d[0] == DBL_MAX && boundingBox3d[1] == -DBL_MAX)
    {
        boundingBoxValid3d = false;
        return;
    }

    //
    // Set the scale.  They are choosen such that the object should fit
    // within a square window no matter the orientation when doing an
    // orthographic projection.  The parallelScale controls the scale
    // with orthographic projections, whereas the distance controls the
    // scale with perspective projections.
    //
    double    width;
    double    distance;

    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));

    view3D.viewAngle = 30.;
    distance = width / tan (view3D.viewAngle * 3.1415926535 / 360.);

    view3D.parallelScale = width;

    //
    // Set the view up vector, the focal point and the camera position.
    //
    view3D.normal[0] = 0.;
    view3D.normal[1] = 0.;
    view3D.normal[2] = 1.;

    view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2.;
    view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2.;
    view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2.;

    view3D.viewUp[0] = 0.;
    view3D.viewUp[1] = 1.;
    view3D.viewUp[2] = 0.;

    //
    // Calculate the near and far clipping planes.  The clipping planes are
    // set such that the object should not get clipped in the front or
    // back no matter the orientation when doing an orthographic projection.
    //
    view3D.nearPlane = - 2.0 * width;
    view3D.farPlane  =   2.0 * width;

    //
    // Reset the image pan and image zoom.
    //
    view3D.imagePan[0] = 0.;
    view3D.imagePan[1] = 0.;
    view3D.imageZoom = 1.;

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::ResetViewCurve
//
//  Purpose: 
//    Reset the window's Curve view.
//
//  Notes:
//    We do not reset the window.  That would be an annoying feature.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002. 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 17:28:31 PDT 2002
//    Use the scale factor when setting the window.
//
//    Brad Whitlock, Mon Sep 23 12:55:33 PDT 2002
//    I made it use the new GetExtents method.
//
//    Eric Brugger, Tue Jan 14 07:59:31 PST 2003
//    I added the number of dimensions to the GetExtents call.
//
//    Kathleen Bonnell, Thu Apr 24 17:17:21 PDT 2003 
//    Create a square bounding box when it is reported as 1D.
//
// ****************************************************************************

void
ViewerWindow::ResetViewCurve()
{
    avtViewCurve viewCurve=visWindow->GetViewCurve();

    //
    // Set the bounding box based on the plot limits.
    //
    GetExtents(2, boundingBox2d);

    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (boundingBox2d[0] == DBL_MAX && boundingBox2d[1] == -DBL_MAX)
    {
        boundingBoxValid2d = false;
        return;
    }

    if (boundingBox2d[2] == boundingBox2d[3])
    {
        double fudge = (boundingBox2d[1] - boundingBox2d[0]) * 0.5;
        boundingBox2d[2] -= fudge;
        boundingBox2d[3] += fudge;
    }
    else if (boundingBox2d[0] == boundingBox2d[1])
    {
        double fudge = (boundingBox2d[3] - boundingBox2d[2]) * 0.5;
        boundingBox2d[0] -= fudge;
        boundingBox2d[1] += fudge;
    }

    //
    // Set the window.
    //
    viewCurve.window[0]   = boundingBox2d[0];
    viewCurve.window[1]   = boundingBox2d[1];
    viewCurve.window[2]   = boundingBox2d[2]*viewCurve.yScale;
    viewCurve.window[3]   = boundingBox2d[3]*viewCurve.yScale;

    visWindow->SetViewCurve(viewCurve);
}

// ****************************************************************************
//  Method: ViewerWindow::AdjustView3d
//
//  Purpose: 
//    Adjust the window's 3d view using the specified limits.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Eric Brugger
//  Creation:   May 9, 2003
//
// ****************************************************************************

void
ViewerWindow::AdjustView3d(const double *limits)
{
    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (limits[0] == DBL_MAX && limits[1] == -DBL_MAX)
    {
        centeringValid3d = false;
        return;
    }

    //
    // Get the current view.
    //
    avtView3D view3D=visWindow->GetView3D();

    //
    // Determine the zoom factor.
    //
    double    width;
    double    zoomFactor;

    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));
    zoomFactor = width / view3D.parallelScale;

    //
    // Determine the pan factor.
    //
    double panFactor[3];
 
    panFactor[0] = - (((boundingBox3d[1] + boundingBox3d[0]) / 2.) -
                   view3D.focus[0]) / (boundingBox3d[1] - boundingBox3d[0]);
    panFactor[1] = - (((boundingBox3d[3] + boundingBox3d[2]) / 2.) -
                   view3D.focus[1]) / (boundingBox3d[3] - boundingBox3d[2]);
    panFactor[2] = - (((boundingBox3d[5] + boundingBox3d[4]) / 2.) -
                   view3D.focus[2]) / (boundingBox3d[5] - boundingBox3d[4]);

    //
    // Set the new window.
    //
    int       i;

    for (i = 0; i < 6; i++)
    {
        boundingBox3d[i] = limits[i];
    }

    //
    // Calculate the new focal point.
    //
    view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2. +
                      panFactor[0] * (boundingBox3d[1] - boundingBox3d[0]);
    view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2. +
                      panFactor[1] * (boundingBox3d[3] - boundingBox3d[2]);
    view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2. +
                      panFactor[2] * (boundingBox3d[5] - boundingBox3d[4]);

    //
    // Calculate the new parallel scale.
    //
    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));

    view3D.parallelScale = width / zoomFactor;

    //
    // Calculate the near and far clipping planes.
    //
    view3D.nearPlane = - 2.0 * width;
    view3D.farPlane  =   2.0 * width;

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::UpdateView2d
//
//  Purpose: 
//    Update the 2d view for the window.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//   
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
//    Hank Childs, Thu Jul 18 15:04:20 PDT 2002
//    Don't be so aggressive about reseting the view.  The old test (whether
//    or not the bbox is valid) is not as applicable because the actual vs
//    original spatial extents can invalidate bbox extents.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I replaced autoCenter with maintainView.  I added logic to deal with
//    viewModified2d and mergeViewLimits.
//
//    Eric Brugger, Fri May  9 14:11:48 PDT 2003
//    I modified the routine to calculate the merged limits in a scratch
//    array instead of using a boundingBox2d which messed things up.
//
// ****************************************************************************

void
ViewerWindow::UpdateView2d(const double *limits)
{
    //
    // If this is the first time that this routine is being called for this
    // window, set the limits and reset the view.
    //
    if (boundingBoxValid2d == false)
    {
        for (int i = 0; i < 4; i++)
            boundingBox2d[i] = limits[i];

        boundingBoxValid2d = true;
        centeringValid2d   = true;

        if (!haveRenderedIn2d)
        {
            ResetView2d();
        }

        ViewerWindowManager::Instance()->UpdateViewAtts();
    }
    //
    // If the centering is invalid or if maintain view is off, the view has
    // not been modified and the limits have changed, recenter the view.
    // The recentering uses the current limits or the merged limits from
    // the previous plots based on the mergeViewLimits flag.
    //
    else if (centeringValid2d == false ||
             maintainView == false && viewModified2d == false &&
             (limits[0] != boundingBox2d[0] || limits[1] != boundingBox2d[1] ||
              limits[2] != boundingBox2d[2] || limits[3] != boundingBox2d[3]))
    {
        if (centeringValid2d == true && mergeViewLimits == true)
        {
            double mergedLimits[4];

            mergedLimits[0] = boundingBox2d[0] < limits[0] ?
                              boundingBox2d[0] : limits[0];
            mergedLimits[1] = boundingBox2d[1] > limits[1] ?
                              boundingBox2d[1] : limits[1];
            mergedLimits[2] = boundingBox2d[2] < limits[2] ?
                              boundingBox2d[2] : limits[2];
            mergedLimits[3] = boundingBox2d[3] > limits[3] ?
                              boundingBox2d[3] : limits[3];

            RecenterView2d(mergedLimits);
        }
        else
        {
            centeringValid2d   = true;

            RecenterView2d(limits);
        }

        ViewerWindowManager::Instance()->UpdateViewAtts();
    }

    haveRenderedIn2d = true;
}

// ****************************************************************************
//  Method: ViewerWindow::UpdateView3d
//
//  Purpose: 
//    Update the 3d view for the window.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:01:02 PDT 2001
//    I modified the way the window handles view information.
//   
//    Eric Brugger, Mon Apr 15 13:15:09 PDT 2002
//    I modified the routine to handle the case where there are no plots in
//    the window.
//
//    Hank Childs, Thu Jul 18 15:04:20 PDT 2002
//    Don't be so aggressive about reseting the view.  The old test (whether
//    or not the bbox is valid) is not as applicable because the actual vs
//    original spatial extents can invalidate bbox extents.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I replaced autoCenter with maintainView.  I added logic to deal with
//    mergeViewLimits.
//
//    Eric Brugger, Fri May  9 14:11:48 PDT 2003
//    I changed a couple of calls of RecenterView3d to AdjustView3d.  I
//    modified the routine to calculate the merged limits in a scratch array
//    instead of using a boundingBox3d which messed things up.
//
// ****************************************************************************

void
ViewerWindow::UpdateView3d(const double *limits)
{
    //
    // If this is the first time that this routine is being called for this
    // window, set the limits and reset the view.
    //
    if (boundingBoxValid3d == false)
    {
        for (int i = 0; i < 6; i++)
            boundingBox3d[i] = limits[i];

        boundingBoxValid3d = true;
        centeringValid3d   = true;

        if (!haveRenderedIn3d)
        {
            ResetView3d();
        }

        ViewerWindowManager::Instance()->UpdateViewAtts();
    }
    //
    // If the centering is invalid or if maintain view is off and the limits
    // have changed, recenter the view.  The recentering uses the current
    // limits or the merged limits from the previous plots based on the
    // mergeViewLimits flag.
    //
    else if (centeringValid3d == false || maintainView == false &&
             (limits[0] != boundingBox3d[0] || limits[1] != boundingBox3d[1] ||
              limits[2] != boundingBox3d[2] || limits[3] != boundingBox3d[3] ||
              limits[4] != boundingBox3d[4] || limits[5] != boundingBox3d[5]))
    {
        if (centeringValid3d == true && mergeViewLimits == true)
        {
            double mergedLimits[6];

            mergedLimits[0] = boundingBox3d[0] < limits[0] ?
                              boundingBox3d[0] : limits[0];
            mergedLimits[1] = boundingBox3d[1] > limits[1] ?
                              boundingBox3d[1] : limits[1];
            mergedLimits[2] = boundingBox3d[2] < limits[2] ?
                              boundingBox3d[2] : limits[2];
            mergedLimits[3] = boundingBox3d[3] > limits[3] ?
                              boundingBox3d[3] : limits[3];
            mergedLimits[4] = boundingBox3d[4] < limits[4] ?
                              boundingBox3d[4] : limits[4];
            mergedLimits[5] = boundingBox3d[5] > limits[5] ?
                              boundingBox3d[5] : limits[5];

            AdjustView3d(mergedLimits);
        }
        else
        {
            centeringValid3d   = true;

            AdjustView3d(limits);
        }

        ViewerWindowManager::Instance()->UpdateViewAtts();
    }

    haveRenderedIn3d = true;
}

// ****************************************************************************
//  Method: ViewerWindow::UpdateViewCurve
//
//  Purpose: 
//    Update the Curve view for the window.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002 
//
//  Modifications:
//
//    Hank Childs, Thu Jul 18 15:04:20 PDT 2002
//    Don't be so aggressive about reseting the view.  The old test (whether
//    or not the bbox is valid) is not as applicable because the actual vs
//    original spatial extents can invalidate bbox extents.
//
//    Eric Brugger, Fri Apr 18 12:21:13 PDT 2003
//    I replaced autoCenter with maintainView.  I added logic to deal with
//    viewModified2d and mergeViewLimits.
//
// ****************************************************************************

void
ViewerWindow::UpdateViewCurve(const double *limits)
{
    //
    // If this is the first time that this routine is being called for this
    // window, set the limits and reset the view.
    //
    if (boundingBoxValid2d == false)
    {
        for (int i = 0; i < 4; i++)
            boundingBox2d[i] = limits[i];

        boundingBoxValid2d = true;
        centeringValid2d   = true;

        if (!haveRenderedIn2d)
        {
            ResetViewCurve();
        }

        ViewerWindowManager::Instance()->UpdateViewAtts();
    }
    //
    // If the centering is invalid or if maintain view is off, the view has
    // not been modified and the limits have changed, recenter the view.
    // The recentering uses the current limits or the merged limits from
    // the previous plots based on the mergeViewLimits flag.
    //
    else if (centeringValid2d == false ||
             maintainView == false && viewModified2d == false &&
             (limits[0] != boundingBox2d[0] || limits[1] != boundingBox2d[1] ||
              limits[2] != boundingBox2d[2] || limits[3] != boundingBox2d[3]))
    {
        if (centeringValid2d == true && mergeViewLimits == true)
        {
            boundingBox2d[0] = boundingBox2d[0] < limits[0] ?
                               boundingBox2d[0] : limits[0];
            boundingBox2d[1] = boundingBox2d[1] > limits[1] ?
                               boundingBox2d[1] : limits[1];
            boundingBox2d[2] = boundingBox2d[2] < limits[2] ?
                               boundingBox2d[2] : limits[2];
            boundingBox2d[3] = boundingBox2d[3] > limits[3] ?
                               boundingBox2d[3] : limits[3];

            RecenterView2d(boundingBox2d);
        }
        else
        {
            centeringValid2d   = true;

            RecenterView2d(limits);
        }

        ViewerWindowManager::Instance()->UpdateViewAtts();
    }

    haveRenderedIn2d = true;
}


// ****************************************************************************
//  Method: ViewerWindow::ShowMenuCallback
//
//  Purpose: 
//    This is a static method that is passed to VisWindow as a callback
//    function. When the right mouse button is pressed, this method is
//    called by the VisWindow's interactor.
//
//  Arguments:
//    data      A pointer to the ViewerWindow that owns the VisWindow that
//              invoked this call.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:22:48 PDT 2000
//
// ****************************************************************************

void ViewerWindow::ShowMenuCallback(void *data)
{
    if(data == 0)
        return;

    ViewerWindow *win = (ViewerWindow *)data;
    win->ShowMenu();
}

// ****************************************************************************
//  Method: ViewerWindow::HideMenuCallback
//
//  Purpose: 
//    This is a static method that is passed to VisWindow as a callback
//    function. When the right mouse button is released, this method is
//    called by the VisWindow's interactor.
//
//  Arguments:
//    data      A pointer to the ViewerWindow that owns the VisWindow that
//              invoked this call.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 11:22:48 PDT 2000
//
// ****************************************************************************

void
ViewerWindow::HideMenuCallback(void *data)
{
    if(data == 0)
        return;

    ViewerWindow *win = (ViewerWindow *)data;
    win->HideMenu();
}

// ****************************************************************************
// Method: ViewerWindow::CloseCallback
//
// Purpose: 
//   This is a static method that is passed to the VisWindow as a callback
//   function to be called when the VisWindow is destroyed from the window
//   manager. It sends a message to the rendering thread to delay when the
//   window is actually destroyed.
//
// Arguments:
//   data : A pointer to the ViewerWindow.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 11:53:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Jul 24 15:21:06 PST 2002
//   I made animation stop for the window being deleted so that the
//   ViewerWindowManager does not try to animate the window before its
//   delete message can be processed.
//
// ****************************************************************************

void
ViewerWindow::CloseCallback(void *data)
{
    if(data == 0)
        return;

    //
    // Send a message to the rendering thread to delete this window.
    //
    ViewerWindow *win = (ViewerWindow *)data;
    ViewerWindowManager::Instance()->StopTimer();
    win->SendDeleteMessage();
}

// ****************************************************************************
// Method: ViewerWindow::HideCallback
//
// Purpose: 
//   This callback function is called when the window is hidden.
//
// Arguments:
//   data : A pointer to the ViewerWindow.
//
// Note:       We set the visible flag to false and then update the animation
//             timer so that the window will not be considered for animation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:23:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::HideCallback(void *data)
{
    ViewerWindow *win = (ViewerWindow *)data;
    win->SetVisible(false);
    ViewerWindowManager::Instance()->UpdateAnimationTimer();
}

// ****************************************************************************
// Method: ViewerWindow::ShowCallback
//
// Purpose: 
//   This callback function is called when the window is shown.
//
// Arguments:
//   data : A pointer to the ViewerWindow.
//
// Note:       We set the visible flag to true and then update the animation
//             timer so that the window will be considered for animation if
//             its animation is playing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:23:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::ShowCallback(void *data)
{
    ViewerWindow *win = (ViewerWindow *)data;
    win->SetVisible(true);
    ViewerWindowManager::Instance()->UpdateAnimationTimer();
}

// ****************************************************************************
// Method: ViewerWindow::CreateToolbar
//
// Purpose: 
//   Tells the vis window to create a new toolbar widget and return a pointer
//   to it so that we can use it.
//
// Arguments:
//   name : The name of the new toolbar to create.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 11:56:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void *
ViewerWindow::CreateToolbar(const std::string &name)
{
    return visWindow->CreateToolbar(name.c_str());
}

// ****************************************************************************
// Method: ViewerWindow::GetWindowAttributes
//
// Purpose: 
//   Returns a WindowAttributes object.
//
// Returns:    A WindowAttributes object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 5 11:37:19 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002  
//   Support avtViewCurve. 
//
//   Mark C. Miller, Mon Dec  9 19:18:40 PST 2002
//   Added calls to fill in rendering attributes 
//
//   Eric Brugger, Tue Jun 10 13:08:55 PDT 2003
//   I renamed camera to view normal in the view attributes.  I added
//   image pan and image zoom to the 3d view attributes.
//
//   Brad Whitlock, Tue Jul 1 14:07:52 PST 2003
//   I used new convenience methods for setting the viewAtts with the avt
//   view objects.
//
// ****************************************************************************

WindowAttributes
ViewerWindow::GetWindowAttributes() const
{
    WindowAttributes winAtts;

    //
    // Set the view
    //
    ViewAttributes viewAtts;
    if (viewDimension == 2)
    {
        if (!GetTypeIsCurve())
        {
            const avtView2D &view2d = GetView2D();
            view2d.SetToViewAttributes(&viewAtts);
        }
        else 
        {
            const avtViewCurve &viewCurve = GetViewCurve();
            viewCurve.SetToViewAttributes(&viewAtts);
        }
    }
    else
    {
        const avtView3D &view3d = GetView3D();
        view3d.SetToViewAttributes(&viewAtts);
    }
    winAtts.SetView(viewAtts);

    //
    // Set the size
    //
    int size[2];
    visWindow->GetSize(size[0], size[1]);
    winAtts.SetSize(size);

    //
    // Set the background colors
    //
    const AnnotationAttributes *annot = GetAnnotationAttributes();
    unsigned char c[3];
    c[0] = (unsigned char)(annot->GetBackgroundColor().Red());
    c[1] = (unsigned char)(annot->GetBackgroundColor().Green());
    c[2] = (unsigned char)(annot->GetBackgroundColor().Blue());
    winAtts.SetBackground(c);
    c[0] = (unsigned char)(annot->GetForegroundColor().Red());
    c[1] = (unsigned char)(annot->GetForegroundColor().Green());
    c[2] = (unsigned char)(annot->GetForegroundColor().Blue());
    winAtts.SetForeground(c);
    float gbg[3];
    gbg[0] = float(annot->GetGradientColor1().Red()) / 255.;
    gbg[1] = float(annot->GetGradientColor1().Green()) / 255.;
    gbg[2] = float(annot->GetGradientColor1().Blue()) / 255.;
    winAtts.SetGradBG1(gbg);
    gbg[0] = float(annot->GetGradientColor2().Red()) / 255.;
    gbg[1] = float(annot->GetGradientColor2().Green()) / 255.;
    gbg[2] = float(annot->GetGradientColor2().Blue()) / 255.;
    winAtts.SetGradBG2(gbg);
    int bgMode;
    if(annot->GetBackgroundMode() == 0)
        bgMode = 0;
    else
        bgMode = annot->GetGradientBackgroundStyle() + 1;
    winAtts.SetBackgroundMode(bgMode);

    // Relevant rendering attributes (only those for scalable renderer)
    RenderingAttributes renderAtts;
    renderAtts.SetScalableRendering(GetScalableRendering());
    renderAtts.SetScalableThreshold(GetScalableThreshold());
    winAtts.SetRenderAtts(renderAtts);

    //
    // Set the color tables.
    //
    winAtts.SetColorTables(*avtColorTables::Instance()->GetColorTables());

    //
    // Set light lists
    //
    winAtts.SetLights(*GetLightList());

    return winAtts;
}

// ****************************************************************************
// Method: ViewerWindow::Pick
//
// Purpose: 
//   Adds a pick point (specified in screen space) to the window.
//
// Arguments:
//   x : The screen x location of the pick point.
//   y : The screen y location of the pick point.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 15:32:38 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Jun 25 16:55:18 PDT 2003
//   Added pickMode argument, so that correct interaction mode is set
//   before picking. 
//   
// ****************************************************************************

void
ViewerWindow::Pick(int x, int y, const INTERACTION_MODE pickMode)
{
    if(GetTypeIsCurve())
        Error("Curve windows cannot be picked for values.");
    else
    {
        // Set the interaction mode to pick.
        INTERACTION_MODE iMode = visWindow->GetInteractionMode();
        SetInteractionMode(pickMode);

        // Perform a screen space pick operation.
        visWindow->Pick(x, y);

        // Restore the interaction mode.
        SetInteractionMode(iMode);
    }
}

// ****************************************************************************
//  Method: ViewerWindow::PerformPickCallback
//
//  Purpose: 
//    Perform pick. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 20, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002   
//    Retrieve domain from pickpoint info.
//
//    Kathleen Bonnell, Fri Jun 28 09:15:57 PDT 2002 
//    PickLetter in PickAttributes is now a string. 
//
//    Kathleen Bonnell, Tue Jul 30 14:44:37 PDT 2002 
//    Set dimension in pickAtts. 
//
//    Kathleen Bonnell, Fri Aug 16 11:52:18 PDT 2002     
//    Intialize pick.Fulfilled to false before sending pick request
//    to the plot list. 
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Moved bulk of code to ViewerQueryManager. 
//
// ****************************************************************************

void
ViewerWindow::PerformPickCallback(void *data)
{
    if(data == 0)
        return;
    // 
    // Let the Query manager handle this.
    //
    ViewerQueryManager::Instance()->Pick((PICK_POINT_INFO*)data);
}


// ****************************************************************************
//  Method: ViewerWindow::ClearPickPoints
//
//  Purpose: 
//    Clear the Pick points from the window. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 27, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 10:43:23 PST 2002
//    Added code to clear the pick Window.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Let ViewerQueryManager handle clearing the PickWindow. 
//
// ****************************************************************************

void
ViewerWindow::ClearPickPoints()
{
    visWindow->ClearPickPoints();
    ViewerQueryManager::Instance()->ClearPickPoints();
}


// ****************************************************************************
// Method: ViewerWindow::IsTheSameWindow
//
// Purpose:
//     Determines if we have the same window without violating encapsulation.
//
// Arguments:
//     vw      A visualization window.
//
// Returns:    true if they are the same, false otherwise.
//
// Programmer: Hank Childs
// Creation:   March 25, 2002
//
// ****************************************************************************

bool
ViewerWindow::IsTheSameWindow(VisWindow *vw)
{
    return (vw == visWindow);
}

// ****************************************************************************
//  Method: ViewerWindow::ClearRefLines
//
//  Purpose: 
//    Clear the Ref lines from the window. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 14, 2002 
//
// ****************************************************************************

void
ViewerWindow::ClearRefLines()
{
    visWindow->ClearRefLines();
}


// ****************************************************************************
//  Method: ViewerWindow::PerformLineoutCallback 
//
//  Purpose: 
//    Perform lineout. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 14, 2002. 
//
//  Modifications:
//    Kathleen Bonnell, Wed May 15 16:40:35 PDT 2002'
//    Check for invalid variable and send error message if necessary.
//
//    Kathleen Bonnell, Wed May 29 11:19:49 PDT 2002  
//    Pass window to ViewerWindowManager method, insted of dbName and vName. 
//
//    Kathleen Bonnell, Mon Jun 10 17:22:57 PDT 2002  
//    Moved functionality from ViewerWindowManager to ViewerQueryManager. 
//
//    Kathleen Bonnell, Fri Jul 26 16:52:47 PDT 2002 
//    Removed references to YScale, not used any more. 
//
//    Kathleen Bonnell, Fri Jun  6 16:42:06 PDT 2003 
//    Changed call from AddQuery to Lineout. 
//
// ****************************************************************************

void
ViewerWindow::PerformLineoutCallback(void *data)
{
    if(data == 0)
        return;

    LINE_OUT_INFO *loData = (LINE_OUT_INFO *)data;

    ViewerWindow *win = (ViewerWindow *)loData->callbackData;
    ViewerQueryManager::Instance()->Lineout(win, &loData->atts);
}


// ****************************************************************************
//  Method: ViewerWindow::ValidateQuery
//
//  Purpose: 
//    Notifies the vis window that the query was valid. 
//
//  Arguments:
//    designator   Specifies which query is to be validated.
//    lineAtts     Attributes for lineout queries. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002
//    Changed argument to Line* to convey more information than just color.
//
//    Kathleen Bonnell, Thu Dec 19 16:52:45 PST 2002  
//    Added argument designator. 
//
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003  
//    Removed argument designator, added PickAtts argument. 
//
// ****************************************************************************

void
ViewerWindow::ValidateQuery(const PickAttributes *pa, const Line *lineAtts)
{
    visWindow->QueryIsValid(pa, lineAtts);
}



// ****************************************************************************
//  Method: ViewerWindow::SetTypeIsCurve
//
//  Purpose: 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 17, 2002 
//
// ****************************************************************************

void
ViewerWindow::SetTypeIsCurve(const bool val)
{
    visWindow->SetTypeIsCurve(val);
}



// ****************************************************************************
//  Method: ViewerWindow::GetTypeIsCurve
//
//  Purpose: 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 17, 2002 
//
// ****************************************************************************

bool
ViewerWindow::GetTypeIsCurve() const 
{
    return visWindow->GetTypeIsCurve();
}


// ****************************************************************************
//  Method: ViewerWindow::UpdateQuery
//
//  Purpose:
//    Notifies the vis window that the query should be updated. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 10, 2002
//
// ****************************************************************************
 
void
ViewerWindow::UpdateQuery(const Line *lineAtts)
{
    visWindow->UpdateQuery(lineAtts);
}


// ****************************************************************************
//  Method: ViewerWindow::DeleteQuery
//
//  Purpose:
//    Notifies the vis window that the query should be deleted. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 10, 2002
//
// ****************************************************************************
 
void
ViewerWindow::DeleteQuery(const Line *lineAtts)
{
    visWindow->DeleteQuery(lineAtts);
}


// ****************************************************************************
//  Method: ViewerWindow::ScalePlots
//
//  Purpose:
//    Notifies the vis window that the plots should be scaled. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002 
//
// ****************************************************************************
 
void
ViewerWindow::ScalePlots(const float v[3])
{
    visWindow->ScalePlots(v);
}

// ****************************************************************************
// Method: ViewerWindow::SetAntialiasing
//
// Purpose: 
//   Sets the window's AA mode.
//
// Arguments:
//   enabled : Whether or not AA is enabled.
//   frames  : The number of frames to use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:38:31 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  4 17:38:27 PST 2002
//   Removed frames argument, no longer needed.
//   
// ****************************************************************************

void
ViewerWindow::SetAntialiasing(bool enabled)
{
    visWindow->SetAntialiasing(enabled);
}

// ****************************************************************************
// Method: ViewerWindow::GetAntialiasing
//
// Purpose: 
//   Returns the window's AA mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:39:11 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetAntialiasing() const
{
    return visWindow->GetAntialiasing();
}


// ****************************************************************************
// Method: ViewerWindow::GetRenderTimes
//
// Purpose: 
//   Gets the render times.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:39:41 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::GetRenderTimes(float times[3]) const
{
    visWindow->GetRenderTimes(times);
}

// ****************************************************************************
// Method: ViewerWindow::SetStereoRendering
//
// Purpose: 
//   Sets the window's stereo rendering mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:39:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetStereoRendering(bool enabled, int type)
{
    visWindow->SetStereoRendering(enabled, type);
}

// ****************************************************************************
// Method: ViewerWindow::GetStereo
//
// Purpose: 
//   Returns whether or not the window is rendering in stereo.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:40:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetStereo() const
{
    return visWindow->GetStereo();
}

// ****************************************************************************
// Method: ViewerWindow::GetStereoType
//
// Purpose: 
//   Returns the window's stereo type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:40:44 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
ViewerWindow::GetStereoType() const
{
    return visWindow->GetStereoType();
}

// ****************************************************************************
// Method: ViewerWindow::SetImmediateModeRendering
//
// Purpose: 
//   Tells the window whether or not to use immediate mode rendering.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:41:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetImmediateModeRendering(bool mode)
{
    visWindow->SetImmediateModeRendering(mode);
}

// ****************************************************************************
// Method: ViewerWindow::GetImmediateModeRendering
//
// Purpose: 
//   Returns the window's immediate mode rendering flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:41:21 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetImmediateModeRendering() const
{
    return visWindow->GetImmediateModeRendering();
}

// ****************************************************************************
// Method: ViewerWindow::SetSurfaceRepresentation
//
// Purpose: 
//   Sets the window's surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:41:42 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetSurfaceRepresentation(int rep)
{
    visWindow->SetSurfaceRepresentation(rep);
}

// ****************************************************************************
// Method: ViewerWindow::GetSurfaceRepresentation
//
// Purpose: 
//   Returns the window's surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:41:59 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
ViewerWindow::GetSurfaceRepresentation() const
{
    return visWindow->GetSurfaceRepresentation();
}

// ****************************************************************************
// Method: ViewerWindow::GetNumTriangles
//
// Purpose: 
//   Gets the number of triangles rendered by the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:42:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
ViewerWindow::GetNumTriangles() const
{
    return visWindow->GetNumTriangles();
}

// ****************************************************************************
// Method: ViewerWindow::SetNotifyForEachRender
//
// Purpose: 
//   Tells the window whether or not it should tell the client about the
//   rendering information after each render.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:42:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetNotifyForEachRender(bool val)
{
    visWindow->SetNotifyForEachRender(val);
}

// ****************************************************************************
// Method: ViewerWindow::GetNotifyForEachRender
//
// Purpose: 
//   Gets the window's rendering information notification flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:43:05 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetNotifyForEachRender() const
{
    return visWindow->GetNotifyForEachRender();
}

// ****************************************************************************
// Method: ViewerWindow::SetScalableRendering
//
// Purpose: 
//   Enables or disables scalable rendering mode 
//
// Arguments:
//   mode   : Whether scalable rendering is on or off.
//   update : Whether we're allowing an update of the window.
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// Modifications:
//   Brad Whitlock, Tue Jul 8 10:48:25 PDT 2003
//   I added an optional update flag that can be used to prevent the window
//   from being updated when we're just setting window attributes.
//
// ****************************************************************************

void
ViewerWindow::SetScalableRendering(bool mode, bool update)
{
    bool updatesEnabled = UpdatesEnabled();
    bool differentModes = (GetScalableRendering() != mode);

    visWindow->SetScalableRendering(mode);

    //
    // Update the window
    //
    if(differentModes && update)
    {
        DisableUpdates();
        animation->GetPlotList()->ClearPlots();
        animation->GetPlotList()->RealizePlots();
        if (updatesEnabled)
            EnableUpdates();
    }
}

// ****************************************************************************
// Method: ViewerWindow::GetScalableRendering
//
// Purpose: 
//   returns boolean indicating if scalable rendering is enabled or not 
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetScalableRendering() const
{
    return visWindow->GetScalableRendering();
}

// ****************************************************************************
// Method: ViewerWindow::SetScalableThreshold
//
// Purpose: 
//   Sets scalable rendering threshold (and modality) 
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetScalableThreshold(int threshold)
{
    visWindow->SetScalableThreshold(threshold);
}

// ****************************************************************************
// Method: ViewerWindow::GetScalableThreshold
//
// Purpose: 
//   returns current scalable rendering threshold (and modality)
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

int
ViewerWindow::GetScalableThreshold() const
{
    return visWindow->GetScalableThreshold();
}

// ****************************************************************************
// Method: ViewerWindow::SetPopupEnabled
//
// Purpose: 
//   Sets the enabled state for the popup menu.
//
// Arguments:
//   val : The enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 15:22:13 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 4 15:45:44 PST 2003
//   I removed the call to UpdateMenu since it no longer exists.
//
// ****************************************************************************

void
ViewerWindow::SetPopupEnabled(bool val)
{
    popupMenu->SetEnabled(val);
}

// ****************************************************************************
// Method: ViewerWindow::CreateNode
//
// Purpose: 
//   Lets the window save its information to a config file's DataNode.
//
// Arguments:
//   parentNode : The node to which we're saving information.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 13:10:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *windowNode = new DataNode("ViewerWindow");
    parentNode->AddNode(windowNode);

    //
    // Add information specific to the ViewerWindow.
    //

    //
    // Let other objects add their information.
    //
    actionMgr->CreateNode(windowNode);
}

// ****************************************************************************
// Method: ViewerWindow::SetFromNode
//
// Purpose: 
//   Lets the window reset its values from a config file.
//
// Arguments:
//   parentNode : The config file information DataNode pointer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 13:11:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *windowNode = parentNode->GetNode("ViewerWindow");
    if(windowNode == 0)
        return;

    //
    // Get information specific to ViewerWindow.
    //

    //
    // Let other objects get their information.
    //
    actionMgr->SetFromNode(windowNode);
}


// Only place where ViewerWindow should need to talk to ViewerEngineManager
#include <ViewerEngineManager.h>

// ****************************************************************************
// Function: ExternalRenderCallback 
//
// Note that the external render request can respond to the caller with
// one of three responses...
//
//   a. no change in the dob passed in means nothing changed
//   b. null means nothing to be externally rendered
//   c. an externally rendered image 
//
// Arguments:
//   data : pointer to the ViewerWindow object instance that set the callback 
//
// Programmer: Mark C. Miller 
// Creation:   03Apr03 
//
// ****************************************************************************
void
ViewerWindow::ExternalRenderCallback(void *data, avtDataObject_p& dob)
{
    static const char *me = "ViewerWindow::ExternalRenderCallback";

    debug2 << "Entering " << me << endl; 

    ViewerWindow *win = (ViewerWindow *)data;
    ViewerEngineManager *eMgr = ViewerEngineManager::Instance();
    ExternalRenderRequestInfo &lastRequest = win->lastExternalRenderRequest;
    ExternalRenderRequestInfo thisRequest;

    // get all the plot's attributes in this window
    std::map<std::string,std::vector<int> > perEnginePlotIds;
    win->GetAnimation()->GetPlotList()->
       GetCurrentPlotAtts(thisRequest.pluginIDsList, thisRequest.hostsList,
                          thisRequest.plotIdsList, thisRequest.attsList);

    // get this window's attributes
    thisRequest.winAtts = win->GetWindowAttributes();
    thisRequest.annotAtts = *(win->GetAnnotationAttributes());

    // see if anything has really changed, release data where necessary
    bool canSkipExternalRender = true;
    {
       if (thisRequest.winAtts != lastRequest.winAtts)
          canSkipExternalRender = false;

       if (thisRequest.annotAtts != lastRequest.annotAtts)
          canSkipExternalRender = false;

       // we don't break early so we can build a list of plots to release data on
       for (int i = 0; i < thisRequest.plotIdsList.size(); i++)
       {
          // search for index of current plot in last list
          int indexOfPlotInLastList = -1;
          for (int j = 0; j < lastRequest.plotIdsList.size(); j++)
          {
             if (lastRequest.plotIdsList[j] == thisRequest.plotIdsList[i])
             {
                indexOfPlotInLastList = j;
                break;
             }
          }

          if (indexOfPlotInLastList == -1)
             canSkipExternalRender = false;
          else
          {
             bool shouldReleasePlot = false;

             // compare plugin ids
             if (thisRequest.pluginIDsList[i] != lastRequest.pluginIDsList[indexOfPlotInLastList])
                shouldReleasePlot = true;

             // compare host names
             if (thisRequest.hostsList[i] != lastRequest.hostsList[indexOfPlotInLastList])
                shouldReleasePlot = true;

             // compare plot attributes
             if (!thisRequest.attsList[i]->EqualTo(lastRequest.attsList[indexOfPlotInLastList]))
                shouldReleasePlot = true;

             if (shouldReleasePlot)
             {
                eMgr->ReleaseData(lastRequest.hostsList[indexOfPlotInLastList].c_str(),
                                     indexOfPlotInLastList);
                canSkipExternalRender = false;
             }
          }
       }
    }

    if (!canSkipExternalRender && (thisRequest.plotIdsList.size() > 0))
    {
       // send per-plot RPCs (which engine handled by ViewerEngineManager)
       for (int i = 0; i < thisRequest.plotIdsList.size(); i++)
       {
          eMgr->UpdatePlotAttributes(thisRequest.hostsList[i].c_str(),thisRequest.pluginIDsList[i],
                   thisRequest.plotIdsList[i],thisRequest.attsList[i]);
          perEnginePlotIds[thisRequest.hostsList[i]].push_back(thisRequest.plotIdsList[i]);
       }

       int numEnginesToRender = perEnginePlotIds.size();
       bool sendZBuffer = numEnginesToRender > 1 ? true : false;

       // send per-engine rpcs
       std::vector<avtImage_p> imgList;
       std::map<std::string,std::vector<int> >::iterator pos;
       for (pos = perEnginePlotIds.begin(); pos != perEnginePlotIds.end(); pos++)
       {
          eMgr->SetWinAnnotAtts(pos->first.c_str(), &thisRequest.winAtts, &thisRequest.annotAtts);
          avtDataObjectReader_p rdr = eMgr->GetDataObjectReader(sendZBuffer, pos->first.c_str(), pos->second);

          // do some magic to update the network so we don't need the reader anymore
          avtDataObject_p tmpDob = rdr->GetOutput();
          avtPipelineSpecification_p spec = 
             tmpDob->GetTerminatingSource()->GetGeneralPipelineSpecification();
          tmpDob->Update(spec);

          // put the resultant image in a list
          avtImage_p img;
          CopyTo(img,tmpDob);
          imgList.push_back(img);
       }

       // composite images from different engines as necessary
       if (numEnginesToRender > 1)
       {
          debug2 << me << ": doing composite from " << imgList.size() << " engines" << endl;
          avtWholeImageCompositer imageCompositer;
          imageCompositer.SetOutputImageSize(thisRequest.winAtts.GetSize()[1], thisRequest.winAtts.GetSize()[0]);
          for (int i = 0; i < imgList.size(); i++)
             imageCompositer.AddImageInput(imgList[i], 0, 0);
          imageCompositer.Execute();
          dob = imageCompositer.GetOutput();
       }
       else
          CopyTo(dob, imgList[0]);
    }
    else
    {
       if (thisRequest.plotIdsList.size() == 0)
          dob = NULL;
    }

    // update last request info
    win->lastExternalRenderRequest.pluginIDsList = thisRequest.pluginIDsList;
    win->lastExternalRenderRequest.hostsList     = thisRequest.hostsList;
    win->lastExternalRenderRequest.plotIdsList   = thisRequest.plotIdsList;
    win->lastExternalRenderRequest.attsList      = thisRequest.attsList;
    win->lastExternalRenderRequest.winAtts       = thisRequest.winAtts;
    win->lastExternalRenderRequest.annotAtts     = thisRequest.annotAtts;

    debug2 << "Leaving " << me << endl;
}
