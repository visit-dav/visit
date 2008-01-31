/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
#include <avtWholeImageCompositerWithZ.h>
#include <avtToolInterface.h>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <AttributeSubjectMap.h>
#include <DataNode.h>
#include <LightList.h> 
#include <Line.h>
#include <LineoutInfo.h>
#include <PickAttributes.h>
#include <GlobalAttributes.h>
#include <PickPointInfo.h>
#include <RenderingAttributes.h>
#include <ViewerActionManager.h>
#include <ViewerEngineManager.h>
#include <ViewerFileServer.h>
#include <ViewerPlot.h> 
#include <ViewerPlotList.h> 
#include <ViewerPopupMenu.h>
#include <ViewerQueryManager.h>
#include <ViewerToolbar.h>
#include <ViewerSubject.h>
#include <ViewerWindowManager.h> 
#include <VisItException.h>
#include <VisualCueInfo.h>
#include <VisWindow.h>
#include <QtVisWindow.h>

#include <DebugStream.h>

#include <Utility.h>

//
// Definition of static variables associated with ViewerWindow.
//
bool    ViewerWindow::doNoWinMode = false;
bool    ViewerWindow::doFullScreenMode = false;

//
// Local macros.
//
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

//
// Global variables.  These should be removed.
//
extern ViewerSubject  *viewerSubject;

//
// Function prototypes.
//
static void RotateAroundY(const avtView3D&, double, avtView3D&);

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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.  I replaced view dimension with window mode.
//
//    Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//    I moved the handling of full frame mode to VisWindow.
//
//    Brad Whitlock, Fri Nov 7 17:54:47 PST 2003
//    I added code to make the default annotation objects be created in
//    this new window.
//
//    Brad Whitlock, Wed Jan 7 09:52:59 PDT 2004
//    I initialized pickFunction and pickFunctionData.
//
//    Hank Childs, Thu Mar  4 08:10:34 PST 2004
//    Use a QtVisWindow.
//
//    Brad Whitlock, Mon Jan 26 22:59:12 PST 2004
//    I removed the animation and replaced it with the plot list.
//
//    Eric Brugger, Mon Mar 29 15:34:50 PST 2004
//    I added maintainData.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed the haveRenderedIn* flags to viewSetIn*, since it was
//    more accurate.  I also added viewPartialSetIn3d to distinguish the
//    view being set from a session file and from the client.
//
//    Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//    Removed preparingToChangeScalableRenderingMode
//
//    Kathleen Bonnell, Wed Aug 18 09:39:29 PDT 2004 
//    Added call to SetInteractorAtts. 
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to manage name of last color table to change
//
//    Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//    Initialize doShading.
//
//    Brad Whitlock, Tue Mar 7 17:37:24 PST 2006
//    Initialized undoViewStack and redoViewStack.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added scalableStereoType to support stereo SR
//
//    Mark C. Miller, Wed Aug  9 16:35:25 PDT 2006
//    Removed scalableStereoType
//
//    Brad Whitlock, Mon Feb 12 17:43:04 PST 2007
//    Added ViewerBase base class.
//
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support to the QtVisWindow.
//
//    Jeremy Meredith, Wed Aug 29 15:21:38 EDT 2007
//    Added initialization of depth cueing properties.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

ViewerWindow::ViewerWindow(int windowIndex) : ViewerBase(0, "ViewerWindow"),
    undoViewStack(true), redoViewStack()
{
    if (doNoWinMode)
    {
        visWindow = new VisWindow();
    }
    else
    {
        visWindow = new QtVisWindow(doFullScreenMode);
    }

    visWindow->SetCloseCallback(CloseCallback, (void *)this);
    visWindow->SetHideCallback(HideCallback, (void *)this);
    visWindow->SetShowCallback(ShowCallback, (void *)this);
    visWindow->SetExternalRenderCallback(ExternalRenderCallback, (void*)this);
    visWindow->CreateAnnotationObjectsFromList(
        *ViewerWindowManager::GetDefaultAnnotationObjectList());
    SetAnnotationAttributes(ViewerWindowManager::GetAnnotationDefaultAtts());
    SetLightList(ViewerWindowManager::GetLightListDefaultAtts());
    SetInteractorAtts(ViewerWindowManager::GetInteractorDefaultAtts());

    // Set some default values.
    cameraView = false;
    maintainView = false;
    maintainData = false;
    viewIsLocked = false;
    windowMode = WINMODE_NONE;
    boundingBoxValidCurve = false;
    viewSetInCurve = false;
    viewModifiedCurve = false;
    boundingBoxValid2d = false;
    viewSetIn2d = false;
    viewModified2d = false;
    boundingBoxValid3d = false;
    viewSetIn3d = false;
    viewPartialSetIn3d = false;
    boundingBoxValidAxisArray = false;
    viewSetInAxisArray = false;
    viewModifiedAxisArray = false;
    mergeViewLimits = false;
    plotExtentsType = AVT_ORIGINAL_EXTENTS;
    timeLocked = false;
    toolsLocked = false;
    windowId = windowIndex;
    isVisible = false;
    isChangingScalableRenderingMode = false;
    targetScalableRenderingMode = false;
    nameOfCtChangedSinceLastRender = "";
    isCompressingScalableImage = false;
    compressionActivationMode = RenderingAttributes::Never;

    // Create the popup menu and the toolbar.
    popupMenu = new ViewerPopupMenu(this);
    toolbar = new ViewerToolbar(this);

    plotList = new ViewerPlotList(this);
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
    pickFunction = 0;
    pickFunctionData = 0;

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

    //
    // Initialize the axis array view information.
    //
    avtViewAxisArray viewAxisArray;

    viewAxisArray.viewport[0] = 0.2;
    viewAxisArray.viewport[1] = 0.95;
    viewAxisArray.viewport[2] = 0.15;
    viewAxisArray.viewport[3] = 0.95;
    visWindow->SetViewAxisArray(viewAxisArray);

    curViewCurve     = new ViewCurveAttributes;
    curView2D        = new View2DAttributes;
    curView3D        = new View3DAttributes;
    curViewAxisArray = new ViewAxisArrayAttributes;
    viewCurveAtts    = new AttributeSubjectMap;
    view2DAtts       = new AttributeSubjectMap;
    view3DAtts       = new AttributeSubjectMap;
    viewAxisArrayAtts= new AttributeSubjectMap;

    //
    // Create the window's action manager.
    //
    actionMgr = new ViewerActionManager(this);

    doShading = false;
    shadingStrength = 0.5;

    doDepthCueing = false;
    startCuePoint[0] = -10;  startCuePoint[0] = 0;  startCuePoint[0] = 0;
    endCuePoint[0]   =  10;  endCuePoint[0]   = 0;  endCuePoint[0]   = 0;
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Brad Whitlock, Tue Jan 27 17:03:28 PST 2004
//    I removed animation and put plotList.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

ViewerWindow::~ViewerWindow()
{
    delete plotList;
    delete popupMenu;
    delete visWindow;
    delete toolbar;

    delete curViewCurve;
    delete curView2D;
    delete curView3D;
    delete curViewAxisArray;
    delete viewCurveAtts;
    delete view2DAtts;
    delete view3DAtts;
    delete viewAxisArrayAtts;
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
//  Method: ViewerWindow::SetFullScreenMode
//
//  Purpose:
//      Controls whether VisWindows should be created in fullscreen mode.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 17, 2007
//
// ****************************************************************************

void
ViewerWindow::SetFullScreenMode(bool newMode)
{
    doFullScreenMode = newMode;
}

// ****************************************************************************
// Method: ViewerWindow::GetFullScreenMode
//
// Purpose: 
//   Returns the fullscreen mode.
//
// Returns:    The fullscreen mode.
//
// Programmer: Jeremy Meredith
// Creation:   July 17, 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetFullScreenMode()
{
    return doFullScreenMode;
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
//  Method: ViewerWindow::GetPlotList
//
//  Purpose: 
//    Return a pointer to the window's plot list.
//
//  Returns:    A pointer to the plot list.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 27 01:07:19 PDT 2004
//
// ****************************************************************************

ViewerPlotList *
ViewerWindow::GetPlotList() const
{
    return plotList;
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
//    Brad Whitlock, Wed Jan 7 09:55:36 PDT 2004
//    I added code to remove the pick function so if we change pick modes,
//    for example, we're back to picking.
//
//    Kathleen Bonnell, Thu Sep  2 13:55:05 PDT 2004 
//    Tell the visWindow that Pick will be normal. 
//
// ****************************************************************************

void
ViewerWindow::SetInteractionMode(const INTERACTION_MODE mode)
{
    const INTERACTION_MODE curMode = visWindow->GetInteractionMode();

    bool changingToPickMode  = (ZONE_PICK == mode || NODE_PICK == mode);
    bool currentlyInPickMode = (ZONE_PICK == curMode || NODE_PICK == curMode);

    // Clear the pick function so it will default to pick instead of a
    // user-defined action.
    pickFunction = 0;
    pickFunctionData = 0;
    visWindow->SetPickTypeToNormal();

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
        GetPlotList()->StopPick();
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
//   Brad Whitlock, Fri Jul 18 12:28:17 PDT 2003
//   Added code to protect us from a bad toolId.
//
// ****************************************************************************

void
ViewerWindow::SetToolEnabled(int toolId, bool enabled)
{
    //
    // If we're turning the tool on, send attributes to it so it can
    // initialize itself if it wants to.
    //
    if(toolId >= 0 && toolId < GetNumTools())
    {
        if(enabled)
        {
            GetPlotList()->InitializeTool(visWindow->GetToolInterface(toolId));
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
    GetPlotList()->HandleTool(ti, applyToAll);
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
//   ViewerQueryManager may also update a tool.
//   
//   Hank Childs, Tue Aug 28 16:06:11 PDT 2007
//   If we initialize the tool to be something different, then call 
//   HandleTool as well, so that the operators are up to date.
//
//   Gunther H. Weber, Wed Aug 29 17:39:39 PDT 2007
//   Make the HandleTool call added by Hank respect the setting of the
//   "Apply operators and selection to all plots" check box.
//
// ****************************************************************************

void
ViewerWindow::UpdateTools()
{
    bool redrawWindow = false;

    //
    // Update any tools that may need to be updated.
    //
    for(int toolId = 0; toolId < visWindow->GetNumTools(); ++toolId)
    {
        if(visWindow->GetToolEnabled(toolId))
        {
            if(GetPlotList()->InitializeTool(visWindow->GetToolInterface(toolId)))
            {
                redrawWindow = true;
                visWindow->UpdateTool(toolId, false);

                // The invocation below makes all of the operators that subscribe 
                // to that tool update.  So you change a slice operator's
                // attributes and the clip operator also snaps to the 
                // same position.  The call below is what makes this
                // happen.  This is the mechanism where the attributes are 
                // sent to (for example) the clip operator.
                ViewerWindowManager *wM = ViewerWindowManager::Instance();
                bool applyOps = wM->GetClientAtts()->GetApplyOperator();
                HandleTool(visWindow->GetToolInterface(toolId), applyOps);
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::RecenterView()
{
    double limits[6];

    switch (visWindow->GetWindowMode())
    {
      case WINMODE_CURVE:
        GetExtents(2, limits);
        RecenterViewCurve(limits);
        break;
      case WINMODE_2D:
        GetExtents(2, limits);
        RecenterView2d(limits);
        break;
      case WINMODE_3D:
        GetExtents(3, limits);
        RecenterView3d(limits);
        break;
      case WINMODE_AXISARRAY:
        GetExtents(2, limits);
        RecenterViewAxisArray(limits);
        break;
      default:
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Wed Sep  1 09:20:09 PDT 2004
//    Modified the routine to reset the view for all the view modes,
//    instead of just the current mode.
//
//    Eric Brugger, Fri Sep  3 15:15:06 PDT 2004
//    Modified the routine to reset the view for all the view modes and
//    then actually reset the view for current view mode.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::ResetView()
{
    //
    // Reset all the view modes.
    //
    boundingBoxValidCurve = false;
    viewSetInCurve = false;
    boundingBoxValid2d = false;
    viewSetIn2d = false;
    boundingBoxValid3d = false;
    viewSetIn3d = false;
    viewPartialSetIn3d = false;
    viewSetInAxisArray = false;
    boundingBoxValidAxisArray = false;

    //
    // Reset the view for the current mode.  This will actually reset
    // the view for any existing plots.
    //
    switch (visWindow->GetWindowMode())
    {
      case WINMODE_CURVE:
        ResetViewCurve();
        break;
      case WINMODE_2D:
        ResetView2d();
        break;
      case WINMODE_3D:
        ResetView3d();
        break;
      case WINMODE_AXISARRAY:
        ResetViewAxisArray();
        break;
      default:
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::ClearViewKeyframes()
{
    viewCurveAtts->ClearAtts();
    view2DAtts->ClearAtts();
    view3DAtts->ClearAtts();
    viewAxisArrayAtts->ClearAtts();
}

// ****************************************************************************
//  Method: ViewerWindow::DeleteViewKeyframe
//
//  Purpose: 
//    Delete a view keyframe.
//
//  Arguments:
//    index : The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Brad Whitlock, Wed Apr 7 13:54:35 PST 2004
//    I rewrote it for multiple time sliders.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::DeleteViewKeyframe(const int index)
{
    if(!GetPlotList()->GetKeyframeMode())
    {
        Error("VisIt does not allow view keyframes to be deleted when the "
              "active window is not in keyframe mode.");
    }
    else
    {
        // Get the number of states for the keyframe time slider.
        int curIndex = 0, nIndices = 1;
        GetPlotList()->GetTimeSliderStates(KF_TIME_SLIDER, curIndex, nIndices);

        //
        // Check that the index is within range.
        //
        if ((index < 0) || (index >= nIndices))
        {
            debug1 << "The index is out of range." << endl;
            return;
        }

        //
        // Delete the keyframe at the specified frame.  DeleteAtts
        // returns the range of frames that were invalidated.  The
        // maximum value is clamped to nFrames since DeleteAtts may
        // return INT_MAX to indicate the end of the animation.
        //
        int i0, i1;
        viewCurveAtts->DeleteAtts(index, i0, i1);
        view2DAtts->DeleteAtts(index, i0, i1);
        view3DAtts->DeleteAtts(index, i0, i1);
        viewAxisArrayAtts->DeleteAtts(index, i0, i1);
        i1 = i1 < (nIndices - 1) ? i1 : (nIndices - 1);

        //
        // If the current index is within the range, update the view.
        //
        if (curIndex >= i0 || curIndex <= i1)
        {
            UpdateCameraView();
            ViewerWindowManager::Instance()->UpdateViewAtts();
        }
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Brad Whitlock, Wed Apr 7 13:54:35 PST 2004
//    I rewrote it for multiple time sliders.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::MoveViewKeyframe(int oldIndex, int newIndex)
{
    if(!GetPlotList()->GetKeyframeMode())
    {
        Error("VisIt does not allow view keyframes to be moved when the "
              "active window is not in keyframe mode.");
    }
    else
    {
        // Get the number of states for the keyframe time slider.
        int curIndex = 0, nIndices = 1;
        GetPlotList()->GetTimeSliderStates(KF_TIME_SLIDER, curIndex, nIndices);

        //
        // Check that the index is within range.
        //
        if ((oldIndex < 0) || (oldIndex >= nIndices) ||
            (newIndex < 0) || (newIndex >= nIndices))
        {
            debug1 << "The index is out of range." << endl;
            return;
        }

        //
        // Delete the keyframe at the specified frame.  DeleteAtts
        // returns the range of frames that were invalidated.  The
        // maximum value is clamped to nFrames since DeleteAtts may
        // return INT_MAX to indicate the end of the animation.
        //
        int i0, i1;
        if (!viewCurveAtts->MoveAtts(oldIndex, newIndex, i0, i1))
            return;
        if (!view2DAtts->MoveAtts(oldIndex, newIndex, i0, i1))
            return;
        if (!view3DAtts->MoveAtts(oldIndex, newIndex, i0, i1))
            return;
        if (!viewAxisArrayAtts->MoveAtts(oldIndex, newIndex, i0, i1))
            return;
        i1 = i1 < (nIndices - 1) ? i1 : (nIndices - 1);

        //
        // If the current index is within the range, update the view.
        //
        if (curIndex >= i0 || curIndex <= i1)
        {
            UpdateCameraView();
            ViewerWindowManager::Instance()->UpdateViewAtts();
        }
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.  I split the view attributes into 2d and 3d parts.
//
//    Hank Childs, Wed Oct 15 12:58:19 PDT 2003
//    Copy over the eye angle.
//
//    Eric Brugger, Thu Oct 16 14:29:28 PDT 2003
//    I added a full frame mode to the 2d view.
//
//    Brad Whitlock, Wed Apr 7 13:54:35 PST 2004
//    I rewrote it for multiple time sliders.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::SetViewKeyframe()
{
    if(!GetPlotList()->GetKeyframeMode())
    {
        Error("VisIt does not allow view keyframes to be added when the "
              "active window is not in keyframe mode.");
    }
    else
    {
        // Get the number of states for the keyframe time slider.
        int curIndex = 0, nIndices = 1;
        GetPlotList()->GetTimeSliderStates(KF_TIME_SLIDER, curIndex, nIndices);
        
        //
        // Set a curve view keyframe.
        //
        const avtViewCurve &viewCurve = visWindow->GetViewCurve();
        curViewCurve->SetDomainCoords(viewCurve.domain);
        curViewCurve->SetRangeCoords(viewCurve.range);
        curViewCurve->SetViewportCoords(viewCurve.viewport);
        viewCurveAtts->SetAtts(curIndex, curViewCurve);

        //
        // Set a 2d view keyframe.
        //
        const avtView2D &view2d = visWindow->GetView2D();
        curView2D->SetWindowCoords(view2d.window);
        curView2D->SetViewportCoords(view2d.viewport);
        curView2D->SetUseFullFrame(view2d.fullFrame);
        view2DAtts->SetAtts(curIndex, curView2D);

        //
        // Set a 3d view keyframe.
        //
        const avtView3D &view3d = visWindow->GetView3D();
        curView3D->SetViewNormal(view3d.normal);
        curView3D->SetFocus(view3d.focus);
        curView3D->SetViewUp(view3d.viewUp);
        curView3D->SetViewAngle(view3d.viewAngle);
        curView3D->SetParallelScale(view3d.parallelScale);
        curView3D->SetNearPlane(view3d.nearPlane);
        curView3D->SetFarPlane(view3d.farPlane);
        curView3D->SetImagePan(view3d.imagePan);
        curView3D->SetImageZoom(view3d.imageZoom);
        curView3D->SetPerspective(view3d.perspective);
        curView3D->SetEyeAngle(view3d.eyeAngle);
        view3DAtts->SetAtts(curIndex, curView3D);

        //
        // Set an axis array view keyframe.
        //
        const avtViewAxisArray &viewAxisArray = visWindow->GetViewAxisArray();
        curViewAxisArray->SetDomainCoords(viewAxisArray.domain);
        curViewAxisArray->SetRangeCoords(viewAxisArray.range);
        curViewAxisArray->SetViewportCoords(viewAxisArray.viewport);
        viewAxisArrayAtts->SetAtts(curIndex, curViewAxisArray);
    }
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
//  Modifications:
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Fri Apr 23 12:56:25 PDT 2004
//    I removed the resetting of the boundingBoxValid flags to avoid
//    problems with setting the view for subsequent plots.
//
// ****************************************************************************

void
ViewerWindow::SetViewExtentsType(avtExtentType viewType)
{
    visWindow->SetViewExtentsType(viewType);
    if (viewType != plotExtentsType)
    {
        plotExtentsType = viewType;
        GetPlotList()->SetSpatialExtentsType(plotExtentsType);
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
//   Brad Whitlock, Sat Jan 31 22:39:26 PST 2004
//   I removed the frame argumnet.
//
//   Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//   Added const qualification 
// ****************************************************************************

void
ViewerWindow::GetExtents(int nDimensions, double *extents) const
{
    GetPlotList()->GetPlotLimits(nDimensions, extents);
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
//  Modifications:
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I replace view dimension with window mode.
//
//    Eric Brugger, Thu Aug 28 12:27:42 PDT 2003
//    I added code to also recenter the view if the window is in curve mode
//    and the curve view has not been modified.
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
        if (visWindow->GetWindowMode() == WINMODE_3D ||
            (visWindow->GetWindowMode() == WINMODE_2D &&
             viewModified2d == false) ||
            (visWindow->GetWindowMode() == WINMODE_CURVE &&
             viewModifiedCurve == false))
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
//  Method: ViewerWindow::SetMaintainDataMode
//
//  Purpose: 
//    Set the window's maintain data mode.
//
//  Arguments:
//    mode      The maintain data mode.
//
//  Programmer: Eric Brugger
//  Creation:   March 29, 2004
//
// ****************************************************************************

void
ViewerWindow::SetMaintainDataMode(const bool mode)
{
    //
    // If maintain data is toggled to false regenerate the actors.
    //
    if (maintainData == true && mode == false)
    {
        maintainData = mode;
        plotList->ClearActors();
        plotList->UpdateFrame();
    }
    else
    {
        maintainData = mode;
    }
}


// ****************************************************************************
//  Method: ViewerWindow::GetMaintainDataMode
//
//  Purpose: 
//    Return the window's maintain data mode.
//
//  Returns:    The maintain data mode.
//
//  Programmer: Eric Brugger
//  Creation:   March 29, 2004
//
// ****************************************************************************

bool
ViewerWindow::GetMaintainDataMode() const
{
    return maintainData;
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
//    Kathleen Bonnell, Wed Jul 16 10:02:52 PDT 2003 
//    Removed scaling of view2D.window, handled by avtView2D. 
//    Moved test for valid bbox to Compute2DScaleFactor.
//
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//    I moved the handling of full frame mode to VisWindow.
//
//    Dave Bremer, Wed Mar 21 17:39:55 PDT 2007
//    If full frame mode is set here in the viewer, also change the full frame
//    activation mode to be explicitly on or off, instead of auto which can
//    cause the mode to later change unexpectedly.
//
//    Dave Bremer, March 26, 2007
//    Undid last change.  It led to full frame activation being turned off
//    at start time.
// ****************************************************************************

void
ViewerWindow::SetFullFrameMode(const bool mode)
{
    //
    // Set the full frame mode and update the view.
    //
    avtView2D view2D=visWindow->GetView2D();
    view2D.fullFrame = mode;
    visWindow->SetView2D(view2D);
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
//  Modifications:
//    Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//    I moved the handling of full frame mode to VisWindow.
//
// ****************************************************************************
bool
ViewerWindow::GetFullFrameMode() const
{
    return visWindow->GetView2D().fullFrame;
}


// ****************************************************************************
//  Method: ViewerWindow::SetFullFrameActivationMode
//
//  Purpose: 
//    Set the window's full frame activation mode.
//
//  Programmer: Dave Bremer
//  Creation:   March, 26 2007
//
// ****************************************************************************
void
ViewerWindow::SetFullFrameActivationMode(const int mode)
{
    avtView2D view2D=visWindow->GetView2D();
    view2D.fullFrameActivationMode = mode;
    visWindow->SetView2D(view2D);
}


// ****************************************************************************
//  Method: ViewerWindow::GetFullFrameActivationMode
//
//  Purpose: 
//    Return the window's full frame activation mode.
//
//  Returns:    The full frame activation mode.
//
//  Programmer: Dave Bremer
//  Creation:   March, 26 2007
//
// ****************************************************************************
int
ViewerWindow::GetFullFrameActivationMode() const
{
    return visWindow->GetView2D().fullFrameActivationMode;
}


// ****************************************************************************
//  Method: ViewerWindow::DoAllPlotsAxesHaveSameUnits
//
//
//  Programmer: Mark C. Miller 
//  Creation:   April 5, 2006
// ****************************************************************************

bool
ViewerWindow::DoAllPlotsAxesHaveSameUnits()
{
    return visWindow->DoAllPlotsAxesHaveSameUnits();
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
//   Mark C. Miller, Tue Oct 19 19:21:49 PDT 2004
//   Added setting of nameOfCtChangedSinceLastRender
//
// ****************************************************************************

void
ViewerWindow::UpdateColorTable(const char *ctName)
{
    // If any plots in the plot list had to be updated, redraw the window.
    if(GetPlotList()->UpdateColorTable(ctName))
    {
        SendRedrawMessage();
        nameOfCtChangedSinceLastRender = ctName;
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
ViewerWindow::ClearWindow(bool clearAllPlots)
{
    ClearRefLines();
    ClearPickPoints();
    GetPlotList()->ClearPlots(clearAllPlots);
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
//   Jeremy Meredith, Fri Nov 14 11:32:03 PST 2003
//   Added specular properties.
//
//   Eric Brugger, Mon Mar 29 15:34:50 PST 2004
//   I added maintainData.
//
//   Hank Childs, Mon May 10 08:04:48 PDT 2004
//   Use "display list mode" instead of immediate rendering mode.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//   Add shading.
//
//   Brad Whitlock, Mon Sep 18 10:56:09 PDT 2006
//   Added color texturing.
//
//    Jeremy Meredith, Wed Aug 29 15:21:38 EDT 2007
//    Added copying of depth cueing properties.
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
    SetDisplayListMode(source->GetDisplayListMode());
    SetSurfaceRepresentation(source->GetSurfaceRepresentation());
    SetNotifyForEachRender(source->GetNotifyForEachRender());
    SetScalableAutoThreshold(source->GetScalableAutoThreshold());
    SetScalableActivationMode(source->GetScalableActivationMode());
    SetSpecularProperties(source->GetSpecularFlag(),
                          source->GetSpecularCoeff(),
                          source->GetSpecularPower(),
                          source->GetSpecularColor());
    SetShadingProperties(source->GetDoShading(), source->GetShadingStrength());
    SetDepthCueingProperties(source->GetDoDepthCueing(),
                             source->GetStartCuePoint(),
                             source->GetEndCuePoint());
    SetColorTexturingFlag(source->GetColorTexturingFlag());

    //
    // Set window mode flags.
    //
    SetBoundingBoxMode(source->GetBoundingBoxMode());
    SetSpinMode(source->GetSpinMode());
    SetCameraViewMode(source->GetCameraViewMode());
    SetMaintainViewMode(source->GetMaintainViewMode());
    SetMaintainDataMode(source->GetMaintainDataMode());
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
// Method: ViewerWindow::SetFrameAndState
//
// Purpose: 
//   Tells the vis window what the current frame and state are.
//
// Arguments:
//   nFrames    : The total number of animation frames.
//   startFrame : The starting animation frame.
//   curFrame   : The current animation frame.
//   endFrame   : The ending animation frame.
//   startState : The timestep of the database at startFrame
//   curState   : The timestep of the database at curFrame
//   endState   : The timestep of the database at endFrame
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 11:19:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetFrameAndState(int nFrames,
    int startFrame, int curFrame, int endFrame,
    int startState, int curState, int endState)
{
    visWindow->SetFrameAndState(nFrames, startFrame, curFrame, endFrame,
                                startState, curState, endState);
}

// ****************************************************************************
// Method: ViewerWindow::GetFrameAndState
//
// Purpose: 
//   Returns frame and state info. 
//
// Programmer: Mark C. Miller 
// Creation:   Tuesday, July 27, 2004 
//
// ****************************************************************************

void
ViewerWindow::GetFrameAndState(int& nFrames,
    int& startFrame, int& curFrame, int& endFrame,
    int& startState, int& curState, int& endState)
{
    visWindow->GetFrameAndState(nFrames, startFrame, curFrame, endFrame,
                                startState, curState, endState);
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
//
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::UpdateView(const WINDOW_MODE mode, const double *limits)
{
    if (windowMode != mode)
        ViewerQueryManager::Instance()->ViewDimChanged(this);

    windowMode = mode;
    switch (visWindow->GetWindowMode())
    {
      case WINMODE_CURVE:
        UpdateViewCurve(limits);
        break;
      case WINMODE_2D:
        UpdateView2d(limits);
        break;
      case WINMODE_3D:
        UpdateView3d(limits);
        break;
      case WINMODE_AXISARRAY:
        UpdateViewAxisArray(limits);
        break;
      default:
        break;
    }
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
//  Modifications:
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedInCurve to viewSetInCurve, since it was more
//    accurate.
//
// ****************************************************************************

void
ViewerWindow::SetViewCurve(const avtViewCurve &v)
{
    visWindow->SetViewCurve(v);

    viewSetInCurve = true;

    viewModifiedCurve = true;
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
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedIn2d to viewSetIn2d, since it was more accurate.
//
//    Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//    Added logic to adjust setting the viewSetIn2d bool if the only thing
//    that changed is is fullFrame
// ****************************************************************************

void
ViewerWindow::SetView2D(const avtView2D &v)
{
    const avtView2D& oldView = visWindow->GetView2D();
    avtView2D oldViewCopy = oldView;
    oldViewCopy.fullFrame = v.fullFrame;

    visWindow->SetView2D(v);

    if (oldViewCopy == v)
        viewSetIn2d = false;
    else
        viewSetIn2d = true;

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
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedIn3d to viewSetIn3d, since it was more accurate.
//
// ****************************************************************************

void
ViewerWindow::SetView3D(const avtView3D &v)
{
    visWindow->SetView3D(v);

    viewPartialSetIn3d = true;
}

// ****************************************************************************
//  Method:  ViewerWindow::SetViewAxisArray
//
//  Purpose:
//    Set the view for AxisArray mode and update window if needed.
//
//  Arguments:
//    v          the new view
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2008
//
// ****************************************************************************

void
ViewerWindow::SetViewAxisArray(const avtViewAxisArray &v)
{
    visWindow->SetViewAxisArray(v);

    viewSetInAxisArray = true;
    viewModifiedAxisArray = true;
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
//  Method: ViewerWindow::GetViewAxisArray 
//
//  Purpose:
//    Return the current AxisArray view.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2008
//
// ****************************************************************************

const avtViewAxisArray &
ViewerWindow::GetViewAxisArray() const
{
    return visWindow->GetViewAxisArray();
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Brad Whitlock, Tue Dec 30 17:06:39 PST 2003
//    I added code to copy the view keyframes.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed the haveRenderedIn* flags to viewSetIn*, since it was
//    more accurate.  I also added viewPartialSetIn3d to distinguish the
//    view being set from a session file and from the client.
//
//    Hank Childs, Sun Dec 19 18:49:27 PST 2004
//    Set the view extents type as well.
//
//    Brad Whitlock, Tue Mar 7 17:40:05 PST 2006
//    Copy the view stacks.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::CopyViewAttributes(const ViewerWindow *source)
{
    SetViewExtentsType(source->plotExtentsType);

    //
    // Copy the views.
    //
    visWindow->SetViewCurve(source->visWindow->GetViewCurve());
    visWindow->SetView2D(source->visWindow->GetView2D());
    visWindow->SetView3D(source->visWindow->GetView3D());
    visWindow->SetViewAxisArray(source->visWindow->GetViewAxisArray());

    //
    // Copy the view stacks
    //
    undoViewStack = source->undoViewStack;
    redoViewStack = source->redoViewStack;

    //
    // Copy the view keyframes.
    //
    *viewCurveAtts = *(source->viewCurveAtts);
    *view2DAtts = *(source->view2DAtts);
    *view3DAtts = *(source->view3DAtts);
    *viewAxisArrayAtts = *(source->viewAxisArrayAtts);

    //
    // Copy the bounding boxes.
    //
    boundingBoxValidCurve = source->boundingBoxValidCurve;
    centeringValidCurve   = source->centeringValidCurve;
    viewSetInCurve        = source->viewSetInCurve;
    viewModifiedCurve     = source->viewModifiedCurve;
    boundingBoxValid2d    = source->boundingBoxValid2d;
    centeringValid2d      = source->centeringValid2d;
    viewSetIn2d           = source->viewSetIn2d;
    viewModified2d        = source->viewModified2d;
    boundingBoxValid3d    = source->boundingBoxValid3d;
    centeringValid3d      = source->centeringValid3d;
    viewSetIn3d           = source->viewSetIn3d;
    viewPartialSetIn3d    = source->viewPartialSetIn3d;
    centeringValidAxisArray= source->centeringValidAxisArray;
    viewSetInAxisArray    = source->viewSetInAxisArray;
    viewModifiedAxisArray = source->viewModifiedAxisArray;

    int       i;
    for (i = 0; i < 4; i++)
        boundingBoxCurve[i] = source->boundingBoxCurve[i];
    for (i = 0; i < 4; i++)
        boundingBox2d[i] = source->boundingBox2d[i];
    for (i = 0; i < 6; i++)
        boundingBox3d[i] = source->boundingBox3d[i];
    for (i = 0; i < 4; i++)
        boundingBoxAxisArray[i] = source->boundingBoxAxisArray[i];
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
//   Kathleen Bonnell, Wed Jul 16 10:02:52 PDT 2003 
//   Don't instantiate new avtView2D, but retrieve from VisWindow so that
//   axisScale information can be preserved. 
//   
//   Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//   I added a curve view.
//
//   Hank Childs, Wed Oct 15 12:58:19 PDT 2003
//   Added eye angle.
//
//   Eric Brugger, Thu Oct 16 14:29:28 PDT 2003
//   I added a full frame mode to the 2d view.
//
//   Brad Whitlock, Wed Apr 7 14:07:46 PST 2004
//   I made some changes to make it work now there are multiple time
//   sliders. I also changed the code so it does not copy view components
//   value by value since there are helper methods already for that.
//
//   Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//   Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::UpdateCameraView()
{
    //
    // If we are in keyframe mode and we want to see the keyframed camera
    // and at least one view keyframe has been defined then set the view based
    // on the view keyframes.
    //
    if (GetPlotList()->GetKeyframeMode() && cameraView &&
        view2DAtts->GetNIndices() > 0)
    {
        // Get the number of states for the keyframe time slider.
        int curIndex = 0, nIndices = 1;
        GetPlotList()->GetTimeSliderStates(KF_TIME_SLIDER, curIndex, nIndices);

        if (visWindow->GetWindowMode() == WINMODE_CURVE)
        {
            viewCurveAtts->GetAtts(curIndex, curViewCurve);
            avtViewCurve viewCurve;
            viewCurve.SetFromViewCurveAttributes(curViewCurve);
            visWindow->SetViewCurve(viewCurve);
        }
        else if (visWindow->GetWindowMode() == WINMODE_2D)
        {
            view2DAtts->GetAtts(curIndex, curView2D);
            avtView2D view2d;
            view2d.SetFromView2DAttributes(curView2D);
            visWindow->SetView2D(view2d);
        }
        else if (visWindow->GetWindowMode() == WINMODE_3D)
        {
            view3DAtts->GetAtts(curIndex, curView3D);
            avtView3D view3d;
            view3d.SetFromView3DAttributes(curView3D);
            visWindow->SetView3D(view3d);
        }
        else if (visWindow->GetWindowMode() == WINMODE_AXISARRAY)
        {
            viewAxisArrayAtts->GetAtts(curIndex, curViewAxisArray);
            avtViewAxisArray viewAxisArray;
            viewAxisArray.SetFromViewAxisArrayAttributes(curViewAxisArray);
            visWindow->SetViewAxisArray(viewAxisArray);
        }
    }
}

// ****************************************************************************
// Method: ViewerWindow::UndoView
//
// Purpose: 
//   Undoes the last view change by popping off the top view on the view stack.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:50:44 PST 2006
//
// Modifications:
//   
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::UndoView()
{
    if(GetWindowMode() == WINMODE_CURVE)
    {
        avtViewCurve view;

        // Pop off the previous view.
        if(undoViewStack.PopViewCurve(view))
        {
            // Put the current view on the redo stack.
            redoViewStack.PushViewCurve(GetViewCurve());

            // Restore the previous view
            SetViewCurve(view);
        }
    }
    else if(GetWindowMode() == WINMODE_2D)
    {
        avtView2D view;

        // Pop off the previous view.
        if(undoViewStack.PopView2D(view))
        {
            // Put the current view on the redo stack.
            redoViewStack.PushView2D(GetView2D());

            // Restore the previous view
            SetView2D(view);
        }
    }
    else if(GetWindowMode() == WINMODE_3D)
    {
        avtView3D view;

        // Pop off the previous view.
        if(undoViewStack.PopView3D(view))
        {
            // Put the current view on the redo stack.
            redoViewStack.PushView3D(GetView3D());

            // Restore the previous view
            SetView3D(view);
        }
    }
    else if(GetWindowMode() == WINMODE_AXISARRAY)
    {
        avtViewAxisArray view;

        // Pop off the previous view.
        if(undoViewStack.PopViewAxisArray(view))
        {
            // Put the current view on the redo stack.
            redoViewStack.PushViewAxisArray(GetViewAxisArray());

            // Restore the previous view
            SetViewAxisArray(view);
        }
    }
}

// ****************************************************************************
// Method: ViewerWindow::RedoView
//
// Purpose: 
//   Re-does the last undo view.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:51:17 PST 2006
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::RedoView()
{
    if(GetWindowMode() == WINMODE_CURVE)
    {
        avtViewCurve view;
        if(redoViewStack.PopViewCurve(view))
        {
            undoViewStack.PushViewCurve(view);
            SetViewCurve(view);
        }
    }
    else if(GetWindowMode() == WINMODE_2D)
    {
        avtView2D view;
        if(redoViewStack.PopView2D(view))
        {
            undoViewStack.PushView2D(view);
            SetView2D(view);
        }
    }
    else if(GetWindowMode() == WINMODE_3D)
    {
        avtView3D view;
        if(redoViewStack.PopView3D(view))
        {
            undoViewStack.PushView3D(view);
            SetView3D(view);
        }
    }
    else if(GetWindowMode() == WINMODE_AXISARRAY)
    {
        avtViewAxisArray view;
        if(redoViewStack.PopViewAxisArray(view))
        {
            undoViewStack.PushViewAxisArray(view);
            SetViewAxisArray(view);
        }
    }
}

// ****************************************************************************
// Method: ViewerWindow::PushCurrentViews
//
// Purpose: 
//   Pushes the current view onto the view stack.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:52:06 PST 2006
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//   
// ****************************************************************************

void
ViewerWindow::PushCurrentViews()
{
    if(GetWindowMode() == WINMODE_CURVE)
        undoViewStack.PushViewCurve(GetViewCurve());
    else if(GetWindowMode() == WINMODE_2D)
        undoViewStack.PushView2D(GetView2D());
    else if(GetWindowMode() == WINMODE_3D)
        undoViewStack.PushView3D(GetView3D());
    else if(GetWindowMode() == WINMODE_AXISARRAY)
        undoViewStack.PushViewAxisArray(GetViewAxisArray());
}

// ****************************************************************************
// Method: ViewerWindow::UndoViewEnabled
//
// Purpose: 
//   Returns whether the undo view operation is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:52:23 PST 2006
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//   
// ****************************************************************************

bool
ViewerWindow::UndoViewEnabled() const
{
    bool retval = false;
    if(GetWindowMode() == WINMODE_CURVE)
        retval = undoViewStack.HasViewCurves();
    else if(GetWindowMode() == WINMODE_2D)
        retval = undoViewStack.HasView2Ds();
    else if(GetWindowMode() == WINMODE_3D)
        retval = undoViewStack.HasView3Ds();
    else if(GetWindowMode() == WINMODE_AXISARRAY)
        retval = undoViewStack.HasViewAxisArrays();
    return retval;
}

// ****************************************************************************
// Method: ViewerWindow::RedoViewEnabled
//
// Purpose: 
//   Returns whether the redo view operation is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:52:23 PST 2006
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//   
// ****************************************************************************

bool
ViewerWindow::RedoViewEnabled() const
{
    bool retval = false;
    if(GetWindowMode() == WINMODE_CURVE)
        retval = redoViewStack.HasViewCurves();
    else if(GetWindowMode() == WINMODE_2D)
        retval = redoViewStack.HasView2Ds();
    else if(GetWindowMode() == WINMODE_3D)
        retval = redoViewStack.HasView3Ds();
    else if(GetWindowMode() == WINMODE_AXISARRAY)
        retval = redoViewStack.HasViewAxisArrays();
    return retval;
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
//  Modifications:
//    Brad Whitlock, Tue Oct 7 11:32:18 PDT 2003
//    Added playbackMode.
//
// ****************************************************************************

void
ViewerWindow::SetAnimationAttributes(const AnimationAttributes *atts)
{
    // Set the pipeline caching flag.
    GetPlotList()->SetPipelineCaching(atts->GetPipelineCachingMode());
    // Set the animation's playback style.
    if(atts->GetPlaybackMode() == AnimationAttributes::Looping)
        GetPlotList()->SetPlaybackMode(ViewerPlotList::Looping);
    else if(atts->GetPlaybackMode() == AnimationAttributes::PlayOnce)
        GetPlotList()->SetPlaybackMode(ViewerPlotList::PlayOnce);
    else if(atts->GetPlaybackMode() == AnimationAttributes::Swing)
        GetPlotList()->SetPlaybackMode(ViewerPlotList::Swing);
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
// Modifications:
//   Brad Whitlock, Tue Oct 7 11:31:16 PDT 2003
//   Added playbackMode.
//
// ****************************************************************************

static AnimationAttributes animationAttributes;

const AnimationAttributes *
ViewerWindow::GetAnimationAttributes() const
{
    // Set the caching mode.
    animationAttributes.SetPipelineCachingMode(
        GetPlotList()->GetPipelineCaching());
    // Set the playback mode.
    if(GetPlotList()->GetPlaybackMode() == ViewerPlotList::Looping)
        animationAttributes.SetPlaybackMode(AnimationAttributes::Looping);
    else if(GetPlotList()->GetPlaybackMode() == ViewerPlotList::PlayOnce)
        animationAttributes.SetPlaybackMode(AnimationAttributes::PlayOnce);
    else if(GetPlotList()->GetPlaybackMode() == ViewerPlotList::Swing)
        animationAttributes.SetPlaybackMode(AnimationAttributes::Swing);

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
// Method: ViewerWindow::CopyAnnotationObjectList
//
// Purpose: 
//   Copies the annotation objects from the source window to this window.
//
// Arguments:
//   source      : The window from which we're copying annotation objects.
//   copyLegends : Whether the legend annotation objects should be copied.
//
// Returns:    
//
// Note:       All annotations in this window are deleted before copying
//             the new annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 17:50:42 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Mar 26 14:49:53 PST 2007
//   Added copyLegends.
//
// ****************************************************************************

void
ViewerWindow::CopyAnnotationObjectList(const ViewerWindow *source, 
    bool copyLegends)
{
    // Get the properties of all of the source window's annotations.
    AnnotationObjectList annots;
    source->UpdateAnnotationObjectList(annots);

    // Add the annotations to this window.
    if(copyLegends)
    {
        // First delete all of the annotation objects.
        visWindow->DeleteAllAnnotationObjects();

        visWindow->CreateAnnotationObjectsFromList(annots);
    }
    else
    {
        // Get the legends for this window so they can be preserved. Do this
        // before we clear the annotations.
        AnnotationObjectList withLegends, allAnnots;
        UpdateAnnotationObjectList(withLegends);
        for(int i = 0; i < withLegends.GetNumAnnotations(); ++i)
        {
            if(withLegends[i].GetObjectType() == AnnotationObject::LegendAttributes)
                allAnnots.AddAnnotation(withLegends[i]);
        }

        // Grab all of the non-legend annotations from the other window and 
        // add them to this window.
        for(int i = 0; i < annots.GetNumAnnotations(); ++i)
        {
            if(annots[i].GetObjectType() != AnnotationObject::LegendAttributes)
                allAnnots.AddAnnotation(annots[i]);
        }

        // First delete all of the annotation objects.
        visWindow->DeleteAllAnnotationObjects();

        visWindow->CreateAnnotationObjectsFromList(allAnnots);
    }
}

// ****************************************************************************
// Method: ViewerWindow::AddAnnotationObject
//
// Purpose: 
//   Tells the vis window to create a new annotation object.
//
// Arguments:
//   annotType : The type of annotation object to create.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:24:08 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 20 10:01:35 PDT 2007
//   Name the new object.
//
// ****************************************************************************

bool
ViewerWindow::AddAnnotationObject(int annotType, const std::string &annotName)
{
    bool ret;

    if((ret = visWindow->AddAnnotationObject(annotType, annotName)) == true)
        SendUpdateFrameMessage();
    else
    {
        Warning("The annotation object could not be added.");
    }

    return ret;
}

// ****************************************************************************
// Method: ViewerWindow::HideActiveAnnotationObjects
//
// Purpose: 
//   Hides the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:27:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::HideActiveAnnotationObjects()
{
    visWindow->HideActiveAnnotationObjects();
    SendUpdateFrameMessage();
}

// ****************************************************************************
// Method: ViewerWindow::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Deletes the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:28:05 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::DeleteActiveAnnotationObjects()
{
    visWindow->DeleteActiveAnnotationObjects();
    SendUpdateFrameMessage();
}

// ****************************************************************************
// Method: ViewerWindow::DeleteAnnotationObject
//
// Purpose: 
//   Deletes the specified annotation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 12:18:09 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::DeleteAnnotationObject(const std::string &name)
{
    bool ret = visWindow->DeleteAnnotationObject(name);
    if(ret)
        SendUpdateFrameMessage();
    return ret;
}

// ****************************************************************************
// Method: ViewerWindow::DeleteAllAnnotationObjects
//
// Purpose: 
//   Deletes all annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:28:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::DeleteAllAnnotationObjects()
{
    visWindow->DeleteAllAnnotationObjects();
    SendUpdateFrameMessage();
}

// ****************************************************************************
// Method: ViewerWindow::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Raise the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:28:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::RaiseActiveAnnotationObjects()
{
    visWindow->RaiseActiveAnnotationObjects();
    SendUpdateFrameMessage();
}

// ****************************************************************************
// Method: ViewerWindow::LowerActiveAnnotationObjects
//
// Purpose: 
//   Lowers the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:29:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::LowerActiveAnnotationObjects()
{
    visWindow->LowerActiveAnnotationObjects();
    SendUpdateFrameMessage();
}

// ****************************************************************************
// Method: ViewerWindow::SetAnnotationObjectOptions
//
// Purpose: 
//   Tells the vis window to update the properties of its annotation objects
//   using the passed in annotation objects list.
//
// Arguments:
//   al : The annotation object list to use to set the properties for the
//        annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 17:49:33 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetAnnotationObjectOptions(const AnnotationObjectList &al)
{
    visWindow->SetAnnotationObjectOptions(al);
    SendUpdateFrameMessage();
}

// ****************************************************************************
// Method: ViewerWindow::UpdateAnnotationObjectList
//
// Purpose: 
//   Populates the annotation object list with the annotations' properties.
//
// Arguments:
//   al : The annotation object list that we're populating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 17:48:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::UpdateAnnotationObjectList(AnnotationObjectList &al) const
{
    visWindow->UpdateAnnotationObjectList(al);
}

// ****************************************************************************
// Method: ViewerWindow::CreateAnnotationObjectsFromList
//
// Purpose: 
//   Creates annotations according to the objects in the passed in annotation
//   object list.
//
// Arguments:
//   al : The annotation object list that we're using to create annotation
//        objects.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 17:55:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::CreateAnnotationObjectsFromList(const AnnotationObjectList &al)
{
    visWindow->CreateAnnotationObjectsFromList(al);
    SendUpdateMessage();
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
//    Brad Whitlock, Thu Sep 11 08:55:25 PDT 2003
//    I added code to suspend spin mode.
//
// ****************************************************************************

void
ViewerWindow::ShowMenu()
{
    visWindow->SetSpinModeSuspended(true);
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
//  Modifications:
//    Brad Whitlock, Thu Sep 11 08:55:45 PDT 2003
//    I added code to turn of spin mode suspend.
//
// ****************************************************************************

void
ViewerWindow::HideMenu()
{
    popupMenu->HideMenu();
    visWindow->SetSpinModeSuspended(false);
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
// Method: ViewerWindow::SetVisible
//
// Purpose: 
//   Set the window visibility flag.
//
// Arguments:
//   val       The new visibility flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 09:38:19 PDT 2003
//
// ****************************************************************************

void
ViewerWindow::SetVisible(bool val)
{
    isVisible = val;
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
// ****************************************************************************

bool
ViewerWindow::IsVisible() const
{
    return visWindow->IsVisible();
}

// ****************************************************************************
//  Method: ViewerWindow::GetWindowMode
//
//  Purpose: 
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2003 
//
// ****************************************************************************

WINDOW_MODE
ViewerWindow::GetWindowMode() const 
{
    return visWindow->GetWindowMode();
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
    redraw |= GetPlotList()->SetBackgroundColor(bg);
    redraw |= GetPlotList()->SetForegroundColor(fg);
    if (redraw) 
    {
        SendRedrawMessage();
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//    I moved the handling of full frame mode to VisWindow.
//
// ****************************************************************************

void
ViewerWindow::GetScaleFactorAndType(double &s, int &t)
{
    visWindow->GetScaleFactorAndType(s, t);
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Thu Aug 28 12:27:42 PDT 2003
//    I modified the routine to set the viewModifiedCurve flag.
//
//    Eric Brugger, Tue Nov 18 11:13:48 PST 2003
//    I removed coding for handling degenerate views since this was now
//    handled in avtViewCurve.
//
//    Kathleen Bonnell, Thu Mar 22 19:24:21 PDT 2007 
//    Added support for Log scaling.
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Record when log of the domain/range coords has been done.
//    Ensure we aren't attempting log scaling when plots don't support it.
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
        centeringValidCurve = false;
        return;
    }

    //
    // Set the new window.
    //
    int       i;

    for (i = 0; i < 4; i++)
    {
        boundingBoxCurve[i] = limits[i];
    }

    //
    // Update the view.
    //
    avtViewCurve viewCurve=visWindow->GetViewCurve();
    viewCurve.domain[0] = boundingBoxCurve[0];
    viewCurve.domain[1] = boundingBoxCurve[1];
    viewCurve.range[0]  = boundingBoxCurve[2];
    viewCurve.range[1]  = boundingBoxCurve[3];
#define SMALL 1e-100
    if (viewCurve.domainScale == LOG || viewCurve.rangeScale == LOG)
    {
        bool logsOkay = GetPlotList()->CanDoLogViewScaling(WINMODE_CURVE);
        if (logsOkay)
        {
            if (viewCurve.domainScale == LOG)
            {
                viewCurve.domain[0] = log10(fabs(viewCurve.domain[0]) + SMALL);
                viewCurve.domain[1] = log10(fabs(viewCurve.domain[1]) + SMALL);
                viewCurve.havePerformedLogDomain = true;
            }
            if (viewCurve.rangeScale == LOG)
            {
                viewCurve.range[0] = log10(fabs(viewCurve.range[0]) + SMALL);
                viewCurve.range[1] = log10(fabs(viewCurve.range[1]) + SMALL);
                viewCurve.havePerformedLogRange = true;
            }
        }
        else 
        {
            viewCurve.domainScale = LINEAR;
            viewCurve.rangeScale = LINEAR;
            viewCurve.havePerformedLogDomain = false;
            viewCurve.havePerformedLogRange = false;
            Warning("There are plots in the window that do not\n"
                    "support log-scaling.  It will not be done.");
        }
    }

    visWindow->SetViewCurve(viewCurve);

    //
    // Flag the view as unmodified.
    //
    viewModifiedCurve = false;
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
//    Kathleen Bonnell, Wed Jul 16 10:02:52 PDT 2003 
//    Don't scale view2D's window, handled in avtView2d. 
//
//    Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//    I moved the handling of full frame mode to VisWindow.
//
//    Kathleen Bonnell, Wed May  9 17:33:41 PDT 2007 
//    Support log scaling.
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

#define SMALL 1e-100
    if (view2D.xScale == LOG || view2D.yScale == LOG)
    {
        bool logsOkay = GetPlotList()->CanDoLogViewScaling(WINMODE_2D);
        if (logsOkay)
        {
            if (view2D.xScale == LOG)
            {
                view2D.window[0] = log10(fabs(view2D.window[0]) + SMALL);
                view2D.window[1] = log10(fabs(view2D.window[1]) + SMALL);
                view2D.havePerformedLogX = true;
            }
            if (view2D.yScale == LOG)
            {
                view2D.window[2] = log10(fabs(view2D.window[2]) + SMALL);
                view2D.window[3] = log10(fabs(view2D.window[3]) + SMALL);
                view2D.havePerformedLogY = true;
            }
        }
        else
        {
            view2D.xScale = LINEAR;
            view2D.yScale = LINEAR;
            view2D.havePerformedLogX = false;
            view2D.havePerformedLogY = false;
            Warning("There are plots in the window that do not\n"
                    "support log-scaling.  It will not be done.");
        }
    }

    visWindow->SetView2D(view2D);

    //
    // Flag the view as unmodified.
    //
    viewModified2d = false;
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
//    Eric Brugger, Tue Feb 10 09:59:15 PST 2004
//    I modified the routine to also reset the center of rotation.
//
//    Eric Brugger, Fri Apr 23 13:12:27 PDT 2004
//    I added coding to reset the focus in the case where the window had
//    no plots and the bounding box was valid.
//    
//    Eric Brugger, Tue Dec 28 08:44:10 PST 2004
//    I added code to set the center of rotation to handle the case where
//    the navigation mode is dolly.
//
// ****************************************************************************

void
ViewerWindow::RecenterView3d(const double *limits)
{
    //
    // Get the current view.
    //
    avtView3D view3D=visWindow->GetView3D();

    //
    // If the plot limits are invalid then there are no plots so mark the
    // centering as invalid so that the view is set using only the next
    // plots limits.  If the bounding box is valid then reset the focus
    // so that the next time the view is set it will be centered on the
    // new bounding box.
    //
    if (limits[0] == DBL_MAX && limits[1] == -DBL_MAX)
    {
        centeringValid3d = false;
        if (boundingBoxValid3d)
        {
            //
            // Calculate the new focal point.
            //
            view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2.;
            view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2.;
            view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2.;

            //
            // Update the view.
            //
            visWindow->SetView3D(view3D);
        }

        return;
    }

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
    // Reset the center of rotation.
    //
    view3D.centerOfRotationSet = false;
    view3D.centerOfRotation[0] = view3D.focus[0];
    view3D.centerOfRotation[1] = view3D.focus[1];
    view3D.centerOfRotation[2] = view3D.focus[2];

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::RecenterViewAxisArray
//
//  Purpose: 
//    Recenter the window's axis array view using the specified limits.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::RecenterViewAxisArray(const double *limits)
{
    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (limits[0] == DBL_MAX && limits[1] == -DBL_MAX)
    {
        centeringValidAxisArray = false;
        return;
    }

    //
    // Set the new window.
    //
    int       i;

    for (i = 0; i < 4; i++)
    {
        boundingBoxAxisArray[i] = limits[i];
    }

    //
    // Update the view.
    //
    avtViewAxisArray viewAxisArray=visWindow->GetViewAxisArray();
    viewAxisArray.domain[0] = boundingBoxAxisArray[0];
    viewAxisArray.domain[1] = boundingBoxAxisArray[1];
    viewAxisArray.range[0]  = boundingBoxAxisArray[2];
    viewAxisArray.range[1]  = boundingBoxAxisArray[3];

    visWindow->SetViewAxisArray(viewAxisArray);

    //
    // Flag the view as unmodified.
    //
    viewModifiedAxisArray = false;
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Thu Aug 28 12:27:42 PDT 2003
//    I modified the routine to set the viewModifiedCurve flag.
//
//    Eric Brugger, Tue Nov 18 11:13:48 PST 2003
//    I removed coding for handling degenerate views since this was now
//    handled in avtViewCurve.
//
//    Eric Brugger, Thu Sep 16 10:17:28 PDT 2004
//    I set all the flags indicating the view is valid if we successfully 
//    reset the view.  I also set more flags indicating the view is invalid
//    in the case were we don't reset the view just to be on the safe side.
//
//    Kathleen Bonnell, Thu Mar 22 19:24:21 PDT 2007 
//    Added support for Log scaling.
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Record when log of the domain/range coords has been done.  Ensure
//    we aren't attempting log scaling when plots don't support it.
//
// ****************************************************************************

void
ViewerWindow::ResetViewCurve()
{
    avtViewCurve viewCurve=visWindow->GetViewCurve();

    //
    // Set the bounding box based on the plot limits.
    //
    GetExtents(2, boundingBoxCurve);

    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (boundingBoxCurve[0] == DBL_MAX && boundingBoxCurve[1] == -DBL_MAX)
    {
        boundingBoxValidCurve = false;
        centeringValidCurve = false;
        viewSetInCurve = false;
        return;
    }

    //
    // Mark the view as valid.
    //
    boundingBoxValidCurve = true;
    centeringValidCurve = true;
    viewSetInCurve = true;

    //
    // Set the window.
    //
    viewCurve.domain[0]  = boundingBoxCurve[0];
    viewCurve.domain[1]  = boundingBoxCurve[1];
    viewCurve.range[0]   = boundingBoxCurve[2];
    viewCurve.range[1]   = boundingBoxCurve[3];
#define SMALL 1e-100
    
    if (viewCurve.domainScale == LOG || viewCurve.rangeScale == LOG)
    {
        bool logsOkay = GetPlotList()->CanDoLogViewScaling(WINMODE_CURVE);
        if (logsOkay)
        {
            if (viewCurve.domainScale == LOG)
            {
                viewCurve.domain[0] = log10(fabs(viewCurve.domain[0]) +SMALL);
                viewCurve.domain[1] = log10(fabs(viewCurve.domain[1]) +SMALL);
                viewCurve.havePerformedLogDomain = true;
            }
            if (viewCurve.rangeScale == LOG)
            {
                viewCurve.range[0] = log10(fabs(viewCurve.range[0]) +SMALL);
                viewCurve.range[1] = log10(fabs(viewCurve.range[1]) +SMALL);
                viewCurve.havePerformedLogRange = true;
            }
        }
        else
        {
            viewCurve.domainScale = LINEAR;
            viewCurve.rangeScale = LINEAR;
            viewCurve.havePerformedLogDomain = false;
            viewCurve.havePerformedLogRange = false;
            Warning("There are plots in the window that do not\n"
                  "support log-scaling.  It will not be done.");
 
        }
    }
    visWindow->SetViewCurve(viewCurve);

    //
    // Flag the view as unmodified.
    //
    viewModifiedCurve = false;
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
//    Kathleen Bonnell, Wed Jul 16 10:02:52 PDT 2003 
//    Don't scale view2D's window, handled in avtView2d. 
//
//    Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//    I moved the handling of full frame mode to VisWindow.
//
//    Eric Brugger, Thu Sep 16 10:17:28 PDT 2004
//    I set all the flags indicating the view is valid if we successfully 
//    reset the view.  I also set more flags indicating the view is invalid
//    in the case were we don't reset the view just to be on the safe side.
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added support for Log scaling.
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
        centeringValid2d = false;
        viewSetIn2d = false;
        return;
    }

    //
    // Mark the view as valid.
    //
    boundingBoxValid2d = true;
    centeringValid2d = true;
    viewSetIn2d = true;

    //
    // Set the window.
    //
    view2D.window[0]   = boundingBox2d[0];
    view2D.window[1]   = boundingBox2d[1];
    view2D.window[2]   = boundingBox2d[2];
    view2D.window[3]   = boundingBox2d[3];

#define SMALL 1e-100
    if (view2D.xScale == LOG || view2D.yScale == LOG)
    {
        bool logsOkay = GetPlotList()->CanDoLogViewScaling(WINMODE_2D);
        if (logsOkay)
        {
            if (view2D.xScale == LOG)
            {
                view2D.window[0] = log10(fabs(view2D.window[0]) + SMALL);
                view2D.window[1] = log10(fabs(view2D.window[1]) + SMALL);
                view2D.havePerformedLogX = true;
            }
            if (view2D.yScale == LOG)
            {
                view2D.window[2] = log10(fabs(view2D.window[2]) + SMALL);
                view2D.window[3] = log10(fabs(view2D.window[3]) + SMALL);
                view2D.havePerformedLogY = true;
            }
        }
        else
        {
            view2D.xScale = LINEAR;
            view2D.yScale = LINEAR;
            view2D.havePerformedLogX = false;
            view2D.havePerformedLogY = false;
            Warning("There are plots in the window that do not\n"
                  "support log-scaling.  It will not be done.");
        }
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
//    Eric Brugger, Mon Dec 22 10:24:14 PST 2003
//    Modify the routine to also set haveRenderedIn3d to false if the
//    bounding box is invalid so that the view really gets reset.
//
//    Eric Brugger, Tue Feb 10 09:59:15 PST 2004
//    I modified the routine to also reset the center of rotation.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedIn3d to viewSetIn3d, since it was more accurate.
//    I also added viewPartialSetIn3d to distinguish the view being set from
//    a session file and from the client.
//
//    Eric Brugger, Thu Sep 16 10:17:28 PDT 2004
//    I set all the flags indicating the view is valid if we successfully 
//    reset the view.  I also set more flags indicating the view is invalid
//    in the case were we don't reset the view just to be on the safe side.
//
//    Eric Brugger, Thu Oct 28 14:41:35 PDT 2004
//    I modified the routine to set the view differently depending on the
//    navigation mode.
//
//    Eric Brugger, Tue Dec 28 08:44:10 PST 2004
//    I modified the routine to treat the flythrough navigation mode
//    differently from the trackball and dolly modes.  I added code to set
//    the center of rotation to handle the case where the navigation mode
//    is dolly.
//
// ****************************************************************************

void
ViewerWindow::ResetView3d()
{
    avtView3D view3D=visWindow->GetView3D();

    //
    // Get the navigation mode.  The view is set differently depending
    // on whether the navigation mode is Trackball or Flythrough.
    //
    InteractorAttributes::NavigationMode navigationMode =
        visWindow->GetInteractorAtts()->GetNavigationMode();

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
        centeringValid3d = false;
        viewSetIn3d = false;
        viewPartialSetIn3d = false;
        return;
    }

    //
    // Mark the view as valid.
    //
    boundingBoxValid3d = true;
    centeringValid3d = true;
    viewSetIn3d = true;
    viewPartialSetIn3d = true;

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

    if (navigationMode == InteractorAttributes::Flythrough)
    {
        view3D.parallelScale = width / 20.;
    }
    else
    {
        view3D.parallelScale = width;
    }

    //
    // Set the view up vector, the focal point and the camera position.
    //
    view3D.normal[0] = 0.;
    view3D.normal[1] = 0.;
    view3D.normal[2] = 1.;

    view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2.;
    view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2.;
    if (navigationMode == InteractorAttributes::Flythrough)
    {
        view3D.focus[2] += (1.0 - 1.0 / 20.) * distance;
    }
    else
    {
        view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2.;
    }

    view3D.viewUp[0] = 0.;
    view3D.viewUp[1] = 1.;
    view3D.viewUp[2] = 0.;

    //
    // Calculate the near and far clipping planes.  The clipping planes are
    // set such that the object should not get clipped in the front or
    // back no matter the orientation when doing an orthographic projection.
    //
    if (navigationMode == InteractorAttributes::Flythrough)
    {
        view3D.nearPlane = - (1.0 / 20.) * distance * 0.99;
        view3D.farPlane  = (1.0 - 1.0 / 20.) * distance + 2.0 * width;
    }
    else
    {
        view3D.nearPlane = - 2.0 * width;
        view3D.farPlane  =   2.0 * width;
    }

    //
    // Reset the image pan and image zoom.
    //
    view3D.imagePan[0] = 0.;
    view3D.imagePan[1] = 0.;
    view3D.imageZoom = 1.;

    //
    // Reset the center of rotation.
    //
    view3D.centerOfRotationSet = false;
    view3D.centerOfRotation[0] = view3D.focus[0];
    view3D.centerOfRotation[1] = view3D.focus[1];
    view3D.centerOfRotation[2] = view3D.focus[2];

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::ResetViewAxisArray
//
//  Purpose: 
//    Reset the window's AxisArray view.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
// ****************************************************************************
void
ViewerWindow::ResetViewAxisArray()
{
    avtViewAxisArray viewAxisArray=visWindow->GetViewAxisArray();

    //
    // Set the bounding box based on the plot limits.
    //
    GetExtents(2, boundingBoxAxisArray);

    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (boundingBoxAxisArray[0] == DBL_MAX &&
        boundingBoxAxisArray[1] == -DBL_MAX)
    {
        boundingBoxValidAxisArray = false;
        centeringValidAxisArray = false;
        viewSetInAxisArray = false;
        return;
    }

    //
    // Mark the view as valid.
    //
    boundingBoxValidAxisArray = true;
    centeringValidAxisArray = true;
    viewSetInAxisArray = true;

    //
    // Set the window.
    //
    viewAxisArray.domain[0]   = boundingBoxAxisArray[0];
    viewAxisArray.domain[1]   = boundingBoxAxisArray[1];
    viewAxisArray.range[0]    = boundingBoxAxisArray[2];
    viewAxisArray.range[1]    = boundingBoxAxisArray[3];

    visWindow->SetViewAxisArray(viewAxisArray);

    //
    // Flag the view as unmodified.
    //
    viewModifiedAxisArray = false;
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
//  Modifications:
//    Eric Brugger, Fri Sep 17 12:28:17 PDT 2004
//    I modified the routine to use the diagonal of the bounding box to
//    determine the pan factor instead of the individual extents of the
//    bounding box.  This handles degenerate extents in either one or two
//    directions, whereas the previous method did not.
//
//    Eric Brugger, Tue Dec 28 08:44:10 PST 2004
//    I added code to set the center of rotation to handle the case where
//    the navigation mode is dolly.
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
                   view3D.focus[0]) / width;
    panFactor[1] = - (((boundingBox3d[3] + boundingBox3d[2]) / 2.) -
                   view3D.focus[1]) / width;
    panFactor[2] = - (((boundingBox3d[5] + boundingBox3d[4]) / 2.) -
                   view3D.focus[2]) / width;

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
    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));

    view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2. +
                      panFactor[0] * width;
    view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2. +
                      panFactor[1] * width;
    view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2. +
                      panFactor[2] * width;

    //
    // Calculate the new parallel scale.
    //
    view3D.parallelScale = width / zoomFactor;

    //
    // Calculate the near and far clipping planes.
    //
    view3D.nearPlane = - 2.0 * width;
    view3D.farPlane  =   2.0 * width;

    //
    // Set the center of rotation if the user hasn't specified it.
    //
    if (!view3D.centerOfRotationSet)
    {
        view3D.centerOfRotation[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2.;
        view3D.centerOfRotation[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2.;
        view3D.centerOfRotation[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2.;
    }

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
}

// ****************************************************************************
//  Method: ViewerWindow::SetInitialView3d
//
//  Purpose: 
//    Set the window's initial 3d view preserving any rotations or image
//    pans or zooms.
//
//  Arguments:
//
//  Programmer: Eric Brugger
//  Creation:   December 22, 2003
//
//  Modifications:
//    Eric Brugger, Thu Oct 28 14:41:35 PDT 2004
//    I modified the routine to set the view differently depending on the
//    navigation mode.
//
//    Eric Brugger, Tue Dec 28 08:44:10 PST 2004
//    I modified the routine to treat the flythrough navigation mode
//    differently from the trackball and dolly modes.  I added code to set
//    the center of rotation to handle the case where the navigation mode
//    is dolly.
//
// ****************************************************************************

void
ViewerWindow::SetInitialView3d()
{
    //
    // If the plot limits are invalid then there are no plots so mark the
    // bounding box as invalid so that the view is set from scratch the
    // next time it is updated.
    //
    if (boundingBox3d[0] == DBL_MAX && boundingBox3d[1] == -DBL_MAX)
    {
        centeringValid3d = false;
        return;
    }

    //
    // Get the navigation mode.  The view is set differently depending
    // on whether the navigation mode is Trackball or Flythrough.
    //
    InteractorAttributes::NavigationMode navigationMode =
        visWindow->GetInteractorAtts()->GetNavigationMode();

    //
    // Get the current view.
    //
    avtView3D view3D=visWindow->GetView3D();

    //
    // Calculate the new parallel scale.
    //
    double    width;
    double    distance;

    width = 0.5 * sqrt(((boundingBox3d[1] - boundingBox3d[0]) *
                        (boundingBox3d[1] - boundingBox3d[0])) +
                       ((boundingBox3d[3] - boundingBox3d[2]) *
                        (boundingBox3d[3] - boundingBox3d[2])) +
                       ((boundingBox3d[5] - boundingBox3d[4]) *
                        (boundingBox3d[5] - boundingBox3d[4])));
    distance = width / tan (view3D.viewAngle * 3.1415926535 / 360.);

    if (navigationMode == InteractorAttributes::Flythrough)
    {
        view3D.parallelScale = width / 20.;
    }
    else
    {
        view3D.parallelScale = width;
    }

    //
    // Calculate the new focal point.
    //
    view3D.focus[0] = (boundingBox3d[1] + boundingBox3d[0]) / 2.;
    view3D.focus[1] = (boundingBox3d[3] + boundingBox3d[2]) / 2.;
    if (navigationMode == InteractorAttributes::Flythrough)
    {
        view3D.focus[2] += (1.0 - 1.0 / 20.) * distance;
    }
    else
    {
        view3D.focus[2] = (boundingBox3d[5] + boundingBox3d[4]) / 2.;
    }

    //
    // Calculate the near and far clipping planes.
    //
    if (navigationMode == InteractorAttributes::Flythrough)
    {
        view3D.nearPlane = - (1.0 / 20.) * distance * 0.99;
        view3D.farPlane  = (1.0 - 1.0 / 20.) * distance + 2.0 * width;
    }
    else
    {
        view3D.nearPlane = - 2.0 * width;
        view3D.farPlane  =   2.0 * width;
    }

    //
    // Set the center of rotation if the user hasn't specified it.
    //
    if (!view3D.centerOfRotationSet)
    {
        view3D.centerOfRotation[0] = view3D.focus[0];
        view3D.centerOfRotation[1] = view3D.focus[1];
        view3D.centerOfRotation[2] = view3D.focus[2];
    }

    //
    // Update the view.
    //
    visWindow->SetView3D(view3D);
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
//    Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//    I added a curve view.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedInCurve to viewSetInCurve, since it was more
//    accurate.
//
//    Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//    Made it set bools appropriately for call to vwm->UpdateViewAtts
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added support for Log scaling.
//
// ****************************************************************************

void
ViewerWindow::UpdateViewCurve(const double *limits)
{
    //
    // If this is the first time that this routine is being called for this
    // window, set the limits and reset the view.
    //
    if (boundingBoxValidCurve == false)
    {
        for (int i = 0; i < 4; i++)
            boundingBoxCurve[i] = limits[i];

        boundingBoxValidCurve = true;
        centeringValidCurve   = true;
        const avtViewCurve &viewCurve = GetViewCurve();
        bool mustReset = 
            (viewCurve.domainScale == LOG && !viewCurve.havePerformedLogDomain)
         || (viewCurve.rangeScale  == LOG && !viewCurve.havePerformedLogRange);
          

        if (!viewSetInCurve || mustReset)
        {
            ResetViewCurve();
        }
        else
        {
            //
            // Update the view to scale any new plots appropriately.
            //
            visWindow->UpdateView();
        }

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, true, false, false);
    }
    //
    // If the centering is invalid or if maintain view is off, the view has
    // not been modified and the limits have changed, recenter the view.
    // The recentering uses the current limits or the merged limits from
    // the previous plots based on the mergeViewLimits flag.
    //
    else if (centeringValidCurve == false ||
             maintainView == false && viewModifiedCurve == false &&
             (limits[0] != boundingBoxCurve[0] ||
              limits[1] != boundingBoxCurve[1] ||
              limits[2] != boundingBoxCurve[2] ||
              limits[3] != boundingBoxCurve[3]))
    {
        if (centeringValidCurve == true && mergeViewLimits == true)
        {
            boundingBoxCurve[0] = boundingBoxCurve[0] < limits[0] ?
                                  boundingBoxCurve[0] : limits[0];
            boundingBoxCurve[1] = boundingBoxCurve[1] > limits[1] ?
                                  boundingBoxCurve[1] : limits[1];
            boundingBoxCurve[2] = boundingBoxCurve[2] < limits[2] ?
                                  boundingBoxCurve[2] : limits[2];
            boundingBoxCurve[3] = boundingBoxCurve[3] > limits[3] ?
                                  boundingBoxCurve[3] : limits[3];

            RecenterViewCurve(boundingBoxCurve);
        }
        else
        {
            centeringValidCurve   = true;

            RecenterViewCurve(limits);
        }

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, true, false, false);
    }
    //
    // Update the view to scale any new plots appropriately.
    //
    else
    {
        const avtViewCurve &viewCurve = GetViewCurve();
        bool mustReset = 
            ((viewCurve.domainScale == LOG  || viewCurve.rangeScale == LOG ) &&
              !GetPlotList()->CanDoLogViewScaling(WINMODE_CURVE));
       
        if (!mustReset) 
            visWindow->UpdateView();
        else 
            ResetViewCurve();
    }

    viewSetInCurve = true;
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
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedIn2d to viewSetIn2d, since it was more accurate.
//
//    Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//    Added logic to support auto full frame mode. Made call to
//    vwm->UpdateViewAtts use appropriate bool flags
//
//    Mark C. Miller, Tue Mar 14 17:49:26 PST 2006
//    Retrieved full frame activation mode from avtView class, not atts.
//
//    Brad Whitlock, Wed Jul 26 13:33:25 PST 2006
//    Added code to set a fullframe scale into the plots so their mappers
//    can compensate for fullframe if they need to.
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added support for Log scaling.
//
// ****************************************************************************

void
ViewerWindow::UpdateView2d(const double *limits)
{
    const avtView2D &view2d = GetView2D();
    View2DAttributes view2dAtts;
    view2d.SetToView2DAttributes(&view2dAtts);

    bool currentFullFrameMode = GetFullFrameMode();
    if (view2d.fullFrameActivationMode == (int) View2DAttributes::Auto)
    {
        bool newFullFrameMode = view2dAtts.GetUseFullFrame(limits); 

        // if plot's units are different, that overrides other considerations 
        if (plotList)
        {
            if (!DoAllPlotsAxesHaveSameUnits())
                newFullFrameMode = true;
        }
            
        if (currentFullFrameMode != newFullFrameMode)
        {
            SetFullFrameMode(newFullFrameMode);
            currentFullFrameMode = newFullFrameMode;
        }
    }

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

        bool mustReset = (view2d.xScale == LOG && !view2d.havePerformedLogX)
                      || (view2d.yScale == LOG && !view2d.havePerformedLogY);

        if (!viewSetIn2d || mustReset)
        {
            ResetView2d();
        }

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, false, true, false);
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

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, false, true, false);
    }


    //
    // Get the fullframe scale and pass it to all of the plots in the plot
    // list so the ones that have mappers that need to readjust can do so.
    //
    double scale[] = {1., 1., 1.}, s;
    int t = 0;
    visWindow->GetScaleFactorAndType(s, t);
    if(t == 0)
        scale[0] = s;
    else if(t == 1)
        scale[1] = s;
    debug5 << "ViewerWindow::UpdateView2d: Fullframe="
           << (currentFullFrameMode?"true":"false")
           << ". scale={" << scale[0] << ", " << scale[1] << ", " << scale[2]
           << "}" << endl;
    plotList->SetFullFrameScaling(currentFullFrameMode, scale);

    // 
    // Ensure we aren't attempting log scaling when plots don't support it.
    // 
    if ((view2d.xScale == LOG  || view2d.yScale == LOG ) &&
        !GetPlotList()->CanDoLogViewScaling(WINMODE_2D))
    {
        ResetView2d();
    }

    viewSetIn2d = true;
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
//    Eric Brugger, Mon Dec 22 10:24:14 PST 2003
//    Modify the way the routine sets the view for the first time or after
//    a reset with no plots so that coordinate extent parameters are reset
//    but rotations and image pans and zooms are preserved.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed haveRenderedIn3d to viewSetIn3d, since it was more accurate.
//    I also added viewPartialSetIn3d to distinguish the view being set from
//    a session file and from the client.
//
//    Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//    Added bools to call to vwm->UpdateViewAtts
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

        if (!viewSetIn3d)
        {
            if (!viewPartialSetIn3d)
            {
                ResetView3d();
            }
            else
            {
                SetInitialView3d();
            }
        }

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, false, false, true);
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

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, false, false, true);
    }

    viewSetIn3d = true;
    viewPartialSetIn3d = true;
}

// ****************************************************************************
//  Method: ViewerWindow::UpdateViewAxisArray
//
//  Purpose: 
//    Update the axis array view for the window.
//
//  Arguments:
//    limits    The limits of all the plots.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
// ****************************************************************************
void
ViewerWindow::UpdateViewAxisArray(const double *limits)
{
    //
    // If this is the first time that this routine is being called for this
    // window, set the limits and reset the view.
    //
    if (boundingBoxValidAxisArray == false)
    {
        for (int i = 0; i < 4; i++)
            boundingBoxAxisArray[i] = limits[i];

        boundingBoxValidAxisArray = true;
        centeringValidAxisArray   = true;
        const avtViewAxisArray &viewAxisArray = GetViewAxisArray();

        if (!viewSetInAxisArray)
        {
            ResetViewAxisArray();
        }
        else
        {
            //
            // Update the view to scale any new plots appropriately.
            //
            visWindow->UpdateView();
        }

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, true, false, false);
    }
    //
    // If the centering is invalid or if maintain view is off, the view has
    // not been modified and the limits have changed, recenter the view.
    // The recentering uses the current limits or the merged limits from
    // the previous plots based on the mergeViewLimits flag.
    //
    else if (centeringValidAxisArray == false ||
             maintainView == false && viewModifiedAxisArray == false &&
             (limits[0] != boundingBoxAxisArray[0] ||
              limits[1] != boundingBoxAxisArray[1] ||
              limits[2] != boundingBoxAxisArray[2] ||
              limits[3] != boundingBoxAxisArray[3]))
    {
        if (centeringValidAxisArray == true && mergeViewLimits == true)
        {
            boundingBoxAxisArray[0] = boundingBoxAxisArray[0] < limits[0] ?
                                      boundingBoxAxisArray[0] : limits[0];
            boundingBoxAxisArray[1] = boundingBoxAxisArray[1] > limits[1] ?
                                      boundingBoxAxisArray[1] : limits[1];
            boundingBoxAxisArray[2] = boundingBoxAxisArray[2] < limits[2] ?
                                      boundingBoxAxisArray[2] : limits[2];
            boundingBoxAxisArray[3] = boundingBoxAxisArray[3] > limits[3] ?
                                      boundingBoxAxisArray[3] : limits[3];

            RecenterViewAxisArray(boundingBoxAxisArray);
        }
        else
        {
            centeringValidAxisArray   = true;

            RecenterViewAxisArray(limits);
        }

        ViewerWindowManager::Instance()->UpdateViewAtts(-1, true, false, false);
    }
    //
    // Update the view to scale any new plots appropriately.
    //
    else
    {
        const avtViewAxisArray &viewAxisArray = GetViewAxisArray();
        visWindow->UpdateView();
    }

    viewSetInAxisArray = true;
}

// ****************************************************************************
// Method: ViewerWindow::SetCenterOfRotation
//
// Purpose: 
//   Sets the center of rotation for the plots in the window.
//
// Arguments:
//   x,y,z : The new center of rotation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 6 10:29:13 PDT 2004
//
// Modifications:
//   Eric Brugger, Tue Feb 10 09:59:15 PST 2004
//   I changed the code to set the center of rotation instead of the focus.
//   
// ****************************************************************************

void
ViewerWindow::SetCenterOfRotation(double x, double y, double z)
{
    avtView3D view(GetView3D());

    view.centerOfRotationSet = true;
    view.centerOfRotation[0] = x;
    view.centerOfRotation[1] = y;
    view.centerOfRotation[2] = z;

    SetView3D(view);
}

// ****************************************************************************
// Method: ViewerWindow::ChooseCenterOfRotation
//
// Purpose: 
//   Picks the center of rotation to be the world space point that lies at
//   the screen point.
//
// Arguments:
//   sx, sy : The screen point where sx,sy in [0,1].
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 6 10:27:53 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006
//   Use direct visWindow->FindIntersection if !SR, pick is too complex for
//   this simple operation.  If SR mode, call GetPickAttributesForScreenPoint,
//   which will have the engine query it's viswin for the intersection. 
//   
// ****************************************************************************

void
ViewerWindow::ChooseCenterOfRotation(double sx, double sy)
{
    double isect[3];
    bool success = false;
    int w,h;
    visWindow->GetSize(w, h);
    int x = (int)(sx * w);
    int y = (int)(sy * h);

    if (!visWindow->GetScalableRendering())
    {
        success = visWindow->FindIntersection(x, y, isect);
    }
    else
    {
        PickAttributes pick;
        GetPickAttributesForScreenPoint((double)x, (double)y, pick);
        if (pick.GetFulfilled())
        {
            isect[0] = pick.GetPickPoint()[0];
            isect[1] = pick.GetPickPoint()[1];
            isect[2] = pick.GetPickPoint()[2];
            success = true;
        }
    }
    if (success)
    {
        // Use the pick point as the new center of rotation.
        SetCenterOfRotation(isect[0], isect[1], isect[2]);
    }
    else
    {
        Warning("VisIt could not pick the center of rotation. "
                "You might not have clicked on a plot.");
    }
}


// ****************************************************************************
//  Method: ViewerWindow::ConvertFromLeftEyeToRightEye
//
//  Purpose:
//      Converts from a left eye view to a right eye view.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
ViewerWindow::ConvertFromLeftEyeToRightEye(void)
{
    const avtView3D &curView = GetView3D();
    double eyeAngle = curView.eyeAngle;
    avtView3D newView;
    RotateAroundY(curView, -eyeAngle, newView);
    SetView3D(newView);
}

// ****************************************************************************
//  Method: ViewerWindow::ConvertFromRightEyeToLeftEye
//
//  Purpose:
//      Converts from a right eye view to a left eye view.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
ViewerWindow::ConvertFromRightEyeToLeftEye(void)
{
    const avtView3D &curView = GetView3D();
    double eyeAngle = curView.eyeAngle;
    avtView3D newView;
    RotateAroundY(curView, +eyeAngle, newView);
    SetView3D(newView);
}

// ****************************************************************************
//  Function: RotateAroundY
//
//  Purpose:
//      Rotates around the y-axis by a specified angle.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2003
//
// ****************************************************************************

static void
RotateAroundY(const avtView3D &curView, double angle,
                            avtView3D &newView)
{
    double angleRadians;
    double v1[3], v2[3], v3[3];
    double m1[9], m2[9], m3[9], r[9];
    double rotationMatrix[9];
    double viewNormal[3];
    double viewUp[3];

    //
    // Calculate the rotation matrix in screen coordinates.
    //
    angleRadians = angle * (3.141592653589793 / 180.);
    r[0] = cos(angleRadians);
    r[1] = 0.;
    r[2] = sin(angleRadians);
    r[3] = 0.;
    r[4] = 1.;
    r[5] = 0.;
    r[6] = - sin(angleRadians);
    r[7] = 0.;
    r[8] = cos(angleRadians);

    //
    // Calculate the matrix to rotate from object coordinates to screen
    // coordinates and its inverse.
    //
    v1[0] = curView.normal[0];
    v1[1] = curView.normal[1];
    v1[2] = curView.normal[2];
    double mag = sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]);
    if (mag == 0)
        mag = 1.;
    v1[0] /= mag;
    v1[1] /= mag;
    v1[2] /= mag;

    v2[0] = curView.viewUp[0];
    v2[1] = curView.viewUp[1];
    v2[2] = curView.viewUp[2];
    mag = sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
    if (mag == 0)
        mag = 1.;
    v2[0] /= mag;
    v2[1] /= mag;
    v2[2] /= mag;

    v3[0] =   v2[1]*v1[2] - v2[2]*v1[1];
    v3[1] = - v2[0]*v1[2] + v2[2]*v1[0];
    v3[2] =   v2[0]*v1[1] - v2[1]*v1[0];
    mag = sqrt(v3[0]*v3[0] + v3[1]*v3[1] + v3[2]*v3[2]);
    if (mag == 0)
        mag = 1.;
    v3[0] /= mag;
    v3[1] /= mag;
    v3[2] /= mag;

    // View normal and view up may not be orthogonal -- make sure that they
    // are by crossing v3 and view normal again.
    v2[0] =   v1[1]*v3[2] - v1[2]*v3[1];
    v2[1] = - v1[0]*v3[2] + v1[2]*v3[0];
    v2[2] =   v1[0]*v3[1] - v1[1]*v3[0];
    mag = sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
    if (mag == 0)
        mag = 1.;
    v2[0] /= mag;
    v2[1] /= mag;
    v2[2] /= mag;

    m1[0] = v3[0];
    m1[1] = v2[0];
    m1[2] = v1[0];
    m1[3] = v3[1];
    m1[4] = v2[1];
    m1[5] = v1[1];
    m1[6] = v3[2];
    m1[7] = v2[2];
    m1[8] = v1[2];

    m2[0] = m1[0];
    m2[1] = m1[3];
    m2[2] = m1[6];
    m2[3] = m1[1];
    m2[4] = m1[4];
    m2[5] = m1[7];
    m2[6] = m1[2];
    m2[7] = m1[5];
    m2[8] = m1[8];

    //
    // Form the composite transformation matrix m1 X r X m2.
    //
    m3[0] = m1[0]*r[0] + m1[1]*r[3] + m1[2]*r[6];
    m3[1] = m1[0]*r[1] + m1[1]*r[4] + m1[2]*r[7];
    m3[2] = m1[0]*r[2] + m1[1]*r[5] + m1[2]*r[8];
    m3[3] = m1[3]*r[0] + m1[4]*r[3] + m1[5]*r[6];
    m3[4] = m1[3]*r[1] + m1[4]*r[4] + m1[5]*r[7];
    m3[5] = m1[3]*r[2] + m1[4]*r[5] + m1[5]*r[8];
    m3[6] = m1[6]*r[0] + m1[7]*r[3] + m1[8]*r[6];
    m3[7] = m1[6]*r[1] + m1[7]*r[4] + m1[8]*r[7];
    m3[8] = m1[6]*r[2] + m1[7]*r[5] + m1[8]*r[8];

    rotationMatrix[0] = m3[0]*m2[0] + m3[1]*m2[3] + m3[2]*m2[6];
    rotationMatrix[1] = m3[0]*m2[1] + m3[1]*m2[4] + m3[2]*m2[7];
    rotationMatrix[2] = m3[0]*m2[2] + m3[1]*m2[5] + m3[2]*m2[8];
    rotationMatrix[3] = m3[3]*m2[0] + m3[4]*m2[3] + m3[5]*m2[6];
    rotationMatrix[4] = m3[3]*m2[1] + m3[4]*m2[4] + m3[5]*m2[7];
    rotationMatrix[5] = m3[3]*m2[2] + m3[4]*m2[5] + m3[5]*m2[8];
    rotationMatrix[6] = m3[6]*m2[0] + m3[7]*m2[3] + m3[8]*m2[6];
    rotationMatrix[7] = m3[6]*m2[1] + m3[7]*m2[4] + m3[8]*m2[7];
    rotationMatrix[8] = m3[6]*m2[2] + m3[7]*m2[5] + m3[8]*m2[8];

    //
    // Calculate the new view normal and view up.
    //
    viewNormal[0] = curView.normal[0] * rotationMatrix[0] +
                    curView.normal[1] * rotationMatrix[3] +
                    curView.normal[2] * rotationMatrix[6];
    viewNormal[1] = curView.normal[0] * rotationMatrix[1] +
                    curView.normal[1] * rotationMatrix[4] +
                    curView.normal[2] * rotationMatrix[7];
    viewNormal[2] = curView.normal[0] * rotationMatrix[2] +
                    curView.normal[1] * rotationMatrix[5] +
                    curView.normal[2] * rotationMatrix[8];

    viewUp[0] = curView.viewUp[0] * rotationMatrix[0] +
                curView.viewUp[1] * rotationMatrix[3] +
                curView.viewUp[2] * rotationMatrix[6];
    viewUp[1] = curView.viewUp[0] * rotationMatrix[1] +
                curView.viewUp[1] * rotationMatrix[4] +
                curView.viewUp[2] * rotationMatrix[7];
    viewUp[2] = curView.viewUp[0] * rotationMatrix[2] +
                curView.viewUp[1] * rotationMatrix[5] +
                curView.viewUp[2] * rotationMatrix[8];

    newView = curView;
    newView.normal[0] = viewNormal[0];
    newView.normal[1] = viewNormal[1];
    newView.normal[2] = viewNormal[2];
    newView.viewUp[0] = viewUp[0];
    newView.viewUp[1] = viewUp[1];
    newView.viewUp[2] = viewUp[2];
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
// Method: ViewerWindow::SetLargeIcons
//
// Purpose: 
//   Sets whether the window should use large icons.
//
// Arguments:
//   val : Whether the window should use large icons.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:35:59 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetLargeIcons(bool val)
{
    visWindow->SetLargeIcons(val);
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
//   Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//   I added a curve view.
//
//   Jeremy Meredith, Fri Nov 14 11:33:24 PST 2003
//   Added specular properties.
//
//   Hank Childs, Mon May 10 08:04:48 PDT 2004
//   Use "display list mode" instead of immediate rendering mode.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified scalable rendering controls to use activation mode and auto
//   threshold
//
//   Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//   Add shading.
//
//   Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//   Added code to set stereo rendering information
//
//   Brad Whitlock, Mon Sep 18 10:57:15 PDT 2006
//   Added color texturing flag.
//
//   Jeremy Meredith, Wed Aug 29 15:21:38 EDT 2007
//   Added depth cueing properties.
//
//   Brad Whitlock, Mon Nov 19 14:37:52 PST 2007
//   Added support for background images.
//
//   Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//   Added new axis array window mode.
//
// ****************************************************************************

WindowAttributes
ViewerWindow::GetWindowAttributes() const
{
    WindowAttributes winAtts;

    //
    // Set the view
    //
    ViewCurveAttributes viewCurveAtts;
    const avtViewCurve &viewCurve = GetViewCurve();
    viewCurve.SetToViewCurveAttributes(&viewCurveAtts);
    winAtts.SetViewCurve(viewCurveAtts);
   
    View2DAttributes view2DAtts;
    const avtView2D &view2d = GetView2D();
    view2d.SetToView2DAttributes(&view2DAtts);
    winAtts.SetView2D(view2DAtts);

    View3DAttributes view3DAtts;
    const avtView3D &view3d = GetView3D();
    view3d.SetToView3DAttributes(&view3DAtts);
    winAtts.SetView3D(view3DAtts);

    ViewAxisArrayAttributes viewAxisArrayAtts;
    const avtViewAxisArray &viewAxisArray = GetViewAxisArray();
    viewAxisArray.SetToViewAxisArrayAttributes(&viewAxisArrayAtts);
    winAtts.SetViewAxisArray(viewAxisArrayAtts);

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
    double gbg[3];
    gbg[0] = double(annot->GetGradientColor1().Red()) / 255.;
    gbg[1] = double(annot->GetGradientColor1().Green()) / 255.;
    gbg[2] = double(annot->GetGradientColor1().Blue()) / 255.;
    winAtts.SetGradBG1(gbg);
    gbg[0] = double(annot->GetGradientColor2().Red()) / 255.;
    gbg[1] = double(annot->GetGradientColor2().Green()) / 255.;
    gbg[2] = double(annot->GetGradientColor2().Blue()) / 255.;
    winAtts.SetGradBG2(gbg);
    winAtts.SetBackgroundMode((int)annot->GetBackgroundMode());
    winAtts.SetGradientBackgroundStyle((int)annot->GetGradientBackgroundStyle());
    winAtts.SetBackgroundImage(annot->GetBackgroundImage());
    winAtts.SetImageRepeatX(annot->GetImageRepeatX());
    winAtts.SetImageRepeatY(annot->GetImageRepeatY());    winAtts.SetBackgroundMode((int)annot->GetBackgroundMode());

    // Rendering attributes
    RenderingAttributes renderAtts;
    renderAtts.SetScalableAutoThreshold(GetScalableThreshold());
    renderAtts.SetScalableActivationMode(
        (RenderingAttributes::TriStateMode) GetScalableActivationMode());

    renderAtts.SetDisplayListMode(
        (RenderingAttributes::TriStateMode) GetDisplayListMode());

    renderAtts.SetAntialiasing(GetAntialiasing());

    renderAtts.SetGeometryRepresentation(
       (RenderingAttributes::GeometryRepresentation) GetSurfaceRepresentation());

    renderAtts.SetSpecularFlag(GetSpecularFlag());
    renderAtts.SetSpecularCoeff(GetSpecularCoeff());
    renderAtts.SetSpecularPower(GetSpecularPower());
    renderAtts.SetSpecularColor(GetSpecularColor());

    renderAtts.SetStereoRendering(GetStereo());
    renderAtts.SetStereoType((RenderingAttributes::StereoTypes)GetStereoType());

    renderAtts.SetDoShadowing(GetDoShading());
    renderAtts.SetShadowStrength(GetShadingStrength());

    renderAtts.SetDoDepthCueing(GetDoDepthCueing());
    renderAtts.SetStartCuePoint(GetStartCuePoint());
    renderAtts.SetEndCuePoint(GetEndCuePoint());

    renderAtts.SetColorTexturingFlag(GetColorTexturingFlag());

    renderAtts.SetCompressionActivationMode(
        (RenderingAttributes::TriStateMode) compressionActivationMode);

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
// Method: ViewerWindow::SendWindowEnvironmentToEngine
//
// Purpose: 
//   Sends the window environment (i.e. window size, colors, lights, etc.)
//   to the engine that might need it for scalable rendering.
//
// Arguments:
//   host : The host to which we want to send the window environment.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jan 25 23:49:44 PST 2004
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 11:04:28 PST 2004
//    Added an engine key used to index (and restart) engines.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added code to get and pass extents type string to SetWinAnnotAtts
//   
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added code to pass annotation object list
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to deal with visual cues (pick points and ref lines)
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added code to deal with frame and state info
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to deal with view extents
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to manage name of last color table to change
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to pass window id to Engine Manager
// ****************************************************************************

bool
ViewerWindow::SendWindowEnvironmentToEngine(const EngineKey &ek)
{
    WindowAttributes winAtts(GetWindowAttributes());
    AnnotationAttributes annotAtts(*GetAnnotationAttributes());
    AnnotationObjectList annotObjs;
    UpdateAnnotationObjectList(annotObjs);
    string extStr(avtExtentType_ToString(GetViewExtentsType())); 
    VisualCueList visCues;
    UpdateVisualCueList(visCues);
    int fns[7];
    visWindow->GetFrameAndState(fns[0], fns[1], fns[2], fns[3],
                                        fns[4], fns[5], fns[6]);
    double vexts[6];
    VisItInitExtentsToLimits(vexts, 6);
    GetExtents(visWindow->GetWindowMode()==WINMODE_3D?3:2, vexts);
    if (visWindow->GetWindowMode()!=WINMODE_3D)
    {
        vexts[4] = 0.0;
        vexts[5] = 0.0;
    }

    return ViewerEngineManager::Instance()->SetWinAnnotAtts(ek,
               &winAtts, &annotAtts, &annotObjs, extStr, &visCues,
               fns, vexts,"", GetWindowId());
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
//   Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//   I added a curve view.
//
//   Kathleen Bonnell, Tue Dec  2 17:36:44 PST 2003 
//   Allow pick to work in curve view. 
//
// ****************************************************************************

void
ViewerWindow::Pick(int x, int y, const INTERACTION_MODE pickMode)
{
    // Set the interaction mode to pick.
    INTERACTION_MODE iMode = visWindow->GetInteractionMode();
    SetInteractionMode(pickMode);

    // Perform a screen space pick operation.
    visWindow->Pick(x, y);

    // Restore the interaction mode.
    SetInteractionMode(iMode);
}

// ****************************************************************************
// Method: ViewerWindow::GetPickAttributesForScreenPoint
//
// Purpose: 
//   Performs a pick at the specified screen location but does not add any sort
//   of pick letter. Instead, just return the pick attributes that were
//   determined for the point.
//
// Arguments:
//   sx, sy : The screen location of the pick point. Both sx, sy are in [0,1].
//   pa     : The pick attributes to populate.
//
// Returns:    True if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 6 09:37:03 PDT 2004
//
// Modifications:
//    Kathleen Bonnell, Thu Sep  2 13:55:05 PDT 2004 
//    Tell the visWindow that Pick will be an intersection-only. 
//   
//    Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006 
//    Made this an SR-only method.  It directly calls an Engine-Pick that
//    will perform an intersection-only pick, less complex than an ordinary
//    pick. 
//   
//    Hank Childs, Tue Sep  4 15:03:48 PDT 2007
//    Add better error handling.
//
// ****************************************************************************

bool
ViewerWindow::GetPickAttributesForScreenPoint(double sx, double sy,
    PickAttributes &pa)
{
    if (!visWindow->GetScalableRendering())
        return false;

    bool retval = false;
    TRY
    {
        ViewerPlotList *plist = GetPlotList();
        intVector plotIds;
        plist->GetActivePlotIDs(plotIds);
        EngineKey key = plist->GetPlot(plotIds[0])->GetEngineKey();
        if (ViewerEngineManager::Instance()->EngineExists(key))
        {
            intVector netIds;
            for (int i = 0; i < plotIds.size(); i++)
            {
                netIds.push_back(plist->GetPlot(plotIds[i])->GetNetworkID());
            }
            PickAttributes pick;
            pick.SetIncidentElements(netIds);
            double sc[3];
            sc[0] = sx;
            sc[1] = sy;
            sc[2] = 0.; 
            pick.SetRayPoint1(sc);
            ViewerEngineManager::Instance()->Pick(key, -1, GetWindowId(),
                                                  &pick, pick);
            pa = pick;
            retval = pick.GetFulfilled();
        }
    }
    CATCH2(VisItException, e)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "An error occurred while trying to take a pick."
                 "\nThis happens most often when trying to do a choose center"
                 " while the engine times out or otherwise has problems."
                 "\n\nThe type of exception was: %s"
                 "\nThe error message was: %s",
                 e.GetExceptionType().c_str(),
                 e.Message().c_str());
        Error(msg);
    }
    ENDTRY

    return retval;
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
//    Brad Whitlock, Wed Jan 7 09:15:33 PDT 2004
//    I made the pick be handled in the HandlePick method so we'd have access
//    to member fields. Another reason I did it this way is so I can make pick
//    do other things without having to change its callback with the vis window.
//
// ****************************************************************************

void
ViewerWindow::PerformPickCallback(void *data)
{
    if(data == 0)
        return;

    //
    // Let the ViewerWindow handle the pick since it could do various things
    // with the pick request.
    //
    PICK_POINT_INFO *ppi = (PICK_POINT_INFO*)data;
    ViewerWindow *win = (ViewerWindow *)ppi->callbackData;
    win->HandlePick(data);
}

// ****************************************************************************
// Method: ViewerWindow::HandlePick
//
// Purpose: 
//   Handles the pick request by either doing a complete pick or by picking
//   and then giving the pick information to a user-supplied pick function.
//
// Arguments:
//   data : A pointer to the PICK_POINT_INFO.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 09:18:58 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Sep  2 13:55:05 PDT 2004
//   If the pick was an intersection-only, set the PickPoint to be the
//   pickpoint info's rayPoint (or intersection point). Reset the visWindow's 
//   pick type to Normal
//
// ****************************************************************************

void
ViewerWindow::HandlePick(void *data)
{
    ViewerQueryManager *qMgr = ViewerQueryManager::Instance();
    PICK_POINT_INFO *ppi = (PICK_POINT_INFO*)data;

    TRY
    {
        if(pickFunction == 0)
        {
            // 
            // Let the Query manager do a pick
            //
            qMgr->Pick(ppi);
        }
        else if (ppi->intersectionOnly)
        {
            PickAttributes *p = qMgr->GetPickAtts();
            p->SetPickPoint(ppi->rayPt1);
            bool pickWorked = ppi->validPick;
            (*pickFunction)(pickFunctionData, pickWorked, p);
        }
        else
        {
            // 
            // Let the Query manager do a pick but don't add anything to the
            // vis window.
            //
            qMgr->NoGraphicsPick(ppi);

            //
            // Call the pick function with the pick attributes and a flag
            // that indicates whether pick really did something.
            //
            PickAttributes *p = qMgr->GetPickAtts();
            if(p)
            {
                bool pickWorked = ppi->validPick;
                (*pickFunction)(pickFunctionData, pickWorked, p);
            }
        }

        //
        // Clear the pick function so it defaults to Pick and not some
        // user-defined function.
        //
        pickFunction = 0;
        pickFunctionData = 0;
        visWindow->SetPickTypeToNormal();
    }
    CATCH(VisItException)
    {
        pickFunction = 0;
        pickFunctionData = 0;
        visWindow->SetPickTypeToNormal();
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerWindow::SetPickFunction
//
// Purpose: 
//   Sets the pick function to execute after pick has been performed.
//
// Arguments:
//   func : The callback function.
//   data : The callback function data.
//   intersectOnly : whether the pick will be performed as intersection only
//                   or normally.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 09:51:59 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Sep  2 13:55:05 PDT 2004
//   Set the VisWindow's pick type based on the new flag.
//   
// ****************************************************************************

void
ViewerWindow::SetPickFunction(void (*func)(void *, bool, const PickAttributes *),
    void *data, bool intersectOnly)
{
    pickFunction = func;
    pickFunctionData = data;
    if (intersectOnly)
        visWindow->SetPickTypeToIntersection();
    else 
        visWindow->SetPickTypeToNormal();
}

// ****************************************************************************
// Method: ViewerWindow::PickFunctionSetSuccessFlag
//
// Purpose: 
//   Causes no action to be done after a pick.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 6 09:35:33 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::PickFunctionSetSuccessFlag(void *data, bool success,
    const PickAttributes *)
{
    // This function is used to set the return value for
    // GetPickAttributesForScreenPoint.
    if(data)
    {
        // Make it return whether or not we got a valid pick.
        bool *retval = (bool *)data;
        *retval = success;
    }
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
//    Hank Childs, Wed Sep 12 20:53:11 PDT 2007
//    Tell all the plots in the window that the pick points were cleared.
//    (This is important for the spreadsheet.)
//
// ****************************************************************************

void
ViewerWindow::ClearPickPoints()
{
    visWindow->ClearPickPoints();
    ViewerQueryManager::Instance()->ClearPickPoints();

    // Tell each plot in the window that the pick points have been cleared.
    // This is important for the spreadsheet plot.
    PickAttributes pickAtts;
    pickAtts.SetClearWindow(true);
    ViewerPlotList *plist = GetPlotList();
    int numPlots = plist->GetNumPlots();
    for (int i = 0 ; i < numPlots ; i++)
        plist->GetPlot(i)->SetPlotAtts(&pickAtts);
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
//    Kathleen Bonnell, Fri Jul  9 16:24:56 PDT 2004 
//    Changed call from "Lineout" to "StartLineout".  Added call to
//    RendererMessageThread so that the Lineout will finish after the
//    new window (if necessary) has been created and properly initialized. 
//
// ****************************************************************************

void
ViewerWindow::PerformLineoutCallback(void *data)
{
    if(data == 0)
        return;

    LINE_OUT_INFO *loData = (LINE_OUT_INFO *)data;

    ViewerWindow *win = (ViewerWindow *)loData->callbackData;
    ViewerQueryManager::Instance()->StartLineout(win, &loData->atts);
    viewerSubject->MessageRendererThread("finishLineout;");
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
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to build visual cue infos from pick and line attributes
// ****************************************************************************

void
ViewerWindow::ValidateQuery(const PickAttributes *pa, const Line *lineAtts)
{
    VisualCueInfo pickCue, lineCue;
    pickCue.SetFromP(pa);
    lineCue.SetFromL(lineAtts);
    visWindow->QueryIsValid(pa==NULL ? NULL : &pickCue,
                            lineAtts==NULL ? NULL : &lineCue);
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
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to build visual cue infos from line attributes
// ****************************************************************************
 
void
ViewerWindow::UpdateQuery(const Line *lineAtts)
{
    VisualCueInfo cue;
    cue.SetFromL(lineAtts);
    visWindow->UpdateQuery(&cue);
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
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to build visual cue infos from line attributes
// ****************************************************************************
 
void
ViewerWindow::DeleteQuery(const Line *lineAtts)
{
    VisualCueInfo cue;
    cue.SetFromL(lineAtts);
    visWindow->DeleteQuery(&cue);
}

// ****************************************************************************
//  Method: ViewerWindow::UpdateVisualCueList
//
//  Purpose: Populate the VisualCueList object passed in with the visual
//      visual cues currently in the window.
//
//  Programmer: Mark C. Miller 
//  Creation:   June 7, 2004 
//
// ****************************************************************************
void
ViewerWindow::UpdateVisualCueList(VisualCueList& visCues) const
{
    // get visual cues from the vis window
    vector<const VisualCueInfo*> cuesVec;
    visWindow->GetVisualCues(VisualCueInfo::Unknown, cuesVec);

    // use VisualCueList's method to set from a vector of VisualCueInfo* 
    visCues.SetFrom(cuesVec);
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
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added 3 most recent rendering times to set of times returned
//   
// ****************************************************************************

void
ViewerWindow::GetRenderTimes(double times[6]) const
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
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    For certain cases of stereo and SR, all the stereo work is done on the
//    engine and the Viewer's visWindow object should NOT be in stereo. In
//    Other cases, it is necessary for the Viewer's visWindow object to also
//    be in stereo.
//
//    Mark C. Miller, Wed Aug  9 16:35:25 PDT 2006
//    Removed above optimization
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
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added logic to deal with cases in which the visWindow object itself
//    may not be in stereo even though we are indeed doing stereo rendering.
//
//    Mark C. Miller, Wed Aug  9 16:35:25 PDT 2006
//    Removed above optimization
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
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added logic to deal with cases in which the visWindow object itself
//    may not be in stereo even though we are indeed doing stereo rendering.
// ****************************************************************************

int
ViewerWindow::GetStereoType() const
{
    return visWindow->GetStereoType();
}

// ****************************************************************************
// Method: ViewerWindow::SetDisplayListMode
//
// Purpose: 
//     Sets the display list mode -- never, always, or auto.
//
// Programmer: Hank Childs
// Creation:   May 10, 2004
//
// ****************************************************************************

void
ViewerWindow::SetDisplayListMode(int mode)
{
    visWindow->SetDisplayListMode(mode);
}

// ****************************************************************************
// Method: ViewerWindow::GetDisplayListMode
//
// Purpose: 
//     Gets the display list mode.
//
// Programmer: Hank Childs
// Creation:   May 10, 2004
//
// ****************************************************************************

int
ViewerWindow::GetDisplayListMode() const
{
    return visWindow->GetDisplayListMode();
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
//  Method:  ViewerWindow::GetDoShading
//
//  Purpose:
//    Returns the window's shading flag.
//
//  Arguments:
//    none
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
// ****************************************************************************

bool
ViewerWindow::GetDoShading() const
{
    return doShading;
}

// ****************************************************************************
//  Method:  ViewerWindow::GetShadingStrength
//
//  Purpose:
//    Returns the window's shading strength.
//
//  Arguments:
//    none
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
// ****************************************************************************

double
ViewerWindow::GetShadingStrength() const
{
    return shadingStrength;
}

// ****************************************************************************
//  Method:  ViewerWindow::SetShadingProperties
//
//  Purpose:
//    Sets the window's shading properites.
//
//  Arguments:
//      flag  :  the new shading flag
//      str   :  the new shading strength
//
//  Programmer:  Hank Childs
//  Creation:    October 24, 2004
//
// ****************************************************************************

void
ViewerWindow::SetShadingProperties(bool flag, double str)
{
    doShading = flag;
    shadingStrength = str;
}

// ****************************************************************************
//  Method:  ViewerWindow::GetDoDepthCueing
//
//  Purpose:
//    Returns the window's depth cueing flag.
//
//  Arguments:
//    none
//
//  Programmer:  Hank Childs
//  Creation:    August 29, 2007
//
// ****************************************************************************

bool
ViewerWindow::GetDoDepthCueing() const
{
    return doDepthCueing;
}

// ****************************************************************************
//  Method:  ViewerWindow::GetStartCuePoint
//
//  Purpose:
//    returns the 3D world space point at which depth cueing starts
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
const double*
ViewerWindow::GetStartCuePoint() const
{
    return startCuePoint;
}

// ****************************************************************************
//  Method:  ViewerWindow::GetEndCuePoint
//
//  Purpose:
//    returns the 3D world space point at which depth cueing ends
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
const double*
ViewerWindow::GetEndCuePoint() const
{
    return endCuePoint;
}

// ****************************************************************************
//  Method:  ViewerWindow::SetDepthCueingProperties
//
//  Purpose:
//    Set all depth cueing properties at once.
//
//  Arguments:
//    flag       the depth cueing flag
//    start      the depth cueing start point in world space
//    end        the depth cueing end point in world space
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2007
//
// ****************************************************************************
void
ViewerWindow::SetDepthCueingProperties(bool flag,
                                       const double start[3],
                                       const double end[3])
{
    doDepthCueing = flag;
    startCuePoint[0] = start[0];
    startCuePoint[1] = start[1];
    startCuePoint[2] = start[2];
    endCuePoint[0] = end[0];
    endCuePoint[1] = end[1];
    endCuePoint[2] = end[2];
}

// ****************************************************************************
//  Method:  ViewerWindow::GetSpecularFlag
//
//  Purpose:
//    Returns the window's specular flag.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

bool
ViewerWindow::GetSpecularFlag() const
{
    return visWindow->GetSpecularFlag();
}

// ****************************************************************************
//  Method:  ViewerWindow::GetSpecularCoeff
//
//  Purpose:
//    Returns the window's specular coeff.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

double
ViewerWindow::GetSpecularCoeff() const
{
    return visWindow->GetSpecularCoeff();
}

// ****************************************************************************
//  Method:  ViewerWindow::SetSpecularProperties
//
//  Purpose:
//    Sets the window's specular properites.
//
//  Arguments:
//      flag  :  the new specular flag
//      coeff :  the new specular coefficient
//      power :  the new specular power
//      color :  the new specular color
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

void
ViewerWindow::SetSpecularProperties(bool flag, double coeff, double power,
                                    const ColorAttribute &color)
{
    visWindow->SetSpecularProperties(flag,coeff,power,color);
}

// ****************************************************************************
//  Method:  ViewerWindow::GetSpecularPower
//
//  Purpose:
//    Returns the window's specular power.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

double
ViewerWindow::GetSpecularPower() const
{
    return visWindow->GetSpecularPower();
}

// ****************************************************************************
//  Method:  ViewerWindow::GetSpecularColor
//
//  Purpose:
//    Returns the window's specular color.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

const ColorAttribute&
ViewerWindow::GetSpecularColor() const
{
    return visWindow->GetSpecularColor();
}

// ****************************************************************************
// Method: ViewerWindow::SetColorTexturingFlag
//
// Purpose: 
//   Set the color texturing flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 10:58:27 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SetColorTexturingFlag(bool val)
{
    visWindow->SetColorTexturingFlag(val);
}

// ****************************************************************************
// Method: ViewerWindow::GetColorTexturingFlag
//
// Purpose: 
//   Get the color texturing flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 10:58:48 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::GetColorTexturingFlag() const
{
    return visWindow->GetColorTexturingFlag();
}

// ****************************************************************************
// Method: ViewerWindow::GetNumPrimitives
//
// Purpose: 
//   Gets the number of graphics primitives rendered by the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:42:16 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Thu Mar  3 17:38:36 PST 2005
//   Changed name from GetNumTriangles to GetNumPrimitives
//   
// ****************************************************************************

int
ViewerWindow::GetNumPrimitives() const
{
    return visWindow->GetNumPrimitives();
}

// ****************************************************************************
// Method: ViewerWindow::GetNumberOfCells
//
// Programmer: Mark C. Miller 
// Creation:   Tue May 11 09:45:30 PDT 2004 
//
// ****************************************************************************

int
ViewerWindow::GetNumberOfCells(bool polysOnly) const
{
    return GetPlotList()->GetNumberOfCells(polysOnly);
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
// Method: ViewerWindow::ChangeScalableRenderingMode
//
// Purpose: 
//   Enables or disables scalable rendering mode 
//
// Arguments:
//   mode   : Whether scalable rendering is to be turned on or off.
//
// Programmer: Mark C. Miller
// Creation:   Mon Nov  3 17:08:14 PST 2003 
//
// Modifications:
//   Brad Whitlock, Fri Jan 23 17:04:36 PST 2004
//   I changed the interface to UpdateWindowInformation and I changed enough
//   viewer classes so the frame does not have to be passed around all over
//   the place.
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Removed checks for IsChangingScalableRenderingMode since this couldn't
//   happen anyways. Removed preparingToChangeScalableRenderingMode
//
//   Mark C. Miller, Wed Jun  8 11:03:31 PDT 2005
//   Made transitions into and out of SR mode smoother, less work
//
//   Mark C. Miller, Wed Sep 21 18:00:53 PDT 2005
//   Made it send window env. to engine before transmuting plots. Otherwise,
//   engine would be working from old scalable rendering parameters and not
//   the new ones we're just setting here.
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Reset compression flag 
//
//   Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//   Since the visWindow object isn't always in stereo for stereo SR, we
//   need logic here to correctly put the visWindow into stereo when changing
//   out of SR, if needed.
//
//   Mark C. Miller, Wed Aug  9 16:35:25 PDT 2006
//   Removed above logic. Now visWindow will be in stereo mode
// ****************************************************************************

void
ViewerWindow::ChangeScalableRenderingMode(bool newMode)
{
    bool updatesEnabled = UpdatesEnabled();

    // if we aren't actually changing the mode, do nothing
    if (GetScalableRendering() != newMode)
    {
        // clear support info for external render requests
        ClearLastExternalRenderRequestInfo();

        // remove all plot's actors from the VisWindow
        if (updatesEnabled)
            DisableUpdates();
        ClearPlots();

        // transmute the plots
        SendWindowEnvironmentToEngine(GetPlotList()->GetEngineKey());
        GetPlotList()->TransmutePlots(!newMode);

        // set scalable rendering mode in the vis window 
        visWindow->SetScalableRendering(newMode);

        if (updatesEnabled)
        {
            EnableUpdates();
            GetPlotList()->UpdateFrame();
        }

        // update the window information
        ViewerWindowManager::Instance()->UpdateWindowInformation(
            WINDOWINFO_WINDOWFLAGS, windowId);
    }

    // note, this flag is set to true when the render message is sent
    isChangingScalableRenderingMode = false;

    if (newMode == false)
        isCompressingScalableImage = false;
}

// ****************************************************************************
// Method: ViewerWindow::IsChangingScalableRenderingMode
//
// Purpose: 
//   Returns false if we are NOT in the midst of changing scalable rendering
//   mode, regardless of the toMode argument. When we ARE in the midst of 
//   changing scalable rendering mode, this method will return true if the
//   toMode argument matches the mode we are changing into and false otherwise.
//
// Arguments:
//   toMode : mode to test against for which mode we are switching into 
//
// Programmer: Mark C. Miller
// Creation:   Mon Nov  3 17:08:14 PST 2003 
//
// ****************************************************************************

bool
ViewerWindow::IsChangingScalableRenderingMode(bool toMode) const
{
   if (!isChangingScalableRenderingMode)
       return false;

   if (toMode == targetScalableRenderingMode)
       return true;
   else
       return false;
}

// ****************************************************************************
// Method: ViewerWindow::DisableExternalRenderRequests
//
// Purpose: Temporarily disable external render requests
//
// Programmer: Mark C. Miller
// Creation:   Mon Nov  3 17:08:14 PST 2003 
//
// Modified:
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to clear the external renderer's cached image if disabling it.
// ****************************************************************************

bool
ViewerWindow::DisableExternalRenderRequests(bool bClearImage)
{
   return visWindow->DisableExternalRenderRequests(bClearImage);
}

// ****************************************************************************
// Method: ViewerWindow::EnableExternalRenderRequests
//
// Purpose: Re-enable external render requests
//
// Programmer: Mark C. Miller
// Creation:   Mon Nov  3 17:08:14 PST 2003 
//
// ****************************************************************************

bool
ViewerWindow::EnableExternalRenderRequests()
{
   return visWindow->EnableExternalRenderRequests();
}

// ****************************************************************************
// Method: ViewerWindow::GetScalableRendering
//
// Purpose: 
//   returns boolean indicating if window is in scalable rendering mode or not 
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// ****************************************************************************

bool
ViewerWindow::GetScalableRendering() const
{
    return visWindow->GetScalableRendering(); 
}

// ****************************************************************************
// Method: ViewerWindow::GetScalableThreshold
//
// Purpose: 
//   returns effective scalable rendering threshold 
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// ****************************************************************************

int
ViewerWindow::GetScalableThreshold() const
{
    return visWindow->GetScalableThreshold(); 
}

// ****************************************************************************
// Method: ViewerWindow::SetScalableActivationMode
//
// Purpose: 
//   Sets scalable rendering activation mode
//
// Programmer: Mark C. Miller
// Creation:   Tue May 11 08:40:36 PDT 2004 
//
// Modifications:
//
//   Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//   Moved call to set visWindows SR activation mode to after switch stmt
//   
// ****************************************************************************

int
ViewerWindow::SetScalableActivationMode(int activationMode)
{
    int oldActivationMode = GetScalableActivationMode();

    // only change the activation mode if it truly is different from current
    if (activationMode != oldActivationMode)
    {
        switch (activationMode)
        {
            case RenderingAttributes::Always:
                SendScalableRenderingModeChangeMessage(true);
                break;
            case RenderingAttributes::Never:
                SendScalableRenderingModeChangeMessage(false);
                break;
            case RenderingAttributes::Auto:
            default:
            {
                bool newMode;
                if (ShouldSendScalableRenderingModeChangeMessage(&newMode))
                    SendScalableRenderingModeChangeMessage(newMode);
                break;
            }
        }

        visWindow->SetScalableActivationMode(activationMode);
    }

    return oldActivationMode;
}

// ****************************************************************************
// Method: ViewerWindow::GetScalableActivationMode
//
// Purpose: 
//   returns current scalable activation mode 
//
// Programmer: Mark C. Miller
// Creation:   May 11, 2004 
//
// ****************************************************************************

int
ViewerWindow::GetScalableActivationMode() const
{
    return visWindow->GetScalableActivationMode();
}

// ****************************************************************************
// Method: ViewerWindow::SetScalableAutoThreshold
//
// Purpose: 
//   Sets scalable rendering threshold (and modality) 
//
// Programmer: Mark C. Miller
// Creation:   May 11, 2004 
//
// Modifications:
//
//   Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//   Moved call to set visWindows SR threshold to after if stmt
//   
// ****************************************************************************

int
ViewerWindow::SetScalableAutoThreshold(int autoThreshold)
{
    int oldAutoThreshold = GetScalableAutoThreshold();

    if (autoThreshold != oldAutoThreshold)
    {

       if (GetScalableActivationMode() == RenderingAttributes::Auto)
       {
           bool newMode;
           if (ShouldSendScalableRenderingModeChangeMessage(&newMode))
               SendScalableRenderingModeChangeMessage(newMode);
       }

       visWindow->SetScalableAutoThreshold(autoThreshold);
    }

    return oldAutoThreshold;
}

// ****************************************************************************
// Method: ViewerWindow::GetScalableAutoThreshold
//
// Purpose: 
//   returns current scalable rendering threshold (and modality)
//
// Programmer: Mark C. Miller
// Creation:   Tue Dec  3 19:25:11 PST 2002 
//
// ****************************************************************************

int
ViewerWindow::GetScalableAutoThreshold() const
{
    return visWindow->GetScalableAutoThreshold();
}

// ****************************************************************************
//
// Purpose: Set/Get compression activation mode
//
// Programmer: Mark C. Miller
// Creation:   November 3, 2005 
// ****************************************************************************
int
ViewerWindow::SetCompressionActivationMode(int val)
{
    int oldVal = compressionActivationMode;
    compressionActivationMode = val;
    return oldVal;
}
int
ViewerWindow::GetCompressionActivationMode() const
{
    return compressionActivationMode;
}

bool
ViewerWindow::GetIsCompressingScalableImage() const
{
    return isCompressingScalableImage;
}

// ****************************************************************************
// Method: ViewerWindow::CreateNode
//
// Purpose: 
//   Lets the window save its information to a config file's DataNode.
//
// Arguments:
//   parentNode : The node to which we're saving information.
//   dbToSource : A map from database name to source ids used in the session.
//   detailed   : Indicates whether detailed information should be added.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 13:10:30 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//   I added a curve view.
//   
//   Eric Brugger, Thu Oct 16 11:21:53 PDT 2003
//   I moved the handling of full frame mode to VisWindow.
//
//   Hank Childs, Sat Nov 15 14:28:26 PST 2003
//   Save out specular properties.
//
//   Brad Whitlock, Fri Nov 7 10:14:08 PDT 2003
//   Added code to save the annotation object list.
//
//   Brad Whitlock, Fri Dec 19 15:26:10 PST 2003
//   I added code to save the bounding box navigation mode.
//
//   Eric Brugger, Mon Mar 29 15:34:50 PST 2004
//   I added maintainData.
//
//   Hank Childs, Mon May 10 08:04:48 PDT 2004
//   Use "display list mode" instead of immediate rendering mode.
//
//   Kathleen Bonnell, Thu Aug 19 14:23:18 PDT 2004 
//   Added InteractorAttributes. 
//
//   Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//   Save out shading properties.
//
//   Brad Whitlock, Wed Feb 23 16:31:10 PST 2005
//   Added code to prevent the view from being saved if there are no plots
//   that have actually been realized.
//
//   Brad Whitlock, Tue Mar 7 15:51:48 PST 2006
//   Added code to save the window's image size, which is the interior OpenGL
//   part of the window. We need that in order to produce movies with the
//   exact right size in visit -movie when the -geometry flag is not given.
//
//   Brad Whitlock, Mon Sep 18 10:59:44 PDT 2006
//   Added colorTexturingFlag.
//
//   Jeremy Meredith, Wed Jun 20 16:54:14 EDT 2007
//   Made specularColor be written underneath a new object data node with
//   a specific name.  This forces ViewerWindow::SetFromNode to pick up
//   the correct ColorAttribute values.
//
//   Jeremy Meredith, Wed Aug 29 15:21:38 EDT 2007
//   Added depth cueing properties.
//
//   Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//   Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::CreateNode(DataNode *parentNode, 
    const std::map<std::string, std::string> &dbToSource,
    bool detailed)
{
    if(parentNode == 0)
        return;

    DataNode *windowNode = new DataNode("ViewerWindow");
    parentNode->AddNode(windowNode);

    //
    // Save the window size and location.
    //
    int windowSize[2], windowImageSize[2], windowLocation[2];
    GetWindowSize(windowSize[0], windowSize[1]);
    GetSize(windowImageSize[0], windowImageSize[1]);
    GetLocation(windowLocation[0], windowLocation[1]);
    windowNode->AddNode(new DataNode("windowSize", windowSize, 2));
    windowNode->AddNode(new DataNode("windowImageSize", windowImageSize, 2));
    windowNode->AddNode(new DataNode("windowLocation", windowLocation, 2));

    //
    // Add information specific to the ViewerWindow.
    //
    if(detailed)
    {
        windowNode->AddNode(new DataNode("boundingBoxMode", GetBoundingBoxMode()));
        windowNode->AddNode(new DataNode("cameraView", cameraView));
        windowNode->AddNode(new DataNode("maintainView", maintainView));
        windowNode->AddNode(new DataNode("maintainData", maintainData));
        windowNode->AddNode(new DataNode("viewExtentsType", avtExtentType_ToString(plotExtentsType)));
        windowNode->AddNode(new DataNode("viewIsLocked", viewIsLocked));
        windowNode->AddNode(new DataNode("timeLocked", timeLocked));
        windowNode->AddNode(new DataNode("toolsLocked", toolsLocked));

        //
        // Interaction mode.
        //
        windowNode->AddNode(new DataNode("interactionMode",
            INTERACTION_MODE_ToString(GetInteractionMode())));

        //
        // Active tools.
        //
        stringVector activeTools;
        for(int i = 0; i < GetNumTools(); ++i)
        {
            if(GetToolEnabled(i))
                activeTools.push_back(GetToolName(i));
        }
        if(activeTools.size() > 0)
            windowNode->AddNode(new DataNode("activeTools", activeTools));

        //
        // Save out the annotations.
        //
        AnnotationAttributes annot(*visWindow->GetAnnotationAtts());
        annot.CreateNode(windowNode, true, true);

        //
        // Save out the lights
        //
        LightList lights(*visWindow->GetLightList());
        lights.CreateNode(windowNode, true, true);

        //
        // Save out important rendering attributes.
        //
        windowNode->AddNode(new DataNode("scalableAutoThreshold", GetScalableAutoThreshold()));
        windowNode->AddNode(new DataNode("scalableActivationMode", GetScalableActivationMode()));
        windowNode->AddNode(new DataNode("notifyForEachRender", GetNotifyForEachRender()));
        windowNode->AddNode(new DataNode("surfaceRepresentation", GetSurfaceRepresentation()));
        windowNode->AddNode(new DataNode("displayListMode", GetDisplayListMode()));
        windowNode->AddNode(new DataNode("stereoRendering", GetStereo()));
        windowNode->AddNode(new DataNode("stereoType", GetStereoType()));
        windowNode->AddNode(new DataNode("antialiasing", GetAntialiasing()));
        windowNode->AddNode(new DataNode("specularFlag", GetSpecularFlag()));
        windowNode->AddNode(new DataNode("specularCoeff", GetSpecularCoeff()));
        windowNode->AddNode(new DataNode("specularPower", GetSpecularPower()));
        DataNode *specularColorNode = new DataNode("specularColor");
        ColorAttribute specColor(GetSpecularColor());
        specColor.CreateNode(specularColorNode, true, true);
        windowNode->AddNode(specularColorNode);
        windowNode->AddNode(new DataNode("doShading", GetDoShading()));
        windowNode->AddNode(new DataNode("shadingStrength", GetShadingStrength()));
        windowNode->AddNode(new DataNode("doDepthCueing", GetDoDepthCueing()));
        windowNode->AddNode(new DataNode("startCuePoint", GetStartCuePoint(), 3));
        windowNode->AddNode(new DataNode("endCuePoint", GetEndCuePoint(), 3));
        windowNode->AddNode(new DataNode("colorTexturingFlag", GetColorTexturingFlag()));

        //
        // View
        //
        if(GetPlotList()->GetNumRealizedPlots() > 0)
        {
            ViewCurveAttributes tmpViewCurveAtts;
            const avtViewCurve &viewCurve = GetViewCurve();
            viewCurve.SetToViewCurveAttributes(&tmpViewCurveAtts);
            tmpViewCurveAtts.CreateNode(windowNode, true, true);

            View2DAttributes tmpView2DAtts;
            const avtView2D &view2d = GetView2D();
            view2d.SetToView2DAttributes(&tmpView2DAtts);
            tmpView2DAtts.CreateNode(windowNode, true, true);

            View3DAttributes tmpView3DAtts;
            const avtView3D &view3d = GetView3D();
            view3d.SetToView3DAttributes(&tmpView3DAtts);
            tmpView3DAtts.CreateNode(windowNode, true, true);

            ViewAxisArrayAttributes tmpViewAxisArrayAtts;
            const avtViewAxisArray &viewAxisArray = GetViewAxisArray();
            viewAxisArray.SetToViewAxisArrayAttributes(&tmpViewAxisArrayAtts);
            tmpViewAxisArrayAtts.CreateNode(windowNode, true, true);
        }

        //
        // Save out the view keyframes.
        //
        DataNode *viewCurveNode = new DataNode("viewCurveKeyframes");
        if(viewCurveAtts->CreateNode(viewCurveNode))
            windowNode->AddNode(viewCurveNode);
        else
            delete viewCurveNode;

        DataNode *view2DNode = new DataNode("view2DKeyframes");
        if(view2DAtts->CreateNode(view2DNode))
            windowNode->AddNode(view2DNode);
        else
            delete view2DNode;

        DataNode *view3DNode = new DataNode("view3DKeyframes");
        if(view3DAtts->CreateNode(view3DNode))
            windowNode->AddNode(view3DNode);
        else
            delete view3DNode;

        DataNode *viewAxisArrayNode = new DataNode("viewAxisArrayKeyframes");
        if(viewAxisArrayAtts->CreateNode(viewAxisArrayNode))
            windowNode->AddNode(viewAxisArrayNode);
        else
            delete viewAxisArrayNode;

        //
        // Save the annotation object list.
        //
        AnnotationObjectList annots;
        UpdateAnnotationObjectList(annots);
        annots.CreateNode(windowNode, true, true);

        //
        // Save out the interactor attributes.
        //
        InteractorAttributes interactor(*visWindow->GetInteractorAtts());
        interactor.CreateNode(windowNode, true, true);

        //
        // Let the plot list add its information.
        //
        GetPlotList()->CreateNode(windowNode, dbToSource);
    }

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
//   sourceToDB : The source to DB map.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 13:11:52 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 11:15:07 PDT 2003
//   I added a curve view.
//
//   Brad Whitlock, Fri Nov 7 10:15:22 PDT 2003
//   I added code to read in the annotation object list.
//
//   Hank Childs, Sat Nov 15 14:28:26 PST 2003
//   Read in specular properties.
//
//   Eric Brugger, Fri Dec  5 14:10:11 PST 2003
//   Correct an error reading viewExtentsType.
//
//   Brad Whitlock, Fri Dec 19 15:27:49 PST 2003
//   Added boundingBoxNavigate.
//
//   Brad Whitlock, Wed Dec 31 14:11:50 PST 2003
//   I added code to delete plots before reading the new plots so we don't
//   have the window redrawing the old plots with the new view.
//
//   Brad Whitlock, Mon Feb 2 15:26:00 PST 2004
//   Added code to translate old config files that had ViewerAnimation.
//
//   Eric Brugger, Mon Mar 29 15:34:50 PST 2004
//   I added maintainData.
//
//   Brad Whitlock, Wed Apr 7 13:53:24 PST 2004
//   Added code to translate keyframing information for old config files.
//
//    Eric Brugger, Thu Apr 22 15:18:37 PDT 2004
//    I renamed the haveRenderedIn* flags to viewSetIn*, since it was
//    more accurate.  I also added viewPartialSetIn3d to distinguish the
//    view being set from a session file and from the client.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified scalable rendering controls to use activation mode and auto
//   threshold
//
//   Kathleen Bonnell, Thu Aug 19 14:23:18 PDT 2004 
//   Added InteractorAttributes. 
//
//   Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//   Read in shading properties.
//
//   Hank Childs, Mon Feb 20 17:03:19 PST 2006
//   Fix type of shading strength ['5654].
//
//   Brad Whitlock, Mon Sep 18 11:00:09 PDT 2006
//   Added color texturing.
//
//   Brad Whitlock, Fri Nov 10 10:03:10 PDT 2006
//   Added sourceToDB.
//
//   Jeremy Meredith, Wed Jun 20 16:54:14 EDT 2007
//   Made specularColor be written underneath a new object data node with
//   a specific name.  This ensures we pick up the correct ColorAttribute.
//
//   Jeremy Meredith, Wed Aug 29 15:21:38 EDT 2007
//   Added depth cueing properties.
//
//   Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//   Added new axis array window mode.
//
// ****************************************************************************

void
ViewerWindow::SetFromNode(DataNode *parentNode, 
    const std::map<std::string, std::string> &sourceToDB)
{
    DataNode *node;

    if(parentNode == 0)
        return;

    DataNode *windowNode = parentNode->GetNode("ViewerWindow");
    if(windowNode == 0)
        return;
    //
    // Reset the view centering flags.
    //
    boundingBoxValidCurve = false;
    viewSetInCurve = false;
    viewModifiedCurve = false;
    boundingBoxValid2d = false;
    viewSetIn2d = false;
    viewModified2d = false;
    boundingBoxValid3d = false;
    viewSetIn3d = false;
    viewPartialSetIn3d = false;
    boundingBoxValidAxisArray = false;
    viewSetInAxisArray = false;
    viewModifiedAxisArray = false;
    mergeViewLimits = false;
    centeringValidCurve = false;
    centeringValid2d = false;
    centeringValid3d = false;
    centeringValidAxisArray = false;

    //
    // Delete the plots first and update the frame so when we set the 
    // view, etc no updates can happen.
    //
    intVector plots, tmp;
    for(int j = 0; j < GetPlotList()->GetNumPlots(); ++j)
        plots.push_back(j);
    GetPlotList()->SetActivePlots(plots, tmp, tmp, false);
    GetPlotList()->DeleteActivePlots();

    ////////////////////// Reformat pre 1.3 session files /////////////////////
    //
    // If we find a ViewerAnimation, reparent its ViewerPlotList to the
    // windowNode so old config files will still work. This should be removed
    // in the future as configs with the now defunct ViewerAnimation become
    // less common.
    //
    DataNode *vaNode = windowNode->GetNode("ViewerAnimation");
    if(vaNode != 0)
    {
        debug1 << "***\n*** Converting the session file from pre 1.3 version.\n***\n";
        DataNode *vplNode = vaNode->GetNode("ViewerPlotList");
        // Remove the plot list node from the animation node without
        // deleting it. Then reparent it in the window node.
        vaNode->RemoveNode("ViewerPlotList", false);
        windowNode->AddNode(vplNode);

        // Add a few things from the animation node to the viewer plot
        // list node.
        DataNode *aNode;
        if((aNode = vaNode->GetNode("pipelineCaching")) != 0)
            vplNode->AddNode(new DataNode("pipelineCaching", aNode->AsBool()));
        if((aNode = vaNode->GetNode("playbackMode")) != 0)
        {
            if(aNode->GetNodeType() == INT_NODE)
                vplNode->AddNode(new DataNode("playbackMode", aNode->AsInt()));
            else if(aNode->GetNodeType() == STRING_NODE)
                vplNode->AddNode(new DataNode("playbackMode", aNode->AsString()));
        }
        //
        // Get the curFrame and nFrames out of the ViewerAnimation and try
        // to put it into the plots in the ViewerPlotList.
        //
        int curFrame = -1;
        int nFrames = -1;
        if((aNode = vaNode->GetNode("curFrame")) != 0)
            curFrame = aNode->AsInt();
        if((aNode = vaNode->GetNode("nFrames")) != 0)
            nFrames = aNode->AsInt();
        if(nFrames != -1 && curFrame != -1 &&
           curFrame >= 0 && curFrame < nFrames)
        {
            DataNode *hostNode = vplNode->GetNode("hostName");
            DataNode *databaseNode = vplNode->GetNode("databaseName");
            DataNode *kfModeNode = vplNode->GetNode("keyframeMode");

            //
            // Determine whether the plot list is in keyframe mode.
            //
            bool kfMode = false;
            if(kfModeNode != 0)
                kfMode = kfModeNode->AsBool();

            if(kfMode)
            {
                //
                // We're in keyframe mode so set the number of keyframes and add a
                // keyframing time slider.
                //
                vplNode->AddNode(new DataNode("nKeyframes", nFrames));
                DataNode *tsNode = new DataNode("timeSliders");
                tsNode->AddNode(new DataNode(KF_TIME_SLIDER, curFrame));
                vplNode->AddNode(tsNode);
                vplNode->AddNode(new DataNode("activeTimeSlider",
                    std::string(KF_TIME_SLIDER)));
                debug3 << "Created a time slider (" << KF_TIME_SLIDER
                       << ") at state" << curFrame << " for old session file." << endl;
            }
            else if(hostNode != 0 && databaseNode != 0)
            {
                //
                // We're not in keyframing mode so create a time slider for the
                // active source.
                //
                std::string tsHost(hostNode->AsString());
                std::string tsDB(databaseNode->AsString());
                std::string tsName(ViewerFileServer::Instance()->
                    ComposeDatabaseName(tsHost, tsDB));

                //
                // If the database has multiple time states then add a time
                // slider for it in the ViewerPlotList.
                //
                DataNode *tsNode = new DataNode("timeSliders");
                tsNode->AddNode(new DataNode(tsName, curFrame));
                vplNode->AddNode(tsNode);
                vplNode->AddNode(new DataNode("activeTimeSlider", tsName));
                debug3 << "Created a time slider (" << tsName.c_str()
                       << ") at state" << curFrame << " for old session file." << endl;
            }
        }
        // Remove the ViewerAnimation node.
        windowNode->RemoveNode("ViewerAnimation");
    }
    ///////////////// Done reformatting pre 1.3 session files /////////////////

    //
    // Read in the plot list.
    //
    if(GetPlotList()->SetFromNode(windowNode, sourceToDB))
        SendUpdateFrameMessage();

    //
    // Read in the view and set the view.
    //
    if(windowNode->GetNode("ViewCurveAttributes") != 0)
    {
        ViewCurveAttributes viewCurveAtts;
        avtViewCurve viewCurve;

        viewCurveAtts.SetFromNode(windowNode);
        viewCurve.SetFromViewCurveAttributes(&viewCurveAtts);
        SetViewCurve(viewCurve);
    }
    if(windowNode->GetNode("View2DAttributes") != 0)
    {
        View2DAttributes view2dAtts;
        avtView2D view2d;

        view2dAtts.SetFromNode(windowNode);
        view2d.SetFromView2DAttributes(&view2dAtts);
        SetView2D(view2d);
    }
    if(windowNode->GetNode("View3DAttributes") != 0)
    {
        View3DAttributes view3dAtts;
        avtView3D view3d;

        view3dAtts.SetFromNode(windowNode);
        view3d.SetFromView3DAttributes(&view3dAtts);
        SetView3D(view3d);
        viewSetIn3d = true;
    }
    if(windowNode->GetNode("ViewAxisArrayAttributes") != 0)
    {
        ViewAxisArrayAttributes viewAxisArrayAtts;
        avtViewAxisArray viewAxisArray;

        viewAxisArrayAtts.SetFromNode(windowNode);
        viewAxisArray.SetFromViewAxisArrayAttributes(&viewAxisArrayAtts);
        SetViewAxisArray(viewAxisArray);
    }
    if((node = windowNode->GetNode("boundingBoxMode")) != 0)
        SetBoundingBoxMode(node->AsBool());
    if((node = windowNode->GetNode("cameraView")) != 0)
        SetCameraViewMode(node->AsBool());
    if((node = windowNode->GetNode("maintainView")) != 0)
        SetMaintainViewMode(node->AsBool());
    if((node = windowNode->GetNode("maintainData")) != 0)
        SetMaintainDataMode(node->AsBool());
    if((node = windowNode->GetNode("viewExtentsType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            ival = (ival < 0 || ival > 3) ? 0 : ival;
            SetViewExtentsType(avtExtentType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            avtExtentType value;
            if(avtExtentType_FromString(node->AsString(), value))
                SetViewExtentsType(value);
        }
    }

    //
    // Read in lock flags.
    //
    if((node = windowNode->GetNode("viewIsLocked")) != 0)
        SetViewIsLocked(node->AsBool());
    if((node = windowNode->GetNode("timeLocked")) != 0)
        timeLocked = node->AsBool();
    if((node = windowNode->GetNode("toolsLocked")) != 0)
        toolsLocked = node->AsBool();

    //
    // Read in and set any rendering attributes.
    //
    if((node = windowNode->GetNode("scalableAutoThreshold")) != 0)
        SetScalableAutoThreshold(node->AsInt());
    if((node = windowNode->GetNode("scalableActivationMode")) != 0)
        SetScalableActivationMode(node->AsInt());
    if((node = windowNode->GetNode("notifyForEachRender")) != 0)
        SetNotifyForEachRender(node->AsBool());
    if((node = windowNode->GetNode("surfaceRepresentation")) != 0)
        SetSurfaceRepresentation(node->AsInt());
    if((node = windowNode->GetNode("displayListMode")) != 0)
        SetDisplayListMode(node->AsInt());
    int stereoType = 0;
    if((node = windowNode->GetNode("stereoType")) != 0)
        stereoType = node->AsInt();
    if((node = windowNode->GetNode("stereoRendering")) != 0)
        SetStereoRendering(node->AsBool(), stereoType);
    if((node = windowNode->GetNode("antialiasing")) != 0)
        SetAntialiasing(node->AsBool());

    //
    // The specular parameters are all set as a group.
    //
    int numParamsSaved = 0;
    bool tmpSpecularFlag = false;
    double tmpSpecularCoeff = 0.;
    double tmpSpecularPower = 0.;
    ColorAttribute tmpSpecularColor;
    if((node = windowNode->GetNode("specularFlag")) != 0)
    {
        tmpSpecularFlag = node->AsBool();
        numParamsSaved++;
    }
    if((node = windowNode->GetNode("specularCoeff")) != 0)
    {
        tmpSpecularCoeff = node->AsFloat();
        numParamsSaved++;
    }
    if((node = windowNode->GetNode("specularPower")) != 0)
    {
        tmpSpecularPower = node->AsFloat();
        numParamsSaved++;
    }
    if((node = windowNode->GetNode("specularColor")) != 0)
    {
        tmpSpecularColor.SetFromNode(node);
        numParamsSaved++;
    }
    if (numParamsSaved == 4)
    {
        SetSpecularProperties(tmpSpecularFlag, tmpSpecularCoeff, 
                              tmpSpecularPower, tmpSpecularColor);
    }

    numParamsSaved = 0;
    bool tmpDoShading = false;
    double tmpShadingStrength = 0.;
    if((node = windowNode->GetNode("doShading")) != 0)
    {
        tmpDoShading = node->AsBool();
        numParamsSaved++;
    }
    if((node = windowNode->GetNode("shadingStrength")) != 0)
    {
        tmpShadingStrength = node->AsDouble();
        numParamsSaved++;
    }
    if (numParamsSaved == 2)
    {
        SetShadingProperties(tmpDoShading, tmpShadingStrength);
    }

    numParamsSaved = 0;
    bool tmpDoDepthCueing = false;
    double tmpStartCuePoint[3];
    double tmpEndCuePoint[3];
    if((node = windowNode->GetNode("doDepthCueing")) != 0)
    {
        tmpDoDepthCueing = node->AsBool();
        numParamsSaved++;
    }
    if((node = windowNode->GetNode("startCuePoint")) != 0)
    {
        tmpStartCuePoint[0] = node->AsDoubleArray()[0];
        tmpStartCuePoint[1] = node->AsDoubleArray()[1];
        tmpStartCuePoint[2] = node->AsDoubleArray()[2];
        numParamsSaved++;
    }
    if((node = windowNode->GetNode("endCuePoint")) != 0)
    {
        tmpEndCuePoint[0] = node->AsDoubleArray()[0];
        tmpEndCuePoint[1] = node->AsDoubleArray()[1];
        tmpEndCuePoint[2] = node->AsDoubleArray()[2];
        numParamsSaved++;
    }
    if (numParamsSaved == 3)
    {
        SetDepthCueingProperties(tmpDoDepthCueing,
                                 tmpStartCuePoint,
                                 tmpEndCuePoint);
    }

    // Set the color texturing properties.
    if((node = windowNode->GetNode("colorTexturingFlag")) != 0)
    {
        SetColorTexturingFlag(node->AsBool());
    }

    //
    // Read in and set the annotation attributes.
    //
    if((node = windowNode->GetNode("AnnotationAttributes")) != 0)
    {
        AnnotationAttributes annot;
        annot.SetFromNode(windowNode);
        SetAnnotationAttributes(&annot);
    }

    //
    // Read in and set the annotation object list.
    //
    if((node = windowNode->GetNode("AnnotationObjectList")) != 0)
    {
        AnnotationObjectList annots;
        annots.SetFromNode(windowNode);
        visWindow->DeleteAllAnnotationObjects();
        visWindow->CreateAnnotationObjectsFromList(annots);
    }

    //
    // Read in and set the light list.
    //
    if((node = windowNode->GetNode("LightList")) != 0)
    {
        LightList lights;
        lights.SetFromNode(windowNode);
        SetLightList(&lights);
    }

    //
    // Read in and set the interactor attributes.
    //
    if((node = windowNode->GetNode("InteractorAttributes")) != 0)
    {
        InteractorAttributes interactorAtts;
        interactorAtts.SetFromNode(windowNode);
        SetInteractorAtts(&interactorAtts);
    }

    //
    // Read in the view keyframes.
    //
    ViewCurveAttributes tmpViewCurve;
    if((node = windowNode->GetNode("viewCurveKeyframes")) != 0)
        viewCurveAtts->SetFromNode(node, &tmpViewCurve);
    View2DAttributes tmpView2D;
    if((node = windowNode->GetNode("view2DKeyframes")) != 0)
        view2DAtts->SetFromNode(node, &tmpView2D);
    View3DAttributes tmpView3D;
    if((node = windowNode->GetNode("view3DKeyframes")) != 0)
        view3DAtts->SetFromNode(node, &tmpView3D);
    ViewAxisArrayAttributes tmpViewAxisArray;
    if((node = windowNode->GetNode("viewAxisArrayKeyframes")) != 0)
        viewAxisArrayAtts->SetFromNode(node, &tmpViewAxisArray);

    //
    // Let other objects get their information.
    //
    actionMgr->SetFromNode(windowNode);

    //
    // Read in and set the interaction mode.
    //
    if((node = windowNode->GetNode("interactionMode")) != 0)
    {
        INTERACTION_MODE m;
        if(INTERACTION_MODE_FromString(node->AsString(), m))
            SendInteractionModeMessage(m);
    }

    //
    // Read in the list of active tools and send a message to the
    // viewer's message buffer so that the tool will be activated later
    // when control returns to the event loop.
    //
    if((node = windowNode->GetNode("activeTools")) != 0)
    {
        const stringVector &activeTools = node->AsStringVector();
        for(int i = 0; i < activeTools.size(); ++i)
        {
            for(int j = 0; j < GetNumTools(); ++j)
            {
                if(GetToolName(j) == activeTools[i])
                {
                    SendActivateToolMessage(j);
                    break;
                }
            }
        }
    }
}

// ****************************************************************************
// Method: ViewerWindow::SessionContainsErrors
//
// Purpose: 
//   Determines whether the session contains errors.
//
// Arguments:
//   parentNode : The data node to check.
//
// Returns:    True if the session contains errors; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 11 14:57:09 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindow::SessionContainsErrors(DataNode *parentNode)
{
    if(parentNode == 0)
        return true;

    DataNode *windowNode = parentNode->GetNode("ViewerWindow");
    if(windowNode == 0)
        return true;
 
    return ViewerPlotList::SessionContainsErrors(windowNode);
}

// ****************************************************************************
// Method: ViewerWindow::SendUpdateFrameMessage
//
// Purpose: 
//   Sends a message to the viewer's event loop that tells it to update the
//   current animation frame, which will cause plots to be regenerated.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 12:50:15 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SendUpdateFrameMessage() const
{
    char msg[256];

    SNPRINTF(msg, 256, "updateFrame 0x%p;", this);
    viewerSubject->MessageRendererThread(msg);
}

// ****************************************************************************
// Method: ViewerWindow::SendInteractionModeMessage
//
// Purpose: 
//   Sends a message to the viewer's event loop that tells it to set the
//   window's interaction mode.
//
// Arguments:
//   m : The interaction mode to set.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 12:31:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SendInteractionModeMessage(const INTERACTION_MODE m) const
{
    char msg[256];

    SNPRINTF(msg, 256, "setInteractionMode 0x%p %d;", this, int(m));
    viewerSubject->MessageRendererThread(msg);
}

// ****************************************************************************
// Method: ViewerWindow::SendActivateToolMessage
//
// Purpose: 
//   Sends a message to the viewer's event loop that tells it to activate
//   the specified tool.
//
// Arguments:
//   toolId : The index of the tool to activate.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 12:18:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindow::SendActivateToolMessage(const int toolId) const
{
    char msg[256];

    SNPRINTF(msg, 256, "activateTool 0x%p %d;", this, toolId);
    viewerSubject->MessageRendererThread(msg);
}

// ****************************************************************************
// Method: ViewerWindow::SendScalableRenderingModeChangeMessage
//
// Purpose: 
//   Sends a message to the viewer's event loop that tells it prepare for a
//   change in the scalable rendering mode. 
//
// Arguments:
//   newMode : The new scalable rendering mode to set.
//
// Programmer: Mark C. Miller 
// Creation:   Mon Nov  3 15:48:33 PST 2003
//
// Modifications:
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Bracketed all work with check to if new mode different from current mode
//   Removed preparingToChangeScalableRenderingMode
//
//   Mark C. Miller, Wed Jun  8 11:03:31 PDT 2005
//   Added early return if we're already changing SR modes to avoid possible
//   recursion.
//
// ****************************************************************************

void
ViewerWindow::SendScalableRenderingModeChangeMessage(bool newMode)
{
    if (isChangingScalableRenderingMode)
        return;

    if (GetScalableRendering() != newMode)
    {
        char msg[256];

        // we set these flags here so that everywhere else, we can ask if
        // we're about to change modes
        isChangingScalableRenderingMode = true;
        targetScalableRenderingMode = newMode;

        SNPRINTF(msg, 256, "setScalableRenderingMode 0x%p %d;", this,
            (newMode?1:0));
        viewerSubject->MessageRendererThread(msg);
    }
}

// ****************************************************************************
// Method: ViewerWindow::ShouldSendScalableRenderingModeChangeMessage
//
// Purpose: 
//   Determine if the polygon counts of current plots are such that we need to
//   change into or out of scalable rendering mode
//
// Programmer: Mark C. Miller 
// Creation:   Tue May 11 09:03:07 PDT 2004
//
// ****************************************************************************

bool
ViewerWindow::ShouldSendScalableRenderingModeChangeMessage(bool *newMode) const
{
    bool oldMode      = GetScalableRendering();
    int autoThreshold = GetScalableAutoThreshold();

    // an auto-threshold of 0 is same as Always mode
    if ((autoThreshold == 0) && (oldMode == false))
    {
        *newMode = true;
        return true;
    }

    // an auto-threshold of INT_MAX is same as Never mode
    if ((autoThreshold == INT_MAX) && (oldMode == true))
    {
        *newMode = false;
        return true;
    }

    if (oldMode == false)
    {
        bool polysOnly = true;
        int cellCount = GetNumberOfCells(polysOnly);

        if (cellCount < 0.5 * autoThreshold)
            return false;

        if (cellCount > autoThreshold)
        {
            *newMode = true;
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: ViewerWindow::ClearExternalRenderRequest
//
// Purpose: 
//    Clears stuff from an external render info object 
//
// Programmer: Mark C. Miller 
// Creation:   Monday, December 13, 2004
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye information
// ****************************************************************************

void
ViewerWindow::ClearExternalRenderRequestInfo(ExternalRenderRequestInfo& info) const
{
    info.pluginIDsList.clear();
    info.engineKeysList.clear();
    info.plotIdsList.clear();
    info.attsList.clear();
    info.annotObjs.ClearAnnotations();
    info.extStr =
        avtExtentType_ToString(AVT_UNKNOWN_EXTENT_TYPE);
    info.visCues.ClearCues();
    int i;
    for (i = 0; i < 7; i++)
        info.frameAndState[i] = 0;
    for (i = 0; i < 6; i++)
        info.viewExtents[i] = (i%2 ? 1.0 : 0.0);
    info.lastChangedCtName = "";
    info.leftEye = true;
}

// ****************************************************************************
// Method: ViewerWindow::ClearLastExternalRenderRequest
//
// Purpose: 
//    Clears stuff from the external render request arrays 
//
// Programmer: Mark C. Miller 
// Creation:   Mon Nov  3 15:48:33 PST 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 30 11:04:04 PST 2004
//    Added an engine key used to index (and restart) engines.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added code to set the extents type string to the unknown value
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to deal with visual cues (pick points and ref lines)
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to deal with view extents and frame and state
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to manage name of last color table to change
//
//    Mark C. Miller, Mon Dec 13 14:06:33 PST 2004
//    Moved implementation to ClearExternalRenderRequestInfo
// ****************************************************************************
void
ViewerWindow::ClearLastExternalRenderRequestInfo()
{
    ClearExternalRenderRequestInfo(lastExternalRenderRequest);
}

// ****************************************************************************
// Method: ViewerWindow::UpdateLastExternalRenderRequest
//
// Purpose: 
//    Updates the last external render request info 
//
// Programmer: Mark C. Miller 
// Creation:   November 10, 2003 
//
// Modifications:
//
//   Mark C. Miller, Mon Nov 24 22:02:25 PST 2003
//   Modifed it to make copies of plot attributes
//
//   Jeremy Meredith, Tue Mar 30 11:04:04 PST 2004
//   Added an engine key used to index (and restart) engines.
//
//   Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//   Added code to update the extents type string
//
//   Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//   Added code to set annotation object list
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to deal with visual cues (pick points and ref lines)
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to deal with view extents and frame and state
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to manage name of last color table to change
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye information
// ****************************************************************************

void
ViewerWindow::UpdateLastExternalRenderRequestInfo(
    const ExternalRenderRequestInfo &newRequest)
{
    int i = 0;

    // delete any old copies of plot attributes
    for (i = 0; i < lastExternalRenderRequest.plotIdsList.size(); i++)
    {
        if (lastExternalRenderRequest.attsList[i] != NULL)
            delete lastExternalRenderRequest.attsList[i];
    }

    // make copies of the plot attributes 
    lastExternalRenderRequest.attsList.clear();
    for (i = 0; i < newRequest.plotIdsList.size(); i++)
    {
        AttributeSubject *tmp = newRequest.attsList[i]->NewInstance(true);
        lastExternalRenderRequest.attsList.push_back(tmp);
    }

    // copy everything else over
    lastExternalRenderRequest.pluginIDsList = newRequest.pluginIDsList;
    lastExternalRenderRequest.engineKeysList= newRequest.engineKeysList;
    lastExternalRenderRequest.plotIdsList   = newRequest.plotIdsList;
    lastExternalRenderRequest.winAtts       = newRequest.winAtts;
    lastExternalRenderRequest.annotAtts     = newRequest.annotAtts;
    lastExternalRenderRequest.annotObjs     = newRequest.annotObjs;
    lastExternalRenderRequest.extStr        = newRequest.extStr;
    lastExternalRenderRequest.visCues       = newRequest.visCues;
    for (i = 0; i < 7; i++)
        lastExternalRenderRequest.frameAndState[i] = newRequest.frameAndState[i];
    for (i = 0; i < 6; i++)
        lastExternalRenderRequest.viewExtents[i] = newRequest.viewExtents[i];
    lastExternalRenderRequest.lastChangedCtName = ""; 
    lastExternalRenderRequest.leftEye       = newRequest.leftEye;

    nameOfCtChangedSinceLastRender = "";

}

// ****************************************************************************
// Method: ViewerWindow::CanSkipExternalRender
//
// Purpose: 
//    checks to see if anything in the external render request has really
//    changed sucn that the image needs to be re-rendered and returns false
//    if not, true otherwise.
//
// Programmer: Mark C. Miller 
// Creation:   November 10, 2003 
//
// Modifications:
//
//   Mark C. Miller, Mon Mar 29 19:13:38 PST 2004
//   Made check for annotations consider only the subset of all annotations 
//   that should require a re-render on the engine
//
//   Jeremy Meredith, Tue Mar 30 11:04:04 PST 2004
//   Added an engine key used to index (and restart) engines.
//
//   Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//   Added code to deal with the extents type string
//
//   Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//   Added code to eliminate change in scalable threshold from check to see
//   if we can skip.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified scalable rendering controls to use activation mode and auto
//   threshold
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to deal with visual cues (pick points and ref lines) and
//    annotation object list
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Filtered out View3DAttributes center of rotation stuff so that it does
//    not effect the outcome.
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to deal with view extents and frame and state
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to manage name of last color table to change
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added code to filter compression mode from influencing skip 
//
//    Mark C. Miller, Sat Jul 22 16:55:44 PDT 2006
//    Added logic to deal with multiple renders for stereo. For stereo modes
//    in which left/right images are in separate buffers (e.g. crystal-eyes),
//    two render requests are needed. However, for all others, a single render
//    request for a complete, stereo image, is all that is needed.
//
//    Mark C. Miller, Wed Aug  9 16:29:43 PDT 2006
//    Removed optimization to skip second eye for non-crystal-eyes stereo mode
//
//    Hank Childs, Tue Oct  2 17:16:45 PDT 2007
//    Don't check to see if the annotation objects have changed.
//
// ****************************************************************************

bool
ViewerWindow::CanSkipExternalRender(const ExternalRenderRequestInfo& thisRequest) const
{
    const ExternalRenderRequestInfo& lastRequest = lastExternalRenderRequest;

    //
    // Don't skip render for either eye when in stereo 
    //
    if (thisRequest.leftEye != lastRequest.leftEye)
        return false;

    // compare window attributes but ignore a few of them by forcing
    // old/new to be equal for those few
    WindowAttributes tmpWinAtts = thisRequest.winAtts;
    int lastScalableAutoThreshold = lastRequest.winAtts.GetRenderAtts().GetScalableAutoThreshold();
    int lastScalableActivationMode = lastRequest.winAtts.GetRenderAtts().GetScalableActivationMode();
    int lastCompressionActivationMode = lastRequest.winAtts.GetRenderAtts().GetCompressionActivationMode();
    tmpWinAtts.GetRenderAtts().SetScalableAutoThreshold(lastScalableAutoThreshold);
    tmpWinAtts.GetRenderAtts().SetScalableActivationMode(
        (RenderingAttributes::TriStateMode) lastScalableActivationMode);
    tmpWinAtts.GetRenderAtts().SetCompressionActivationMode(
        (RenderingAttributes::TriStateMode) lastCompressionActivationMode);
    View3DAttributes lastView3D = lastRequest.winAtts.GetView3D();
    tmpWinAtts.GetView3D().SetCenterOfRotationSet(lastView3D.GetCenterOfRotationSet());
    tmpWinAtts.GetView3D().SetCenterOfRotation(lastView3D.GetCenterOfRotation());
    if (tmpWinAtts != lastRequest.winAtts)
        return false;

    // compare annotation attributes but ignore a few of them by forcing
    // old/new to be equal for those few
    AnnotationAttributes tmpAtts = thisRequest.annotAtts;
    tmpAtts.SetUserInfoFlag(lastRequest.annotAtts.GetUserInfoFlag());
    tmpAtts.SetDatabaseInfoFlag(lastRequest.annotAtts.GetDatabaseInfoFlag());
    tmpAtts.SetLegendInfoFlag(lastRequest.annotAtts.GetLegendInfoFlag());
    if (tmpAtts != lastRequest.annotAtts)
        return false;

    if (thisRequest.extStr != lastRequest.extStr)
        return false;

    if (thisRequest.lastChangedCtName != lastRequest.lastChangedCtName)
        return false;

/*
 * We have decided not to check annotation objects.  We only run this function
 * when we are doing an SR for a window (i.e. not a SaveWindow).  In this case,
 * the annotation objects are rendered locally.  So doing an SR is not necessary.
 *
 * Worse, it is hard to test if the annotation objects have really changed.
 * If you add a new plot (without click Draw), then a new legend annotation
 * object gets added.  There does not appear to be an easy way of determining
 * if this new annotation object should actually be drawn.  (It appears to be
 * done in the VisWindow by checking to see if there is a drawn plot associated
 * with the legend.)
 *
 * So, since determining if the annotation objects is hard, the easiest thing is
 * to just not check ... because they are rendered here on the viewer instead
 * of on the engine ... except for the SaveWindow case, which does not call
 * this function.
 *
    if (thisRequest.annotObjs != lastRequest.annotObjs)
    {
        return false;
    }
 */

    if (thisRequest.visCues != lastRequest.visCues)
        return false;

    bool sameFrameAndState = true;
    int i;
    for (i = 0; i < 7; i++)
    {
        if (thisRequest.frameAndState[i] != lastRequest.frameAndState[i])
        {
            sameFrameAndState = false;
            break;
        }
    }
    if (!sameFrameAndState)
        return false;

    bool sameViewExtents = true;
    for (i = 0; i < 6; i++)
    {
        if (thisRequest.viewExtents[i] != lastRequest.viewExtents[i])
        {
            sameViewExtents = false;
            break;
        }
    }
    if (!sameViewExtents)
        return false;

    if ((thisRequest.plotIdsList.size() != 0) &&
        (thisRequest.plotIdsList.size() != lastRequest.plotIdsList.size()))
        return false;

    for (i = 0; i < thisRequest.plotIdsList.size(); i++)
    {
        // search for index of current plot in last list
        int indexOfPlotInLastList = -1;
        for (int j = 0; j < lastRequest.plotIdsList.size(); j++)
        {
            if ((lastRequest.plotIdsList[j] == thisRequest.plotIdsList[i]) &&
                (lastRequest.engineKeysList[j] == thisRequest.engineKeysList[i]))
            {
                indexOfPlotInLastList = j;
                break;
            }
        }

        if (indexOfPlotInLastList == -1)
            return false;
        else
        {
            // compare plugin ids
            if (thisRequest.pluginIDsList[i] != lastRequest.pluginIDsList[indexOfPlotInLastList])
                return false;

            // compare plot attributes
            if (!thisRequest.attsList[i]->EqualTo(lastRequest.attsList[indexOfPlotInLastList]))
                return false;

        }
    }

    return true;
}

// ****************************************************************************
// Method: ViewerWindow::GetExternalRenderRequestInfo
//
// Purpose: Populates an ExternalRenderRequestInfo struct with all the
// information needed to process an external render request
//
// Programmer: Mark C. Miller
// Creation:   March 25, 2004 
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 30 11:04:04 PST 2004
//    Added an engine key used to index (and restart) engines.
//
//    Brad Whitlock, Mon Apr 5 12:23:31 PDT 2004
//    Renamed a method.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added code to get the extents type string
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added code to set annotation object list
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to deal with visual cues (pick points and ref lines)
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added code to deal with frame and state
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to deal with view extents
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to manage name of last color table to change
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye information to support stereo SR
//
//    Brad Whitlock, Wed Mar 21 22:18:06 PST 2007
//    Added the plot names (unique viewer names for the plot).
//
// ****************************************************************************

void
ViewerWindow::GetExternalRenderRequestInfo(
    ExternalRenderRequestInfo &theRequest,
    bool leftEye) const
{
    ClearExternalRenderRequestInfo(theRequest);

    // get information about the plots, their hosts, ids, and attributes
    GetPlotList()->GetPlotAtts(theRequest.plotNames,
                               theRequest.pluginIDsList,
                               theRequest.engineKeysList,
                               theRequest.plotIdsList,
                               theRequest.attsList);

    // get information about this window's attributes
    theRequest.winAtts = GetWindowAttributes();
    theRequest.annotAtts = *GetAnnotationAttributes();
    AnnotationObjectList aolist;
    UpdateAnnotationObjectList(aolist);
    theRequest.annotObjs = aolist;
    theRequest.extStr = avtExtentType_ToString(GetViewExtentsType());
    VisualCueList cuelist;
    UpdateVisualCueList(cuelist);
    theRequest.visCues = cuelist;

    int fns[7];
    visWindow->GetFrameAndState(fns[0], fns[1], fns[2], fns[3],
                                        fns[4], fns[5], fns[6]);
    for (int i = 0; i < 7; i++)
        theRequest.frameAndState[i] = fns[i];

    GetExtents(visWindow->GetWindowMode()==WINMODE_3D?3:2,
        theRequest.viewExtents);
    if (visWindow->GetWindowMode()!=WINMODE_3D)
    {
        theRequest.viewExtents[4] = 0.0;
        theRequest.viewExtents[5] = 0.0;
    }

    theRequest.lastChangedCtName = nameOfCtChangedSinceLastRender;
    theRequest.leftEye = leftEye;
}

// ****************************************************************************
// Method: ViewerWindow::ExternalRender
//
// Purpose: Make an explicit external render request for the window. Do not
// consider things like whether to skip the external render request, to
// update the last request information, etc or whether external rendering
// should be turned off
//
// Programmer: Mark C. Miller
// Creation:   March 25, 2004 
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 30 11:04:04 PST 2004
//    Added an engine key used to index (and restart) engines.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added argument for extents type string in call to ExternalRender
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Replaced long list of args relating to external render request with
//    single struct in call to eMgr->ExternalRender
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Changed name of image compositer class
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to pass window id to engine manager
//
//    Hank Childs, Fri Jan 28 11:55:12 PST 2005
//    Make sure returns inside 'try' are wrapped.
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added code to set whether window is being compressed or not 
//
//    Eric Brugger, Fri Mar 23 11:01:10 PDT 2007
//    Modified the routine to no longer return a NULL image if there were
//    no plots.
//
// ****************************************************************************
bool
ViewerWindow::ExternalRender(const ExternalRenderRequestInfo& thisRequest,
    bool& shouldTurnOffScalableRendering, bool doAllAnnotations,
    avtDataObject_p& dob)
{
    ViewerEngineManager *eMgr = ViewerEngineManager::Instance();
    bool success = false;
    std::vector<avtImage_p> imgList;

    TRY
    {
        // let the engine manager do the render
        success = eMgr->ExternalRender(thisRequest,
                                       shouldTurnOffScalableRendering,
                                       doAllAnnotations,
                                       imgList, GetWindowId());


    }
    CATCH2(VisItException, e)
    {
        char message[2048];
        //
        // Add as much information to the message as we can,
        // including plot name, exception type and exception 
        // message.
        // 
        SNPRINTF(message, sizeof(message), "%s:  (%s)\n%s", 
            "Scalable Render Request Failed",
            e.GetExceptionType().c_str(),
            e.Message().c_str());

        Error(message);

        // finally, make sure we return a "blank" image
        dob = NULL;
        CATCH_RETURN2(1, false);
    }
    ENDTRY

    // return noting if the request failed
    if (!success)
    {
        dob = NULL;
        return false;
    }

    if (imgList.size() == 0)
    {
        dob = NULL;
        return true;
    }

    // composite images from different engines as necessary
    if (imgList.size() > 1)
    {
        //
        // NOTE: YOU NEED TO MAKE SURE ALL ENGINES HAVE USED
        // SAME BACKGROUND COLOR IN ORDER FOR THIS TO WORK
        //
        avtWholeImageCompositerWithZ imageCompositer;
        int numRows = thisRequest.winAtts.GetSize()[1];
        int numCols = thisRequest.winAtts.GetSize()[0];

        imageCompositer.SetOutputImageSize(numRows, numCols);
        for (int i = 0; i < imgList.size(); i++)
            imageCompositer.AddImageInput(imgList[i], 0, 0);
        imageCompositer.Execute();
        dob = imageCompositer.GetOutput();
    }
    else if (imgList.size() == 1)
    {
        CopyTo(dob, imgList[0]);
    }

    if (imgList[0]->GetCompressionRatio() != -1.0)
        isCompressingScalableImage = true;
    else
        isCompressingScalableImage = false;

    return true;
}

// ****************************************************************************
// Method: ViewerWindow::ExternalRenderManual
//
// Purpose: Handle a manual external render request for the window. Do not
// consider things like whether to skip the external render request, to
// update the last request information, etc or whether external rendering
// could be turned off
//
// Programmer: Mark C. Miller
// Creation:   March 25, 2004 
//
// Modifications
//
//   Mark C. Miller, Wed Apr 21 12:42:13 PDT 2004
//   I made it issue a warning message on failure
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified scalable rendering controls to use activation mode and auto
//   threshold
//
//   Brad Whitlock, Thu May 11 15:19:46 PST 2006
//   I added code to clear the actors and update the frame when the image
//   can't be externally rendered. This prevents a problem where we end up
//   asking for an old plot on a new compute engine and it fails because
//   the plot has never been seen on the new compute engine. Since this
//   method seems to only be called from when we save or print images,
//   and since we don't want that to fail if we can help it, clear the
//   error message and then iterate through one more time. Hopefully, the
//   right compute engine gets launched.
//
//   Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//   Added leftEye arg to GetExternalRenderRequestInfo
//  
// ****************************************************************************

void
ViewerWindow::ExternalRenderManual(avtDataObject_p& dob, int w, int h)
{
    const char *mName = "ViewerWindow::ExternalRenderManual: ";
    bool dummyBool;
    int tries = 0;
    bool success = false;

    debug4 << mName << "start" << endl;
    do
    {
        debug4 << mName << "Calling GetExternalRenderRequestInfo" << endl;
        ExternalRenderRequestInfo thisRequest;
        GetExternalRenderRequestInfo(thisRequest, 1);

        // adjust rendering request info for this manual render
        debug4 << mName << "Making it do scalable rendering always" << endl;
        int size[2] = {w, h};
        thisRequest.winAtts.SetSize(size);
        thisRequest.winAtts.GetRenderAtts().
            SetScalableActivationMode(RenderingAttributes::Always);

        debug4 << mName << "Calling ExternalRender" << endl;
        success = ExternalRender(thisRequest, dummyBool, true, dob);

        if (!success)
        {
            debug4 << mName << "ExternalRender method returned false so"
                   << "let's clear the actors in the plot list and "
                   << "try to render again." << endl;
            ErrorClear();
            GetPlotList()->ClearActors();
            GetPlotList()->UpdateFrame();
        }

        ++tries;
    } while(!success && tries < 2);

    if(!success)
        Warning("Unable to obtain rendered image from engine");

    debug4 << mName << "end" << endl;
}

// ****************************************************************************
// Method: ViewerWindow::ExternalRenderAuto
//
// Purpose: Handle an automatic external render request for the window.
//
// Programmer: Mark C. Miller
// Creation:   March 25, 2004 
//
// Modifications
//
//   Mark C. Miller, Wed Apr 21 12:42:13 PDT 2004
//   I made it issue a warning message on failure. I also made it clear
//   plot list's actors and Update the frame.
//
//   Mark C. Miller, Wed Oct 20 15:52:51 PDT 2004
//   Added fault tolerance to external render request
//
//   Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//   Added leftEye to support stereo SR
// ****************************************************************************
void
ViewerWindow::ExternalRenderAuto(avtDataObject_p& dob, bool leftEye)
{
    if (!GetScalableRendering())
    {
       dob = NULL;
       return;
    }

    ExternalRenderRequestInfo thisRequest;
    GetExternalRenderRequestInfo(thisRequest, leftEye);

    if (thisRequest.plotIdsList.size() == 0)
    {
        dob = NULL;
        return;
    }

    // return now without modifying dob if we've decided we can skip 
    if (CanSkipExternalRender(thisRequest))
        return;

    // ok, now make an explict external render request
    bool shouldTurnOffScalableRendering = false;

    //
    // We have to handle fault-tolerance in external render requests
    // here in the ViewerWindow instead of the engine because ViewerWindow's
    // state gets modified upon each retry. Namely, the plot's actors and
    // network ids have to get reset. We can't simply retry the same render
    // request on the engine.
    //
    const int maxTries = 3;
    int trys = 0;
    bool success = false;
    while (!success && trys < maxTries)
    {
        success = ExternalRender(thisRequest,
                                 shouldTurnOffScalableRendering, false, dob);
        if (!success)
        {
            GetPlotList()->ClearActors();
            GetPlotList()->UpdateFrame();
            GetExternalRenderRequestInfo(thisRequest, leftEye);
        }
        trys++;
    }

    // return nothing if the request failed
    if (!success)
    {
        Warning("Unable to update view with new image from engine");
        GetPlotList()->SetErrorFlagAllPlots(true);
        GetPlotList()->ClearActors();
        GetPlotList()->UpdateFrame();
        dob = NULL;
        return;
    }

    // send an SR mode change message, if necessary
    if (shouldTurnOffScalableRendering)
    {
        SendScalableRenderingModeChangeMessage(false);
        dob = NULL;
        return;
    }

    // only update last request if this request wasn't empty
    if (thisRequest.plotIdsList.size() > 0)
        UpdateLastExternalRenderRequestInfo(thisRequest);
}

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
// Note: as of this writing, this is the only place where ViewerEngineManager
// is referenced. If we can remove the reference, don't forget to remove the
// include directive.
//
// Arguments:
//   data : pointer to the ViewerWindow object instance that set the callback 
//
// Programmer: Mark C. Miller 
// Creation:   03Apr03 
//
// Modifications:
//
//   Mark C. Miller, Thu Mar 25 21:08:25 PST 2004
//   Paired down to just dereference the void * for the window and call into
//   the objects method
//
//   Mark C. Miller, Fri Jul 21 08:05:15 PDT 2006
//   Extract multiple args from data buffer and pass to ExternalRenderAuto
//   This is to support stereo SR.
//
// ****************************************************************************
void
ViewerWindow::ExternalRenderCallback(void *data, avtDataObject_p& dob)
{
    unsigned char* argsBuf = (unsigned char*) data;
    ViewerWindow *win;
    bool leftEye;
    int i = 0;

    memcpy(&win, &argsBuf[0], sizeof(void*));
    i += sizeof(void*);
    memcpy(&leftEye, &argsBuf[i], sizeof(bool));

    win->ExternalRenderAuto(dob, leftEye);
}


// ****************************************************************************
//  Method: ViewerWindow::Lineout
//
//  Purpose: 
//    Perform lineout. 
//
//  Arguments:  
//    fromDefault  Specifies if the Lineout is to be performed based on the
//                 operator's default attributes.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    July 9, 2004A
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::Lineout(const bool fromDefault)
{
    ViewerQueryManager::Instance()->StartLineout(this, fromDefault);

    viewerSubject->MessageRendererThread("finishLineout;");
}


// ****************************************************************************
//  Method: ViewerWindow::SetInteractorAtts
//
//  Purpose: 
//    Set the interactor attributes of the window.
//
//  Arguments:
//    atts      The interactor attributes for this window. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//   
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::SetInteractorAtts(const InteractorAttributes *atts)
{
    visWindow->SetInteractorAtts(atts);
}

// ****************************************************************************
// Method: ViewerWindow::GetInteractorAtts
//
// Purpose: 
//   Returns a pointer to the VisWindow's interactor attributes.
//
// Note:       Note that the pointer returned by this method cannot be used
//             to set attributes of the interactor attributes.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004 
//
// Modifications:
//   
// ****************************************************************************

const InteractorAttributes *
ViewerWindow::GetInteractorAtts() const
{
    return (const InteractorAttributes *)visWindow->GetInteractorAtts();
}

// ****************************************************************************
// Method: ViewerWindow::CopyInteractorAtts
//
// Purpose: 
//   Copies the interactor attributes from the source window to this window.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004 
//
// ****************************************************************************

void
ViewerWindow::CopyInteractorAtts(const ViewerWindow *source)
{
    SetInteractorAtts(source->GetInteractorAtts());
}

// ****************************************************************************
// Method: ViewerWindow::GlyphPick
//
// Purpose: 
//   Tells the VisWindow to perform a glyph-type pick operation. 
//
// Programmer: Kathleen Bonnell 
// Creation:   September 27, 2004 
//
// Modifications:
//
// ****************************************************************************

void
ViewerWindow::GlyphPick(const double pt1[3], const double pt2[3], 
                        int &dom, int &elNum, bool &forCell)
{
    visWindow->GlyphPick(pt1, pt2, dom, elNum, forCell, GetNoWinMode()); 
}

// ****************************************************************************
//  Method: ViewerWindow::SetScaleMode
//
//  Purpose: Set the scale modes for plots and udpates the window.
//
//  Arguments:
//    ds        The domain scale mode.
//    rs        The range scale mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2007
//
//  Modifications:
//    Kathleen Bonnell, Tue Apr  3 17:19:28 PDT 2007
//    Added early termination for non-curve windows.
//
//    Kathleen Bonnell, Wed May  9 17:33:41 PDT 2007
//    Support 2d log scaling, use new WINDOW_MODE arg.
//
// ****************************************************************************

void
ViewerWindow::SetScaleMode(ScaleMode ds, ScaleMode rs, WINDOW_MODE wm)
{
    if (windowMode != wm)
    {
        return;
    }

    bool updatesEnabled = UpdatesEnabled();

    // remove all plot's actors from the VisWindow
    if (updatesEnabled)
        DisableUpdates();
    ClearPlots();

    // scale the plots
    GetPlotList()->SetScaleMode(ds, rs, wm);

    if (updatesEnabled)
    {
        EnableUpdates();
        GetPlotList()->UpdateFrame();
    }

    // update the window information
    ViewerWindowManager::Instance()->UpdateWindowInformation(
        WINDOWINFO_WINDOWFLAGS, windowId);
}

// ****************************************************************************
//  Method: ViewerWindow::GetScaleMode
//
//  Purpose: 
//    Retrieves the scaling mode from the appropriate view.
//
//  Arguments:
//    ds        A place to store the domain scale.
//    rs        A place to store the range scale.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed May  9 17:33:41 PDT 2007
//    Support 2d log scaling.
//
// ****************************************************************************

void
ViewerWindow::GetScaleMode(ScaleMode &ds, ScaleMode &rs, WINDOW_MODE wm)
{
    if (wm == WINMODE_CURVE)
    {
        const avtViewCurve &viewCurve = visWindow->GetViewCurve();
        ds = viewCurve.domainScale;
        rs = viewCurve.rangeScale;
    }
    else if (wm == WINMODE_2D)
    {
        const avtView2D &view2D = visWindow->GetView2D();
        ds = view2D.xScale;
        rs = view2D.yScale;
    }
    else
    {
        ds = LINEAR;
        rs = LINEAR; 
    }
}
// ****************************************************************************
//  Method: ViewerWindow::SetPlotFollowsTime
//
//  Purpose: Set the follows time flag
//
//  Arguments:
//
//  Programmer: Ellen Tarwater
//  Creation:   December 27, 2007
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindow::SetPlotFollowsTime()
{

    // toggle the follows time flag:
    GetPlotList()->SetPlotFollowsTime();

}
