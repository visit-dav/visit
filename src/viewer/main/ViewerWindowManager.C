// ************************************************************************* //
//                            ViewerWindowManager.C                          //
// ************************************************************************* //
#include <stdio.h> // for sscanf

#include <ViewerWindowManager.h>

#include <visitstream.h>
#include <string.h>
#include <snprintf.h>
#if !defined(_WIN32)
#include <signal.h> // for signal
#endif

#include <visit-config.h>

// for off-screen save window
#include <InitVTK.h>

#include <AnimationAttributes.h>
#include <AnnotationObjectList.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DataNode.h>
#include <EngineKey.h>
#include <GlobalAttributes.h>
#include <InteractorAttributes.h>
#include <KeyframeAttributes.h>
#include <LightList.h>
#include <LightAttributes.h>
#include <PrinterAttributes.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>
#include <ViewerActionManager.h>
#include <ViewerEngineManager.h>
#include <ViewerFileServer.h>
#include <ViewerWindow.h>
#include <ViewerWindowManagerAttributes.h>
#include <ViewerMessaging.h>
#include <ViewerPlotList.h>
#include <ViewerPlot.h>
#include <ViewerOperator.h>
#include <ViewerQueryManager.h>
#include <ViewerSubject.h>
#include <ViewerToolbar.h>
#include <VisWindow.h>
#include <VisItException.h>
#include <VisitInteractor.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginInfo.h>
#include <Line.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtImage.h>
#include <avtImageTiler.h>
#include <avtFileWriter.h>
#include <avtToolInterface.h>
#include <ImproperUseException.h>

#include <vtkQtImagePrinter.h>

#include <qtimer.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <DebugStream.h>
#include <ViewerOperatorFactory.h>

#include <algorithm>
using std::string;

//
// Storage for static data elements.
//
ViewerWindowManager *ViewerWindowManager::instance=0;
const int ViewerWindowManager::maxWindows=16;
const int ViewerWindowManager::maxLayouts=6;
const int ViewerWindowManager::validLayouts[]={1, 4, 9, 2, 6, 8};
GlobalAttributes *ViewerWindowManager::clientAtts=0;
SaveWindowAttributes *ViewerWindowManager::saveWindowClientAtts=0;
ViewCurveAttributes *ViewerWindowManager::viewCurveClientAtts=0;
View2DAttributes *ViewerWindowManager::view2DClientAtts=0;
View3DAttributes *ViewerWindowManager::view3DClientAtts=0;
AnimationAttributes *ViewerWindowManager::animationClientAtts=0;
AnnotationAttributes *ViewerWindowManager::annotationClientAtts=0;
AnnotationAttributes *ViewerWindowManager::annotationDefaultAtts=0;
InteractorAttributes *ViewerWindowManager::interactorClientAtts=0;
InteractorAttributes *ViewerWindowManager::interactorDefaultAtts=0;
KeyframeAttributes *ViewerWindowManager::keyframeClientAtts=0;
LightList *ViewerWindowManager::lightListClientAtts=0;
LightList *ViewerWindowManager::lightListDefaultAtts=0;
ViewerWindowManagerAttributes *ViewerWindowManager::windowAtts=0;
PrinterAttributes *ViewerWindowManager::printerAtts=0;
WindowInformation *ViewerWindowManager::windowInfo=0;
RenderingAttributes *ViewerWindowManager::renderAtts=0;
AnnotationObjectList *ViewerWindowManager::annotationObjectList = 0;
AnnotationObjectList *ViewerWindowManager::defaultAnnotationObjectList = 0;

//
// Global variables.  These should be removed.
//
extern ViewerSubject  *viewerSubject;

// ****************************************************************************
//  Method: ViewerWindowManager constructor
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Nov 7 09:42:35 PDT 2000
//    Changed to use ViewerWindow.
//
//    Brad Whitlock, Mon Nov 27 14:12:26 PST 2000
//    Made it inherit from QObject.
//
//    Eric Brugger, Tue Aug 21 10:17:24 PDT 2001
//    I added the registration of a callback for the view changing.
//
//    Kathleen Bonnell, Fri Sep 28 17:12:11 PDT 2001 
//    Initialize shiftX, shiftY, preshiftX, preshiftY to eliminate UMR. 
//
//    Brad Whitlock, Tue Oct 9 17:23:16 PST 2001
//    Added code to set the tool callback.
//
//    Eric Brugger, Fri Oct 26 14:33:26 PDT 2001
//    I added a timer for playing animations.
//
//    Eric Brugger, Fri Nov  2 12:39:55 PST 2001
//    I added a data member to keep track of whether the window has been
//    referenced before.
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002 
//    Added data member to keep track of the designated lineout window. 
//
//    Hank Childs, Fri May 24 11:20:34 PDT 2002
//    Replaced image writer with more general file writer.
//
//    Brad Whitlock, Wed Jul 24 14:44:52 PST 2002
//    I initialized the lastAnimation member.
//
//    Brad Whitlock, Wed Jan 22 16:44:26 PST 2003
//    I initialized the windowsIconified member.
//
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I added undoing of curve views.
//
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004 
//    Added timeQueryWindow. 
//
//    Eric Brugger, Thu Jun 30 11:43:22 PDT 2005
//    Added a 2 x 3 layout and removed the 4 x 4 layout.
//
// ****************************************************************************

ViewerWindowManager::ViewerWindowManager() : QObject()
{
    layout       = 1;
    layoutIndex  = 0;
    activeWindow = 0;

    screenX      = 0;
    screenY      = 0;
    screenWidth  = 800;
    screenHeight = 800;
    borderTop    = 32;
    borderBottom = 8;
    borderLeft   = 8;
    borderRight  = 8;

    shiftX = 0;
    shiftY = 0;
    preshiftX = 0;
    preshiftY = 0;

    windowsHidden = true;
    windowsIconified = false;

    nWindows      = 0;
    windows       = new ViewerWindow*[maxWindows];
    x_locations   = new int[maxWindows];
    y_locations   = new int[maxWindows];
    memset(windows, 0, maxWindows * sizeof(ViewerWindow *));
    referenced    = new bool[maxWindows];
    referenced[0] = true;
    for (int i = 1; i < maxWindows; i++)
    {
        referenced[i] = false;
    }

    windowLimits = new WindowLimits*[6];
    windowLimits[0] = new WindowLimits[1];
    windowLimits[1] = new WindowLimits[4];
    windowLimits[2] = new WindowLimits[9];
    windowLimits[3] = new WindowLimits[2];
    windowLimits[4] = new WindowLimits[6];
    windowLimits[5] = new WindowLimits[8];

    fileWriter  = new avtFileWriter();

    InitWindowLimits();

    //
    // Register a callback function to be called when the view changes from
    // using interactors.
    //
    VisitInteractor::RegisterViewCallback(ViewCallback);

    //
    // Register a callback function to be called when tools want to inform
    // the viewer of a new tool state.
    //
    avtToolInterface::SetCallback(ToolCallback);

    //
    // Create a timer that is used for animations.
    //
    animationTimeout = 1;
    lastAnimation = 0;
    timer = new QTimer(this, "viewerTimer");
    connect(timer, SIGNAL(timeout()), this, SLOT(AnimationCallback()));

    //
    // Initialize the view stacks.
    //
    viewStacking = true;
    viewCurveStackTop = -1;
    view2DStackTop = -1;
    view3DStackTop = -1;

    lineoutWindow = -1;
    timeQueryWindow = -1;
}

// ****************************************************************************
//  Method: ViewerWindowManager destructor
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerWindowManager::~ViewerWindowManager()
{
    //
    // This should never be executed.
    //
}

// ****************************************************************************
//  Method: ViewerWindowManager::Instance
//
//  Purpose:
//    Return a pointer to the sole instance of the ViewerWindowManager
//    class.
//
//  Returns:    A pointer to the sole instance of the ViewerWindowManager
//              class.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
// ****************************************************************************

ViewerWindowManager *
ViewerWindowManager::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new ViewerWindowManager;
    }

    return instance;
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetBorders
//
//  Purpose:
//    Set the sizes of the window borders used by the window manager.
//
//  Arguments:
//    windowBorders  The window borders.  The format for the string is
//                   "top,bottom,left,right".
//
//  Programmer: Eric Brugger
//  Creation:   September 13, 2000
//
// ****************************************************************************

void
ViewerWindowManager::SetBorders(const char *windowBorders)
{
    //
    // Check the arguments.
    //
    if (windowBorders == 0)
    {
        return;
    }

    //
    // Parse the borders string into its pieces.  This will only parse
    // the borders string if it is in the form Top,Bottom,Left,Right.
    //
    sscanf(windowBorders, "%d,%d,%d,%d", &borderTop, &borderBottom,
           &borderLeft, &borderRight);

    //
    // Reset the window limits structure using the new border information.
    //
    InitWindowLimits();
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetShift
//
//  Purpose:
//    Set the amounts of the window shift needed by the window manager.
//
//  Arguments:
//    windowShift    The window shift.  The format for the string is "X,Y"
//
//  Programmer: Jeremy Meredith
//  Creation:   July 19, 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetShift(const char *windowShift)
{
    //
    // Check the arguments.
    //
    if (windowShift == 0)
    {
        return;
    }

    //
    // Parse the shift string into its pieces.  This will only parse
    // the shift string if it is in the form X,Y.
    //
    sscanf(windowShift, "%d,%d", &shiftX, &shiftY);

    //
    // Reset the window limits structure using the new border information.
    //
    InitWindowLimits();
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetPreshift
//
//  Purpose:
//    Set the amounts of the window preshift needed by the window manager.
//
//  Arguments:
//    windowPreshift  The window preshift. The format for the string is "X,Y"
//
//  Programmer: Jeremy Meredith
//  Creation:   September 14, 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetPreshift(const char *windowPreshift)
{
    //
    // Check the arguments.
    //
    if (windowPreshift == 0)
    {
        return;
    }

    //
    // Parse the preshift string into its pieces.  This will only parse
    // the preshift string if it is in the form X,Y.
    //
    sscanf(windowPreshift, "%d,%d", &preshiftX, &preshiftY);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetGeometry
//
//  Purpose:
//    Set the area to be used by the window manager.  The window manager
//    will keep the windows in the area as best as possible.
//
//  Arguments:
//    windowGeometry  The area to keep the windows.  It is specified as
//                    an X Window System geometry string.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
// ****************************************************************************

void
ViewerWindowManager::SetGeometry(const char *windowGeometry)
{
    //
    // Check the arguments.
    //
    if (windowGeometry == 0)
    {
        return;
    }

    //
    // Parse the geometry string into its pieces.  This will only parse
    // the geometry if it is in the form WidthxHeight+X+Y.  In X the width
    // and height or x and y position are optional, also a minus sign can
    // be used instead of a plus sign.
    //
    sscanf(windowGeometry, "%dx%d+%d+%d", &screenWidth, &screenHeight,
           &screenX, &screenY);

    //
    // Reset the window limits structure using the new virtual screen
    // information.
    //
    InitWindowLimits();
}

// ****************************************************************************
//  Method: ViewerWindowManager::AddWindow
//
//  Purpose:
//    Add a new window.  The new window becomes the active window.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Apr 23 14:38:41 PST 2001
//    Sent an error message to the GUI instead of cerr.
//
//    Brad Whitlock, Fri Jul 27 09:48:55 PDT 2001
//    Added code to send the new view to the client.
//
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
//    Brad Whitlock, Thu Aug 30 09:42:40 PDT 2001
//    Added code to update the client annotation attributes.
//
//    Brad Whitlock, Fri Sep 14 15:26:17 PST 2001
//    Added code to update the client's light list.
//
//    Eric Brugger, Wed Nov 21 12:12:45 PST 2001
//    I added animation attributes.
//
//    Kathleen Bonnell, Tue Nov 27 16:03:00 PST 2001
//    Added pick attributes. 
//
//    Brad Whitlock, Mon Feb 4 10:27:29 PDT 2002
//    Moved the update code into UpdateAllAtts.
//
//    Brad Whitlock, Tue Oct 15 16:45:35 PST 2002
//    Added a boolean flag that optionally tells the method to copy attributes.
//
//    Mark C. Miller, Mon Jan 13 16:52:33 PST 2003
//    Added code to register the external render callback
//
//    Eric Brugger, Thu Mar 13 10:50:11 PST 2003
//    I implemented CloneWindowOnFirstRef mode, which clones the current
//    window to the newly activated window when it is first referenced.
//
//    Eric Brugger, Fri Apr 11 14:03:19 PDT 2003
//    I moved part of the functionality to SimpleAddWindow and added a
//    call to it.
//
//    Brad Whitlock, Fri Nov 7 10:03:35 PDT 2003
//    I added code to copy the annotation object list.
//
//    Brad Whitlock, Tue Jan 27 17:26:46 PST 2004
//    I changed the copy code a little bit.
//
//    Brad Whitlock, Thu Feb 17 14:23:24 PST 2005
//    Added bool to ViewerPlotList::CopyFrom.
//
//    Brad Whitlock, Fri Apr 15 17:06:49 PST 2005
//    Added code to copy action data.
//
// ****************************************************************************

void
ViewerWindowManager::AddWindow(bool copyAtts)
{
    //
    // Add a simple window.
    //
    int       windowIndex;

    windowIndex = SimpleAddWindow();
    if (windowIndex == -1)
    {
        Error("The maximum number of windows was exceeded.");
        return;
    }

    //
    // Copy attributes.
    //
    if ((copyAtts || clientAtts->GetCloneWindowOnFirstRef()) &&
        (windowIndex != activeWindow))
    {
        ViewerWindow *dest = windows[windowIndex];
        ViewerWindow *src = windows[activeWindow];
        dest->CopyGeneralAttributes(src);
        dest->CopyAnnotationAttributes(src);
        dest->CopyAnnotationObjectList(src);
        dest->CopyLightList(src);
        dest->CopyViewAttributes(src);
        dest->GetPlotList()->CopyFrom(src->GetPlotList(), true);
        dest->GetActionManager()->CopyFrom(src->GetActionManager());
    }
    referenced[windowIndex] = true;

    // Always copy the Interactor atts
    if (windowIndex != activeWindow)
    {
        windows[windowIndex]->CopyInteractorAtts(windows[activeWindow]);
    }

    //
    // Now that the view has been set up (and other things), we can set the
    // window atts.  This is to make sure that we turn on view locking after
    // the view has been set.
    //
    SetWindowAttributes(windowIndex, copyAtts);

    //
    // Make the new window the active window.
    //
    activeWindow = windowIndex;

    //
    // Update all the client window attributes.
    //
    UpdateAllAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::CloneWindow
//
// Purpose: 
//   Creates a new window and copies the current window's attributes to it
//   before making it active.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:43:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::CloneWindow()
{
    AddWindow(true);
}

// ****************************************************************************
//  Method: ViewerWindowManager::ClearAllWindows
//
//  Purpose:
//      Clear all the windows.
//
//  Programmer: Eric Brugger
//  Creation:   October 4, 2000
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindowManager::ClearAllWindows()
{
    //
    // Loop over all the windows, if the window exists, clear all the
    // plots in the associated animation.
    //
    for (int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        ClearWindow(windowIndex);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ClearWindow
//
// Purpose: 
//   Clears the window with the specified index.
//
// Arguments:
//   windowIndex : A zero-origin integer that identifies the window
//                 to clear. If windowIndex happens to be -1, use the
//                 active window's index.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 7 09:50:31 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::ClearWindow(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
        windows[index]->ClearWindow();
}

// ****************************************************************************
// Method: ViewerWindowManager::CopyAnnotationsToWindow
//
// Purpose: 
//   Copies the annotation attributes from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:47:26 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Nov 6 17:15:32 PST 2003
//   I made it copy the annotation object list.
//
// ****************************************************************************

void
ViewerWindowManager::CopyAnnotationsToWindow(int from, int to)
{
    if(from < 0 || from >= maxWindows)
        return;
    if(to < 0 || to >= maxWindows)
        return;

    // If the Window pointers are valid then perform the operation.
    if(windows[from] != 0 && windows[to] != 0)
    {
        windows[to]->CopyAnnotationAttributes(windows[from]);
        windows[to]->CopyAnnotationObjectList(windows[from]);
        if(to == activeWindow)
        {
            UpdateAnnotationAtts();
            UpdateAnnotationObjectList();
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::CopyLightingToWindow
//
// Purpose: 
//   Copies the lighting attributes from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:47:26 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::CopyLightingToWindow(int from, int to)
{
    if(from < 0 || from >= maxWindows)
        return;
    if(to < 0 || to >= maxWindows)
        return;

    // If the Window pointers are valid then perform the operation.
    if(windows[from] != 0 && windows[to] != 0)
    {
        windows[to]->CopyLightList(windows[from]);
        if(to == activeWindow)
            UpdateLightListAtts();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::CopyViewToWindow
//
// Purpose: 
//   Copies the view attributes from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:47:26 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::CopyViewToWindow(int from, int to)
{
    if(from < 0 || from >= maxWindows)
        return;
    if(to < 0 || to >= maxWindows)
        return;

    // If the Window pointers are valid then perform the operation.
    if(windows[from] != 0 && windows[to] != 0)
    {
        windows[to]->CopyViewAttributes(windows[from]);
        if(to == activeWindow)
            UpdateViewAtts();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::CopyPlotListToWindow
//
// Purpose: 
//   Copies the plots from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:39:42 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 17 14:23:24 PST 2005
//   Added bool to ViewerPlotList::CopyFrom.
//
// ****************************************************************************

void
ViewerWindowManager::CopyPlotListToWindow(int from, int to)
{
    if(from < 0 || from >= maxWindows)
        return;
    if(to < 0 || to >= maxWindows)
        return;

    // If the Window pointers are valid then perform the operation.
    if(windows[from] != 0 && windows[to] != 0)
    {
        windows[to]->GetPlotList()->CopyFrom(windows[from]->GetPlotList(),true);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::DeleteWindow
//
//  Purpose:
//    Delete the currently active window.
//
//  Programmer: Eric Brugger
//  Creation:   September 13, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Nov 27 14:04:51 PST 2000
//    Added code to emit a deleteWindow signal.
//
//    Brad Whitlock, Fri Jul 27 09:48:30 PDT 2001
//    Added code to send the new view to the client.
//
//    Brad Whitlock, Wed Aug 22 11:29:11 PDT 2001
//    Moved the code into the new DeleteWindow method.
//
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
//    Eric Brugger, Wed Nov 21 12:12:45 PST 2001
//    I added animation attributes.
//
// ****************************************************************************

void
ViewerWindowManager::DeleteWindow()
{
    //
    // Delete the active window.
    //
    DeleteWindow(windows[activeWindow]);
}

// ****************************************************************************
//  Method: ViewerWindowManager::DeleteWindow
//
//  Purpose: 
//    Deletes the specified viewer window.
//
//  Arguments:
//    win       A pointer to the viewer window that we want to delete.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 22 11:38:24 PDT 2001
//
//  Modifications:
//    Brad Whitlock, Thu Aug 30 09:43:51 PDT 2001
//    Added code to update the client's annotation attributes.
//
//    Brad Whitlock, Fri Sep 14 15:25:44 PST 2001
//    Added code to update the client's light list.
//
//    Kathleen Bonnell, Tue Nov 27 16:03:00 PST 2001
//    Added pick attributes. 
//
//    Brad Whitlock, Mon Feb 4 10:26:48 PDT 2002
//    Moved the update code into UpdateAllAtts.
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002 
//    Added call to ResetLineoutDesignation. 
//    
//    Hank Childs, Wed Jul 10 21:46:55 PDT 2002
//    Unlock a window before deleting it.
//
//    Kathleen Bonnell, Wed Jul 31 16:43:43 PDT 2002  
//    Notify ViewerQueryManager that a window is being deleted. 
//
//    Kathleen Bonnell, Wed Apr 14 16:19:18 PDT 2004 
//    Added call to ResetTimeQueryDesignation. 
//    
// ****************************************************************************

void
ViewerWindowManager::DeleteWindow(ViewerWindow *win)
{
    //
    // The user can't delete the last active window.
    //
    if(nWindows <= 1)
    {
        Error("Can't delete the last window.");
        return;
    }

    //
    // Find the window index of the window we're deleting.
    //
    int windowIndex;
    for(windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if(windows[windowIndex] == win && windows[windowIndex] != 0)
        {
            break;
        }
    }

    // 
    // If we could not find a window index for the specified window, return.
    //
    if(windowIndex >= maxWindows)
        return;

    //
    // Find the first window that isn't the window we're deleting.
    //
    int newWindowIndex;
    for(newWindowIndex = 0; newWindowIndex < maxWindows; ++newWindowIndex)
    {
        if(windows[newWindowIndex] != win && windows[newWindowIndex] != 0)
        {
            break;
        }
    }

    //
    // Emit a signal to other objects that "win" is being deleted.
    //
    emit deleteWindow(win);

    //
    // Tell some of our other mechanisms that this window is going away.
    //
    ViewerQueryManager::Instance()->Delete(win);
    ResetLineoutDesignation(windowIndex);
    ResetTimeQueryDesignation(windowIndex);
    if (windows[windowIndex]->GetViewIsLocked())
    {
        ToggleLockViewMode(windowIndex);
    }

    //
    // Delete the active animation and window.
    //
    delete windows[windowIndex];
    windows[windowIndex] = 0;
    referenced[windowIndex] = false;
    nWindows--;

    //
    // Make the lowest number window the new active window if we're deleting
    // the active window.
    //
    if(windowIndex == activeWindow)
        activeWindow = newWindowIndex;

    //
    // Send a message to the client that indicates which window was deleted.
    //
    char msg[200];
    SNPRINTF(msg, 200, "Window %d was deleted.", windowIndex + 1);
    Message(msg);

    //
    // Update all of the client window attributes
    //
    UpdateAllAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::DisableRedraw
//
// Purpose: 
//   Disallows redraws in the specified window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 15:14:41 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::DisableRedraw(int windowIndex)
{
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->DisableUpdates();
    }    
}

// ****************************************************************************
// Method: ViewerWindowManager::FileInUse
//
// Purpose: 
//   Determines if a file is in use in plots across all windows.
//
// Arguments:
//   host   : The hostname.
//   dbName : The datanase name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:32:14 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 26 14:42:50 PST 2004
//   I made it use strings.
//
// ****************************************************************************

bool
ViewerWindowManager::FileInUse(const std::string &host,
    const std::string &dbName) const
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
        {
            if(windows[i]->GetPlotList()->FileInUse(host, dbName))
                return true;
        } 
    }

    return false;
}

// ****************************************************************************
// Method: ViewerWindowManager::IconifyAllWindows
//
// Purpose: 
//   Iconifies all viewer windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:14:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Jan 22 16:45:24 PST 2003
//   I added code to turn off animation.
//
// ****************************************************************************

void
ViewerWindowManager::IconifyAllWindows()
{
    // Disable animation.
    windowsIconified = true;
    UpdateAnimationTimer();

    // Iconify the windows
    for(int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if(windows[windowIndex] != 0)
            windows[windowIndex]->Iconify();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::DeIconifyAllWindows
//
// Purpose: 
//   De-iconifies all viewer windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:14:29 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Jan 22 16:46:15 PST 2003
//   I added code to enable animation if it was going before the windows
//   were iconified.
//
// ****************************************************************************

void
ViewerWindowManager::DeIconifyAllWindows()
{
    // Disable animation.
    windowsIconified = false;
    UpdateAnimationTimer();

    for(int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if(windows[windowIndex] != 0)
            windows[windowIndex]->DeIconify();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ShowAllWindows
//
// Purpose: 
//   Shows all viewer windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:37:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
ViewerWindowManager::ShowAllWindows()
{
    windowsHidden = false;

    for (int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if (windows[windowIndex] != 0)
        {
            if (windows[windowIndex]->GetRealized() == true)
            {
                windows[windowIndex]->Show();
            }
            else
            {
                windows[windowIndex]->SetLocation(
                    x_locations[windowIndex] - preshiftX,
                    y_locations[windowIndex] - preshiftY);
                windows[windowIndex]->Realize();
                windows[windowIndex]->SetLocation(x_locations[windowIndex],
                                                  y_locations[windowIndex]);
            }
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::HideAllWindows
//
// Purpose: 
//   Hides all viewer windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:37:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
ViewerWindowManager::HideAllWindows()
{
    windowsHidden = true;

    for (int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if (windows[windowIndex] != 0)
            windows[windowIndex]->Hide();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::InvertBackgroundColor
//
// Purpose:
//   This is a Qt slot function that is called when the popup menu's invert
//   button is clicked.
//
// Arguments:
//   windowIndex : The index of the window that called this method.
//
// Programmer: Brad Whitlock
// Creation:   August 26, 2001
//
// Modifications:
//
// ****************************************************************************

void
ViewerWindowManager::InvertBackgroundColor(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->InvertBackgroundColor();
    }

    // Update the annotation attributes and send them to the client if the
    // windowIndex == activeWindow.
    if(index == activeWindow)
        UpdateAnnotationAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::RedrawWindow
//
// Purpose: 
//   Redraws the specified window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 15:14:41 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::RedrawWindow(int windowIndex)
{
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->RedrawWindow();
    }    
}

// ****************************************************************************
//  Method: ViewerWindowManager::ResetView
//
//  Purpose: 
//    This method resets the view for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 13:31:14 PST 2000
//
//  Modifications:
//    Brad Whitlock, Fri Jul 27 09:47:20 PDT 2001
//    Added code to send the new view to the client.
//
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
// ****************************************************************************

void
ViewerWindowManager::ResetView(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->ResetView();

        //
        // Send the new view info to the client.
        //
        UpdateViewAtts(index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::RecenterView
//
//  Purpose: 
//    This method recenters the view for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Programmer: Eric Brugger
//  Creation:   February 23, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jul 27 09:46:52 PDT 2001
//    Added code to sent the new view to the client.
//
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
// ****************************************************************************

void
ViewerWindowManager::RecenterView(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->RecenterView();

        //
        // Send the new view info to the client.
        //
        UpdateViewAtts(index);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::SetCenterOfRotation
//
// Purpose: 
//   Sets the center of rotation using a world space coordinate.
//
// Arguments:
//   windowIndex : The index of the window whose center we're setting.
//   x,y,z       : The new world space center of rotation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 09:59:50 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::SetCenterOfRotation(int windowIndex,
    double x, double y, double z)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    if(windows[windowIndex] != 0)
    {
        windows[windowIndex]->SetCenterOfRotation(x, y, z);

        //
        // Send the new view info to the client.
        //
        UpdateViewAtts(windowIndex);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ChooseCenterOfRotation
//
// Purpose: 
//   Chooses the center of rotation using the surface that lies at point
//   sx,sy in the vis window's screen.
//
// Arguments:
//   sx, sy : The screen point. sx and sy in [0,1].
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 09:58:54 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::ChooseCenterOfRotation(int windowIndex,
    double sx, double sy)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    if(windows[windowIndex] != 0)
    {
        windows[windowIndex]->ChooseCenterOfRotation(sx, sy);

        //
        // Send the new view info to the client.
        //
        UpdateViewAtts(windowIndex);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SaveWindow
//
//  Purpose: 
//    Saves the screen captured contents of the window with the specified index
//
//  Arguments:
//    windowIndex A zero-origin integer that identifies the window
//                to save. If windowIndex happens to be -1, use the
//                active window's index.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 14:29:11 PST 2001
//    Modified code to support saving tiled images and shipping images to
//    the engine.
//
//    Brad Whitlock, Tue May 1 11:32:38 PDT 2001
//    Added code to send status to the GUI.
//
//    Brad Whitlock, Tue Aug 21 10:08:34 PDT 2001
//    Fixed an off by one error in the message that is sent to the client.
//
//    Brad Whitlock, Fri Sep 21 13:17:22 PST 2001
//    Modified the code so window saving status and messages are kept around
//    longer.
//
//    Brad Whitlock, Wed Jan 23 14:16:09 PST 2002
//    Enabled JPEG images.
//
//    Hank Childs, Thu May 23 18:47:33 PDT 2002
//    Added support for writing datasets to files, as well as images.
//
//    Hank Childs, Mon May 27 11:56:02 PDT 2002 
//    Added binary argument to writing a file.  Also add exception handling.
//
//    Jeremy Meredith, Thu Jul 25 11:52:24 PDT 2002
//    Added code to stuff the true filename back into the attributes.
//    This way the CLI/GUI may use the real filename used to save.
//
//    Jeremy Meredith, Fri Jul 26 14:06:52 PDT 2002
//    Added a call to compact parallel domains before saving a dataset.
//
//    Brad Whitlock, Tue Nov 12 13:57:10 PST 2002
//    Added code to catch ImproperUseException when setting the file format.
//
//    Hank Childs, Mon Feb 24 18:14:52 PST 2003
//    Be more leery of NULL return types.
//
//    Hank Childs, Wed Oct 15 09:45:16 PDT 2003
//    Added ability to save out images in stereo.
//
//    Hank Childs, Tue Dec 16 10:58:20 PST 2003
//    Take more care in creating filenames for stereo images in subdirectories.
//
//    Mark C. Miller, Mon Mar 29 20:25:49 PST 2004
//    Changed 'Saving...' status message to indicate that its rendering.
//    Added new status message for saving the image to disk
//
//    Hank Childs, Tue Apr  6 18:05:39 PDT 2004
//    Do not save out empty data objects.
//
//    Brad Whitlock, Thu Jul 15 16:27:15 PST 2004
//    I made it send a different message for tiled images.
//
//    Brad Whitlock, Fri Jul 30 16:23:09 PST 2004
//    I added code to figure out the basename for the filenames when an
//    output directory is specified.
//
//    Hank Childs, Wed Feb 16 07:16:56 PST 2005
//    If we can't save the file, the returned filename is NULL.  Handle this
//    gracefully.
//
//    Brad Whitlock, Wed Feb 23 17:19:41 PST 2005
//    Added a call to ClearStatus.
//
//    Mark C. Miller, Tue May  3 21:49:22 PDT 2005
//    Added error check for attempts to save curve formats from windows
//    in SR mode
//
// ****************************************************************************

void
ViewerWindowManager::SaveWindow(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
    {
        debug1 << "Invalid window index " << windowIndex << " of " 
               << maxWindows << " was specified." << endl;
        return;
    }

    TRY
    {
        // If an image was returned, save it to the appropriate location.
        fileWriter->SetFormat(saveWindowClientAtts->GetFormat());
    }
    CATCH(ImproperUseException)
    {
        Error("VisIt cannot save images in the specified file format.");
        CATCH_RETURN(1);
    }
    ENDTRY

    //
    // Figure out a candidate for the base filename. This involves
    // gluing the output directory on when necessary.
    //
    std::string fileBase;
    if(saveWindowClientAtts->GetOutputToCurrentDirectory())
    {
        fileBase = saveWindowClientAtts->GetFileName();
    }
    else
    {
        fileBase = saveWindowClientAtts->GetOutputDirectory();
        if(fileBase.size() > 0)
        {
            std::string f(saveWindowClientAtts->GetFileName());
            if(f.size() == 0) 
            {
                f = "visit";
                Warning("The specified filename was empty. VisIt will "
                        "use the name \"visit\" as the base for the files "
                        "to be saved.");
            }

            if(fileBase[fileBase.size() - 1] == SLASH_CHAR)
            {
                if(f[0] == SLASH_CHAR)
                    fileBase = fileBase.substr(0,fileBase.size()-1) + f;
                else
                    fileBase += f;
            }
            else
            {
                if(f[0] == SLASH_CHAR)
                    fileBase += f;
                else
                    fileBase = fileBase + std::string(SLASH_STRING) + f;
            }
        }
        else
            fileBase = saveWindowClientAtts->GetFileName();
    }

    //
    // We need to get a file name.  If we are in stereo, then we will need
    // two file names.
    //
    char *filename = NULL;
    char *filename2 = NULL;
    if (saveWindowClientAtts->CurrentFormatIsImageFormat() &&
        saveWindowClientAtts->GetStereo())
    {
        if (saveWindowClientAtts->GetFamily())
        {
            filename = fileWriter->CreateFilename(fileBase.c_str(),
                                  saveWindowClientAtts->GetFamily());
            filename2 = fileWriter->CreateFilename(fileBase.c_str(),
                                  saveWindowClientAtts->GetFamily());
        }
        else
        {
            //
            // Find everything up until the last slash.
            //
            const char *fname = fileBase.c_str();
            const char *tmp = fname, *last = NULL;
            while (tmp != NULL)
            {
                tmp = strstr(tmp, SLASH_STRING);
                if (tmp != NULL)
                {
                    last = tmp;
                    tmp = tmp+1;
                }
            }

            //
            // Now construct the string into directory and filename portions.
            //
            char dir_prefix[1024];
            char stem[1024];
            bool has_dir_prefix = false;
            if (last != NULL)
            {
                strncpy(dir_prefix, fname, last-fname);
                dir_prefix[last-fname] = '\0';
                strcpy(stem, last+1);
                has_dir_prefix = true;
            }
            else
            {
                strcpy(stem, fname);
            }
            
            //
            // Construct the string depending on whether or not there is a dir.
            //
            char left_prefix[1024];
            char right_prefix[1024];
            if (has_dir_prefix)
            {
                sprintf(left_prefix, "%s%cleft_%s", dir_prefix, SLASH_CHAR,
                                                    stem);
                sprintf(right_prefix, "%s%cright_%s", dir_prefix,
                                                      SLASH_CHAR, stem);
            }
            else
            {
                sprintf(left_prefix, "left_%s", stem);
                sprintf(right_prefix, "right_%s", stem);
            }
            filename = fileWriter->CreateFilename(left_prefix,
                                  saveWindowClientAtts->GetFamily());
            filename2 = fileWriter->CreateFilename(right_prefix,
                                  saveWindowClientAtts->GetFamily());
        }
    }
    else
    {
        filename = fileWriter->CreateFilename(fileBase.c_str(),
                              saveWindowClientAtts->GetFamily());
    }

    //
    // Send a status message about starting to render the image and make the
    // status message display for 10 minutes.
    //
    char message[1000];
    if(saveWindowClientAtts->GetSaveTiled())
    {
        strcpy(message, "Saving tiled image...");
    }
    else
    {
        SNPRINTF(message, sizeof(message), "Rendering window %d...",
                (windowIndex == -1) ? (activeWindow + 1) : (windowIndex + 1));
    }
    Status(message, 6000000);
    Message(message);

    avtDataObject_p dob = NULL;
    avtDataObject_p dob2 = NULL;
    if (saveWindowClientAtts->CurrentFormatIsImageFormat())
    {
        avtImage_p image = NULL;
        avtImage_p image2 = NULL;

        if (saveWindowClientAtts->GetSaveTiled())
        {
            // Create a tiled image for the left eye.
            image = CreateTiledImage(saveWindowClientAtts->GetWidth(),
                                     saveWindowClientAtts->GetHeight(),
                                     true);
            // Create a tiled image for the right eye.
            if (saveWindowClientAtts->GetStereo())
            {
                image2 = CreateTiledImage(saveWindowClientAtts->GetWidth(),
                                          saveWindowClientAtts->GetHeight(),
                                          false);
            }
        }
        else 
        {
            // Create the left eye.
            image = CreateSingleImage(windowIndex,
                                      saveWindowClientAtts->GetWidth(),
                                      saveWindowClientAtts->GetHeight(),
                                      saveWindowClientAtts->GetScreenCapture(),
                                      true);

            // Create the right eye.
            if (saveWindowClientAtts->GetStereo())
            {
                image2 = CreateSingleImage(windowIndex,
                                           saveWindowClientAtts->GetWidth(),
                                           saveWindowClientAtts->GetHeight(),
                                           saveWindowClientAtts->GetScreenCapture(),
                                           false);
            }
        }
        CopyTo(dob, image);
        CopyTo(dob2, image2);
    }
    else
    {
        bool windowIsInScalableRenderingMode;
        avtDataset_p ds = GetDataset(windowIndex,
                                     windowIsInScalableRenderingMode);

        if (windowIsInScalableRenderingMode)
        {
            Error("You cannot save curve formats (ultra, curve) from a window "
                  "that is currently in scalable rendering mode.");
            return;
        }

        if (*ds != NULL)
            ds->Compact();
        CopyTo(dob, ds);
    }

    //
    // Send a status message about starting to save the image to disk
    // and make the status message display for 10 minutes.
    //
    if(!saveWindowClientAtts->GetSaveTiled())
    {
        SNPRINTF(message, sizeof(message), "Saving window %d...",
                (windowIndex == -1) ? (activeWindow + 1) : (windowIndex + 1));
        Status(message, 6000000);    
        Message(message);
    }

    // Save the window.
    bool savedWindow = true;
    if (*dob != NULL)
    {
        TRY
        {
            if (filename != NULL)
            {
                // Tell the writer to save the window on the viewer.
                fileWriter->Write(filename, dob,saveWindowClientAtts->GetQuality(),
                                  saveWindowClientAtts->GetProgressive(),
                                  saveWindowClientAtts->GetCompression(),
                                  saveWindowClientAtts->GetBinary());

                if (*dob2 != NULL)
                {
                    // Tell the writer to save the window on the viewer.
                    fileWriter->Write(filename2, 
                                      dob2,saveWindowClientAtts->GetQuality(),
                                      saveWindowClientAtts->GetProgressive(),
                                      saveWindowClientAtts->GetCompression(),
                                      saveWindowClientAtts->GetBinary());
                }
            }
        }
        CATCH2(VisItException, ve)
        {
            Warning(ve.Message().c_str());
            ClearStatus();
            savedWindow = false;
        }
        ENDTRY
    }
    else
    {
        if (saveWindowClientAtts->CurrentFormatIsImageFormat())
        {
            Warning("No image was saved.  This is "
                  "frequently because you have asked to save an empty window."
                  "  If this is not the case, please contact a VisIt "
                  "developer.");
        }
        else
        {
            Warning("No surface was saved.  This is "
                  "frequently because you have asked to save an empty window."
                  "  This also happens if you are in Scalable Rendering mode."
                  "  If this is not the case, please contact a VisIt "
                  "developer.\n\n\n"
                  "If you are in scalable rendering mode and want to save a "
                  "polygonal file, go to Options->Rendering to disable this "
                  "mode.  This may cause VisIt to slow down substantially.");
        }
        ClearStatus();
        savedWindow = false;
    }

    // Send a message to indicate that we're done saving the image.
    if (savedWindow && filename != NULL)
    {
        SNPRINTF(message, 1000, "Saved %s", filename);
        Status(message);
        Message(message);
    }
    else
    {
        SNPRINTF(message, 1000, "Could not save window");
        Status(message);
        Message(message);
    }

    if (filename != NULL)
    {
        saveWindowClientAtts->SetLastRealFilename(filename);
        saveWindowClientAtts->Notify();
    }

    // Delete the filename memory.
    if (filename != NULL)
        delete [] filename;
    if (filename2 != NULL)
        delete [] filename2;
}

// ****************************************************************************
//  Method: ViewerWindowManager::CreateSingleImage
//
//  Purpose: 
//    Returns an avtImage representation of the VisWindow.
//
//  Arguments:
//    windowIndex    The window index for which we want an image.
//    width          The desired width of the return image.
//    height         The desired height of the return image.
//    screenCapture  A flag indicating whether or not to do screen capture.
//    leftEye        True if we want the left eye.
//
//  Returns:    An avtImage representation of the specified VisWindow.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 13 15:17:32 PST 2001
//
//  Modifications:
//
//    Hank Childs, Wed Oct 15 10:30:33 PDT 2003
//    Added stereo support.
//
//    Mark C. Miller, Mon Mar 29 14:05:23 PST 2004
//    Enabled non-screen-capture based mode as well as width and height
//    arguments
//
// ****************************************************************************

avtImage_p
ViewerWindowManager::CreateSingleImage(int windowIndex,
    int width, int height, bool screenCapture, bool leftEye)
{
    int        index = (windowIndex == -1) ? activeWindow : windowIndex;
    avtImage_p retval = NULL;

    if(windows[index] != 0)
    {
        if (!leftEye)
        {
            windows[index]->ConvertFromLeftEyeToRightEye();
        }

        if(screenCapture)
            retval = windows[index]->ScreenCapture();
        else
        {
            avtDataObject_p extImage;
            windows[index]->ExternalRenderManual(extImage, width, height);
            CopyTo(retval, extImage);
        }

        if (!leftEye)
        {
            windows[index]->ConvertFromRightEyeToLeftEye();
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerWindowManager::CreateTiledImage
//
//  Purpose: 
//    This method returns a tiled image of all of the open ViewerWindows.
//
//  Arguments:
//    width     The desired width of the tiled image.
//    height    The desired height of the tiled image.
//
//  Returns:    A tiled image.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 13 15:27:26 PST 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 15 13:38:37 PST 2004
//    Implemented the method finally.
//
// ****************************************************************************

avtImage_p
ViewerWindowManager::CreateTiledImage(int width, int height, bool leftEye)
{
    //
    // Determine how many windows actually have plots to save in the
    // tiled image. Also sort the windows into the sortedWindows array.
    // We need to do this because if any windows have ever been deleted,
    // new windows can appear in the first unused slot in the windows array.
    // Thus the windows array does not contain windows in any given order.
    // Since we want to always tile the image so that windows are in order
    // (1, 2, 3, ...) we sort.
    //
    int windowIndex, windowWithPlot = -1, windowsWithPlots = 0;
    ViewerWindow **sortedWindows = new ViewerWindow*[maxWindows];
    for(windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
        sortedWindows[windowIndex] = 0;
    for(windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        ViewerWindow *win = windows[windowIndex];
        if(win != 0 && win->GetPlotList()->GetNumPlots() > 0)
        {
            sortedWindows[win->GetWindowId()] = win;
            if(windowWithPlot == -1)
                windowWithPlot = windowIndex;
            ++windowsWithPlots;
        }
    }

    // 
    // Return early if none of the windows have plots.
    //
    if(windowsWithPlots == 0)
    {
        delete [] sortedWindows;
        Warning("VisIt did not save a tiled image because none of the "
                "windows had any plots.");
        return NULL;
    }
    else if(windowsWithPlots == 1)
    {
        delete [] sortedWindows;
        // There's just 1 window that has plots don't bother tiling.
        return CreateSingleImage(windowWithPlot,
                width, height,
                saveWindowClientAtts->GetScreenCapture(),
                leftEye);
    }

    //
    // If we're not in screen capture mode then divide up the prescribed
    // image size among the tiles that we have.
    //
    avtImageTiler tiler(windowsWithPlots);

    int imageWidth = width;
    int imageHeight = height;
    if(!saveWindowClientAtts->GetScreenCapture())
    {
        imageWidth = width / tiler.GetNumberOfColumnsForNTiles(windowsWithPlots);
        imageHeight = imageWidth;
    }

    //
    // Get an image for each window that has plots and add the images to the
    // tiler object.
    //
    for(int index = 0; index < maxWindows; ++index)
    {
        if(sortedWindows[index] != 0)
        {
            tiler.AddImage(CreateSingleImage(sortedWindows[index]->GetWindowId(),
                imageWidth, imageHeight,
                saveWindowClientAtts->GetScreenCapture(), leftEye));
            Message("Saving tiled image...");
        }
    }
    delete [] sortedWindows;

    //
    // Return the tiled image returned by the tiler.
    //
    return tiler.CreateTiledImage();
}

// ****************************************************************************
// Method: ViewerWindowManager::GetDataset
//
// Purpose:
//   Goes to the vis window and gets a mesh that comprises all of the plots in
//   the window and makes an attempt at setting up their colors.
//
// Programmer: Hank Childs
// Creation:   May 24, 2002
//
// Modifications:
//
//   Mark C. Miller, Tue May  3 21:49:22 PDT 2005
//   Added windowIsInScalableRenderingMode
//
// ****************************************************************************

avtDataset_p
ViewerWindowManager::GetDataset(int windowIndex,
    bool& windowIsInScalableRenderingMode)
{
    int          index = (windowIndex == -1 ? activeWindow : windowIndex);
    avtDataset_p rv    = NULL;

    windowIsInScalableRenderingMode = windows[index]->GetScalableRendering();
    if(!windowIsInScalableRenderingMode && windows[index] != 0)
    {
        rv = windows[index]->GetAllDatasets();
    }

    return rv;
}


// ****************************************************************************
// Method: ViewerWindowManager::PrintWindow
//
// Purpose: 
//   Prints the specified window.
//
// Arguments:
//   windowIndex : The index of the window to be printed or -1 for the
//                 active window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 14:44:10 PST 2002
//
// Modifications:
//   Hank Childs, Thu May 23 18:47:33 PDT 2002  
//   Renamed SaveImage to SaveWindow.
//
//   Brad Whitlock, Thu Feb 27 08:59:46 PDT 2003
//   I added code to check the printer name and the output file name so that
//   there are no problems with printing if the user never set those values.
//
//   Brad Whitlock, Mon Apr 28 10:24:27 PDT 2003
//   I removed code to handle SIGPIPE since that signal is now ignored.
//
// ****************************************************************************

void
ViewerWindowManager::PrintWindow(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
    {
        debug1 << "Invalid window index " << windowIndex << " of " 
               << maxWindows << " was specified." << endl;
        return;
    }

    //
    // Make sure that we have a printer name if we're printing to a
    // printer and make sure that we have a filename if we're printing
    // to a file.
    //
    if(printerAtts->GetOutputToFile())
    {
        if(printerAtts->GetOutputToFileName().empty())
        {
            Error("You cannot print the window to a file because you "
                  "did not specify an output filename.");
            return;
        }
    }
    else
    {
        if(printerAtts->GetPrinterName().empty())
        {
            Error("You cannot print the window because you have not "
                  "specified a printer name.");
            return;
        }
    }

    //
    // Send a status message about starting to save the image and make the
    // status message display for 10 minutes.
    //
    char message[1000];
    int index = (windowIndex == -1) ? (activeWindow + 1) : (windowIndex + 1);
    SNPRINTF(message, 1000, "Printing window %d...", index);
    Status(message, 6000000);

    //
    // Create a vtkQtImagePrinter object and set its printer attributes.
    //
    vtkQtImagePrinter *imagePrinter = vtkQtImagePrinter::New();
    QPrinter &printer = imagePrinter->printer();
    printer.setPrinterName(printerAtts->GetPrinterName().c_str());
    if(!printerAtts->GetPrintProgram().empty())
        printer.setPrintProgram(printerAtts->GetPrintProgram().c_str());
    printer.setCreator(printerAtts->GetCreator().c_str());
    printer.setDocName(printerAtts->GetDocumentName().c_str());
    printer.setNumCopies(printerAtts->GetNumCopies());
    printer.setOrientation(printerAtts->GetPortrait() ? QPrinter::Portrait :
        QPrinter::Landscape);
    printer.setFromTo(1,1);
    printer.setColorMode(printerAtts->GetPrintColor() ? QPrinter::Color :
        QPrinter::GrayScale);
    printer.setOutputFileName(printerAtts->GetOutputToFileName().c_str());
    printer.setOutputToFile(printerAtts->GetOutputToFile());
    printer.setPageSize((QPrinter::PageSize)printerAtts->GetPageSize());

    //
    // Create an image that will fit on the printer, else scale the
    // image to fit on the printer surface.
    //
    avtImage_p image = CreateSingleImage(windowIndex,
        saveWindowClientAtts->GetWidth(), saveWindowClientAtts->GetHeight(),
        saveWindowClientAtts->GetScreenCapture(), true);

    //
    // Tell the imageWriter to use our writer to write the image. In this
    // case, the writer is an image printer.
    //
    avtDataObject_p dob;
    CopyTo(dob, image);
    fileWriter->WriteImageDirectly(imagePrinter,
                                  printerAtts->GetDocumentName().c_str(), dob);

    //
    // Delete the image printer.
    //
    imagePrinter->Delete();

    //
    // Indicate that the image has been printed.
    //
    if(printerAtts->GetOutputToFile())
    {
        SNPRINTF(message, 1000, "Window %d saved to %s.", index,
                 printerAtts->GetOutputToFileName().c_str());
        Status(message);
    }
    else
    {
        SNPRINTF(message, 1000, "Window %d sent to printer.", index);
        Status(message);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetInteractionMode
//
//  Purpose: 
//    This method sets the interaction mode for the specified window.
//
//  Arguments:
//    m            The interaction mode.
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 13:31:14 PST 2000
//
//  Modifications:
//   Brad Whitlock, Mon Sep 16 15:21:09 PST 2002
//   I made it update the WindowInformation.
//
// ****************************************************************************

void
ViewerWindowManager::SetInteractionMode(INTERACTION_MODE m,
    int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->SetInteractionMode(m);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetViewCurveFromClient
//
//  Purpose: 
//    Sets the view for the active window using the client view attributes.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2003
//
// ****************************************************************************

void
ViewerWindowManager::SetViewCurveFromClient()
{
    avtViewCurve viewCurve = windows[activeWindow]->GetViewCurve();

    const double *viewport=viewCurveClientAtts->GetViewportCoords();
    const double *domain=viewCurveClientAtts->GetDomainCoords();
    const double *range=viewCurveClientAtts->GetRangeCoords();

    for (int i = 0; i < 4; i++)
    {
        viewCurve.viewport[i] = viewport[i];
    }
    viewCurve.domain[0] = domain[0];
    viewCurve.domain[1] = domain[1];
    viewCurve.range[0]  = range[0];
    viewCurve.range[1]  = range[1];

    //
    // Set the 2D view for the active viewer window.
    //
    windows[activeWindow]->SetViewCurve(viewCurve);

    //
    // This will maintain our internal state and also make locked windows
    // get this view.
    //
    UpdateViewAtts(activeWindow, true, false, false);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetView2DFromClient
//
//  Purpose: 
//    Sets the view for the active window using the client view attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 20 10:41:58 PDT 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:05:46 PDT 2001
//    Modify the routine to use an avtView2D to set the 2D view.
//   
//    Hank Childs, Fri Oct 18 15:00:33 PDT 2002
//    Call UpdateViewAtts so that locked windows also update.
//
//    Brad Whitlock, Tue Nov 19 14:40:03 PST 2002
//    I changed UpdateViewAtts so the 3d view will not be sent to the client.
//
//    Kathleen Bonnell, Tue Jul 15 08:30:52 PDT 2003
//    Retrieve active window's 2d view, instead of instantiating a new one,
//    so that scale factor for full-frame mode is not lost during update. 
//
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I changed the call to UpdateViewAtts.
//
//    Eric Brugger, Thu Oct 16 14:32:08 PDT 2003
//    I added a full frame mode to the 2d view.
//
// ****************************************************************************

void
ViewerWindowManager::SetView2DFromClient()
{
    avtView2D view2d = windows[activeWindow]->GetView2D();

    const double *viewport=view2DClientAtts->GetViewportCoords();
    const double *window=view2DClientAtts->GetWindowCoords();

    for (int i = 0; i < 4; i++)
    {
        view2d.viewport[i] = viewport[i];
        view2d.window[i]   = window[i];
    }
    view2d.fullFrame = view2DClientAtts->GetFullFrame();

    //
    // Set the 2D view for the active viewer window.
    //
    windows[activeWindow]->SetView2D(view2d);

    //
    // This will maintain our internal state and also make locked windows
    // get this view.
    //
    UpdateViewAtts(activeWindow, false, true, false);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetView3DFromClient
//
//  Purpose: 
//    Sets the view for the active window using the client view attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 20 10:41:58 PDT 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:05:46 PDT 2001
//    Modify the routine to use an avtView3D to set the 2D view.
//   
//    Hank Childs, Fri Oct 18 15:00:33 PDT 2002
//    Call UpdateViewAtts so that locked windows also update.
//
//    Brad Whitlock, Tue Nov 19 14:40:03 PST 2002
//    I changed UpdateViewAtts so the 2d view will not be sent to the client.
//
//    Eric Brugger, Tue Jun 10 13:10:17 PDT 2003
//    I renamed camera to view normal in the view attributes.  I added
//    image pan and image zoom to the 3d view attributes.
//
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I changed the call to UpdateViewAtts.
//
//    Hank Childs, Wed Oct 15 12:58:19 PDT 2003
//    Copied over eye angle.
//
//    Eric Brugger, Wed Feb 11 08:52:25 PST 2004
//    Added code to copy center of rotation information.
//
// ****************************************************************************

void
ViewerWindowManager::SetView3DFromClient()
{
    avtView3D view3d;

    view3d.normal[0] = view3DClientAtts->GetViewNormal()[0];
    view3d.normal[1] = view3DClientAtts->GetViewNormal()[1];
    view3d.normal[2] = view3DClientAtts->GetViewNormal()[2];
    view3d.focus[0] = view3DClientAtts->GetFocus()[0];
    view3d.focus[1] = view3DClientAtts->GetFocus()[1];
    view3d.focus[2] = view3DClientAtts->GetFocus()[2];
    view3d.viewUp[0] = view3DClientAtts->GetViewUp()[0];
    view3d.viewUp[1] = view3DClientAtts->GetViewUp()[1];
    view3d.viewUp[2] = view3DClientAtts->GetViewUp()[2];
    view3d.viewAngle = view3DClientAtts->GetViewAngle();
    view3d.parallelScale = view3DClientAtts->GetParallelScale();
    view3d.eyeAngle = view3DClientAtts->GetEyeAngle();
    view3d.nearPlane = view3DClientAtts->GetNearPlane();
    view3d.farPlane = view3DClientAtts->GetFarPlane();
    view3d.imagePan[0] = view3DClientAtts->GetImagePan()[0];
    view3d.imagePan[1] = view3DClientAtts->GetImagePan()[1];
    view3d.imageZoom = view3DClientAtts->GetImageZoom();
    view3d.perspective = view3DClientAtts->GetPerspective();
    view3d.centerOfRotationSet = view3DClientAtts->GetCenterOfRotationSet();
    view3d.centerOfRotation[0] = view3DClientAtts->GetCenterOfRotation()[0];
    view3d.centerOfRotation[1] = view3DClientAtts->GetCenterOfRotation()[1];
    view3d.centerOfRotation[2] = view3DClientAtts->GetCenterOfRotation()[2];

    //
    // Set the 3D view for the active viewer window.
    //
    windows[activeWindow]->SetView3D(view3d);

    //
    // This will maintain our internal state and also make locked windows
    // get this view.
    //
    UpdateViewAtts(activeWindow, false, false, true);
}

// ****************************************************************************
//  Method: ViewerWindowManager::ClearViewKeyframes
//
//  Purpose: 
//    Clears the view keyframes for the active window.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Jan 23 15:38:17 PST 2004
//    Moved update code into a method.
//
// ****************************************************************************

void
ViewerWindowManager::ClearViewKeyframes()
{ 
    windows[activeWindow]->ClearViewKeyframes();
    UpdateViewKeyframeInformation();
}

// ****************************************************************************
//  Method: ViewerWindowManager::DeleteViewKeyframe
//
//  Purpose: 
//    Deletes a view keyframe from the active window.
//
//  Arguments:
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Jan 23 15:38:17 PST 2004
//    Moved update code into a method.
//
// ****************************************************************************

void
ViewerWindowManager::DeleteViewKeyframe(const int frame)
{ 
    windows[activeWindow]->DeleteViewKeyframe(frame);
    UpdateViewKeyframeInformation();
}

// ****************************************************************************
//  Method: ViewerWindowManager::MoveViewKeyframe
//
//  Purpose: 
//    Moves the position of a view keyframe.
//
//  Arguments:
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 29, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Jan 23 15:38:17 PST 2004
//    Moved update code into a method.
//
// ****************************************************************************

void
ViewerWindowManager::MoveViewKeyframe(int oldFrame, int newFrame)
{ 
    windows[activeWindow]->MoveViewKeyframe(oldFrame, newFrame);
    UpdateViewKeyframeInformation();
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetViewKeyframe
//
//  Purpose: 
//    Sets a view keyframe for the active window.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Jan 23 15:38:17 PST 2004
//    Moved update code into a method.
//
// ****************************************************************************

void
ViewerWindowManager::SetViewKeyframe()
{ 
    windows[activeWindow]->SetViewKeyframe();
    UpdateViewKeyframeInformation();
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetViewExtentsType
//
//  Purpose: 
//    This method specifies which flavor of view extents to use to determine
//    the view.
//
//  Arguments:
//    viewType     The flavor of spatial extents to use when setting the view.
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
// Programmer: Hank Childs
// Creation:   July 15, 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 16 15:21:09 PST 2002
//   I made it update the WindowInformation. I also made it recenter the
//   view if the window's autocentering flag is on.
//
//   Eric Brugger, Fri Apr 18 12:38:05 PDT 2003 
//   I replaced auto center mode with maintain view mode.
//
//   Brad Whitlock, Tue Feb 3 16:03:19 PST 2004
//   I made it use window information.
//
// ****************************************************************************

void
ViewerWindowManager::SetViewExtentsType(avtExtentType viewType, 
                                        int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->SetViewExtentsType(viewType);
        if(!windows[index]->GetMaintainViewMode())
            RecenterView(index);
        
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::SetRenderingAttributes
//
// Purpose: 
//   Tells the specified window to use the current rendering attributes.
//
// Arguments:
//   windowIndex : The index of the window to use. If the value is -1  then
//                 the active window is used.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 13:43:11 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  4 17:38:27 PST 2002
//   Removed antialisingFrames, no lnger needed.
//
//   Brad Whitlock, Tue Jul 8 11:10:10 PDT 2003
//   Added a flag that lets the window update when turning on scalable
//   rendering.
//
//   Brad Whitlock, Wed Aug 27 17:20:19 PST 2003
//   I changed the code so it redraws the window like it's supposed to after
//   setting rendering options like the surface representation.
//
//   Mark C. Miller, Mon Nov  3 15:29:57 PST 2003
//   I made it so only those parts of rendereing attributes that actually
//   were changed are changed on the window 
//
//   Jeremy Meredith, Fri Nov 14 12:23:19 PST 2003
//   Added specular properties.
//
//   Brad Whitlock, Tue Feb 3 16:03:47 PST 2004
//   I made it use window information.
//
//   Hank Childs, Mon May 10 08:10:40 PDT 2004
//   Replace references to immediate mode rendering with display list mode.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified calls to set scalable controls to accomdate scalable activaation
//   mode and scalable auto threshold
//
//   Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//   Added shading properties.
//
// ****************************************************************************

void
ViewerWindowManager::SetRenderingAttributes(int windowIndex)
{
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool updatesEnabled = windows[index]->UpdatesEnabled();
        windows[index]->DisableUpdates();

        if (windows[index]->GetAntialiasing() != renderAtts->GetAntialiasing())
            windows[index]->SetAntialiasing(renderAtts->GetAntialiasing());

        if (windows[index]->GetSurfaceRepresentation() !=
            (int) renderAtts->GetGeometryRepresentation())
            windows[index]->SetSurfaceRepresentation((int)
            renderAtts->GetGeometryRepresentation());

        if (windows[index]->GetDisplayListMode() != 
            renderAtts->GetDisplayListMode())
            windows[index]->SetDisplayListMode(renderAtts->GetDisplayListMode());

        if ((windows[index]->GetStereo() != renderAtts->GetStereoRendering()) ||
            (windows[index]->GetStereoType() != (int) renderAtts->GetStereoType()))
            windows[index]->SetStereoRendering(renderAtts->GetStereoRendering(),
                (int)renderAtts->GetStereoType());

        if (windows[index]->GetNotifyForEachRender() != 
            renderAtts->GetNotifyForEachRender())
            windows[index]->SetNotifyForEachRender(renderAtts->GetNotifyForEachRender());

        if (windows[index]->GetScalableAutoThreshold() !=
            renderAtts->GetScalableAutoThreshold())
            windows[index]->SetScalableAutoThreshold(renderAtts->GetScalableAutoThreshold());

        if (windows[index]->GetScalableActivationMode() !=
            renderAtts->GetScalableActivationMode())
            windows[index]->SetScalableActivationMode(renderAtts->GetScalableActivationMode());

        if (windows[index]->GetSpecularFlag()  != renderAtts->GetSpecularFlag() ||
            windows[index]->GetSpecularCoeff() != renderAtts->GetSpecularCoeff() ||
            windows[index]->GetSpecularPower() != renderAtts->GetSpecularPower() ||
            windows[index]->GetSpecularColor() != renderAtts->GetSpecularColor())
        {
            windows[index]->SetSpecularProperties(renderAtts->GetSpecularFlag(),
                                                 renderAtts->GetSpecularCoeff(),
                                                 renderAtts->GetSpecularPower(),
                                                 renderAtts->GetSpecularColor());
        }

        if (windows[index]->GetDoShading() != renderAtts->GetDoShadowing() ||
            windows[index]->GetShadingStrength() != 
                                              renderAtts->GetShadowStrength())
        {
            windows[index]->SetShadingProperties(renderAtts->GetDoShadowing(),
                                                 renderAtts->GetShadowStrength());
        }

        // If the updatesEnabled flag was true before we temporarily disabled
        // updates, turn updates back on and force the window to redraw so the
        // rendering options such as the surface representation and antialiasing
        // are noticed in the vis window right away.
        if (updatesEnabled)
        {
            windows[index]->EnableUpdates();
            windows[index]->RedrawWindow();
        }

        UpdateRenderingAtts(index);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToggleBoundingBoxMode
//
//  Purpose: 
//    This method toggles the bbox mode for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 7 13:31:14 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 16 15:21:09 PST 2002
//   I made it update the WindowInformation.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleBoundingBoxMode(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool bboxMode = windows[index]->GetBoundingBoxMode();
        windows[index]->SetBoundingBoxMode(!bboxMode);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToggleSpinMode
//
//  Purpose: 
//    This method toggles the spin mode for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
// Programmer: Hank Childs
// Creation:   May 29, 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 16 15:21:09 PST 2002
//   I made it update the WindowInformation.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleSpinMode(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool spinMode = windows[index]->GetSpinMode();
        windows[index]->SetSpinMode(!spinMode);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToggleCameraViewMode
//
//  Purpose: 
//    This method toggles the camera view mode for the specified window.
//
//  Programmer: Eric Brugger
//  Creation:   January 6, 2003
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Modifications:
//    Jeremy Meredith, Tue Feb  4 17:43:19 PST 2003
//    Added code to update the window information if the camera view mode
//    changes.  (I also added the mode info to the WindowInformation, which
//    is what made this necessary.)
//
//    Brad Whitlock, Tue Feb 3 16:04:12 PST 2004
//    I made it use window information.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleCameraViewMode(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool cameraViewMode = windows[index]->GetCameraViewMode();
        windows[index]->SetCameraViewMode(!cameraViewMode);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::AskForCorrelationPermission
//
// Purpose: 
//   Asks the user if a correlation should be created.
//
// Arguments:
//   msg   : The message to display.
//   title : The title of the dialog window.
//   dbs   : The databases that will be correlated.
//
// Returns:    True if the correlation should be created; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 09:53:14 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindowManager::AskForCorrelationPermission(const char *msg,
    const char *title, const stringVector &dbs) const
{
    bool permission;

    if(avtCallback::GetNowinMode())
        permission = true;
    else
    {
        // Pop up a Qt dialog to ask the user whether or not to correlate
        // the specified databases.
        QString text(msg); text += "\n";
        for(int i = 0; i < dbs.size(); ++i)
            text += (QString(dbs[i].c_str()) + QString("\n"));

        viewerSubject->BlockSocketSignals(true);
        permission = (QMessageBox::information(0, title,
            text, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ==
            QMessageBox::Yes);
        viewerSubject->BlockSocketSignals(false);
    }

    return permission;
}

// ****************************************************************************
// Method: ViewerWindowManager::CreateMultiWindowCorrelationHelper
//
// Purpose: 
//   Creates a new multiwindow database correlation or alters an existing
//   database correlation so it supports all of the specified databases.
//
// Arguments:
//   dbs : The list of databases for which we want a database correlation.
//
// Returns:    A pointer to the database correlation that we'll use or 0
//             if there is no correlation to use.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 09:54:27 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Mar 18 10:37:21 PDT 2005
//   I made the correlation be returned in the event that the user chose
//   to create a new multiwindow database correlation or alter a database
//   correlation.
//
// ****************************************************************************

DatabaseCorrelation *
ViewerWindowManager::CreateMultiWindowCorrelationHelper(const stringVector &dbs)
{
    //
    // Get the most suitable correlation for the list of dbs.
    //
    bool createNewCorrelation = true;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
    DatabaseCorrelation *correlation = fs->GetMostSuitableCorrelation(dbs);
    char msg[400];

    if(correlation)
    {
        //
        // If the number of databases in the correlation is less than the
        // number of databases in the dbs list then ask the user if the
        // correlation should be modfified.
        //
        if(correlation->GetNumDatabases() < dbs.size())
        {
            SNPRINTF(msg, 400, "Would you like to modify the %s correlation so "
                "it correlates the following databases?",
                 correlation->GetName().c_str());
            if(AskForCorrelationPermission(msg, "Alter correlation?", dbs))
            {
                createNewCorrelation = false;

                //
                // Alter the correlation
                //
                AlterDatabaseCorrelation(correlation->GetName(), dbs,
                    correlation->GetMethod());
            }
            else
                correlation = 0;
        }
        else
        {
            // We found a correlation that matched perfectly. Do nothing.
            createNewCorrelation = false;
        }
    }

    const char *prompt =
        "Would you like to correlate the following databases\n"
        "to ensure that changing the time will apply to all\n"
        "windows that are locked in time?\n";
    if(createNewCorrelation)
    {
        if(AskForCorrelationPermission(prompt, "Create correlation?", dbs))
        {
            //
            // Create a new database correlation since there was no suitable
            // database correlation.
            //
            std::string newName(fs->CreateNewCorrelationName());
            correlation = fs->CreateDatabaseCorrelation(newName, dbs,
            cL->GetDefaultCorrelationMethod());
            if(correlation)
            {
                // Add the new correlation to the correlation list.
                cL->AddDatabaseCorrelation(*correlation);
                cL->Notify();
                delete correlation; 
                correlation = cL->FindCorrelation(newName);

                debug3 << "Created a new correlation called: "
                       << newName.c_str() << endl << *correlation << endl;

                // Tell the user about the new correlation.
                SNPRINTF(msg, 400, "VisIt created a new database correlation "
                         "called %s.", correlation->GetName().c_str());
                Message(msg);
            }
        }
        else
        {
            //
            // The user opted to not create a multi-window correlation.
            // Issue a warning message.
            //
            Warning("Since you opted not to create a database correlation, "
                    "changing time sliders in one locked window might not "
                    "affect other locked windows.");
            correlation = 0;
        }
    }

    return correlation;
}

// ****************************************************************************
// Method: ViewerWindowManager::CreateMultiWindowCorrelation
//
// Purpose: 
//   Creates a database correlation that involves multiple windows.
//
// Arguments:
//   windowIds : The window ids of the windows that we want to involve in
//               creating the database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 11:20:16 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Mar 16 17:40:02 PST 2005
//   I made it return the database correlation to use for multiple windows.
//
// ****************************************************************************

DatabaseCorrelation *
ViewerWindowManager::CreateMultiWindowCorrelation(const intVector &windowIds)
{
    //
    // Get the list of databases for all of the time-locked windows.
    //
    stringVector dbs;
    GetDatabasesForWindows(windowIds, dbs);
    DatabaseCorrelation *correlation = 0;

    if(dbs.size() == 1)
    {
        // All of the locked windows used a single time slider so nothing
        // needs to be done. Alternatively, none of the locked windows
        // even had an active time slider so nothing needs to be done.
        correlation = ViewerFileServer::Instance()->
            GetDatabaseCorrelationList()->FindCorrelation(dbs[0]);
    }
    else if(dbs.size() > 1)
    {
        //
        // Get a correlation that has all of the databases in it. If such a
        // correlation does not exist then prompt the user to create one.
        // Either modify an existing correlation or create a brand new one.
        //
        correlation = CreateMultiWindowCorrelationHelper(dbs);

        //
        // If we had to create or edit a correlation, set the active time
        // slider for each time-locked window to be that correlation if the
        // time-locked window has an active time slider. We don't want to 
        // change windows that don't have an active time slider. Use the
        // old time slider in each window to set the state for the new 
        // time slider.
        //
        for(int i = 0; i < windowIds.size(); ++i)
        {
            ViewerPlotList *pl = windows[windowIds[i]]->GetPlotList();
            if(correlation != 0 && pl->HasActiveTimeSlider())
            {
                std::string ts(pl->GetActiveTimeSlider());

                //
                // If the window's current time slider is the same as 
                // the new correlation, then don't change the time slider.
                //
                if(ts != correlation->GetName())
                {
                    //
                    // We found a correlation for the time slider so it's
                    // a valid time slider. Get the time state for the
                    // time slider and feed it through the multiwindow
                    // correlation to get the new time slider's time
                    // state.
                    int state = 0, nStates = 0;
                    pl->GetTimeSliderStates(ts, state, nStates);
                    int cts = correlation->GetCorrelatedTimeState(ts, state);

                    //
                    // Create a new time slider using the name of the
                    // new multiwindow correlation.
                    //
                    if(!pl->TimeSliderExists(correlation->GetName()))
                    {
                        debug2 << "Creating " << correlation->GetName().c_str()
                               << " time slider in window " << i+1
                               << " and making it the active time slider.\n";
                        pl->CreateTimeSlider(correlation->GetName(), cts);
                    }
 
                    //
                    // Make the new time slider be the active time slider.
                    //
                    pl->SetActiveTimeSlider(correlation->GetName());
                }
            }
        }
    }

    return correlation;
}

// ****************************************************************************
// Method: ViewerWindowManager::ToggleLockTime
//
// Purpose: 
//   This method toggles the lock time mode for the specified window.
//
// Arguments:
//    windowIndex : This is a zero-origin integer that specifies the index
//                  of the window we want to change. If the value is -1, use
//                  use the active window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:59:05 PDT 2002
//
// Modifications:
//   Eric Brugger, Mon Dec  8 08:24:16 PST 2003
//   I corrected the test to make sure that the number of frames matched
//   between the windows being locked.  I added code to turn on view
//   limit merging.
//
//   Brad Whitlock, Tue Mar 16 09:06:32 PDT 2004
//   I completely rewrote the method so it supports database correlations
//   across windows when locking the windows in time.
//
//   Brad Whitlock, Wed Mar 16 17:34:38 PST 2005
//   Made it use GetTimeLockedWindowIndices.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleLockTime(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        int flags = WINDOWINFO_WINDOWFLAGS;
        bool lockTime = windows[index]->GetTimeLock();
        windows[index]->SetTimeLock(!lockTime);

        if (windows[index]->GetTimeLock())
        {
            //
            // Get the list of window id's that are time-locked windows.
            //
            intVector windowIds;
            GetTimeLockedWindowIndices(windowIds);

            //
            // Create a correlation for all of the time-locked windows.
            //
            CreateMultiWindowCorrelation(windowIds);

            //
            // We have just locked time for this window. Find another
            // window that has locked time and copy its time.
            //
            int winner = -1;
            for (int i = 0; i < windowIds.size(); ++i)
            {
                if (windowIds[i] != index)
                {
                    winner = windowIds[i];
                    break;
                }
            }

            //
            // If we found another window from which to copy time and
            // animation settings, copy the values now. It should be okay
            // to do this since we'll have created a correlation to use for
            // all of the time-locked windows.
            //
            if (winner != -1)
            {
                //
                // Get the time slider state from the time slider that we're
                // copying from. The call to GetMultiWindowCorrelation
                // should have changed the active time slider if it was
                // required. If the user did not allow the correlation to
                // be created and the time slider to be changed, they will
                // be different at this point to don't bother setting
                // the time for the window that we just locked.
                //
                ViewerPlotList *fromPL = windows[winner]->GetPlotList();

                if(fromPL->GetActiveTimeSlider() ==
                   windows[index]->GetPlotList()->GetActiveTimeSlider())
                {
                    int tsState, nStates;
                    fromPL->GetTimeSliderStates(fromPL->GetActiveTimeSlider(),
                        tsState, nStates);

                    // Set the time slider state in the window that we
                    // just locked.
                    windows[index]->GetPlotList()->SetTimeSliderState(tsState);
                }
                
                windows[index]->SetMergeViewLimits(true);
                // Copy the animation mode.
                ViewerPlotList::AnimationMode mode = 
                    fromPL->GetAnimationMode();
                windows[index]->GetPlotList()->SetAnimationMode(mode);
            }

            // We likely changed time sliders so update them.
            flags |= WINDOWINFO_TIMESLIDERS;
        }

        // Send information back to the client.
        UpdateWindowInformation(flags, index);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ToggleLockTools
//
// Purpose: 
//   This method toggles the lock tools mode for the specified window.
//
// Arguments:
//    windowIndex : This is a zero-origin integer that specifies the index
//                  of the window we want to change. If the value is -1, use
//                  use the active window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:59:05 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 3 16:05:39 PST 2004
//   Changed how UpdateWindowInformation is called.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleLockTools(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool lockTools = windows[index]->GetToolLock();
        windows[index]->SetToolLock(!lockTools);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToggleLockViewMode
//
//  Purpose: 
//    This method toggles whether or not the view is locked for the specified
//    window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
// Programmer: Hank Childs
// Creation:   Mon Mar 25 13:56:33 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 16 15:21:09 PST 2002
//   I made it update the WindowInformation and the view atts if the view
//   changes as a result of locking views.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleLockViewMode(int windowIndex)
{
    if (windowIndex < -1 || windowIndex >= maxWindows)
    {
        return;
    }

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if (windows[index] != 0)
    {
        bool vil = windows[index]->GetViewIsLocked();
        windows[index]->SetViewIsLocked(!vil);
        if (windows[index]->GetViewIsLocked())
        {
            //
            // We have just locked this view.  Find another window that has
            // a locked view and copy its view.
            //
            int winner = -1;
            for (int i = 0; i < maxWindows; i++)
            {
                if (windows[i] != NULL && i != index)
                {
                    if (windows[i]->GetViewIsLocked())
                    {
                        winner = i;
                        break;
                    }
                }
            }
            if (winner != -1)
            {
                const avtView3D &view3d = windows[winner]->GetView3D();
                const avtView2D &view2d = windows[winner]->GetView2D();
                windows[index]->SetView3D(view3d);
                windows[index]->SetView2D(view2d);

                // Update the view attributes.
                if(index == activeWindow)
                    UpdateViewAtts();
            }
        }

        // Update the view information.
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::TogglePerspective
//
//  Purpose: 
//    This method toggles the perspective mode for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Programmer: Hank Childs
//  Creation:   November 10, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Jul 27 09:45:47 PDT 2001
//    Added code to send the new view to the client.
//
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
//    Brad Whitlock, Tue Sep 17 11:41:33 PDT 2002
//    I made the window information update.
//
//    Brad Whitlock, Tue Nov 19 14:38:58 PST 2002
//    Changed it so only the 3d view updates.
//
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I changed the call to UpdateViewAtts.
//
// ****************************************************************************

void
ViewerWindowManager::TogglePerspective(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool perspective = windows[index]->GetPerspectiveProjection();
        windows[index]->SetPerspectiveProjection(!perspective);

        //
        // Send the new view info to the client.
        //
        UpdateViewAtts(index, false, false, true);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}


// ****************************************************************************
//  Method: ViewerWindowManager::ToggleFullFrameMode
//
//  Purpose: 
//    This method toggles the full frame mode for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 13, 2003 
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I changed the call to UpdateViewAtts.
//
//    Brad Whitlock, Tue Feb 3 16:06:34 PST 2004
//    I changed the call to UpdateWindowInformation.
//
// ****************************************************************************

void
ViewerWindowManager::ToggleFullFrameMode(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool fullframe = windows[index]->GetFullFrameMode();
        windows[index]->SetFullFrameMode(!fullframe);

        //
        // Send the new view info to the client.
        //
        UpdateViewAtts(index, false, true, false);
        UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToggleMaintainViewMode
//
//  Purpose:
//    This method toggles the maintain view mode for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 the active window.
//
//  Programmer: Eric Brugger
//  Creation:   April 18, 2003
//
// ****************************************************************************

void
ViewerWindowManager::ToggleMaintainViewMode(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool maintainView = windows[index]->GetMaintainViewMode();
        windows[index]->SetMaintainViewMode(!maintainView);
        UpdateGlobalAtts();
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToggleMaintainDataMode
//
//  Purpose:
//    This method toggles the maintain data mode for the specified window.
//
//  Arguments:
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 the active window.
//
//  Programmer: Eric Brugger
//  Creation:   March 29, 2004
//
// ****************************************************************************

void
ViewerWindowManager::ToggleMaintainDataMode(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool maintainData = windows[index]->GetMaintainDataMode();
        windows[index]->SetMaintainDataMode(!maintainData);
        UpdateGlobalAtts();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::UndoView
//
// Purpose:
//   This is a Qt slot function that is called when the popup menu's undo
//   view button is clicked.
//
// Arguments:
//   windowIndex : The index of the window that called this method.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 28 15:55:50 PST 2002
//
// Modifications:
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I added undoing of curve views.
//
// ****************************************************************************

void
ViewerWindowManager::UndoView(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        if(windows[index]->GetWindowMode() == WINMODE_CURVE)
        {
            // Pop the top off of the stack
            if(viewCurveStackTop > 0)
            {
                //
                // The top of the stack contains the current view.  We
                // want to go back to the previous view, so pop the stack
                // and use the view that is now at the top.
                //
                --viewCurveStackTop;
                windows[index]->SetViewCurve(viewCurveStack[viewCurveStackTop]);
            }
        }
        else if(windows[index]->GetWindowMode() == WINMODE_2D)
        {
            // Pop the top off of the stack
            if(view2DStackTop > 0)
            {
                //
                // The top of the stack contains the current view.  We
                // want to go back to the previous view, so pop the stack
                // and use the view that is now at the top.
                //
                --view2DStackTop;
                windows[index]->SetView2D(view2DStack[view2DStackTop]);
            }
        }
        else if(windows[index]->GetWindowMode() == WINMODE_3D)
        {
            // Pop the top off of the stack
            if(view3DStackTop > 0)
            {
                //
                // The top of the stack contains the current view.  We
                // want to go back to the previous view, so pop the stack
                // and use the view that is now at the top.
                //
                --view3DStackTop;
                windows[index]->SetView3D(view3DStack[view3DStackTop]);
            }
        }

        //
        // Send the view to the clients but do not stack it.
        //
        viewStacking = false;
        UpdateViewAtts(index);
        viewStacking = true;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateColorTable
//
//  Purpose: 
//    Loops through all windows and tells each one to update for the color
//    table ctName.
//
//  Arguments:
//    ctName    The color table that, if plots use it, they will be updated.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 14 16:24:18 PST 2001
//
// ****************************************************************************

void
ViewerWindowManager::UpdateColorTable(const char *ctName)
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
        {
            windows[i]->UpdateColorTable(ctName);
        }
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetWindowLayout
//
//  Purpose:
//    Set the window layout.
//
//  Arguments:
//    windowLayout  The window layout to use (1, 2, 4, 6, 8, or 9).
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Nov 1 16:21:45 PST 2000
//    I changed the code so it subtracts border widths when positioning
//    a window that already exists.
//
//    Jeremy Meredith, Fri Jul 20 11:21:45 PDT 2001
//    Added code to (de)iconify windows according the layout.
//
//    Hank Childs, Fri Oct 18 08:22:45 PDT 2002
//    Since CreateVisWindow got split into two routines, we had to call the
//    second one as well.
//
//    Eric Brugger, Fri Feb 13 14:14:41 PST 2004
//    Modified the routine to use both the width and height for the window
//    size, instead of using with width for both.
//
//    Brad Whitlock, Wed Apr 27 16:48:04 PST 2005
//    Changed the code so window layout 1 is handled specially and we get the
//    active window showing instead of window 1.
//
// ****************************************************************************

void
ViewerWindowManager::SetWindowLayout(const int windowLayout)
{
    //
    // Determine if it is a valid layout and use the index in the valid
    // layout array to set the layoutIndex.
    //
    int       iLayout;

    for (iLayout = 0; iLayout < maxLayouts; iLayout++)
    {
        if (validLayouts[iLayout] == windowLayout) break;
    }

    if (iLayout == maxLayouts)
    {
        char msg[200];
        SNPRINTF(msg, 200, "Window layout %d is an unsupported layout.",
                 windowLayout);
        Error(msg);
        return;
    }

    layout = windowLayout;
    layoutIndex = iLayout;

    if(layoutIndex == 0)
    {
        for (int iWindow = 0; iWindow < maxWindows; iWindow++)
        {  
            if(windows[iWindow] != 0 && windows[iWindow] != GetActiveWindow())
                windows[iWindow]->Iconify();
        }

        int x      = windowLimits[layoutIndex][0].x;
        int y      = windowLimits[layoutIndex][0].y;
        int width  = windowLimits[layoutIndex][0].width;
        int height = windowLimits[layoutIndex][0].height;
        GetActiveWindow()->DeIconify();
        GetActiveWindow()->SetSize(width, height);
        GetActiveWindow()->SetLocation(x, y);
    }
    else
    {
        //
        // Create at least "layout" windows layed out in the appropriate grid.
        // If there are already more windows than the layout calls for then
        // put the first "layout" window in the grid and layout the remaining
        // windows down a diagonal through the grid.
        //
        int nWindowsShort = layout - nWindows;
        int nWindowsProcessed = 0;

        for (int iWindow = 0; iWindow < maxWindows; iWindow++)
        {
            int       x, y;
            int       width, height;

            //
            // If the window exists, position it properly.
            //
            if (windows[iWindow] != 0)
            {
                if (nWindowsProcessed < layout)
                {
                    windows[iWindow]->DeIconify();
                    x      = windowLimits[layoutIndex][nWindowsProcessed].x;
                    y      = windowLimits[layoutIndex][nWindowsProcessed].y;
                    width  = windowLimits[layoutIndex][nWindowsProcessed].width;
                    height = windowLimits[layoutIndex][nWindowsProcessed].height;
                    windows[iWindow]->SetSize(width, height);
                    windows[iWindow]->SetLocation(x, y);
                }
                else
                {
                    windows[iWindow]->Iconify();
                }
                nWindowsProcessed++;
            }
            //
            // If the window doesn't exist and we still don't have enough,
            // then create one in the correct location.
            //
            else if (nWindowsShort > 0)
            {
                x      = windowLimits[layoutIndex][iWindow].x;
                y      = windowLimits[layoutIndex][iWindow].y;
                width  = windowLimits[layoutIndex][iWindow].width;
                height = windowLimits[layoutIndex][iWindow].height;

                CreateVisWindow(iWindow, width, height, x, y);
                SetWindowAttributes(iWindow, false);

                nWindowsProcessed++;
                nWindowsShort--;
            }
        }
    }

    //
    // Update the client global attributes.
    //
    UpdateGlobalAtts();
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetActiveWindow
//
//  Purpose:
//    Set the active window.
//
//  Arguments:
//    windowId  The 1 origin window identifier.
//
//  Programmer: Eric Brugger
//  Creation:   September 13, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Jul 27 09:49:45 PDT 2001
//    Added code to send the view for the active window to the client.
//
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
//    Brad Whitlock, Thu Aug 30 09:45:21 PDT 2001
//    Added code to update the client annotation attributes.
//
//    Brad Whitlock, Fri Sep 14 15:25:06 PST 2001
//    Added code to update the light list.
//
//    Eric Brugger, Fri Nov  2 12:39:55 PST 2001
//    I added code to copy the attributes from the currently active window
//    to the newly active window if it is being referenced for the first
//    time.
//
//    Eric Brugger, Wed Nov 21 12:12:45 PST 2001
//    I added animation attributes.
//
//    Kathleen Bonnell, Tue Nov 27 16:03:00 PST 2001
//    Added pick attributes. 
// 
//    Brad Whitlock, Mon Feb 4 14:58:05 PST 2002
//    Moved some code out into UpdateAllAtts.
//
//    Sean Ahern, Mon May 20 14:15:16 PDT 2002
//    Made windows raise when they are activated.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Removed call to CopyPickAttributes. 
//
//    Brad Whitlock, Wed Feb 12 11:05:38 PDT 2003
//    I added CopyAnimation.
//
//    Eric Brugger, Thu Mar 13 10:50:11 PST 2003
//    I implemented CloneWindowOnFirstRef mode, which clones the current
//    window to the newly activated window when it is first referenced.
//
//    Brad Whitlock, Tue Dec 30 14:41:38 PST 2003
//    Added code to make sure the new active window is not iconified.
//
//    Brad Whitlock, Tue Jan 27 17:37:31 PST 2004
//    I changed the copy codea little bit.
//
//    Brad Whitlock, Wed Mar 16 15:18:43 PST 2005
//    I made the active window's database be copied to the new window's
//    plot list if the new window has not been referenced and its database
//    has not been set. This prevents problems where the new window does not
//    have a database and the right time sliders.
//
// ****************************************************************************

void
ViewerWindowManager::SetActiveWindow(const int windowId)
{
    //
    // Check the window id.
    //
    int winIndex = windowId - 1;
    if (windowId <= 0 || windowId > maxWindows || windows[winIndex] == 0)
    {
        Error("The specified window doesn't exist.");
        return;
    }

    //
    // Copy the window attributes from the current window to the new
    // window if the new window has been referenced for the first time.
    //
    ViewerWindow *dest = windows[winIndex];
    ViewerWindow *src = windows[activeWindow];
    if (clientAtts->GetCloneWindowOnFirstRef())
    {
        if(referenced[winIndex])
        {
            // The window has been referenced before but it does not have
            // a database. In this case, since we are probably going back to
            // window 1 after doing stuff in window N, copy the plot list.
            if(dest->GetPlotList()->GetHostDatabaseName() == "")
                dest->GetPlotList()->CopyFrom(src->GetPlotList(), false);
        }
        else
        {
            //
            // Copy the global attributes, the annotation attributes, the
            // light source attributes, the view attributes and the animation
            // attributes.
            //
            dest->CopyGeneralAttributes(src);
            dest->CopyAnnotationAttributes(src);
            dest->CopyLightList(src);
            dest->CopyViewAttributes(src);
            dest->GetPlotList()->CopyFrom(src->GetPlotList(), true);
        }
    }
    else
    {
        // We're not cloning everything when going to the new window but
        // if the window has not been referenced before then we still want
        // to make sure that the new window has a database and the right
        // time sliders.
        if(!referenced[winIndex] &&
           dest->GetPlotList()->GetHostDatabaseName() == "")
        {
            debug2 << "Window " << windowId << ", a window that is being "
                "referenced for the first time, is having its database "
                "set to: " << src->GetPlotList()->GetHostDatabaseName().c_str()
                   << ".\n";
            dest->GetPlotList()->CopyFrom(src->GetPlotList(), false);
        }
    }
    
    referenced[winIndex] = true;

    //
    // Make the specified window active.
    //
    activeWindow = winIndex;

    // Deiconify the activated window.
    windows[activeWindow]->DeIconify();

    // Raise the activated window.
    windows[activeWindow]->Raise();

    //
    // Update all of the client window attributes.
    //
    UpdateAllAtts();
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetActiveWindow
//
//  Purpose:
//    Return the a pointer to the currently active window.
//
//  Returns:    A pointer to the currently active window.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 12:09:31 PDT 2000'
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 19 09:07:13 PDT 2002
//    Changed error message to report correct method name
//
// ****************************************************************************

ViewerWindow *
ViewerWindowManager::GetActiveWindow() const
{
    //
    // If there are no windows it is an error.
    //
    if (nWindows == 0)
    {
        Error("ViewerWindowManager::GetActiveWindow() There are no windows.\n");
        return 0;
    }

    return windows[activeWindow];
}

// **************************************************************************** 
//  Method: ViewerWindowManager::UpdateGlobalAtts
//
//  Purpose:
//    Update the client global attributes.
//
//  Programmer: Eric Brugger
//  Creation:   September 21, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Nov 7 09:56:23 PDT 2000
//    Changed to reflect that animations are now part of ViewerWindow.
//
//    Eric Brugger, Mon Oct 29 16:44:21 PST 2001
//    Added code to set the animation mode.
//
//    Brad Whitlock, Mon Sep 16 14:58:55 PST 2002
//    Added code to set the window layout.
//
//    Eric Brugger, Fri Nov 15 16:32:25 PST 2002
//    Added support for keyframing.
//
//    Brad Whitlock, Mon Dec 30 14:14:31 PST 2002
//    I added code to actually set the nStates member of the clientAtts.
//
//    Eric Brugger, Fri Jan 31 13:45:27 PST 2003
//    I removed an argument from the call to GetStateIndex.
//
//    Brad Whitlock, Mon Apr 14 17:31:12 PST 2003
//    I removed some code to update the plot list and the plot attributes
//    since that should NEVER happen inside this routine.
//
//    Eric Brugger, Fri Apr 18 12:38:05 PDT 2003 
//    I added maintain view mode.
//
//    Brad Whitlock, Fri Jan 23 15:39:53 PST 2004
//    I changed the list of attributes contained by GlobalAttributes and
//    made changes here to set the right things.
//
//    Eric Brugger, Mon Mar 29 15:21:11 PST 2004
//    I added maintain data mode.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateGlobalAtts() const
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (clientAtts == 0)
    {
        clientAtts  = new GlobalAttributes;
    }

    //
    // Update the list of sources.
    //
    clientAtts->SetSources(ViewerFileServer::Instance()->GetOpenDatabases());

    //
    // Update the window list in the client Global Attributes
    // attribute subject.
    //
    int       i;
    int       activeWindowIndex;
    intVector v;

    activeWindowIndex = 0;
    for (i = 0; i < maxWindows; i++)
    {
        if (windows[i] != 0)
        {
            v.push_back(i+1);
            if (i < activeWindow)
            {
                activeWindowIndex++;
            }
        }
    }

    clientAtts->SetWindows(v);
    clientAtts->SetActiveWindow(activeWindowIndex);
    clientAtts->SetWindowLayout(layout);

    //
    // Update the maintain view and data modes.
    //
    clientAtts->SetMaintainView(windows[activeWindow]->GetMaintainViewMode());
    clientAtts->SetMaintainData(windows[activeWindow]->GetMaintainDataMode());

    clientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateViewAtts
//
//  Purpose: 
//    Causes the view attributes to be sent to the viewer's client.
//
//  Arguments:
//     windowIndex   The index of the window to update view attributs for.  If
//                   this is -1, it will do it for the active window.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 27 08:53:01 PDT 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 12:05:46 PDT 2001
//    Modified the routine to use an avtView2D to set the 2D view.  Modified
//    the routine to use an avtView3D to set the 3D view.  Removed the
//    argument from the routine.
//
//    Brad Whitlock, Mon Jan 28 16:45:46 PST 2002
//    Added code to stack the views.
//
//    Hank Childs, Mon Mar 25 14:34:13 PST 2002
//    Add support for locking views and added argument windowIndex.
//
//    Brad Whitlock, Tue Sep 17 11:43:49 PDT 2002
//    I changed the coding so the view attributes are not sent back to
//    the client unless windows are locked or the window is the active window.
//
//    Hank Childs, Fri Oct 18 15:06:45 PDT 2002
//    Fixed a bug where higher-indexed windows did not always lock into place.
//
//    Brad Whitlock, Tue Nov 19 14:39:19 PST 2002
//    Added default arguments that tell whether or not the 2d, 3d views
//    should be sent to the client.
//
//    Eric Brugger, Tue Jun 10 13:10:17 PDT 2003
//    I renamed camera to view normal in the view attributes.  I added
//    image pan and image zoom to the 3d view attributes.
//
//    Brad Whitlock, Tue Jul 1 14:07:52 PST 2003
//    I used new convenience methods for setting the viewAtts with the avt
//    view objects.
//
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I split the view attributes into 2d and 3d parts.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateViewAtts(int windowIndex, bool updateCurve,
    bool update2d, bool update3d)
{
    int index = (windowIndex == -1 ? activeWindow : windowIndex);
    const avtViewCurve &viewCurve = windows[index]->GetViewCurve();
    const avtView2D &view2d = windows[index]->GetView2D();
    const avtView3D &view3d = windows[index]->GetView3D();

    if(index == activeWindow || windows[index]->GetViewIsLocked())
    {
        //
        // Set the curve attributes from the window's view.
        //
        if(updateCurve)
        {
            viewCurve.SetToViewCurveAttributes(viewCurveClientAtts);
            viewCurveClientAtts->Notify();
        }

        //
        // Set the 2D attributes from the window's view.
        //
        if(update2d)
        {
            view2d.SetToView2DAttributes(view2DClientAtts);
            view2DClientAtts->Notify();
        }

        //
        // Set the 3D attributes from the window's view.
        //
        if(update3d)
        {
            view3d.SetToView3DAttributes(view3DClientAtts);
            view3DClientAtts->Notify();
        }
    }
     
    //
    // Update the other windows if their views are locked.
    //
    if (windows[index]->GetViewIsLocked())
    {
        for (int i = 0; i < maxWindows; i++)
        {
            if (windows[i] != NULL && i != index)
            {
                if (windows[i]->GetViewIsLocked())
                {
                    windows[i]->SetViewCurve(viewCurve);
                    windows[i]->SetView2D(view2d);
                    windows[i]->SetView3D(view3d);
                }
            }
        }
    }

    //
    // Stack the views.
    //
    if(viewStacking)
    {
        if(windows[index]->GetWindowMode() == WINMODE_CURVE)
        {
            if(viewCurveStackTop == VIEWER_WINDOW_MANAGER_VSTACK - 1)
            {
                // Shift down
                for(int i = 0; i < VIEWER_WINDOW_MANAGER_VSTACK - 1; ++i)
                    viewCurveStack[i] = viewCurveStack[i+1];
            }
            else
                ++viewCurveStackTop;

            viewCurveStack[viewCurveStackTop] = viewCurve;
        }
        else if(windows[index]->GetWindowMode() == WINMODE_2D)
        {
            if(view2DStackTop == VIEWER_WINDOW_MANAGER_VSTACK - 1)
            {
                // Shift down
                for(int i = 0; i < VIEWER_WINDOW_MANAGER_VSTACK - 1; ++i)
                    view2DStack[i] = view2DStack[i+1];
            }
            else
                ++view2DStackTop;

            view2DStack[view2DStackTop] = view2d;
        }
        else if(windows[index]->GetWindowMode() == WINMODE_3D)
        {
            if(view3DStackTop == VIEWER_WINDOW_MANAGER_VSTACK - 1)
            {
                // Shift down
                for(int i = 0; i < VIEWER_WINDOW_MANAGER_VSTACK - 1; ++i)
                    view3DStack[i] = view3DStack[i+1];
            }
            else
                ++view3DStackTop;

            view3DStack[view3DStackTop] = view3d;
        }
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateAnimationAtts
//
//  Purpose: 
//    Sends the animation attributes for the active window to the client.
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
ViewerWindowManager::UpdateAnimationAtts()
{
    ViewerWindow *win = windows[activeWindow];
    const AnimationAttributes *winAtts = win->GetAnimationAttributes();

    //
    // Copy the window's animation attributes to the client annotation
    // attributes and notify the client.
    //
    *animationClientAtts = *winAtts;
    animationClientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateAnnotationAtts
//
//  Purpose: 
//    Sends the annotation attributes for the active window to the client.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 09:49:04 PDT 2001
//
//  Modifications:
//   Kathleen Bonnell, Wed Sep 26 11:49:18 PDT 2001
//   Added call to SetPlotColors.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateAnnotationAtts()
{
    ViewerWindow *win = windows[activeWindow];
    const AnnotationAttributes *winAtts = win->GetAnnotationAttributes();

    SetPlotColors(winAtts);
    //
    // Copy the window's annotation attributes to the client annotation
    // attributes and notify the client.
    //
    *annotationClientAtts = *winAtts;
    annotationClientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateLightListAtts
//
//  Purpose: 
//    Sends the light list for the active window to the client.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 15:24:11 PST 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    The viewer window now stores LightList attributes, not avtLightList, so
//    there is no need for the call to MakeAttributes.
//   
// ****************************************************************************

void
ViewerWindowManager::UpdateLightListAtts()
{
    //
    // Copy the window's annotation attributes to the client light list
    // and notify the client.
    //
    ViewerWindow *win = windows[activeWindow];
    const LightList *lightAtts = win->GetLightList();
    *lightListClientAtts = *lightAtts;
    lightListClientAtts->Notify();
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateRenderingAtts
//
// Purpose: 
//   Sends the rendering attributes to the client.
//
// Arguments:
//   windowIndex : The index of the window to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 13:49:10 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  4 17:38:27 PST 2002
//   Removed antialiasing frames, no longer needed.
//   
//   Jeremy Meredith, Fri Nov 14 17:44:22 PST 2003
//   Added updates for specular.
//
//   Hank Childs, Mon May 10 08:10:40 PDT 2004
//   Replace references to immediate mode rendering with display list mode.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified calls to set scalable controls to accomdate scalable activaation
//   mode and scalable auto threshold
//
//   Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//   Added updates for shading.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateRenderingAtts(int windowIndex)
{
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(index == activeWindow)
    {
        ViewerWindow *win = windows[index];

        //
        // Copy the window's rendering attributes to the client rendering
        // attributes and notify the client.
        //
        renderAtts->SetAntialiasing(win->GetAntialiasing());
        renderAtts->SetGeometryRepresentation(
            (RenderingAttributes::GeometryRepresentation)win->GetSurfaceRepresentation());
        renderAtts->SetDisplayListMode(
            (RenderingAttributes::TriStateMode) win->GetDisplayListMode());
        renderAtts->SetStereoRendering(win->GetStereo());
        renderAtts->SetStereoType((RenderingAttributes::StereoTypes)
            win->GetStereoType());
        renderAtts->SetNotifyForEachRender(win->GetNotifyForEachRender());
        renderAtts->SetScalableActivationMode(
            (RenderingAttributes::TriStateMode) win->GetScalableActivationMode());
        renderAtts->SetScalableAutoThreshold(win->GetScalableAutoThreshold());
        renderAtts->SetSpecularFlag(win->GetSpecularFlag());
        renderAtts->SetSpecularCoeff(win->GetSpecularCoeff());
        renderAtts->SetSpecularPower(win->GetSpecularPower());
        renderAtts->SetSpecularColor(win->GetSpecularColor());
        renderAtts->SetDoShadowing(win->GetDoShading());
        renderAtts->SetShadowStrength(win->GetShadingStrength());

        // Tell the client about the new rendering information.
        renderAtts->Notify();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::RenderInformationCallback
//
// Purpose: 
//   This callback sends rendering information back to the client.
//
// Arguments:
//   data : The index of the window that generated the render.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 12:07:30 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 30 12:15:25 PDT 2002
//   Fixed problem with 64 bit to 32 bit typecast.
//
//   Brad Whitlock, Fri Jan 23 15:56:26 PST 2004
//   Made it call UpdateWindowRenderingInformation.
//
// ****************************************************************************

void
ViewerWindowManager::RenderInformationCallback(void *data)
{
    int index = (int)((long)data);
    instance->UpdateWindowRenderingInformation(index);
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateAllAtts
//
// Purpose: 
//   Updates all of the attributes for the window. This means that all the
//   window attributes are sent to the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 4 10:25:01 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 16 15:18:58 PST 2002
//   I made the WindowInformation and the Rendering attributes update.
//
//   Brad Whitlock, Mon Nov 11 17:40:12 PST 2002
//   I added code to update the SIL restriction so the client gets the 
//   right one when we change the active window.
//
//   Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//   Removed UpdatePickAttributes. 
//
//   Brad Whitlock, Mon Apr 14 17:26:24 PST 2003
//   Factored some updates out of UpdateGlobalAtts.
//
//   Eric Brugger, Wed Oct  8 11:37:15 PDT 2003
//   I added code to turn off view stacking when calling UpdateViewAtts so
//   that a new view doesn't get pushed on the view stack.
//
//   Brad Whitlock, Fri Oct 24 17:10:07 PST 2003
//   Added code to update the expression list.
//
//   Brad Whitlock, Wed Oct 29 11:39:22 PDT 2003
//   Added code to update the annotation object list.
//
//   Brad Whitlock, Fri Jan 23 15:55:55 PST 2004
//   I split up UpdateWindowInformation into three methods.
//
//   Kathleen Bonnell, Wed Aug 18 09:28:51 PDT 2004 
//   Added call to update interactor atts. 
//
// ****************************************************************************

void
ViewerWindowManager::UpdateAllAtts()
{
    //
    // Update the client global attributes.
    //
    UpdateGlobalAtts();

    //
    // Update the plot list and the plot attribute windows, and the SIL
    // restriction.
    //
    if(windows[activeWindow] != NULL)
    {
        ViewerPlotList *plotList = windows[activeWindow]->GetPlotList();
        plotList->UpdatePlotList();
        plotList->UpdatePlotAtts();
        plotList->UpdateSILRestrictionAtts();
        plotList->UpdateExpressionList(true);
        UpdateKeyframeAttributes();
        UpdateAnnotationObjectList();
    }

    //
    // Send the new view info to the client.
    //
    viewStacking = false;
    UpdateViewAtts();
    viewStacking = true;

    //
    // Update the client animation attributes.
    //
    UpdateAnimationAtts();

    //
    // Update the client annotation attributes.
    //
    UpdateAnnotationAtts();

    //
    // Update the client interactor attributes.
    //
    UpdateInteractorAtts();

    //
    // Update the client's light list.
    //
    UpdateLightListAtts();

    //
    // Update the window information.
    //
    UpdateWindowInformation(-1);
    UpdateWindowRenderingInformation();
    UpdateViewKeyframeInformation();

    //
    // Update the rendering attributes.
    //
    UpdateRenderingAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateKeyframeAttributes
//
// Purpose: 
//   Sends the keyframing attributes back to the client.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 7 00:33:10 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::UpdateKeyframeAttributes()
{
    ViewerPlotList *plotList = windows[activeWindow]->GetPlotList();
    keyframeClientAtts->SetEnabled(plotList->GetKeyframeMode());
    keyframeClientAtts->SetNFrames(plotList->GetNKeyframes());
    keyframeClientAtts->SetNFramesWasUserSet(plotList->GetNKeyframesWasUserSet());
    keyframeClientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetClientAtts
//
//  Purpose:
//    Return a pointer to the client global attributes attribute subject.
//
//  Returns:    A pointer to the client global attributes attribute subject.
//
//  Programmer: Eric Brugger
//  Creation:   September 21, 2000
//
// ****************************************************************************

GlobalAttributes *
ViewerWindowManager::GetClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (clientAtts == 0)
    {
        clientAtts = new GlobalAttributes;
    }

    return clientAtts;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetSaveWindowClientAtts
//
// Purpose: 
//   Returns the attributes used to save a file.
//
// Returns:    The attributes used to save a file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 15:57:04 PST 2001
//
// Modifications:
//   Hank Childs, Thu May 23 18:44:04 PDT 2002
//   Renamed from SaveImage to SaveWindow.
//
// ****************************************************************************

SaveWindowAttributes *
ViewerWindowManager::GetSaveWindowClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (saveWindowClientAtts == 0)
    {
        saveWindowClientAtts = new SaveWindowAttributes;
    }

    return saveWindowClientAtts;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetViewCurveClientAtts
//
// Purpose: 
//   Returns a pointer to the curve view attributes.
//
// Returns:    A pointer to the curve view attributes.
//
// Programmer: Eric Brugger
// Creation:   August 20, 2003
//
// ****************************************************************************

ViewCurveAttributes *
ViewerWindowManager::GetViewCurveClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (viewCurveClientAtts == 0)
    {
        viewCurveClientAtts = new ViewCurveAttributes;
    }

    return viewCurveClientAtts;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetView2DClientAtts
//
// Purpose: 
//   Returns a pointer to the 2D view attributes.
//
// Returns:    A pointer to the 2D view attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 20 10:25:14 PDT 2001
//
// Modifications:
//   Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//   I split the view attributes into 2d and 3d parts.
//
// ****************************************************************************

View2DAttributes *
ViewerWindowManager::GetView2DClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (view2DClientAtts == 0)
    {
        view2DClientAtts = new View2DAttributes;
    }

    return view2DClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetView3DClientAtts
//
//  Purpose: 
//    Returns a pointer to the 3D view attributes.
//
//  Returns:    A pointer to the 3D view attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 20 10:25:14 PDT 2001
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I split the view attributes into 2d and 3d parts.
//
// ****************************************************************************

View3DAttributes *
ViewerWindowManager::GetView3DClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (view3DClientAtts == 0)
    {
        view3DClientAtts = new View3DAttributes;
    }

    return view3DClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetAnimationClientAtts
//
//  Purpose: 
//    Returns a pointer to the animation attributes.
//
//  Returns:    A pointer to the animation attributes.
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001
//
// ****************************************************************************

AnimationAttributes *
ViewerWindowManager::GetAnimationClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (animationClientAtts == 0)
    {
        animationClientAtts = new AnimationAttributes;
    }

    return animationClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetAnimationAttsFromClient
//
//  Purpose: 
//    Sets the animation attributes for the active window based on the
//    client's animation attributes.
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001
//
//  Modifications:
//    Brad Whitlock, Tue May 14 12:11:56 PDT 2002
//    Added code to update the animation timer.
//
//    Brad Whitlock, Mon Oct 6 17:04:24 PST 2003
//    Made it use SetAnimationAttributes.
//
// ****************************************************************************

void
ViewerWindowManager::SetAnimationAttsFromClient()
{
    // Set the animation attributes using the animationClientAtts.
    windows[activeWindow]->SetAnimationAttributes(animationClientAtts);

    UpdateAnimationTimer();
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetAnnotationClientAtts
//
//  Purpose: 
//    Returns a pointer to the annotation attributes.
//
//  Returns:    A pointer to the annotation attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 08:38:30 PDT 2001
//
// ****************************************************************************

AnnotationAttributes *
ViewerWindowManager::GetAnnotationClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (annotationClientAtts == 0)
    {
        annotationClientAtts = new AnnotationAttributes;
    }

    return annotationClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetAnnotationDefaultAtts
//
//  Purpose: 
//    Returns a pointer to the default annotation attributes.
//
//  Returns:    A pointer to the default annotation attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 08:38:30 PDT 2001
//
// ****************************************************************************

AnnotationAttributes *
ViewerWindowManager::GetAnnotationDefaultAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (annotationDefaultAtts == 0)
    {
        annotationDefaultAtts = new AnnotationAttributes;
    }

    return annotationDefaultAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetClientAnnotationAttsFromDefault
//
//  Purpose: 
//    This method copies the default annotation attributes into the client
//    annotation attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 09:14:46 PDT 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetClientAnnotationAttsFromDefault()
{
    if(annotationDefaultAtts != 0 && annotationClientAtts != 0)
    {
        *annotationClientAtts = *annotationDefaultAtts;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetDefaultAnnotationAttsFromClient
//
//  Purpose: 
//    This method copies the client's annotation attributes into the default
//    annotation attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 09:14:46 PDT 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetDefaultAnnotationAttsFromClient()
{
    if(annotationDefaultAtts != 0 && annotationClientAtts != 0)
    {
        *annotationDefaultAtts = *annotationClientAtts;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetAnnotationAttsFromClient
//
//  Purpose: 
//    Sets the annotation attributes for the active window based on the
//    client's annotation attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 09:39:34 PDT 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 26 11:49:18 PDT 2001
//    Added call to SetPlotColors.
//   
// ****************************************************************************

void
ViewerWindowManager::SetAnnotationAttsFromClient()
{
    windows[activeWindow]->SetAnnotationAttributes(annotationClientAtts);
    SetPlotColors(annotationClientAtts);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetAnnotationAttsFromDefault
//
//  Purpose: 
//    Sets the annotation attributes for the active window based on the
//    default annotation attributes.
//
//  Arguments:
//    windowIndex  The index of the window to whose annotation attributes
//                 are to be reset.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 09:39:34 PDT 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetAnnotationAttsFromDefault()
{
    windows[activeWindow]->SetAnnotationAttributes(annotationDefaultAtts);
    //
    // Update the client's annotation attributes
    //
    UpdateAnnotationAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::AddAnnotationObject
//
// Purpose: 
//   Tells the viewer window to add a new annotation object.
//
// Arguments:
//   annotType : The type of annotation object to add.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:33:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::AddAnnotationObject(int annotType)
{
    if(windows[activeWindow] != 0)
    {
        windows[activeWindow]->AddAnnotationObject(annotType);
        UpdateAnnotationObjectList();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::HideActiveAnnotationObjects
//
// Purpose: 
//   Hides the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:34:33 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::HideActiveAnnotationObjects()
{
    if(windows[activeWindow] != 0)
    {
        windows[activeWindow]->HideActiveAnnotationObjects();
        UpdateAnnotationObjectList();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Deletes the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:34:33 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::DeleteActiveAnnotationObjects()
{
    if(windows[activeWindow] != 0)
    {
        windows[activeWindow]->DeleteActiveAnnotationObjects();
        UpdateAnnotationObjectList();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Raises the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:34:33 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::RaiseActiveAnnotationObjects()
{
    if(windows[activeWindow] != 0)
    {
        windows[activeWindow]->RaiseActiveAnnotationObjects();
        UpdateAnnotationObjectList();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::LowerActiveAnnotationObjects
//
// Purpose: 
//   Lowers the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:34:33 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::LowerActiveAnnotationObjects()
{
    if(windows[activeWindow] != 0)
    {
        windows[activeWindow]->LowerActiveAnnotationObjects();
        UpdateAnnotationObjectList();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::SetAnnotationObjectOptions
//
// Purpose: 
//   Tells the viewer window to set the annotation object options.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:34:33 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::SetAnnotationObjectOptions()
{
    if(windows[activeWindow] != 0)
        windows[activeWindow]->SetAnnotationObjectOptions(*annotationObjectList);
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateAnnotationList
//
// Purpose: 
//   Sends the annotation object list for the active window back to the client.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:26:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::UpdateAnnotationObjectList()
{
    if(windows[activeWindow] != 0)
    {
        windows[activeWindow]->UpdateAnnotationObjectList(*annotationObjectList);
        annotationObjectList->Notify();
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetKeyframeClientAtts
//
//  Purpose:
//    Returns a pointer to the keyframe attributes.
//
//  Returns:    A pointer to the keyframe attributes.
//
//  Programmer: Eric Brugger
//  Creation:   November 25, 2002
//
// ****************************************************************************
 
KeyframeAttributes *
ViewerWindowManager::GetKeyframeClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (keyframeClientAtts == 0)
    {
        keyframeClientAtts = new KeyframeAttributes;
    }
 
    return keyframeClientAtts;
}
 
// ****************************************************************************
//  Method: ViewerWindowManager::SetKeyframeAttsFromClient
//
//  Purpose:
//    Sets the keyframe attributes for the active window based on the
//    client's keyframe attributes.
//
//  Programmer: Eric Brugger
//  Creation:   November 25, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Jan 26 22:44:52 PST 2004
//    Made it use the plot list and I made it set the number of keyframes
//    into the plot list.
//
// ****************************************************************************
 
void
ViewerWindowManager::SetKeyframeAttsFromClient()
{
    windows[activeWindow]->GetPlotList()->SetKeyframeMode(
        keyframeClientAtts->GetEnabled());
    windows[activeWindow]->GetPlotList()->SetNKeyframes(
        keyframeClientAtts->GetNFrames());
}

// ****************************************************************************
// Method: ViewerWindowManager::SetFrameIndex
//
// Purpose: 
//   Sets the frame index for the active window and all windows that have
//   their locktime flag set to true.
//
// Arguments:
//   frame       : The new active frame.
//   windowIndex : The index of the window for which we'll set the frame index.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:08:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 5 10:24:53 PDT 2003
//   Added windowIndex so it does not need to be called only on the active
//   window.
//
//   Eric Brugger, Mon Dec  8 08:24:16 PST 2003
//   Added code to turn on view limit merging.
//
//   Brad Whitlock, Mon Jan 26 16:50:40 PST 2004
//   Changed how animation is done.
//
// ****************************************************************************

void
ViewerWindowManager::SetFrameIndex(int state, int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Set the frame of the active window first.
        windows[index]->SetMergeViewLimits(true);
        ViewerPlotList *activePL = windows[index]->GetPlotList();
        activePL->SetTimeSliderState(state);

        //
        // If the active window is time-locked, update the other windows
        // that are also time locked and have the same time slider or a
        // time slider that is used by the correlation for the active
        // window's time slider.
        //
        if(windows[index]->GetTimeLock() && activePL->HasActiveTimeSlider())
        {
            intVector badWindowIds;
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    ViewerPlotList *winPL = windows[i]->GetPlotList();
                    if(windows[i]->GetTimeLock() &&
                       winPL->HasActiveTimeSlider())
                    {
                        int tsState = -1;
                        if(activePL->GetActiveTimeSlider() ==
                           winPL->GetActiveTimeSlider())
                        {
                            //
                            // The windows have the same active time slider
                            // so we can just set the state.
                            //
                            tsState = state;
                        }
                        else
                        {
                            //
                            // The windows have different active time sliders
                            // so let's see if we can set the time for the i'th
                            // time slider. If not, warn the user.
                            //
                            DatabaseCorrelationList *cL = 
                                ViewerFileServer::Instance()->
                                GetDatabaseCorrelationList();
                            DatabaseCorrelation *c = cL->FindCorrelation(
                                activePL->GetActiveTimeSlider());
                            const std::string &ts = winPL->GetActiveTimeSlider();
                            if(c != 0)
                                tsState = c->GetCorrelatedTimeState(ts, state);
                        }

                        if(tsState != -1)
                        {
                            windows[i]->SetMergeViewLimits(true);
                            windows[i]->GetPlotList()->SetTimeSliderState(tsState);
                        }
                        else
                            badWindowIds.push_back(i);
                    }
                }
            }

            if(badWindowIds.size() > 0)
            {
                std::string msg("VisIt did not set the time state for window");
                if(badWindowIds.size() > 1)
                    msg += "s (";
                else
                    msg += " ";
                char tmp[50];
                for(int j = 0; j < badWindowIds.size(); ++j)
                {
                    SNPRINTF(tmp, 50, "%d", badWindowIds[j] + 1);
                    msg += tmp;
                    if(j < badWindowIds.size() - 1)
                        msg += ", ";
                }

                if(badWindowIds.size() > 1)
                    msg += ") because the time sliders in those windows ";
                else
                    msg += " because the time slider in that window ";

                msg += "cannot be set by the active window's time slider "
                       "since the correlations of the time sliders have "
                       "nothing in common.\n\nTo avoid this warning in the "
                       "future, make sure that locked windows have compatible "
                       "time sliders.";
                Warning(msg.c_str());
            }
        }

        //
        // Send the new time slider state to the client.
        //
        UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::NextFrame
//
// Purpose: 
//   Advances one frame for the active window and all windows that have
//   their locktime flag set to true.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:08:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 5 10:28:13 PDT 2003
//   Made it so it does not have to apply to the active window.
//
//   Brad Whitlock, Mon Jan 26 16:49:06 PST 2004
//   I changed how animation works.
//
// ****************************************************************************

void
ViewerWindowManager::NextFrame(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Advance one frame for the active window first.
        windows[index]->GetPlotList()->SetAnimationMode(
            ViewerPlotList::StopMode);
        windows[index]->GetPlotList()->ForwardStep();

        // If the active window is time-locked, update the other windows that are
        // also time locked.
        if(windows[index]->GetTimeLock())
        {
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    if(windows[i]->GetTimeLock())
                    {
                        windows[i]->GetPlotList()->SetAnimationMode(
                            ViewerPlotList::StopMode);
                        windows[i]->GetPlotList()->ForwardStep();
                    }
                }
            }
        }

        //
        // Modify the animation timer since we set the animation mode
        // for at least one window.
        //
        UpdateAnimationTimer();
        UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::PrevFrame
//
// Purpose: 
//   Advances one frame for the active window and all windows that have
//   their locktime flag set to true.
//
// Arguments:
//   windowIndex : The index of the window that originated the call.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:08:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 5 10:31:01 PDT 2003
//   I made it so the routine does not have to apply to the active window.
//
//   Brad Whitlock, Mon Jan 26 16:46:08 PST 2004
//   I changed how animation is done.
//
// ****************************************************************************

void
ViewerWindowManager::PrevFrame(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Back up one frame for the active window first.
        windows[index]->GetPlotList()->SetAnimationMode(
            ViewerPlotList::StopMode);
        windows[index]->GetPlotList()->BackwardStep();

        // If the active window is time-locked, update the other windows
        // that are also time locked.
        if(windows[index]->GetTimeLock())
        {
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    if(windows[i]->GetTimeLock())
                    {
                        windows[i]->GetPlotList()->SetAnimationMode(
                            ViewerPlotList::StopMode);
                        windows[i]->GetPlotList()->BackwardStep();
                    }
                }
            }
        }

        //
        // Modify the animation timer since we set the animation mode
        // for at least one window.
        //
        UpdateAnimationTimer();
        UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::Stop
//
// Purpose: 
//   Stops animation for the active window and all windows that have
//   their locktime flag set to true.
//
// Arguments:
//   windowIndex : The index of the window that originated the call.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:08:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 5 10:31:01 PDT 2003
//   I made it so the routine does not have to apply to the active window.
//
//   Brad Whitlock, Mon Jan 26 09:58:20 PDT 2004
//   I changed how animation is done.
//
// ****************************************************************************

void
ViewerWindowManager::Stop(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Stop animation for the active window first.
        windows[index]->GetPlotList()->SetAnimationMode(
            ViewerPlotList::StopMode);

        // If the active window is time-locked, update the other windows that are
        // also time locked.
        if(windows[index]->GetTimeLock())
        {
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    if(windows[i]->GetTimeLock())
                    {
                        windows[i]->GetPlotList()->SetAnimationMode(
                            ViewerPlotList::StopMode);
                    }
                }
            }
        }

        //
        // Modify the animation timer since we set the animation mode
        // for at least one window.
        //
        UpdateAnimationTimer();
        UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::Play
//
// Purpose: 
//   Sets animation to forward play for the active window and all windows
//   that have their locktime flag set to true.
//
// Arguments:
//   windowIndex : The index of the window that originated the call.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:08:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 5 10:31:01 PDT 2003
//   I made it so the routine does not have to apply to the active window.
//
//   Brad Whitlock, Mon Jan 26 22:46:27 PST 2004
//   I changed how animation works.
//
// ****************************************************************************

void
ViewerWindowManager::Play(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Start forward animation for the active window first.
        windows[index]->GetPlotList()->SetAnimationMode(ViewerPlotList::PlayMode);

        // If the active window is time-locked, update the other windows that are
        // also time locked.
        if(windows[index]->GetTimeLock())
        {
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    if(windows[i]->GetTimeLock())
                    {
                        windows[i]->GetPlotList()->SetAnimationMode(
                            ViewerPlotList::PlayMode);
                    }
                }
            }
        }

        //
        // Modify the animation timer since we set the animation mode
        // for at least one window.
        //
        UpdateAnimationTimer();
        UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ReversePlay
//
// Purpose: 
//   Sets animation to reverse play for the active window and all windows
//   that have their locktime flag set to true.
//
// Arguments:
//   windowIndex : The index of the window that originated the call.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 12:08:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 5 10:31:01 PDT 2003
//   I made it so the routine does not have to apply to the active window.
//
//   Brad Whitlock, Mon Jan 26 22:48:30 PST 2004
//   I changed how animation works.
//
// ****************************************************************************

void
ViewerWindowManager::ReversePlay(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Start forward animation for the active window first.
        windows[index]->GetPlotList()->SetAnimationMode(
            ViewerPlotList::ReversePlayMode);

        // If the active window is time-locked, update the other windows
        // that are also time locked.
        if(windows[index]->GetTimeLock())
        {
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    if(windows[i]->GetTimeLock())
                    {
                        windows[i]->GetPlotList()->SetAnimationMode(
                            ViewerPlotList::ReversePlayMode);
                    }
                }
            }
        }

        //
        // Modify the animation timer since we set the animation mode
        // for at least one window.
        //
        UpdateAnimationTimer();
        UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}


// ****************************************************************************
// Method: ViewerWindowManager::SetActiveTimeSlider
//
// Purpose: 
//   Sets the active time slider for the specified window.
//
// Arguments:
//   ts : The time slider that we want to make active.
//   windowIndex : The index of the window to change.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jan 25 02:29:13 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 7 00:48:43 PDT 2004
//   I changed the code so it allows the time slider to be set to the 
//   keyframe animation time slider. There was some difficulty because it
//   is a special time slider in that it does not have a database correlation.
//
// ****************************************************************************

void
ViewerWindowManager::SetActiveTimeSlider(const std::string &ts, int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        //
        // Make sure that we can find a correlation for the desired time
        // slider or it is not a valid time slider and we should return.
        //
        DatabaseCorrelationList *cL = ViewerFileServer::Instance()->
            GetDatabaseCorrelationList();
        DatabaseCorrelation *correlation = cL->FindCorrelation(ts);
        bool kfTimeSlider = (ts == KF_TIME_SLIDER);
        if(!kfTimeSlider && correlation == 0)
        {
            Error("VisIt could not find a database correlation "
                  "for the desired time slider so it must not be a valid time "
                  "slider.");
            return;
        }

        //
        // Set the active time slider for the active window first.
        //
        windows[index]->GetPlotList()->SetActiveTimeSlider(ts);

        //
        // If the active window is time-locked, update the other windows
        // that are also time locked.
        //
        if(windows[index]->GetTimeLock())
        {
            //
            // We're making the keyframing time slider be active return early
            // since I don't think we want to make other windows use the 
            // keyframe time slider since other windows are likely to have
            // very different keyframes.
            //
            if(windows[index]->GetPlotList()->GetKeyframeMode() && kfTimeSlider)
            {
                Warning("You've made the keyframe animation time slider be the "
                        "active time slider. Other windows that are also time "
                        "locked will not have their time sliders set to the "
                        "keyframe time slider.");
                return;
            }

            intVector badWindowIds;
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0 && windows[i]->GetTimeLock())
                {
                    ViewerPlotList *pl = windows[i]->GetPlotList();
                    if(pl->HasActiveTimeSlider())
                    {
                        // Get the databases used in window i.
                        intVector winId; winId.push_back(i);
                        stringVector dbs;
                        GetDatabasesForWindows(winId, dbs);

                        //
                        // The window has some MT databases so let's make
                        // sure that they can be handled by the new time
                        // slider. If not, tell the user about it.
                        //
                        if(dbs.size() > 0)
                        {
                            bool usedAll = true;
                            for(int j = 0; j < dbs.size() && usedAll; ++j)
                                usedAll &= correlation->UsesDatabase(dbs[j]);
                            if(!usedAll)
                                badWindowIds.push_back(i);
                            else
                            {
                                //
                                // Set the active time slider.
                                //
                                windows[i]->GetPlotList()->SetActiveTimeSlider(ts);
                            }
                        }
                    }
                }
            }

            //
            // If we found some windows that could not use the new time slider
            // because the correlation did not support some of their databases,
            // tell the user about it.
            //
            if(badWindowIds.size() > 0)
            {
                std::string msg("VisIt could not set the active time slider "
                    "for window");
                if(badWindowIds.size() > 1)
                    msg += "s (";
                else
                    msg += " ";
                char tmp[50];
                for(int j = 0; j < badWindowIds.size(); ++j)
                {
                    SNPRINTF(tmp, 50, "%d", badWindowIds[j] + 1);
                    msg += tmp;
                    if(j < badWindowIds.size() - 1)
                        msg += ", ";
                }

                if(badWindowIds.size() > 1)
                    msg += ") because those windows contain ";
                else
                    msg += " because that window contains ";

                msg += "at least one database that is not used by the new "
                       "time slider.\n\nYou may want to create a new database "
                       "correlation using all of the databases for your "
                       "locked windows and use that database correlation "
                       "for your active time slider or you may find that "
                       "not all windows update when you change the time "
                       "slider's active time state.";
                Warning(msg.c_str());
            }
        }

        //
        // Modify the animation timer since we set the time slider
        // for at least one window.
        //
        UpdateAnimationTimer();
        UpdateWindowInformation(WINDOWINFO_TIMESLIDERS | WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::AlterTimeSlider
//
// Purpose: 
//   Forces the named time slider to be within the bounds of its database
//   correlation.
//
// Arguments:
//   ts : The name of the time slider that we're updating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 3 14:30:15 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::AlterTimeSlider(const std::string &ts)
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
        {
            // Alter the time slider. This means that we're forcing it to be
            // within the bounds of its database correlation. We prevent the
            // window from updating immediately.
            if(windows[i]->GetPlotList()->AlterTimeSlider(ts, false))
            {
                // Send a message to update the window later.
                windows[i]->SendUpdateMessage();
            }
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::CreateDatabaseCorrelation
//
// Purpose: 
//   Creates a new database correlation.
//
// Arguments:
//   name         : The name of the new database correlation.
//   dbs          : The databases to include in the new database correlation.
//   method       : The correlation method.
//   initialState : The initial state for the time slider that will get
//                  created to 
//   nStates      : The number of states in the correlation (currently ignored).
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 11:21:29 PDT 2004
//
// Modifications:
//   Eric Brugger, Fri Apr 16 18:43:42 PDT 2004
//   I modified the logic concerning the creation of a new time slider.
//
//   Brad Whitlock, Mon Apr 19 08:46:00 PDT 2004
//   I removed the code to update the window information. I moved it up
//   a level so the caller of this method is now responsible for updating
//   the window information.
//
// ****************************************************************************

void
ViewerWindowManager::CreateDatabaseCorrelation(const std::string &name,
    const stringVector &dbs, int method, int initialState, int nStates)
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();

    // Make sure that the correlation is not already in the list.
    if(cL->FindCorrelation(name) == 0)
    {
        //
        // Try and create a new correlation.
        //
        DatabaseCorrelation *correlation = fs->CreateDatabaseCorrelation(
            name, dbs, method, nStates);

        //
        // If there was no error in creating the correlation then
        // add the new one to the list and notify the client.
        //
        if(correlation)
        {
            cL->AddDatabaseCorrelation(*correlation);
            cL->Notify();

            //
            // Print the correlation to the log
            //
            debug3 << "New correlation:" << endl
                   << *correlation << endl;

            //
            // Now that the correlation has been created, create a time
            // slider for it in all windows and make it the active time
            // slider in the active window and all windows that are
            // locked to it.
            //
            for(int i = 0; i < maxWindows; ++i)
            {
                ViewerWindow *win = windows[i];
                if(win)
                {
                    bool timeLocked = (windows[activeWindow]->GetTimeLock() &&
                                       windows[i]->GetTimeLock());

                    bool timeSliderExists =
                        win->GetPlotList()->TimeSliderExists(name);

                    //
                    // If we didn't have a time slider, create one. Call
                    // CreateTimeSlider in any case if we were locked in time
                    // since reopening one time locked window should update
                    // others, especially if we're changing the active time
                    // slider later.
                    if(!timeSliderExists || timeLocked)
                    {
                        win->GetPlotList()->CreateTimeSlider(name,
                            initialState);
                    }

                    // Make the new correlation be the active time slider
                    // in the active window or any window locked to it.
                    if(i == activeWindow || timeLocked)
                    {
                        win->GetPlotList()->SetActiveTimeSlider(name);
                    }
                }
            }

            delete correlation;
        }
    }
    else if(!fs->IsDatabase(name))
    {
        // The database correlation is already in the list of correlations
        // so try and alter the existing correlation.
        AlterDatabaseCorrelation(name, dbs, method, nStates);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::AlterDatabaseCorrelation
//
// Purpose: 
//   Alters the named database correlation.
//
// Arguments:
//   name         : The name of the new database correlation.
//   dbs          : The databases to include in the new database correlation.
//   method       : The correlation method.
//   initialState : The initial state for the time slider that will get
//                  created to 
//   nStates      : The number of states in the correlation (currently ignored).
//
// Returns:    
//
// Note:       Trivial correlations can't be modified.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 11:23:35 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Apr 19 08:50:47 PDT 2004
//   I removed the code to update the window information since it is already
//   done by the caller and the update here caused 2 updates.
//
// ****************************************************************************

void
ViewerWindowManager::AlterDatabaseCorrelation(const std::string &name,
    const stringVector &dbs, int method, int nStates)
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
    DatabaseCorrelation *correlation = cL->FindCorrelation(name);
    if(correlation)
    {
        // Make sure that the correlation does not have the same name as
        // an existing source.
        if(fs->IsDatabase(name))
        {
            Error("You cannot alter a database correlation that "
                  "corresponds directly to a database.");
        }
        else
        {
            //
            // Try and create a new correlation.
            //
            DatabaseCorrelation *newCorrelation = 
                fs->CreateDatabaseCorrelation(name, dbs, method, nStates);

            //
            // If there was no error in creating the correlation then
            // add the new one to the list and notify the client.
            //
            if(newCorrelation)
            {
                *correlation = *newCorrelation;
                cL->Notify();

                // Make sure that all time sliders for all windows with the
                // same name as the correlation have a state that still
                // fits into the number of states for the altered correlation.
                for(int j = 0; j < maxWindows; ++j)
                {
                    if(windows[j] != 0)
                    {
                        windows[j]->GetPlotList()->
                            AlterTimeSlider(name);
                    }
                }

                delete newCorrelation;
            }
        }
    }
    else
    {
        char msg[300];
        SNPRINTF(msg, 300, "You cannot alter a database correlation for %s "
                 "because there is no such database correlation.", 
                 name.c_str());
        Error(msg);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::DeleteDatabaseCorrelation
//
// Purpose: 
//   Deletes the named database correlation.
//
// Arguments:
//   name : The name of the correlation to delete.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 29 11:24:10 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::DeleteDatabaseCorrelation(const std::string &name)
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
    DatabaseCorrelation *correlation = cL->FindCorrelation(name);
    if(correlation)
    {
        // Make sure that the correlation does not have the same name as
        // an existing source.
        if(fs->IsDatabase(name))
        {
            Error("You cannot delete a database correlation that "
                  "corresponds directly to a source.");
        }
        else
        {
            //
            // Remove the correlation.
            //
            for(int i = 0; i < cL->GetNumDatabaseCorrelations(); ++i)
            {
                if(cL->GetDatabaseCorrelation(i).GetName() == name)
                {
                    cL->RemoveDatabaseCorrelation(i);
                    cL->Notify();

                    // Make any plot lists that used the correlation that
                    // we just deleted switch to a new time slider.
                    for(int j = 0; j < maxWindows; ++j)
                    {
                        if(windows[j] != 0)
                        {
                            windows[j]->GetPlotList()->
                                DeleteTimeSlider(name);
                        }
                    }

                    // The time sliders changed. Send the new list to
                    // the client.
                    UpdateWindowInformation(WINDOWINFO_TIMESLIDERS);

                    break;
                }
            }
        }
    }
    else
    {
        Error("There is no such database correlation.");
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::GetDatabasesForWindows
//
// Purpose: 
//   Returns a list of databases for the specified windows.
//
// Arguments:
//   windowIds : The list of windows for which we want databases.
//   dbs       : The return list of databases.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:01:45 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::GetDatabasesForWindows(const intVector &windowIds,
    stringVector &dbs) const
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
 
    for(int i = 0; i < windowIds.size(); ++i)
    {
        int index = windowIds[i];
        if(index >= 0 && index < maxWindows && windows[index] != 0)
        {
            ViewerPlotList *pl = windows[index]->GetPlotList();

            // Try and add the active source for the window.
            std::string source(pl->GetHostDatabaseName());
            if(cL->FindCorrelation(source) != 0)
            {
                if(std::find(dbs.begin(), dbs.end(), source) == dbs.end())
                    dbs.push_back(source);
            }

            // Try and add the source for each of the plots.
            for(int j = 0; j < pl->GetNumPlots(); ++j)
            {
                std::string pSource(pl->GetPlot(j)->GetSource());
                if(cL->FindCorrelation(pSource) != 0)
                {
                    if(std::find(dbs.begin(), dbs.end(), pSource) == dbs.end())
                        dbs.push_back(pSource);
                }
            }
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::CloseDatabase
//
// Purpose: 
//   This method closes a database in all plot lists that have that database
//   as their open database. It also removes the time slider for that database
//   from all windows and clears the metadata for the database in the file
//   before finally telling the engine to clear out all networks involving
//   the database.
//
// Arguments:
//   dbName : The name of the databsae to close.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 12:52:19 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 11:04:04 PST 2004
//    Added an engine key used to index (and restart) engines.
//    I had to get the metadata before closing the database to determine
//    which engine the file lives on.
//
//    Brad Whitlock, Fri Feb 4 08:17:31 PDT 2005
//    Added code to tell the mdserver to close its database.
//
//    Brad Whitlock, Tue Feb 22 14:37:14 PST 2005
//    Guarded against a NULL metadata pointer.
//
// ****************************************************************************

void
ViewerWindowManager::CloseDatabase(const std::string &dbName)
{
    char tmp[300];

    //
    // Expand the filename in case it contains relative paths, etc.
    //
    ViewerFileServer *fs = ViewerFileServer::Instance();
    std::string expandedDB(dbName), host, db;
    fs->ExpandDatabaseName(expandedDB, host, db);

    std::string sim = "";
    const avtDatabaseMetaData *md = fs->GetMetaData(host, db);
    if (md != 0 && md->GetIsSimulation())
        sim = db;

    if(FileInUse(host, db))
    {
        SNPRINTF(tmp, 300, "VisIt could not close \"%s\" because it is still "
                 "being used by one or more plots.",
                 expandedDB.c_str());
        Error(tmp);
    }
    else
    {
        //
        // Tell the file server to clear out any metadata related to
        // the database that we're closing. This also deletes the database
        // correlation.
        //
        fs->ClearFile(expandedDB);

        //
        // Tell the mdserver to close its database so the next time we ask
        // for it, it will give back the right information if the database
        // was a virtual database.
        //
        fs->CloseFile(host, db);

        //
        // If the plot list's open database is the database that we're
        // closing, make it have no open database. Also, remove its
        // time slider for the database
        //
        int updateFlags = 0;
        for(int i = 0; i < maxWindows; ++i)
        {
            if(windows[i] != 0)
            {
                ViewerPlotList *pl = windows[i]->GetPlotList();
                int flags = pl->CloseDatabase(expandedDB);
                if(i == activeWindow)
                    updateFlags |= flags;
            }
        }

        // Update the client if there were changes.
        if(updateFlags > 0)
            UpdateWindowInformation(updateFlags);

        //
        // Update the global attributes since clearing out the file in
        // the file server will affect the list of sources.
        //
        UpdateGlobalAtts();

        //
        // Tell the engine to clear any networks that involve the
        // database that we're closing.
        //
        ViewerEngineManager::Instance()->ClearCache(EngineKey(host, sim),
                                                    db.c_str());

        SNPRINTF(tmp, 300, "VisIt closed \"%s\".", expandedDB.c_str());
        Message(tmp);        
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ReplaceDatabase
//
// Purpose: 
//   Replaces the database in all windows.
//
// Arguments:
//   key             : The key of the engine that will generate the plot.
//   database        : The database to use.
//   timeState       : The time state to use.
//   setTimeState    : Whether or not to set the time state.
//   onlyReplaceSame : If true, then file replacement is only done if the
//                     new database is the same as the database in a plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 15 14:03:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Nov 3 10:03:00 PDT 2003
//   Added timeState and setTimeState arguments.
//
//   Brad Whitlock, Mon Jan 26 22:50:56 PST 2004
//   I made it use the plot list directly.
//
//   Brad Whitlock, Mon May 3 12:48:29 PDT 2004
//   I made it use an engine key.
//
//   Mark C. Miller, Mon Dec  6 20:18:43 PST 2004
//   Added code to push final SIL controls to client
//
// ****************************************************************************

void
ViewerWindowManager::ReplaceDatabase(const EngineKey &key,
    const std::string &database, int timeState, bool setTimeState,
    bool onlyReplaceSame)
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
        {
            windows[i]->GetPlotList()->
                ReplaceDatabase(key, database, timeState, setTimeState,
                                onlyReplaceSame);
        }
    }

    //
    // After we've completed the replace, we need to make sure the SIL
    // window displays the correct contents; that of the active plot of
    // the active window
    //
    ViewerWindow *aWin = GetActiveWindow();
    if (aWin != 0)
    {
        aWin->GetPlotList()->UpdateSILRestrictionAtts();
    }

}

// ****************************************************************************
// Method: ViewerWindowManager::CheckForNewStates
//
// Purpose: 
//   Updates the correlations involving the specified database, updates any
//   time sliders, and finally resizes any actor caches if we added states.
//
// Arguments:
//   hostDatabase : The name of the database to check for new states.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 27 10:19:17 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::CheckForNewStates(const std::string &hostDatabase)
{
    const char *mName = "ViewerWindowManager::CheckForNewStates: ";

    std::string hDB(hostDatabase);
    std::string host, db;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    fs->ExpandDatabaseName(hDB, host, db);

    //
    // Determine if VisIt has even opened the specified database.
    //
    stringVector sources(fs->GetOpenDatabases());
    bool usingDatabase = false;
    int i;
    for(i = 0; i < sources.size() && !usingDatabase; ++i)
    {
        debug4 << "source[" << i << "] = " << sources[i].c_str() << endl;
        usingDatabase |= (sources[i] == hDB);
    }

    //
    // If VisIt is using the database then we must determine if the number of
    // time states has changed. If the number of time states has changed then
    // we need to update the database correlations and plot cache sizes.
    //
    if(usingDatabase)
    {
        const avtDatabaseMetaData *md = fs->GetMetaData(host, db);
        if(md != 0 && md->GetIsVirtualDatabase())
        {
            // Get the list of time states.
            int originalNStates = md->GetNumStates();
            stringVector originalTSNames(md->GetTimeStepNames());
            bool isSim = md->GetIsSimulation();

            debug4 << mName << hDB.c_str() << " is a virtual database with "
                   << originalNStates << " time states." << endl;

            debug4 << mName << "Current time steps for " << hDB.c_str()
                   << ": " << endl;
            for(i = 0; i < originalTSNames.size(); ++i)
                debug4 << originalTSNames[i].c_str() << ", " << endl;
            debug4 << endl;

            //
            // Look through all of the windows and figure out the last time
            // state from the database that we're essentially reopening.
            //
            int timeState = 0;
            for(i = 0; i < maxWindows; ++i)
            {
                if(windows[i] != 0)
                {
                    ViewerPlotList *pl = windows[i]->GetPlotList();
                    for(int p = 0; p < pl->GetNumPlots(); ++p)
                    {
                        ViewerPlot *plot = pl->GetPlot(p);
                        if(plot->GetSource() == hDB)
                        {
                            timeState = (timeState < plot->GetState()) ?
                                plot->GetState() : timeState;
                        }
                    }
                }
            }

            // Clear all knowledge of the file from the cache
            fs->CloseFile(hDB);
            fs->ClearFile(hDB);

            // Get the file's metadata again.
            debug4 << mName << "Reopening " << hDB.c_str()
                   << " at state " << timeState << endl;
            md = fs->GetMetaDataForState(host, db, timeState);
            if(md != 0 && md->GetIsVirtualDatabase())
            {
                int newNStates = md->GetNumStates();
                stringVector newTSNames(md->GetTimeStepNames());

                debug4 << mName << "New time steps for " << hDB.c_str()
                       << ": " << endl;
                for(i = 0; i < newTSNames.size(); ++i)
                    debug4 << newTSNames[i].c_str() << ", " << endl;
                debug4 << endl;

                //
                // Alter any database correlations that use the database that
                // changed.
                //
                DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
                stringVector alteredCorrelations;
                for(i = 0; i < cL->GetNumDatabaseCorrelations(); ++i)
                {
                    DatabaseCorrelation &correlation = cL->operator[](i);
                    if(correlation.UsesDatabase(hDB))
                    {
                        if(originalNStates != newNStates ||
                           correlation.GetMethod() == DatabaseCorrelation::TimeCorrelation ||
                           correlation.GetMethod() == DatabaseCorrelation::CycleCorrelation)
                        {
                            debug4 << mName << "Updated correlation for "
                                   << correlation.GetName().c_str() << endl;
                            alteredCorrelations.push_back(correlation.GetName());
                            fs->UpdateDatabaseCorrelation(correlation.GetName());
                        }
                    }
                }

                //
                // The previous call to FileServerList::ClearFile deleted the
                // trivial database correlation. Recreate it.
                //
                stringVector dbs; dbs.push_back(hDB);
                DatabaseCorrelation *newCorr = fs->CreateDatabaseCorrelation(hDB, dbs, 0);
                if(newCorr != 0)
                {
                    debug4 << "New correlation for " << hDB.c_str() << *newCorr << endl;
                    cL->AddDatabaseCorrelation(*newCorr);
                    delete newCorr;
                }
                if(originalNStates != newNStates)
                {
                    debug4 << mName << "Updated correlation for "
                           << hDB.c_str() << endl;
                    alteredCorrelations.push_back(hDB);
                }
                // Send new database correlation list to the client.
                cL->Notify();

                //
                // If the virtual database has 1 time state per file then it
                // might be possible to expand or contract the time sliders
                // and cache sizes.
                //
                bool clearCache = true;
                bool oneTSPerFile = (originalTSNames.size() == originalNStates &&
                   newTSNames.size() == newNStates);
                if(oneTSPerFile)
                {
                    int n = (originalTSNames.size() < newTSNames.size()) ? 
                    originalTSNames.size() : newTSNames.size();
                    bool same = true;
                    for(i = 0; i < n && same; ++i)
                        same &= (originalTSNames[i] == newTSNames[i]);
                    clearCache = !same;
                    debug4 << mName << "Need to clear actor cache: "
                           << (clearCache ? "true":"false") << endl;
                }

                //
                // Clear the cache for the database on the compute engine.
                //
                bool ntsChanged = (newNStates != originalNStates);
                if(ntsChanged)
                {
                    EngineKey key(host, "");
                    if (isSim)
                        key = EngineKey(host, db);
                    debug4 << mName << "Clearing out cached database ";
                    if(isSim)
                        debug4 << " in simulation on " << host.c_str() << endl;
                    else
                    {
                        debug4 << db.c_str() << " in compute engine on "
                               << host.c_str() << endl;
                    }
                    ViewerEngineManager::Instance()->ClearCache(key, db.c_str());
                }

                //
                // Loop over the windows and make sure that their time sliders are
                // within the limits for the new version of the database. Also be
                // sure to adjust their cache sizes if needed.
                //
                bool expressionListUpdated = false;
                for(i = 0; i < maxWindows; ++i)
                {
                    if(windows[i] != 0)
                    { 
                        ViewerPlotList *pl = windows[i]->GetPlotList();
                        int flag = pl->ResizeTimeSliders(alteredCorrelations, clearCache);
                        bool tsSizeChanged = (flag & 1) > 0;
                        bool actorsCleared = (flag & 2) > 0;

                        // Update the time slider states for the active window
                        if((tsSizeChanged || ntsChanged) && i == activeWindow)
                            UpdateWindowInformation(WINDOWINFO_TIMESLIDERS);

                        // If we had to clear the cache then we need to update the window too.
                        if(actorsCleared)
                        {
                            // Update the expression list based on the plots. We have
                            // to update the expression list or risk not being able to
                            // update plots that have
                            expressionListUpdated = true;
                            pl->UpdateExpressionList(true, false);

                            // Update the frame.
                            debug4 << mName << "Updating window "
                                   << windows[i]->GetWindowId() << endl;
                            pl->UpdateFrame(false);
                        }
                    }
                }

                // If we updated an expression then we need to update the expression
                // list to set it back to what it was.
                if(expressionListUpdated)
                    windows[activeWindow]->GetPlotList()->UpdateExpressionList(true);
            }
            else
            {
                debug4 << mName << "Could not get the metadata for " << hDB.c_str()
                       << "or we got it and it was not a virtual database." << endl;
            }
        }
        else
        {
            debug4 << mName << hDB.c_str() << " is not a virtual database." << endl;
        }
    }
    else
    {
        debug4 << mName << hDB.c_str() << " is not an open source." << endl;
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ResetNetworkIds
//
// Purpose: 
//   Resets the network ids for all plots that use the specified engine key.
//
// Arguments:
//   key : The engine key that's getting reset.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 3 14:12:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::ResetNetworkIds(const EngineKey &key)
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            windows[i]->GetPlotList()->ResetNetworkIds(key);
    }    
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetLightListClientAtts
//
//  Purpose: 
//    Returns a pointer to the light list.
//
//  Returns:    A pointer to the light list.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 15:18:41 PST 2001
//
// ****************************************************************************

LightList *
ViewerWindowManager::GetLightListClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (lightListClientAtts == 0)
    {
        lightListClientAtts = new LightList;
    }

    return lightListClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetLightListDefaultAtts
//
//  Purpose: 
//    Returns a pointer to the default light list.
//
//  Returns:    A pointer to the default light list.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 15:18:09 PST 2001
//   
// ****************************************************************************

LightList *
ViewerWindowManager::GetLightListDefaultAtts()
{
    //
    // If the default attributes haven't been allocated then do so.
    //
    if (lightListDefaultAtts == 0)
    {
        lightListDefaultAtts = new LightList;
    }

    return lightListDefaultAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetClientLightListFromDefault
//
//  Purpose: 
//    This method copies the default light list into the client light list.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 15:17:16 PST 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetClientLightListFromDefault()
{
    if(lightListDefaultAtts != 0 && lightListClientAtts != 0)
    {
        *lightListClientAtts = *lightListDefaultAtts;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetDefaultLightListFromClient
//
//  Purpose: 
//    This method copies the client's light list into the default light list.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 15:16:25 PST 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetDefaultLightListFromClient()
{
    if(lightListDefaultAtts != 0 && lightListClientAtts != 0)
    {
        *lightListDefaultAtts = *lightListClientAtts;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetLightListFromClient
//
//  Purpose: 
//    Sets the light list for the active window based on the client's
//    light list.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 30 09:39:34 PDT 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    The viewer window now stores LightList attributes, not avtLightList, so
//    there is no need for a conversion here. 
//
//    Jeremy Meredith, Thu Oct  2 12:34:00 PDT 2003
//    Copy the light list to avtCallback.
//
// ****************************************************************************

void
ViewerWindowManager::SetLightListFromClient()
{
    // HACK: set the light list in the callback
    // for why this is bad, see VisIt00003853.
    avtCallback::SetCurrentLightList(*lightListClientAtts);

    windows[activeWindow]->SetLightList(lightListClientAtts);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetLightListFromDefault
//
//  Purpose: 
//    Sets the light list for the active window based on the default
//    light list.
//
//  Arguments:
//    windowIndex : The index of the window to whose light list is to be reset.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 15:14:11 PST 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    The viewer window now stores LightList attributes, not avtLightList, so
//    there is no need for a conversion here. 
//   
// ****************************************************************************

void
ViewerWindowManager::SetLightListFromDefault()
{
    windows[activeWindow]->SetLightList(lightListDefaultAtts);

    //
    // Update the client's light list.
    //
    UpdateLightListAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::GetWindowAtts
//
// Purpose: 
//   Returns a pointer the ViewerWindowManagerAttributes object.
//
// Returns:    A pointer the ViewerWindowManagerAttributes object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 10:35:49 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

ViewerWindowManagerAttributes *
ViewerWindowManager::GetWindowAtts()
{
    if(windowAtts == 0)
    {
        windowAtts = new ViewerWindowManagerAttributes;
    }

    return windowAtts;
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateWindowAtts
//
// Purpose: 
//   Stores the current viewer window locations, sizes into the
//   ViewerWindowManagerAttributes object so that information can be saved to
//   the configuration file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 10:37:06 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 5 09:53:02 PDT 2002
//   Added code to save the window's navigate mode, etc.
//
//   Brad Whitlock, Mon Sep 16 14:56:57 PST 2002
//   I removed the code to save the layout since it is now part of the
//   global atts instead.
//
//   Brad Whitlock, Wed Feb 5 14:22:29 PST 2003
//   I added support for saving toolbar settings.
//
//   Brad Whitlock, Wed Jul 23 13:57:48 PST 2003
//   I removed the window size, location from windowAtts.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateWindowAtts()
{
    // Make sure the ViewerWindowManager atts object is created.
    GetWindowAtts();

    // Let the active window's action manager update the window atts so we
    // save the current toolbar settings.
    windows[activeWindow]->GetActionManager()->UpdateActionInformation(windowAtts);
}

// ****************************************************************************
// Method: ViewerWindowManager::GetWindowInformation
//
// Purpose: 
//   Returns a pointer to the WindowInformation object.
//
// Returns:    A pointer the WindowInformation object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 15:01:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

WindowInformation *
ViewerWindowManager::GetWindowInformation()
{
    if(windowInfo == 0)
    {
        windowInfo = new WindowInformation;
    }

    return windowInfo;
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateWindowInformation
//
// Purpose: 
//   Sends the window information (button state, etc.) to the client.
//
// Arguments:
//   flags       : The pieces of the window information that we want to
//                 send. See ViewerWindowManager.h for the flags that we use.
//   windowIndex : The index of the window for which we're updating. We use
//                 this to make sure that we only update the for the active
//                 window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 15:14:23 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Nov 11 11:36:43 PDT 2002
//   I added code to update the new lockTools and lockTime fields.
//
//   Eric Brugger, Tue Jan 14 08:11:52 PST 2003
//   I added the number of dimensions to the GetExtents call.
//
//   Jeremy Meredith, Tue Feb  4 17:44:20 PST 2003
//   I added the camera view mode info to the WindowInformation.
//
//   Eric Brugger, Fri Apr 18 12:38:05 PDT 2003 
//   I removed auto center view.
//
//   Brad Whitlock, Wed May 21 07:50:49 PDT 2003
//   I added fullframe to the WindowInformation.
//
//   Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//   I split the view attributes into 2d and 3d parts.
//
//   Brad Whitlock, Fri Jan 23 16:10:38 PST 2004
//   I removed some code into other methods and I added code to set new
//   attributes in the WindowInformation such as the active source and the
//   list of time states. Finally, I added the flags argument so we can
//   do partial sends of data to the client.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateWindowInformation(int flags, int windowIndex)
{
    GetWindowInformation();

    //
    // Set certain window mode information into the state object.
    //
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    ViewerWindow *win = windows[index];
    if(win != 0 && index == activeWindow)
    {
        //
        // Set the active source for the window into the window atts.
        //
        ViewerPlotList  *plotList = win->GetPlotList();
        if((flags & WINDOWINFO_SOURCE) != 0)
        {
            if(plotList->GetHostDatabaseName().size() < 1)
                windowInfo->SetActiveSource("notset");
            else
                windowInfo->SetActiveSource(plotList->GetHostDatabaseName());
        }

        //
        // Get the list of time sliders and their current states from
        // the plot list.
        //
        stringVector timeSliders;
        intVector    timeSliderCurrentStates;
        int          activeTimeSlider = -1;
        plotList->GetTimeSliderInformation(activeTimeSlider, timeSliders,
                                           timeSliderCurrentStates);
        if((flags & WINDOWINFO_TIMESLIDERS) != 0)
        {
            windowInfo->SetTimeSliders(timeSliders);
            windowInfo->SetActiveTimeSlider(activeTimeSlider);
        }

        //
        // Return the current animation mode for the window.
        // (i.e. is the window playing an animation?)
        //
        if((flags & WINDOWINFO_ANIMATION) != 0 ||
           (flags & WINDOWINFO_TIMESLIDERS) != 0)
        {
            windowInfo->SetTimeSliderCurrentStates(timeSliderCurrentStates);

            if (plotList->GetAnimationMode() == ViewerPlotList::PlayMode)
                windowInfo->SetAnimationMode(3);
            else if (plotList->GetAnimationMode() == ViewerPlotList::ReversePlayMode)
                windowInfo->SetAnimationMode(1);
            else
                windowInfo->SetAnimationMode(2);
        }

        //
        // Set window mode, etc.
        //
        if((flags & WINDOWINFO_WINDOWFLAGS) != 0)
        {
            windowInfo->SetWindowMode(int(win->GetInteractionMode()));
            windowInfo->SetBoundingBoxNavigate(win->GetBoundingBoxMode());
            windowInfo->SetSpin(win->GetSpinMode());
            windowInfo->SetLockView(win->GetViewIsLocked());
            windowInfo->SetViewExtentsType(int(win->GetViewExtentsType()));
            windowInfo->SetWindowMode(win->GetWindowMode());
            windowInfo->SetPerspective(win->GetPerspectiveProjection());
            windowInfo->SetLockTools(win->GetToolLock());
            windowInfo->SetLockTime(win->GetTimeLock());
            windowInfo->SetCameraViewMode(win->GetCameraViewMode());
            windowInfo->SetFullFrame(win->GetFullFrameMode());

            // indicate if we're in scalable rendering mode
            windowInfo->SetUsingScalableRendering(win->GetScalableRendering());
        }

        //
        // Get the window size.
        //
        if((flags & WINDOWINFO_WINDOWSIZE) != 0)
        {
            int wsize[2];
            win->GetSize(wsize[0], wsize[1]);
            windowInfo->SetWindowSize(wsize);
        }

        //
        // Get some rendering information.
        //
        if((flags & WINDOWINFO_RENDERINFO) != 0)
        {
            float times[3] = {0., 0., 0.};
            win->GetRenderTimes(times);
            windowInfo->SetLastRenderMin(times[0]);
            windowInfo->SetLastRenderAvg(times[1]);
            windowInfo->SetLastRenderMax(times[2]);

            // Set the approximate number of triangles.
            windowInfo->SetNumPrimitives(win->GetNumPrimitives());

            // Set the bounding box.
            double extents[6] = {0., 0., 0., 0., 0., 0.};
            if (win->GetWindowMode() == WINMODE_3D)
                win->GetExtents(3, extents);
            else
                win->GetExtents(2, extents);
            windowInfo->SetExtents(extents);
        }

        windowInfo->Notify();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateWindowRenderingInformation
//
// Purpose: 
//   Sends information about rendering such as render time and primitve count
//   to the client.
//
// Arguments:
//   windowIndex : The index of the window that caused the update.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 23 16:00:25 PST 2004
//
// Modifications:
//   Mark C. Miller, Thu Mar  3 17:38:36 PST 2005
//   Changed GetNumTriangles to GetNumPrimitives
//
//   Brad Whitlock, Tue Jun 21 12:16:25 PDT 2005
//   I made the query for the window size happen with each render but we
//   only send it back to the client if the window size is different from the
//   values that we've already sent.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateWindowRenderingInformation(int windowIndex)
{
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    ViewerWindow *win = windows[index];
    if(win != 0 && index == activeWindow)
    {
        if(win->GetNotifyForEachRender())
            UpdateWindowInformation(WINDOWINFO_RENDERINFO | WINDOWINFO_WINDOWSIZE);
        else
        {
            // Get the window size. If it differs from what we've stored in
            // the object then send it back to the client.
            int wsize[2];
            win->GetSize(wsize[0], wsize[1]);
            if(windowInfo->GetWindowSize()[0] != wsize[0] ||
               windowInfo->GetWindowSize()[1] != wsize[1])
            {
                windowInfo->SetWindowSize(wsize);
                windowInfo->Notify();
            }
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateViewKeyframeInformation
//
// Purpose: 
//   Updates the view keyframes in the window info and notifies the client.
//
// Arguments:
//   notify : Whether the client should be notified.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 23 15:37:06 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::UpdateViewKeyframeInformation()
{
    //
    // Update the view keyframe list.
    //
    int        nIndices = 0;
    const int *keyframeIndices = windows[activeWindow]->GetViewKeyframeIndices(nIndices);
    intVector  keyframeIndices2;
    for (int i = 0; i < nIndices; ++i)
        keyframeIndices2.push_back(keyframeIndices[i]);
    windowInfo->SetViewKeyframes(keyframeIndices2);
    windowInfo->Notify();
}

// ****************************************************************************
//  Method: ViewerWindowManager::InitWindowLimits
//
//  Purpose:
//      Initialize the window positions and sizes for various window layouts
//      based on the virtual screen size and the size of the window borders.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Jul 20 11:22:25 PDT 2001
//    Added a shift to the x,y positions of windows.
//
//    Eric Brugger, Fri Feb 13 14:14:41 PST 2004
//    Modified the layout code to create rectangular windows that completely
//    fill the available space rather than creating square windows that fill
//    the space as best as possible.  In the case of 1 x 2 and 2 x 4 layouts
//    the height is restricted to the width.
//
//    Eric Brugger, Thu Jun 30 11:43:22 PDT 2005
//    Added a 2 x 3 layout and removed the 4 x 4 layout.  I also changed the
//    window size creation logic to always create the largest square windows
//    in the allowed space.
//
// ****************************************************************************

void
ViewerWindowManager::InitWindowLimits()
{
    //
    // Create the window layouts.
    //
    int       i, j;
    int       borderWidth, borderHeight;
    int       tempWidth, tempHeight, tempSize;
    int       x, y;
    int       cnt;

    borderWidth  = borderLeft + borderRight;
    borderHeight = borderTop  + borderBottom;

    //
    // The layout for one window.
    //
    x          = screenX + borderLeft - shiftX;
    y          = screenY + borderTop  - shiftY;
    tempWidth  = screenWidth - borderWidth;
    tempHeight = screenHeight - borderHeight;
    tempSize   = tempWidth < tempHeight ? tempWidth : tempHeight;
    windowLimits[0][0].x      = x;
    windowLimits[0][0].y      = y;
    windowLimits[0][0].width  = tempSize;
    windowLimits[0][0].height = tempSize;

    //
    // The layout for a 2x2 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/2)  - borderWidth;
    tempHeight = (screenHeight/2) - borderHeight;
    tempSize   = tempWidth < tempHeight ? tempWidth : tempHeight;
    for (i = 0; i < 2; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 2; j++)
        {
            windowLimits[1][cnt].x      = x;
            windowLimits[1][cnt].y      = y;
            windowLimits[1][cnt].width  = tempSize;
            windowLimits[1][cnt].height = tempSize;
            x = x + windowLimits[1][cnt].width + borderWidth;
            cnt ++;
        }
        y = y + windowLimits[1][0].height + borderHeight;
    }

    //
    // The layout for a 3x3 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/3)  - borderWidth;
    tempHeight = (screenHeight/3) - borderHeight;
    tempSize   = tempWidth < tempHeight ? tempWidth : tempHeight;
    for (i = 0; i < 3; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 3; j++)
        {
            windowLimits[2][cnt].x      = x;
            windowLimits[2][cnt].y      = y;
            windowLimits[2][cnt].width  = tempSize;
            windowLimits[2][cnt].height = tempSize;
            x = x + windowLimits[2][cnt].width + borderWidth;
            cnt ++;
        }
        y = y + windowLimits[2][0].height + borderHeight;
    }

    //
    // The layout for a 1x2 grid.
    //
    cnt = 0;
    x          = screenX + borderLeft - shiftX;
    y          = screenY + borderTop  - shiftY;
    tempWidth  = (screenWidth/2) - borderWidth;
    tempHeight =  screenHeight   - borderHeight;
    tempSize   = tempWidth < tempHeight ? tempWidth : tempHeight;
    for (i = 0; i < 2; i++)
    {
        windowLimits[3][cnt].x      = x;
        windowLimits[3][cnt].y      = y;
        windowLimits[3][cnt].width  = tempSize;
        windowLimits[3][cnt].height = tempSize;
        x = x + windowLimits[3][cnt].width + borderWidth;
        cnt ++;
    }

    //
    // The layout for a 2x3 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/3)  - borderWidth;
    tempHeight = (screenHeight/2) - borderHeight;
    tempSize   = tempWidth < tempHeight ? tempWidth : tempHeight;
    for (i = 0; i < 2; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 3; j++)
        {
            windowLimits[4][cnt].x      = x;
            windowLimits[4][cnt].y      = y;
            windowLimits[4][cnt].width  = tempSize;
            windowLimits[4][cnt].height = tempSize;
            x = x + windowLimits[4][cnt].width + borderWidth;
            cnt ++;
        }
        y = y + windowLimits[4][0].height + borderHeight;
    }

    //
    // The layout for a 2x4 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/4)  - borderWidth;
    tempHeight = (screenHeight/2) - borderHeight;
    tempSize   = tempWidth < tempHeight ? tempWidth : tempHeight;
    for (i = 0; i < 2; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 4; j++)
        {
            windowLimits[5][cnt].x      = x;
            windowLimits[5][cnt].y      = y;
            windowLimits[5][cnt].width  = tempSize;
            windowLimits[5][cnt].height = tempSize;
            x = x + windowLimits[5][cnt].width + borderWidth;
            cnt ++;
        }
        y = y + windowLimits[5][0].height + borderHeight;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SimpleAddWindow
//
//  Purpose:
//    Add a simple window.
//
//  Returns:    The index of the newly added window.
//
//  Programmer: Eric Brugger
//  Creation:   April 11, 2003
//
//  Modifications:
//    Eric Brugger, Fri Feb 13 14:14:41 PST 2004
//    Modified the routine to use both the width and height for the window
//    size, instead of using with width for both.
//
//    Hank Childs, Mon Feb 14 13:37:59 PST 2005
//    Get around a bug with window manager.  If you offset new windows
//    by 32 pixels, the old window is blanked out.  But if you use 33 pixels,
//    the old window is okay (see '5913).  So we are changing the offset.
//
// ****************************************************************************

int
ViewerWindowManager::SimpleAddWindow()
{
    //
    // Check that we have available windows.
    //
    if (nWindows >= maxWindows)
    {
        return -1;
    }

    //
    // Find the first unused window.
    //
    int       windowIndex;

    for (windowIndex = 0; windowIndex < maxWindows; windowIndex++)
    {
        if (windows[windowIndex] == 0) break;
    }

    //
    // Determine the position and size of the new window.
    //
    int       x, y;
    int       width, height;

    if (windowIndex < layout)
    {
        x      = windowLimits[layoutIndex][windowIndex].x;
        y      = windowLimits[layoutIndex][windowIndex].y;
        width  = windowLimits[layoutIndex][windowIndex].width;
        height = windowLimits[layoutIndex][windowIndex].height;
    }
    else
    {
        int offset = 33; // Don't change this number without seeing '5913.
        x      = windowLimits[0][0].x + (nWindows - layout + 1) * offset;
        y      = windowLimits[0][0].y + (nWindows - layout + 1) * offset;
        width  = windowLimits[layoutIndex][0].width;
        height = windowLimits[layoutIndex][0].height;
    }

    //
    // Create the new window along with its animation.
    //
    CreateVisWindow(windowIndex, width, height, x, y);

    return windowIndex;
}

// ****************************************************************************
//  Method: ViewerWindowManager::CreateVisWindow
//
//  Purpose:
//    Create a window at the specified size and location.
//
//  Arguments:
//    windowIndex  The 0-origin index of the window to create.
//    width        The window's width.
//    height       The window's height.
//    x            The window's x position.
//    y            The window's y position.
//
//  Programmer: Eric Brugger
//  Creation:   September 20, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Nov 7 10:02:09 PDT 2000
//    Relocated animations to ViewerWindow.
//
//    Brad Whitlock, Mon Nov 27 14:07:50 PST 2000
//    Added code to emit a createWindow signal.
//
//    Jeremy Meredith, Fri Jul 20 11:22:48 PDT 2001
//    Added code to resize/reposition the window after it is realized.
//    Some window managers place them incorrectly the first time they are
//    created, so we must fix it afterwards.
//
//    Jeremy Meredith, Fri Sep 14 13:33:36 PDT 2001
//    Added a preshift to the initial position of the window.
//    Also removed the second setsize call since it was redundant.
//
//    Brad Whitlock, Tue Feb 5 09:56:09 PDT 2002
//    Added code to set the navigation, perspective, autocenter modes of the
//    window from the defaults.
//
//    Sean Ahern, Tue Apr 16 14:06:25 PDT 2002
//    Added the ability to have deferred creation of windows.
//
//    Brad Whitlock, Mon Sep 16 17:40:10 PST 2002
//    I made it use the windowInfo and renderAtts state objects.
//
//    Hank Childs, Fri Oct 18 08:22:45 PDT 2002
//    Put all of the code for setting the window attributes into a separate
//    routine.
//
//    Brad Whitlock, Wed Mar 16 13:26:42 PST 2005
//    I set the referenced flag for the window to false so it is right if
//    we delete and recreate a window.
//
// ****************************************************************************

void
ViewerWindowManager::CreateVisWindow(const int windowIndex,
                                     const int width, const int height,
                                     const int x, const int y)
{
    char      title[24];

    windows[windowIndex] = new ViewerWindow(windowIndex);

    windows[windowIndex]->SetSize(width, height);

    SNPRINTF(title, 24, "Window %d", windowIndex+1);
    windows[windowIndex]->SetTitle(title);
    if (windowsHidden == false)
    {
        windows[windowIndex]->SetLocation(x - preshiftX, y - preshiftY);
        windows[windowIndex]->Realize();
        windows[windowIndex]->SetLocation(x, y);
    }
    x_locations[windowIndex] = x;
    y_locations[windowIndex] = y;
    referenced[windowIndex] = false;

    nWindows++;

    //
    // Tell other objects that a window has been created.
    //
    emit createWindow(windows[windowIndex]);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetWindowAttributes
//
//  Purpose:
//      Sets the window attributes, like bounding box mode, autocenter, etc.
//
//  Arguments:
//    windowIndex  The 0-origin index of the window to create.
//    copyAtts     A boolean saying if this should be a clone of the previous
//                 window.
//
//  Notes:      This routine was primarily taken from CreateVisWindow.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Nov 12 14:50:16 PST 2002
//    I added code to copy the locktime and locktool modes.
//
//    Kathleen Bonnell, Wed Dec  4 17:38:27 PST 2002 
//    Removed antialiasing frames, no longer needed.  
//
//    Eric Brugger, Fri Apr 18 12:38:05 PDT 2003 
//    I removed auto center view.
//
//    Brad Whitlock, Wed May 21 07:52:21 PDT 2003
//    I made fullframe be copied to the new window.
//
//    Jeremy Meredith, Fri Nov 14 17:57:31 PST 2003
//    Added specular properties.
//
//    Hank Childs, Mon May 10 08:10:40 PDT 2004
//    Replace references to immediate mode rendering with display list mode.
//
//   Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//   Modified calls to set scalable controls to accomdate scalable activation
//   mode and scalable auto threshold
//
//    Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//    Added shading properties.
//
// ****************************************************************************

void
ViewerWindowManager::SetWindowAttributes(int windowIndex, bool copyAtts)
{
    ViewerWindow *w = windows[windowIndex];

    w->SetBoundingBoxMode(windowInfo->GetBoundingBoxNavigate());
    w->SetViewExtentsType((avtExtentType)windowInfo->GetViewExtentsType());
    w->SetPerspectiveProjection(windowInfo->GetPerspective());
    w->SetFullFrameMode(windowInfo->GetFullFrame());
    if (copyAtts)
    {
        w->SetViewIsLocked(windowInfo->GetLockView());
        w->SetTimeLock(windowInfo->GetLockTime());
        w->SetToolLock(windowInfo->GetLockTools());
    }
    else
    {
        w->SetViewIsLocked(false);
        w->SetTimeLock(false);
        w->SetToolLock(false);
    }
    w->SetAntialiasing(renderAtts->GetAntialiasing());
    int rep = (int)renderAtts->GetGeometryRepresentation();
    w->SetSurfaceRepresentation(rep);
    w->SetDisplayListMode(renderAtts->GetDisplayListMode());
    w->SetStereoRendering(renderAtts->GetStereoRendering(),
        (int)renderAtts->GetStereoType());
    w->SetNotifyForEachRender(renderAtts->GetNotifyForEachRender());
    w->SetScalableActivationMode(renderAtts->GetScalableActivationMode());
    w->SetScalableAutoThreshold(renderAtts->GetScalableAutoThreshold());
    w->SetSpecularProperties(renderAtts->GetSpecularFlag(),
                             renderAtts->GetSpecularCoeff(),
                             renderAtts->GetSpecularPower(),
                             renderAtts->GetSpecularColor());
    w->SetShadingProperties(renderAtts->GetDoShadowing(),
                            renderAtts->GetShadowStrength());
}

// ****************************************************************************
//  Method: ViewerWindowManager::ViewCallback
//
//  Purpose:
//    This method is a callback function for VisItInteractor. It gets called
//    each time the view changes and is responsible for telling the client
//    about the new view information.
//
//  Arguments:
//    vw        The vis window where the view changed.
//
//  Programmer: Eric Brugger
//  Creation:   August 21, 2001
//
//  Modifications:
//    Eric Brugger, Wed Aug 22 14:52:37 PDT 2001
//    I removed an argument from UpdateViewAtts.
//
//    Hank Childs, Fri Nov 30 18:04:11 PST 2001
//    Update the current window atts whenever we move the mouse.  This was put
//    in so that the volume renderer could determine if it could use the last
//    image or if it must be recalculated.
//
//    Eric Brugger, Fri Apr 18 12:38:05 PDT 2003 
//    I added code to set the 2d view as modified if the view dimension is 2.
//
//    Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//    I modified the routine to set curve, 2d and 3d view attributes in the
//    window attributes.  I removed some unused code.
//
//    Eric Brugger, Thu Aug 28 12:22:14 PDT 2003
//    I added a call to mark the view as changed if the window is in
//    curve mode.
//
// ****************************************************************************

void
ViewerWindowManager::ViewCallback(VisWindow *vw)
{
    ViewerWindowManager *instance = ViewerWindowManager::Instance();

    //
    // We may be getting a view callback from something besides the active
    // window.  If so, figure out which window it is and use that.
    //
    int index = -1;
    for (int i = 0 ; i < instance->maxWindows ; i++)
    {
        if (instance->windows[i] != 0 &&
            instance->windows[i]->IsTheSameWindow(vw))
        {
            index = i;
            break;
        }
    }

    //
    // Mark the view as having been modified.
    //
    if(instance->windows[index]->GetWindowMode() == WINMODE_CURVE)
        instance->windows[index]->SetViewModifiedCurve();
    else if(instance->windows[index]->GetWindowMode() == WINMODE_2D)
        instance->windows[index]->SetViewModified2d();

    //
    // Update the view attributes in the client and any locked windows.
    //
    instance->UpdateViewAtts(index);

    //
    // For software rendering.
    //
    WindowAttributes winAtts;
    winAtts.SetViewCurve(*GetViewCurveClientAtts());
    winAtts.SetView2D(*GetView2DClientAtts());
    winAtts.SetView3D(*GetView3DClientAtts());
    avtCallback::SetCurrentWindowAtts(winAtts);
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetToolEnabled
//
//  Purpose: 
//    This is a Qt slot function that sets the enabled state for the tool in
//    the specified window.
//
//  Arguments:
//    toolId       The index of the tool being set.
//    enabled      A flag indicating if the tool is enabled.
//    windowIndex  This is a zero-origin integer that specifies the index
//                 of the window we want to change. If the value is -1, use
//                 use the active window.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 1 16:15:15 PST 2001
//
// ****************************************************************************

void
ViewerWindowManager::SetToolEnabled(int toolId, bool enabled, int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->SetToolEnabled(toolId, enabled);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::DisableAllTools
//
// Purpose: 
//   Disables all of the interactive tools for the specified window.
//
// Arguments:
//   windowIndex : The index of the window whose tools we're disabling.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 23 16:16:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::DisableAllTools(int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        windows[index]->DisableAllTools();
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetPlotColors
//
//  Purpose:
//    Sends background and foreground color information to the viewerwindow,
//    to be used by the plots.  If gradient background is specified in the
//    atts, then the average of the two gradient colors is used.
//
//  Arguments:
//    atts      The annotation attributes containing background color info. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 26, 2001 
//
//  Modifications:
//    Brad Whitlock, Tue Nov 6 17:12:06 PST 2001
//    Modified the code to account for the change in the gradient color
//    representation.
//
//    Eric Brugger, Nov  1 10:32:25 PST 2002
//    Modified the code to account for changes to AnnotationAttributes.
//
//    Mark C. Miller, Wed May 26 12:49:52 PDT 2004
//    Modified to use GetDiscernibleBackground color
//   
// ****************************************************************************

void
ViewerWindowManager::SetPlotColors(const AnnotationAttributes *atts)
{
    double bg[4];
    double fg[4];

    atts->GetForegroundColor().GetRgba(fg);
    atts->GetDiscernibleBackgroundColor().GetRgba(bg);
    windows[activeWindow]->SetPlotColors(bg, fg);  
}

// ****************************************************************************
//  Method: ViewerWindowManager::ToolCallback
//
//  Purpose: 
//    This is a static callback function that handles new information coming
//    from tools.
//
//  Arguments:
//    ti        Information from the tool.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 9 15:12:31 PST 2001
//
//  Modfications:
//    Brad Whitlock, Thu Apr 11 17:36:00 PST 2002
//    I made it care about the "Apply to all operators" flag.
//
//    Brad Whitlock, Tue Nov 12 14:11:55 PST 2002
//    I changed the code so it can lock tools together across windows.
//
//    Brad Whitlock, Wed Mar 12 11:44:08 PDT 2003
//    I made it return early if the engine is executing.
//
// ****************************************************************************

void
ViewerWindowManager::ToolCallback(const avtToolInterface &ti)
{
    //
    // Return without doing anything if the engine is executing.
    //
    if(ViewerEngineManager::Instance()->InExecute())
        return;

    //
    // Let the window that caused the tool "event" handle the event first.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    bool applyToAll = GetClientAtts()->GetApplyOperator();
    ViewerWindow *toolWin = 0;
    int iWindow;
    for(iWindow = 0; iWindow < wM->maxWindows; iWindow++)
    {
        ViewerWindow *win = wM->windows[iWindow];
        if (win != 0)
        {
            if(win->IsTheSameWindow((VisWindow *)ti.GetVisWindow()))
            {
                toolWin = win;
                toolWin->HandleTool(ti, applyToAll);
                break;
            }
        }
    }

    //
    // Update the other windows if the window that originated the tool
    // event is in locked tool mode.
    //
    if(toolWin != 0 && toolWin->GetToolLock())
    {
        for(iWindow = 0; iWindow < wM->maxWindows; iWindow++)
        {
            ViewerWindow *win = wM->windows[iWindow];
            if (win != 0 && win != toolWin)
            {
                win->HandleTool(ti, applyToAll);
            }
        }
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateAnimationTimer
//
//  Purpose: 
//    This routine determines if the timer for performing animations should
//    be changed (either turned on or off) based on the current state of the
//    timer and the state of all the animations.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Brad Whitlock, Tue May 14 11:25:01 PDT 2002
//    Added code to allow the playback speed to be changed.
//
//    Brad Whitlock, Tue Jul 23 17:03:59 PST 2002
//    I fixed a bug that prevented animations from playing if the first
//    window does not exist.
//
//    Brad Whitlock, Wed Jan 22 16:48:41 PST 2003
//    I added code to turn off the animation timer if the windows are hidden
//    or iconified.
//
//    Brad Whitlock, Wed Mar 12 09:44:55 PDT 2003
//    I added a check to make sure that individual windows are checked for
//    visibility before they are considered for animation.
//
//    Brad Whitlock, Mon Jan 26 09:49:19 PDT 2004
//    I changed how we check the window for animation.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateAnimationTimer()
{
    //
    // Determine if any animations are playing.
    //
    int       i;
    bool      playing = false;

    if(windowsHidden || windowsIconified)
         playing = false;
    else
    {
         // Look for an animation that is playing.
        for (i = 0; i < maxWindows; i++)
        {
            if (windows[i] != NULL && windows[i]->IsVisible())
            {
                ViewerPlotList::AnimationMode mode =
                    windows[i]->GetPlotList()->GetAnimationMode();

                if (mode == ViewerPlotList::PlayMode ||
                    mode == ViewerPlotList::ReversePlayMode)
                {
                    playing = true;
                    break;
                }
            }
        }
    }

    //
    // Turn on timer if one doesn't already exist and an animation is
    // playing or turn off the timer if it is on and no animations are
    // playing.
    //
    if (playing)
    {
        int timeout = GetAnimationClientAtts()->GetTimeout();
        if (!timer->isActive())
        {
            timer->start(timeout);
        }
        else if(timeout != animationTimeout)
        {
            // Change the playback speed.
            timer->changeInterval(timeout);
        }
        animationTimeout = timeout;
    }
    else if (timer->isActive())
        timer->stop();
}

// ****************************************************************************
// Method: ViewerWindowManager::StopTimer
//
// Purpose: 
//   Turns off the animation timer and makes all animations stop.
//
// Note:       This method is only called when a window is deleted using
//             the window decorations.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 24 17:50:04 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jan 26 09:48:30 PDT 2004
//   I changed how animations are stopped.
//
// ****************************************************************************

void
ViewerWindowManager::StopTimer()
{
    if(timer->isActive())
    {
        //
        // Turn off the timer so we don't try to animate anything while
        // we're waiting for the window to delete.
        //
        timer->stop();

        //
        // Turn off animation in all windows.
        //
        int numWindows = 0;
        for(int i = 0; i < maxWindows; ++i)
        {
            if(windows[i] != NULL)
            {
                windows[i]->GetPlotList()->SetAnimationMode(
                    ViewerPlotList::StopMode);
                ++numWindows;
            }
        }

        //
        // If there is only one window, update the global atts since there
        // will be no pending delete to update them.
        //
        if(numWindows < 2)
            UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::AnimationCallback
//
//  Purpose: 
//    This routine gets called whenever the animation timer goes off.  It
//    advances the appropriate animation to the next frame.  The routine
//    uses a round robin approach to decide which animation to advance so
//    so that all the animations will get advanced synchronously.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu May 9 12:33:53 PDT 2002
//    Added code to prevent going to the next frame when the engine is
//    already executing.
//
//    Brad Whitlock, Wed Jul 24 14:55:50 PST 2002
//    I fixed the scheduling algorithm for choosing the next window.
//
//    Brad Whitlock, Tue Sep 9 15:16:55 PST 2003
//    I added code to tell the ViewerSubject to process some of the input
//    that it received from the client. I had to do this because the animation's
//    NextFrame and PrevFrame methods often need to get a new plot from the
//    compute engine. It uses an RPC to do that and while it is in the RPC,
//    it checks for new input from the client and it looks for an interrupt
//    opcode in that input. If it finds an interrupt then it interrupts,
//    otherwise the input is left unprocessed in the input buffer. The RPC
//    also calls some code to process Qt window events. Unfortunately, that
//    function call does not process client input because the socket has been
//    read. This is okay because it would process client input, which could
//    potentially alter the plot, in the middle of executing a plot. To fix
//    the situation, I tell the ViewerSubject to process any client input
//    that it has after the plot has been executed. This lets us process
//    client input without the danger of being inside the engine proxy's
//    Execute RPC and it is pretty much a noop when we get to this function
//    with an animation that's been cached.
//
//    Brad Whitlock, Mon Jan 26 09:51:08 PDT 2004
//    I changed how we check for animation since there are now multiple
//    time sliders that could update.
//
// ****************************************************************************

void
ViewerWindowManager::AnimationCallback()
{
    //
    // Return without doing anything if the engine is executing.
    //
    if(ViewerEngineManager::Instance()->InExecute() || windowsHidden || windowsIconified)
        return;

    //
    // Determine the next animation to update.
    //
    int i, startFrame = lastAnimation + 1;
    if(startFrame == maxWindows)
        startFrame = 0;
    for(i = startFrame; i != lastAnimation; )
    {
        if (windows[i] != NULL)
        {
            ViewerPlotList::AnimationMode mode =
                windows[i]->GetPlotList()->GetAnimationMode();

            if (mode == ViewerPlotList::PlayMode ||
                mode == ViewerPlotList::ReversePlayMode)
            {
                lastAnimation = i;
                break;
            }
        }

        // Go to the next window index wrapping around if needed.
        if(i == (maxWindows - 1))
            i = 0;
        else
            ++i;
    }

    //
    // Advance the animation if animation is allowed for the new
    // animation. We check the flag first in case the window was deleted.
    //
    if(windows[lastAnimation] != NULL)
    {
        ViewerPlotList::AnimationMode mode =
            windows[lastAnimation]->GetPlotList()->GetAnimationMode();

        // Prevent the timer from emitting any signals since the
        // code to handle animation may get back to the Qt event
        // loop which makes it possible to get back here reentrantly.
        timer->blockSignals(true);

        if (mode == ViewerPlotList::PlayMode)
        {
            // Change to the next frame in the animation, which will likely
            // cause us to have to read a plot from the compute engine.
            windows[lastAnimation]->GetPlotList()->ForwardStep();

            // Send new window information to the client if we're animating
            // the active window.
            UpdateWindowInformation(WINDOWINFO_ANIMATION, lastAnimation);

            // Process any client input that we had to ignore while reading
            // the plot from the compute engine.
            viewerSubject->ProcessFromParent();
        }
        else if(mode == ViewerPlotList::ReversePlayMode)
        {
            // Change to the next frame in the animation, which will likely
            // cause us to have to read a plot from the compute engine.
            windows[lastAnimation]->GetPlotList()->BackwardStep();

            // Send new window information to the client if we're animating
            // the active window.
            UpdateWindowInformation(WINDOWINFO_ANIMATION, lastAnimation);

            // Process any client input that we had to ignore while reading
            // the plot from the compute engine.
            viewerSubject->ProcessFromParent();
        }

        // Start the timer up again.
        timer->blockSignals(false);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::GetPrinterClientAtts
//
// Purpose: 
//   Returns a pointer to the printer attributes.
//
// Returns:    A pointer to the printer attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 14:41:29 PST 2002
//   
// ****************************************************************************

PrinterAttributes *
ViewerWindowManager::GetPrinterClientAtts()
{
    //
    // If the printer attributes haven't been allocated then do so.
    //
    if (printerAtts == 0)
    {
        printerAtts = new PrinterAttributes;
    }

    return printerAtts;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetRenderingAttributes
//
// Purpose: 
//   Returns a pointer to the rendering attributes.
//
// Returns:    A pointer to the rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 15:22:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

RenderingAttributes *
ViewerWindowManager::GetRenderingAttributes()
{
    if(renderAtts == 0)
       renderAtts = new RenderingAttributes;

    return renderAtts;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetAnnotationObjectList
//
// Purpose: 
//   Returns a pointer to the annotation object list.
//
// Returns:    A pointer to the annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:20:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AnnotationObjectList *
ViewerWindowManager::GetAnnotationObjectList()
{
    if(annotationObjectList == 0)
        annotationObjectList = new AnnotationObjectList;

    return annotationObjectList;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetDefaultAnnotationObjectList
//
// Purpose: 
//   Returns a pointer to the default annotation object list.
//
// Returns:    A pointer to the annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:20:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AnnotationObjectList *
ViewerWindowManager::GetDefaultAnnotationObjectList()
{
    if(defaultAnnotationObjectList == 0)
        defaultAnnotationObjectList = new AnnotationObjectList;

    return defaultAnnotationObjectList;
}

// ****************************************************************************
// Method: ViewerWindowManager::SetDefaultAnnotationObjectListFromClient
//
// Purpose: 
//   Copies the client annotation object list into the default annotation
//   object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:04:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::SetDefaultAnnotationObjectListFromClient()
{
    *(GetDefaultAnnotationObjectList()) = *(GetAnnotationObjectList());
}

// ****************************************************************************
// Method: ViewerWindowManager::GetLineoutWindow
//
// Purpose:    
//   Returns a pointer to a window that can be used for lineout. 
//   Adds a new window if necessary.
//
// Returns:
//   The lineout window, null if it couldn't be created.. 
//
// Programmer: Kathleen Bonnell
// Creation:   June 10, 2002 
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 23 15:25:22 PDT 2002  
//   Initialize the lineout window as a curve.
//
//   Eric Brugger, Wed Apr  9 09:48:25 PDT 2003
//   I added code to mark the window as referenced so that the window
//   attributes would not be copied on first reference.
//
//   Eric Brugger, Fri Apr 11 14:03:19 PDT 2003
//   I modified the routine to use SimpleAddWindow instead of AddWindow
//   to create a new window if it needs to do so.  This eliminates the
//   possibility of copying window attributes and doesn't make the new
//   window active.
//
//   Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//   Removed call to SetTypeIsCurve since it is no longer necessary.
//
//   Brad Whitlock, Mon Jan 26 22:50:15 PST 2004
//   Made it use the plot list directly.
//
//   Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//   Added optional bool to immediately return NULL if the window doesn't
//   already exist
//
//   Kathleen Bonnell, Thu Jul 22 15:20:29 PDT 2004 
//   Removed bool arg (no longer needed due to changes from ViewerQueryManager)
//   added int arg to specify a particular window to return (or create). 
//   Added logic to return specified window (if useThisId not -1).
//   
// ****************************************************************************

ViewerWindow *
ViewerWindowManager::GetLineoutWindow(int useThisId)
{
    int returnId = lineoutWindow;
    if (useThisId != -1)
    {
        // 
        // Use or create the requested window.
        // 
        if (useThisId >= maxWindows || windows[useThisId] == 0)
        {
            int newWin = SimpleAddWindow();
            if (newWin == -1)
            {
                Error("VisIt could not open a window for Lineout because "
                      "the maximum number of windows was exceeded.");
                return NULL;
            }
            SetWindowAttributes(newWin, false);
            returnId = newWin;
            windows[returnId]->SetInteractionMode(NAVIGATE);
            referenced[returnId] = true;
        }
        else
        {
            returnId = useThisId;
        }

    }
    else if (lineoutWindow == -1)
    {
        //
        //  Search for an open, empty window.  If none exists,
        //  create one && designate it as the default Lineout window.
        //
        int       winIdx;
        for (winIdx = 0; winIdx < maxWindows; winIdx++)
        {
            if (windows[winIdx] == 0)
            {
                winIdx = -1;
                break;
            }
            if (windows[winIdx]->GetPlotList()->GetNumPlots() == 0)
            {
                break;
            }
        }
        if (winIdx == -1)
        {
            lineoutWindow = SimpleAddWindow();
            SetWindowAttributes(lineoutWindow, false);
        }
        else if (winIdx < maxWindows)
        {
            lineoutWindow = winIdx;
        }
        else
        {
            Error("VisIt could not open a window for Lineout because the "
                  "maximum number of windows was exceeded.");
            return NULL;
        }
        windows[lineoutWindow]->SetInteractionMode(NAVIGATE);
        referenced[lineoutWindow] = true;
        returnId = lineoutWindow;
    }
    return windows[returnId];
}


// ****************************************************************************
// Method: ViewerWindowManager::ResetLineoutDesignation
//
// Purpose:    Turns off the lineout designation for the active window
//             if it was previously set.  This occurs if all the curve plots
//             have been removed from the window, or the window has been deleted.
//
// Programmer: Kathleen Bonnell
// Creation:   May 7, 2002 
//
// Modifications:
//   Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//   Removed call to SetTypeIsCurve since it is no longer necessary.
//
// ****************************************************************************

void ViewerWindowManager::ResetLineoutDesignation(int winIndex)
{
    if (lineoutWindow == -1)
    {
        return;
    }

    if ((winIndex == -1 && activeWindow == lineoutWindow) ||
         winIndex == lineoutWindow)
    {
        lineoutWindow = -1;
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::GetWindowIndices
//
// Purpose: 
//   Returns an array that contains the list of valid window indices. The
//   caller is responsible for freeing the memory.
//
// Arguments:
//   nwin : A pointer to a variable that contains the number of window indices.
//
// Returns:    An array of window indices.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:23:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int *
ViewerWindowManager::GetWindowIndices(int *nwin) const
{
    int id = 0;
    int *indices = new int[maxWindows];

    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            indices[id++] = i;
    }

    *nwin = id;
    return indices;
}

// ****************************************************************************
// Method: ViewerWindowManager::GetTimeLockedWindowIndices
//
// Purpose: 
//   Gets the indices of the locked windows.
//
// Arguments:
//   windowIds : The return vector for the locked window indices.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 16 17:35:53 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::GetTimeLockedWindowIndices(intVector &windowIds) const
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0 && windows[i]->GetTimeLock() &&
           windows[i]->GetPlotList()->HasActiveTimeSlider())
            windowIds.push_back(i);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::GetNumWindows
//
// Purpose: 
//   Returns the number of windows.
//
// Returns:    The number of windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 12:47:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
ViewerWindowManager::GetNumWindows() const
{
    return nWindows;
}

// ****************************************************************************
// Method: ViewerWindowManager::UpdateActions
//
// Purpose: 
//   Updates the actions in all windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 13:13:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::UpdateActions()
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            windows[i]->GetActionManager()->UpdateSingleWindow();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::HideToolbarsForAllWindows
//
// Purpose: 
//   Hides the toolbar in all windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 13:13:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::HideToolbarsForAllWindows()
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            windows[i]->GetToolbar()->HideAll();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::ShowToolbarsForAllWindows
//
// Purpose: 
//   Shows the toolbar in all windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 13:13:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::ShowToolbarsForAllWindows()
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            windows[i]->GetToolbar()->ShowAll();
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::SetLargeIcons
//
// Purpose: 
//   Tells all windows to use large icons.
//
// Arguments:
//   val : Whether the windows should use large icons.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:38:36 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::SetLargeIcons(bool val)
{
    windowAtts->SetLargeIcons(val);

    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            windows[i]->SetLargeIcons(val);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::UsesLargeIcons
//
// Purpose: 
//   Returns whether the windows should use large icons.
//
// Returns:    Whether the windows should use large icons.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 14:30:23 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerWindowManager::UsesLargeIcons() const
{
    return windowAtts->GetLargeIcons();
}

// ****************************************************************************
// Method: ViewerWindowManager::BeginEngineExecute
//
// Purpose: 
//   Tells the client that an engine is executing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 15:00:11 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Apr 15 16:23:02 PST 2005
//   Removed code to disable popup menus.
//
// ****************************************************************************

void
ViewerWindowManager::BeginEngineExecute()
{
    //
    // Tell the client that an engine is executing.
    //
    clientAtts->SetExecuting(true);
    UpdateGlobalAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::EndEngineExecute
//
// Purpose: 
//   Tells the client that an engine is done executing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 15:00:11 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Apr 15 16:23:02 PST 2005
//   Removed code to enable popup menus.
//   
// ****************************************************************************

void
ViewerWindowManager::EndEngineExecute()
{
    //
    // Tell the client that an engine is done executing.
    //
    clientAtts->SetExecuting(false);
    UpdateGlobalAtts();
}

// ****************************************************************************
// Method: ViewerWindowManager::DisableExternalRenderRequestsAllWindows
//
// Purpose: Temporarily disables external render requests in all windows.
//
// Arguments:
//   oldAble: a vector of bools indicating the previous enabled state of each
//   window's external render requests
//
// Programmer: Mark C. Miller
// Creation:   November 11, 2003
//   
// ****************************************************************************

void
ViewerWindowManager::DisableExternalRenderRequestsAllWindows(
    std::vector<bool>& oldAble)
{
    for (int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if (windows[windowIndex] != 0)
        {
            oldAble.push_back(windows[windowIndex]->DisableExternalRenderRequests());
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::EnableExternalRenderRequestsAllWindows
//
// Purpose: Re-enable external render requests in all windows.
//
// Arguments:
//   oldAble: a vector of bools indicating the previous enabled state of each
//   window's external render requests
//
// Programmer: Mark C. Miller
// Creation:   November 11, 2003
//   
// ****************************************************************************

void
ViewerWindowManager::EnableExternalRenderRequestsAllWindows(
    const std::vector<bool> oldAble)
{
    for (int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if (windows[windowIndex] != 0)
        {
            if (oldAble[windowIndex])
                windows[windowIndex]->EnableExternalRenderRequests();
        }
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::CreateNode
//
// Purpose: 
//   Lets the ViewerWindowManager save itself to a config file.
//
// Arguments:
//   parentNode : The node to which we're adding information.
//   detailed   : A flag that tells whether we should write detailed info.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 12:58:36 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 14:20:27 PST 2003
//   Added information for a full restart.
//
//   Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//   Added writing out of lineout window.
//
//   Brad Whitlock, Tue Oct 21 14:54:51 PST 2003
//   I prevented the activeWindow and the lineoutWindow from being saved
//   if we're not producing a detailed log.
//
//   Eric Brugger, Fri Dec  5 13:42:39 PST 2003
//   Added writing of maintainView, cameraView and viewExtentsType.
//
//   Eric Brugger, Mon Mar 29 15:21:11 PST 2004
//   Added writing of maintainData.
//
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004 
//    Added timeQueryWindow. 
//
// ****************************************************************************

void
ViewerWindowManager::CreateNode(DataNode *parentNode, bool detailed)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = new DataNode("ViewerWindowManager");
    parentNode->AddNode(mgrNode);

    //
    // Add information about the ViewerWindowManager.
    //
    if(detailed)
    {
        mgrNode->AddNode(new DataNode("activeWindow", activeWindow));
        mgrNode->AddNode(new DataNode("lineoutWindow", lineoutWindow));
        mgrNode->AddNode(new DataNode("timeQueryWindow", timeQueryWindow));
    }

    //
    // The following attributes are actually per window, but are being
    // treated as global so that they are saved when saving settings.
    //
    mgrNode->AddNode(new DataNode("maintainView",
       windows[activeWindow]->GetMaintainViewMode()));
    mgrNode->AddNode(new DataNode("maintainData",
       windows[activeWindow]->GetMaintainDataMode()));
    mgrNode->AddNode(new DataNode("cameraView",
       windows[activeWindow]->GetCameraViewMode()));
    mgrNode->AddNode(new DataNode("viewExtentsType",
       avtExtentType_ToString(windows[activeWindow]->GetViewExtentsType())));

    //
    // Let each window add its own data.
    //
    DataNode *windowsNode = new DataNode("Windows");
    mgrNode->AddNode(windowsNode);
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
            windows[i]->CreateNode(windowsNode, detailed);
    }
}

// ****************************************************************************
// Method: ViewerWindowManager::SetFromNode
//
// Purpose: 
//   Lets the ViewerWindowManager initialize itself from the information stored
//   in a config file's DataNode.
//
// Arguments:
//   parentNode : The node from which to get information about how to initialize
//                the ViewerWindowManager.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 12:56:30 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Jul 17 14:25:02 PST 2003
//   Added code to reconstruct all of the windows in the config file.
//
//   Eric Brugger, Wed Aug 20 13:22:14 PDT 2003
//   Removed call to SetTypeIsCurve since it is no longer necessary.
//
//   Brad Whitlock, Mon Aug 25 11:42:49 PDT 2003
//   Added code to temporarily disable "clone window on first reference" when
//   setting the active window once all the windows are established.
//
//   Brad Whitlock, Tue Dec 2 17:13:31 PST 2003
//   Added code to change the referenced flag so windows with plots are not
//   considered unreferenced.
//
//   Eric Brugger, Fri Dec  5 13:42:39 PST 2003
//   Added loading of maintainView, cameraView and viewExtentsType.
//
//   Eric Brugger, Thu Dec 18 14:40:20 PST 2003
//   Modified the routine to set the referenced flag for the active window
//   to true.
//
//   Eric Brugger, Mon Mar 29 15:21:11 PST 2004
//   Added loading of maintainData.
//
//   Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004 
//   Added timeQueryWindow. 
//
//   Brad Whitlock, Tue Aug 3 11:42:48 PDT 2004
//   I added code to override the window sizes in the session file if the
//   viewer is run in -nowin mode.
//
// ****************************************************************************

void
ViewerWindowManager::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ViewerWindowManager");
    if(searchNode == 0)
        return;

    //
    // Load information specific to ViewerWindowManager.  The following
    // attributes are actually per window, but are being treated as
    // global so that they are saved when saving settings.
    //
    DataNode *node = 0;
    if((node = searchNode->GetNode("cameraView")) != 0)
        windows[activeWindow]->SetCameraViewMode(node->AsBool());
    if((node = searchNode->GetNode("maintainView")) != 0)
        windows[activeWindow]->SetMaintainViewMode(node->AsBool());
    if((node = searchNode->GetNode("maintainData")) != 0)
        windows[activeWindow]->SetMaintainDataMode(node->AsBool());
    if((node = searchNode->GetNode("viewExtentsType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            ival = (ival < 0 || ival > 3) ? 0 : ival;
            windows[activeWindow]->SetViewExtentsType(avtExtentType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            avtExtentType value;
            if(avtExtentType_FromString(node->AsString(), value))
                windows[activeWindow]->SetViewExtentsType(value);
        }
    }

    //
    // Create the right number of windows.
    //
    DataNode *windowsNode = parentNode->GetNode("Windows");
    if(windowsNode == 0)
        return;

    int i, c;
    DataNode *sizeNode = 0;
    DataNode *locationNode = 0;

    //
    // Get the number of viewer windows that we've found in the Windows node.
    // Print to the debug logs if we get an object that is *not* a 
    // ViewerWindow.
    //
    int newNWindows = 0;
    DataNode **vWindowNodes = windowsNode->GetChildren();
    for(int k = 0; k < windowsNode->GetNumChildren(); ++k)
    {
        if(vWindowNodes[k]->GetKey() == "ViewerWindow")
            ++newNWindows;
        else
        {
            debug1 << "ViewerWindowManager::SetFromNode: Bad Window Name!"
                   << "name=" << vWindowNodes[k]->GetKey().c_str() << endl;
        }
    }

    if(nWindows > newNWindows)
    {
        int d = nWindows - newNWindows;
        for(i = 0; i < d; ++i)
            DeleteWindow();

        //
        // Create an array of pointers to the existing windows such that
        // the pointer array has no gaps.
        //
        ViewerWindow **existingWindows = new ViewerWindow *[nWindows + 1];
        for(i = 0, c = 0; i < maxWindows; ++i)
        {
           if(windows[i] != 0)
               existingWindows[c++] = windows[i];
        }

        //
        // Try and resize or reposition the existing windows.
        //
        for(i = 0; i < newNWindows; ++i)
        {
            DataNode *windowINode = windowsNode->GetChildren()[i];
            if((sizeNode = windowINode->GetNode("windowSize")) != 0 &&
               (locationNode = windowINode->GetNode("windowLocation")) != 0)
            {
                // We're able to read in the size and location.
                int  w, h, x, y;
                if(ViewerWindow::GetNoWinMode())
                {
                    x = y = 0;
                    w = windowLimits[0][0].width;
                    h = windowLimits[0][0].height;
                }
                else
                {
                    w = sizeNode->AsIntArray()[0];
                    h = sizeNode->AsIntArray()[1];
                    x = locationNode->AsIntArray()[0];
                    y = locationNode->AsIntArray()[1];
                }

                // If we're considering an existing window, just set the
                // size and position.
                existingWindows[i]->SetSize(w, h);
                existingWindows[i]->SetLocation(x, y);
            }
        }

        delete [] existingWindows;
    }
    else if(nWindows < newNWindows)
    {
        //
        // Create an array of pointers to the existing windows such that
        // the pointer array has no gaps.
        //
        ViewerWindow **existingWindows = new ViewerWindow *[nWindows + 1];
        for(i = 0, c = 0; i < maxWindows; ++i)
        {
           if(windows[i] != 0)
               existingWindows[c++] = windows[i];
        }

        //
        // Loop over the saved windows either using their information to
        // resize existing windows or to create new windows.
        //
        int numExistingWindows = nWindows;
        for(i = 0; i < newNWindows; ++i)
        {
            //
            // Read the location and size for the window.
            //
            DataNode *windowINode = windowsNode->GetChildren()[i];
            if((sizeNode = windowINode->GetNode("windowSize")) != 0 &&
               (locationNode = windowINode->GetNode("windowLocation")) != 0)
            {
                // We're able to read in the size and location.
                int  w, h, x, y;
                if(ViewerWindow::GetNoWinMode())
                {
                    x = y = 0;
                    w = windowLimits[0][0].width;
                    h = windowLimits[0][0].height;
                }
                else
                {
                    w = sizeNode->AsIntArray()[0];
                    h = sizeNode->AsIntArray()[1];
                    x = locationNode->AsIntArray()[0];
                    y = locationNode->AsIntArray()[1];
                }

                // If we're considering an existing window, just set the
                // size and position.
                if(i < numExistingWindows)
                {
                    existingWindows[i]->SetSize(w, h);
                    existingWindows[i]->SetLocation(x, y);
                }
                // We have the size for a window that does not exist yet so
                // create the vis window with the correct size.
                else
                {
                    for(int windowIndex = 0;
                        windowIndex < maxWindows;
                        ++windowIndex)
                    {
                        if(windows[windowIndex] == 0)
                        {
                            // Create the vis window so that it has the
                            // right size and location.
                            CreateVisWindow(windowIndex, w, h, x, y);

                            // HACK - set the location again because it could
                            // be shifted a little by some window managers.
                            windows[windowIndex]->SetLocation(x, y);
                            break;
                        }
                    }
                }
            }
            else if(nWindows < newNWindows)
                AddWindow(false);
        }

        delete [] existingWindows;
    }

    //
    // Load window-specific information.
    //
    DataNode **wNodes = windowsNode->GetChildren();
    int childCount = 0;
    for(i = 0; i < maxWindows; ++i)
    {
        referenced[i] = false;
        if(windows[i] != 0 && childCount < newNWindows)
        {
            windows[i]->SetFromNode(wNodes[childCount++]);
            if(windows[i]->GetPlotList()->GetNumPlots() > 0)
                referenced[i] = true;
        }
    }

    //
    // Set the active window.
    //
    if((node = searchNode->GetNode("activeWindow")) != 0)
    {
        int n = node->AsInt();
        if(n >= 0 && n < nWindows && windows[n] != 0)
        {
            referenced[n] = true;
            SetActiveWindow(n + 1);
        }
        else
        {
            referenced[activeWindow] = true;
            UpdateAllAtts();
        }
    }
    else
    {
        referenced[activeWindow] = true;
        UpdateAllAtts();
    }

    //
    // Set the lineout window.
    //
    if((node = searchNode->GetNode("lineoutWindow")) != 0)
    {
        int n = node->AsInt();
        if (n < 0 || n >= maxWindows)
            lineoutWindow = -1;
        else
            lineoutWindow = n;
    }

    //
    // Set the timeQuery window.
    //
    if((node = searchNode->GetNode("timeQueryWindow")) != 0)
    {
        int n = node->AsInt();
        if (n < 0 || n >= maxWindows)
            timeQueryWindow = -1;
        else
            timeQueryWindow = n;
    }
}


// ****************************************************************************
// Method: ViewerWindowManager::GetEmptyWindow
//
// Purpose:    
//   Returns a pointer to an empy window. 
//
// Returns:
//   The empty window, null if it couldn't be created.. 
//
// Programmer: Kathleen Bonnell
// Creation:   March 19, 2004 
//
// Modifications:
//
// ****************************************************************************

ViewerWindow *
ViewerWindowManager::GetEmptyWindow() 
{
    //
    //  Search for an open, empty window.  If none exists,
    //  create one.
    //
    int       winIdx;
    for (winIdx = 0; winIdx < maxWindows; winIdx++)
    {
        if (windows[winIdx] == 0)
        {
                winIdx = -1;
                break;
        }
        if (windows[winIdx]->GetPlotList()->GetNumPlots() == 0)
        {
                break;
        }
    }
    if (winIdx == -1)
    {
        winIdx = SimpleAddWindow();
        if (winIdx == -1)
        {
            Error("The maximum number of windows are already being used.");
            return NULL;
        }
        SetWindowAttributes(winIdx, false);
    }
    else if (winIdx >= maxWindows)
    {
        Error("The maximum number of windows are already being used.");
        return NULL;
    }
    windows[winIdx]->SetInteractionMode(NAVIGATE);
    referenced[winIdx] = true;
    return windows[winIdx];
}


// ****************************************************************************
// Method: ViewerWindowManager::GetWindow
//
// Purpose:    
//   Returns a pointer to the window specified by windowIndex. 
//
// Arguments:
//   windowIndex  The index of the window to return.
//
// Returns:
//   The specified window, null if the index is invalid.
//
// Programmer: Kathleen Bonnell
// Creation:   March 19, 2004 
//
// Modifications:
//
// ****************************************************************************

ViewerWindow *
ViewerWindowManager::GetWindow(int windowIndex)
{
    if(windowIndex < 0 || windowIndex >= maxWindows)
        return NULL;

    return windows[windowIndex];
}


// ****************************************************************************
// Method: ViewerWindowManager::GetTimeQueryWindow
//
// Purpose:    
//   Returns a pointer to a window that can be used for queries-through-time. 
//   Adds a new window if necessary.
//
// Returns:
//   The timequery window, null if it couldn't be created.. 
//
// Programmer: Kathleen Bonnell
// Creation:   April 1, 2004 
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 20 10:47:26 PDT 2004
//   Added optional arg specifiying the windowId to retrieve.
//
// ****************************************************************************

ViewerWindow *
ViewerWindowManager::GetTimeQueryWindow(int useThisId) 
{
    int returnId = timeQueryWindow;
    if (useThisId != -1)
    {
        if (useThisId >= maxWindows || windows[useThisId] == 0)
        {
            int newWin = SimpleAddWindow();
            if (newWin == -1)
            {
                Error("VisIt could not open a window for TimeQuery because "
                      "the maximum number of windows was exceeded.");
                return NULL;
            } 
            SetWindowAttributes(newWin, false);
            returnId = newWin;
            windows[returnId]->SetInteractionMode(NAVIGATE);
            referenced[returnId] = true;
        }
        else 
        {
            returnId = useThisId; 
        }
    }
    else if (timeQueryWindow == -1)
    {
        //
        //  Search for an open, empty window.  If none exists,
        //  create one.
        //
        int       winIdx;
        for (winIdx = 0; winIdx < maxWindows; winIdx++)
        {
            if (windows[winIdx] == 0)
            {
                winIdx = -1;
                break;
            }
            if (windows[winIdx]->GetPlotList()->GetNumPlots() == 0)
            {
                break;
            }
        }
        if (winIdx == -1)
        {
            timeQueryWindow = SimpleAddWindow();
            SetWindowAttributes(timeQueryWindow, false);
        }
        else if (winIdx < maxWindows)
        {
            timeQueryWindow = winIdx;
        }
        else
        {
            Error("VisIt could not open a window for TimeQuery because the "
                  "maximum number of windows was exceeded.");
            return NULL;
        }
        windows[timeQueryWindow]->SetInteractionMode(NAVIGATE);
        referenced[timeQueryWindow] = true;
        returnId = timeQueryWindow;
    }
    return windows[returnId];
}


// ****************************************************************************
// Method: ViewerWindowManager::ResetTimeQueryDesignation
//
// Purpose:    Turns off the timequery designation for the active window
//             if it was previously set.  This occurs if all the curve plots
//             have been removed from the window, or the window has been deleted.
//
// Programmer: Kathleen Bonnell
// Creation:   April 1, 2004 
//
// Modifications:
//
// ****************************************************************************

void ViewerWindowManager::ResetTimeQueryDesignation(int winIndex)
{
    if (timeQueryWindow == -1)
    {
        return;
    }

    if ((winIndex == -1 && activeWindow == timeQueryWindow) ||
         winIndex == timeQueryWindow)
    {
        timeQueryWindow = -1;
    }
}


// ****************************************************************************
//  Method: ViewerWindowManager::GetInteractorClientAtts
//
//  Purpose: 
//    Returns a pointer to the interactor attributes.
//
//  Returns:    A pointer to the interactor attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 16, 2004 
//
// ****************************************************************************

InteractorAttributes *
ViewerWindowManager::GetInteractorClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (interactorClientAtts == 0)
    {
        interactorClientAtts = new InteractorAttributes;
    }

    return interactorClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetInteractorDefaultAtts
//
//  Purpose: 
//    Returns a pointer to the default interactor attributes.
//
//  Returns:    A pointer to the default interactor attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 16, 2004
//
// ****************************************************************************

InteractorAttributes *
ViewerWindowManager::GetInteractorDefaultAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (interactorDefaultAtts == 0)
    {
        interactorDefaultAtts = new InteractorAttributes;
    }

    return interactorDefaultAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetClientInteractorAttsFromDefault
//
//  Purpose: 
//    This method copies the default interactor attributes into the client
//    interactor attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//
// ****************************************************************************

void
ViewerWindowManager::SetClientInteractorAttsFromDefault()
{
    if(interactorDefaultAtts != 0 && interactorClientAtts != 0)
    {
        *interactorClientAtts = *interactorDefaultAtts;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetDefaultInteractorAttsFromClient
//
//  Purpose: 
//    This method copies the client's interactor attributes into the default
//    interactor attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 16, 2004 
//
// ****************************************************************************

void
ViewerWindowManager::SetDefaultInteractorAttsFromClient()
{
    if(interactorDefaultAtts != 0 && interactorClientAtts != 0)
    {
        *interactorDefaultAtts = *interactorClientAtts;
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetInteractorAttsFromClient
//
//  Purpose: 
//    Sets the interactor attributes for the active window based on the
//    client's interactor attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerWindowManager::SetInteractorAttsFromClient()
{
    for (int i = 0; i < maxWindows; i++)
    {
        if (windows[i] != 0)
            windows[i]->SetInteractorAtts(interactorClientAtts);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::SetInteractorAttsFromDefault
//
//  Purpose: 
//    Sets the interactor attributes for the active window based on the
//    default interactor attributes.
//
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//
// ****************************************************************************

void
ViewerWindowManager::SetInteractorAttsFromDefault()
{
    for (int i = 0; i < maxWindows; i++)
    {
        if (windows[i] != 0)
            windows[i]->SetInteractorAtts(interactorDefaultAtts);
    }
    //
    // Update the client's annotation attributes
    //
    *interactorClientAtts = *interactorDefaultAtts;
    interactorClientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateInteractorAtts
//
//  Purpose: 
//    Sends the interactor attributes for the active window to the client.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerWindowManager::UpdateInteractorAtts()
{
    ViewerWindow *win = windows[activeWindow];
    const InteractorAttributes *winAtts = win->GetInteractorAtts();

    //
    // Copy the window's interactor attributes to the client interactor
    // attributes and notify the client.
    //
    *interactorClientAtts = *winAtts;
    interactorClientAtts->Notify();
}

