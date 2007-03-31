// ************************************************************************* //
//                            ViewerWindowManager.C                          //
// ************************************************************************* //
#include <stdio.h> // for sscanf

#include <ViewerWindowManager.h>

#include <iostream.h>
#include <string.h>
#include <snprintf.h>
#if !defined(_WIN32)
#include <signal.h> // for signal
#endif

#include <AnimationAttributes.h>
#include <DataNode.h>
#include <GlobalAttributes.h>
#include <KeyframeAttributes.h>
#include <LightList.h>
#include <LightAttributes.h>
#include <PrinterAttributes.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <ViewAttributes.h>
#include <WindowInformation.h>
#include <ViewerActionManager.h>
#include <ViewerEngineManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManagerAttributes.h>
#include <ViewerAnimation.h>
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
#include <avtImage.h>
#include <avtFileWriter.h>
#include <avtToolInterface.h>
#include <ImproperUseException.h>

#include <vtkQtImagePrinter.h>

#include <qtimer.h>
#include <qprinter.h>
#include <DebugStream.h>
#include <ViewerOperatorFactory.h>

using std::string;

//
// Storage for static data elements.
//
ViewerWindowManager *ViewerWindowManager::instance=0;
const int ViewerWindowManager::maxWindows=16;
const int ViewerWindowManager::maxLayouts=6;
const int ViewerWindowManager::validLayouts[]={1, 4, 9, 16, 2, 8};
GlobalAttributes *ViewerWindowManager::clientAtts=0;
SaveWindowAttributes *ViewerWindowManager::saveWindowClientAtts=0;
ViewAttributes *ViewerWindowManager::view2DClientAtts=0;
ViewAttributes *ViewerWindowManager::view3DClientAtts=0;
AnimationAttributes *ViewerWindowManager::animationClientAtts=0;
AnnotationAttributes *ViewerWindowManager::annotationClientAtts=0;
AnnotationAttributes *ViewerWindowManager::annotationDefaultAtts=0;
KeyframeAttributes *ViewerWindowManager::keyframeClientAtts=0;
LightList *ViewerWindowManager::lightListClientAtts=0;
LightList *ViewerWindowManager::lightListDefaultAtts=0;
ViewerWindowManagerAttributes *ViewerWindowManager::windowAtts=0;
PrinterAttributes *ViewerWindowManager::printerAtts=0;
WindowInformation *ViewerWindowManager::windowInfo=0;
RenderingAttributes *ViewerWindowManager::renderAtts=0;

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
    windowLimits[3] = new WindowLimits[16];
    windowLimits[4] = new WindowLimits[2];
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
    view2DStackTop = -1;
    view3DStackTop = -1;

    lineoutWindow = -1;
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
        windows[windowIndex]->CopyGeneralAttributes(windows[activeWindow]);
        windows[windowIndex]->CopyAnnotationAttributes(windows[activeWindow]);
        windows[windowIndex]->CopyLightList(windows[activeWindow]);
        windows[windowIndex]->CopyViewAttributes(windows[activeWindow]);
        windows[windowIndex]->CopyAnimation(windows[activeWindow]);
    }
    referenced[windowIndex] = true;

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
//     Brad Whitlock, Tue Nov 7 09:47:31 PDT 2000
//     Changed to reflect that ViewerAnimation is now in ViewerWindow.
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
        if(to == activeWindow)
            UpdateAnnotationAtts();
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
// Method: ViewerWindowManager::CopyAnimationToWindow
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
//   
// ****************************************************************************

void
ViewerWindowManager::CopyAnimationToWindow(int from, int to)
{
    if(from < 0 || from >= maxWindows)
        return;
    if(to < 0 || to >= maxWindows)
        return;

    // If the Window pointers are valid then perform the operation.
    if(windows[from] != 0 && windows[to] != 0)
    {
        windows[to]->CopyAnimation(windows[from]);
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
//    Added call to ResetLineoutDesignator. 
//    
//    Hank Childs, Wed Jul 10 21:46:55 PDT 2002
//    Unlock a window before deleting it.
//
//    Kathleen Bonnell, Wed Jul 31 16:43:43 PDT 2002  
//    Notify ViewerQueryManager that a window is being deleted. 
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
    if (windows[windowIndex]->GetViewIsLocked())
    {
        ToggleLockViewMode(windowIndex);
    }

    //
    // Delete the active animation and window.
    //
    delete windows[windowIndex];
    windows[windowIndex] = 0;
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
//   
// ****************************************************************************

bool
ViewerWindowManager::FileInUse(const char *host, const char *dbName) const
{
    for(int i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0)
        {
            if(windows[i]->GetAnimation()->GetPlotList()->FileInUse(host, dbName))
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
//  Method: ViewerWindowManager::SaveWindow
//
//  Purpose: 
//    Saves the window with the specified index.
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

    // Ask the fileWriter to create a filename.
    char *filename = fileWriter->CreateFilename(
        saveWindowClientAtts->GetFileName().c_str(),
        saveWindowClientAtts->GetFamily());

    //
    // Send a status message about starting to save the image and make the
    // status message display for 10 minutes.
    //
    char message[1000];
    SNPRINTF(message, 1000, "Saving window %d...",
            (windowIndex == -1) ? (activeWindow + 1) : (windowIndex + 1));
    Status(message, 6000000);

    avtDataObject_p dob = NULL;
    if (fileWriter->IsImageFormat())
    {
        avtImage_p image = NULL;
        if(saveWindowClientAtts->GetSaveTiled())
        {
            // Create a tiled image.
            image = CreateTiledImage(saveWindowClientAtts->GetWidth(),
                                     saveWindowClientAtts->GetHeight());
        }
        else
        {
            // Create a single image.
            image = CreateSingleImage(windowIndex,
                                      saveWindowClientAtts->GetWidth(),
                                      saveWindowClientAtts->GetHeight(),
                                     saveWindowClientAtts->GetScreenCapture());
        }
        CopyTo(dob, image);
    }
    else
    {
        avtDataset_p ds = GetDataset(windowIndex);
        if (*ds != NULL)
            ds->Compact();
        CopyTo(dob, ds);
    }

    // Save the window.
    bool savedWindow = true;
#if 0
    if(saveWindowClientAtts->GetHostName() == "localhost")
    {
#endif
        TRY
        {
            // Tell the writer to save the window on the viewer.
            fileWriter->Write(filename, dob,saveWindowClientAtts->GetQuality(),
                              saveWindowClientAtts->GetProgressive(),
                              saveWindowClientAtts->GetBinary());
        }
        CATCH2(VisItException, ve)
        {
            Warning(ve.Message().c_str());
            savedWindow = false;
        }
        ENDTRY
#if 0
    }
    else
    {
        // Make the Engine manager send the image back to the appropriate
        // engine so the image can be written to disk there.
        ViewerEngineManager::Instance()->WriteDataObject(
            saveWindowClientAtts->GetHostName(), filename, dob,
            saveWindowClientAtts->GetFormat());
    }
#endif

    // Send a message to indicate that we're done saving the image.
    if (savedWindow)
    {
        SNPRINTF(message, 1000, "Saved %s", filename);
        Status(message);
        Message(message);
    }

    saveWindowClientAtts->SetLastRealFilename(filename);
    saveWindowClientAtts->Notify();

    // Delete the filename memory.
    delete [] filename;
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
//
//  Returns:    An avtImage representation of the specified VisWindow.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 13 15:17:32 PST 2001
//
// ****************************************************************************

avtImage_p
ViewerWindowManager::CreateSingleImage(int windowIndex,
    int /*width*/, int /*height*/, bool /*screenCapture*/)
{
    int        index = (windowIndex == -1) ? activeWindow : windowIndex;
    avtImage_p retval = NULL;

    if(windows[index] != 0)
    {
#if 0
        if(screenCapture)
            retval = windows[index]->ScreenCapture();
        else
        {
            debug1 << "Doing offscreen rendering of window " << index
                   << "..." << endl;
        }
#else
        // Just do screen capture since that is all we can do right now.
        retval = windows[index]->ScreenCapture();
#endif
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
// Creation:   Tue Feb 13 15:27:26 PST 2001
//
// ****************************************************************************

avtImage_p
ViewerWindowManager::CreateTiledImage(int /*width*/, int /*height*/)
{
    // Not yet implemented.
    return NULL;
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
// ****************************************************************************

avtDataset_p
ViewerWindowManager::GetDataset(int windowIndex)
{
    int          index = (windowIndex == -1 ? activeWindow : windowIndex);
    avtDataset_p rv    = NULL;

    if(windows[index] != 0)
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
        saveWindowClientAtts->GetScreenCapture());

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
        UpdateWindowInformation(index);
    }
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

    //
    // Set the 2D view for the active viewer window.
    //
    windows[activeWindow]->SetView2D(view2d);

    //
    // This will maintain our internal state and also make locked windows
    // get this view.
    //
    UpdateViewAtts(activeWindow, true, false);
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
    view3d.nearPlane = view3DClientAtts->GetNearPlane();
    view3d.farPlane = view3DClientAtts->GetFarPlane();
    view3d.imagePan[0] = view3DClientAtts->GetImagePan()[0];
    view3d.imagePan[1] = view3DClientAtts->GetImagePan()[1];
    view3d.imageZoom = view3DClientAtts->GetImageZoom();
    view3d.perspective = view3DClientAtts->GetPerspective();

    //
    // Set the 3D view for the active viewer window.
    //
    windows[activeWindow]->SetView3D(view3d);

    //
    // This will maintain our internal state and also make locked windows
    // get this view.
    //
    UpdateViewAtts(activeWindow, false, true);
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
//
// ****************************************************************************

void
ViewerWindowManager::ClearViewKeyframes()
{ 
    windows[activeWindow]->ClearViewKeyframes();

    //
    // Update the view keyframe list.
    //
    int i;
    int nIndices;
    const int *keyframeIndices = windows[activeWindow]->
                           GetViewKeyframeIndices(nIndices);
    vector<int> keyframeIndices2;
    for (i = 0; i < nIndices; i++)
    {
        keyframeIndices2.push_back(keyframeIndices[i]);
    }
    clientAtts->SetViewKeyframes(keyframeIndices2);

    clientAtts->Notify();
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
//
// ****************************************************************************

void
ViewerWindowManager::DeleteViewKeyframe(const int frame)
{ 
    windows[activeWindow]->DeleteViewKeyframe(frame);

    //
    // Update the view keyframe list.
    //
    int i;
    int nIndices;
    const int *keyframeIndices = windows[activeWindow]->
                           GetViewKeyframeIndices(nIndices);
    vector<int> keyframeIndices2;
    for (i = 0; i < nIndices; i++)
    {
        keyframeIndices2.push_back(keyframeIndices[i]);
    }
    clientAtts->SetViewKeyframes(keyframeIndices2);

    clientAtts->Notify();
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
//
// ****************************************************************************

void
ViewerWindowManager::MoveViewKeyframe(int oldFrame, int newFrame)
{ 
    windows[activeWindow]->MoveViewKeyframe(oldFrame, newFrame);

    //
    // Update the view keyframe list.
    //
    int i;
    int nIndices;
    const int *keyframeIndices = windows[activeWindow]->
                           GetViewKeyframeIndices(nIndices);
    vector<int> keyframeIndices2;
    for (i = 0; i < nIndices; i++)
    {
        keyframeIndices2.push_back(keyframeIndices[i]);
    }
    clientAtts->SetViewKeyframes(keyframeIndices2);

    clientAtts->Notify();
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
//
// ****************************************************************************

void
ViewerWindowManager::SetViewKeyframe()
{ 
    windows[activeWindow]->SetViewKeyframe();

    //
    // Update the view keyframe list.
    //
    int i;
    int nIndices;
    const int *keyframeIndices = windows[activeWindow]->
                           GetViewKeyframeIndices(nIndices);
    vector<int> keyframeIndices2;
    for (i = 0; i < nIndices; i++)
    {
        keyframeIndices2.push_back(keyframeIndices[i]);
    }
    clientAtts->SetViewKeyframes(keyframeIndices2);

    clientAtts->Notify();
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
        
        UpdateWindowInformation(index);
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
// ****************************************************************************

void
ViewerWindowManager::SetRenderingAttributes(int windowIndex)
{
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        bool updatesEnabled = windows[index]->UpdatesEnabled();
        windows[index]->DisableUpdates();
        windows[index]->SetAntialiasing(renderAtts->GetAntialiasing());
        windows[index]->SetSurfaceRepresentation((int)
            renderAtts->GetGeometryRepresentation());
        windows[index]->SetImmediateModeRendering(!renderAtts->GetDisplayLists());
        windows[index]->SetStereoRendering(renderAtts->GetStereoRendering(),
            (int)renderAtts->GetStereoType());
        windows[index]->SetNotifyForEachRender(renderAtts->GetNotifyForEachRender());
        windows[index]->SetScalableRendering(renderAtts->GetScalableRendering(), true);
        windows[index]->SetScalableThreshold(renderAtts->GetScalableThreshold());
        if (updatesEnabled)
           windows[index]->EnableUpdates();
        else
           windows[index]->RedrawWindow();
        UpdateRenderingAtts(index);
        UpdateWindowInformation(index);
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
        UpdateWindowInformation(index);
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
        UpdateWindowInformation(index);
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
        UpdateWindowInformation(index);
    }
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
        bool lockTime = windows[index]->GetTimeLock();
        windows[index]->SetTimeLock(!lockTime);
        if (windows[index]->GetTimeLock())
        {
            //
            // We have just locked time for this window. Find another
            // window that has locked time and copy its time.
            //
            int winner = -1;
            for (int i = 0; i < maxWindows; i++)
            {
                if (windows[i] != NULL && i != index)
                {
                    if (windows[i]->GetTimeLock())
                    {
                        winner = i;
                        break;
                    }
                }
            }

            if (winner != -1)
            {
                // Copy the frame index.
                int fi = windows[winner]->GetAnimation()->GetFrameIndex();
                if(windows[index]->GetAnimation()->SetFrameIndex(fi))
                {
                    // Copy the animation mode.
                    ViewerAnimation::AnimationMode mode;
                    mode = windows[winner]->GetAnimation()->GetMode();
                    if(mode == ViewerAnimation::PlayMode)
                        windows[index]->GetAnimation()->Play();
                    else if(mode == ViewerAnimation::ReversePlayMode)
                        windows[index]->GetAnimation()->ReversePlay();
                    else
                        windows[index]->GetAnimation()->Stop();

                    UpdateGlobalAtts();
                }
                else
                {
                    windows[index]->SetTimeLock(false);
                    char msg[500];
                    SNPRINTF(msg, 500, "Window %d could not lock times "
                        "because it does not have the same number frames as "
                        "window %d.", index+1, winner+1);
                    Error(msg);
                }
            }
        }

        UpdateWindowInformation(index);
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
        UpdateWindowInformation(index);
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
        UpdateWindowInformation(index);
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
        UpdateViewAtts(index, false, true);
        UpdateWindowInformation(index);
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
        UpdateViewAtts(index, false, true);
        UpdateWindowInformation(index);
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
        if(windows[index]->GetViewDimension() == 3)
        {
            // Pop the top off of the stack
            if(view3DStackTop > 0)
            {
                --view3DStackTop;
                windows[index]->SetView3D(view3DStack[view3DStackTop]);
            }
        }
        else
        {
            // Pop the top off of the stack
            if(view2DStackTop > 0)
            {
                --view2DStackTop;
                windows[index]->SetView2D(view2DStack[view2DStackTop]);
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
//    windowLayout  The window layout to use (1, 2, 4, 8, 9, or 16).
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

    //
    // Create at least "layout" windows layed out in the appropriate grid.
    // If there are already more windows than the layout calls for then
    // put the first "layout" window in the grid and layout the remaining
    // windows down a diagonal through the grid.
    //
    int       iWindow;
    int       nWindowsShort;
    int       nWindowsProcessed;

    nWindowsShort = layout - nWindows;
    nWindowsProcessed = 0;

    for (iWindow = 0; iWindow < maxWindows; iWindow++)
    {
        int       x, y;
        int       size;

        //
        // If the window exists, position it properly.
        //
        if (windows[iWindow] != 0)
        {
            if (nWindowsProcessed < layout)
            {
                windows[iWindow]->DeIconify();
                x    = windowLimits[layoutIndex][nWindowsProcessed].x;
                y    = windowLimits[layoutIndex][nWindowsProcessed].y;
                size = windowLimits[layoutIndex][nWindowsProcessed].width;
                windows[iWindow]->SetSize(size, size);
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
            x    = windowLimits[layoutIndex][iWindow].x;
            y    = windowLimits[layoutIndex][iWindow].y;
            size = windowLimits[layoutIndex][iWindow].width;

            CreateVisWindow(iWindow, size, size, x, y);
            SetWindowAttributes(iWindow, false);

            nWindowsProcessed++;
            nWindowsShort--;
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
// ****************************************************************************

void
ViewerWindowManager::SetActiveWindow(const int windowId)
{
    //
    // Check the window id.
    //
    if (windowId <= 0 || windowId > maxWindows || windows[windowId-1] == 0)
    {
        Error("The specified window doesn't exist.");
        return;
    }

    //
    // Copy the window attributes from the current window to the new
    // window if the new window has been referenced for the first time.
    //
    if (clientAtts->GetCloneWindowOnFirstRef() && !referenced[windowId-1])
    {
        //
        // Copy the global attributes, the annotation attributes, the light
        // source attributes, the view attributes and the animation attributes.
        //
        windows[windowId-1]->CopyGeneralAttributes(windows[activeWindow]);
        windows[windowId-1]->CopyAnnotationAttributes(windows[activeWindow]);
        windows[windowId-1]->CopyLightList(windows[activeWindow]);
        windows[windowId-1]->CopyViewAttributes(windows[activeWindow]);
        windows[windowId-1]->CopyAnimation(windows[activeWindow]);
    }
    referenced[windowId-1] = true;

    //
    // Make the specified window active.
    //
    activeWindow = windowId - 1;

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
//  Method: ViewerWindowManager::GetActiveAnimation
//
//  Purpose:
//    Return the animation associated with the currently active window.
//
//  Returns:    The animation associated with the currently active window.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//     Brad Whitlock, Tue Nov 7 09:56:23 PDT 2000
//     Changed to reflect that animations are now part of ViewerWindow.
//
// ****************************************************************************

ViewerAnimation *ViewerWindowManager::GetActiveAnimation() const
{
    //
    // If there are no windows it is an error.
    //
    if (nWindows == 0)
    {
        Error("ViewerWindowManager::GetActiveAnimation() There are no windows.\n");
        return 0;
    }

    return windows[activeWindow]->GetAnimation();
}

// ****************************************************************************
//  Method: ViewerWindowManager::UpdateAnimationState
//
//  Purpose:
//    Update the animation state in the client global attributes.
//
//  Arguments:
//    animation The animation whose state changed.
//    state     The new state.
//    mode      The new mode.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2000
//
//  Modifications:
//    Eric Brugger, Mon Oct 29 08:52:14 PST 2001
//    I added the argument animation which specified which animation state
//    changed so that the state would only be updated if the animation was
//    the active animation. 
//
//    Eric Brugger, Thu Dec 19 11:47:53 PST 2002
//    Added support for keyframing.
//
//    Eric Brugger, Fri Jan 31 13:45:27 PST 2003 
//    I removed the state argument and also had the routine update the
//    current state.
//
// ****************************************************************************

void
ViewerWindowManager::UpdateAnimationState(const ViewerAnimation *animation,
    const int mode) const
{
    //
    // Only update the animation state if this is the active animation.
    //
    if (windows[activeWindow]->GetAnimation() != animation)
        return;

    clientAtts->SetCurrentFrame(animation->GetFrameIndex());
    clientAtts->SetCurrentState(animation->GetFrameIndex());
    clientAtts->SetAnimationMode(mode);
    clientAtts->Notify();
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
    // Set the current file information.
    //
    ViewerWindow    *window = windows[activeWindow];
    ViewerAnimation *animation = window->GetAnimation();
    ViewerPlotList  *plotList = animation->GetPlotList();
    ViewerAnimation::AnimationMode mode = animation->GetMode();

    if (plotList->GetHostDatabaseName().size() < 1)
    {
        clientAtts->SetCurrentFile("notset");
    }
    else
    {
        clientAtts->SetCurrentFile(plotList->GetHostDatabaseName());
    }
    clientAtts->SetCurrentFrame(animation->GetFrameIndex());
    clientAtts->SetNFrames(animation->GetNFrames());
    clientAtts->SetNStates(animation->GetNFrames());
    clientAtts->SetCurrentState(animation->GetFrameIndex());
    if (mode == ViewerAnimation::PlayMode)
    {
        clientAtts->SetAnimationMode(3);
    }
    else if (mode == ViewerAnimation::ReversePlayMode)
    {
        clientAtts->SetAnimationMode(1);
    }
    else
    {
        clientAtts->SetAnimationMode(2);
    }

    //
    // Update the view keyframe list.
    //
    int       nIndices;
    const int *keyframeIndices = window->GetViewKeyframeIndices(nIndices);
    vector<int> keyframeIndices2;
    for (i = 0; i < nIndices; i++)
    {
        keyframeIndices2.push_back(keyframeIndices[i]);
    }
    clientAtts->SetViewKeyframes(keyframeIndices2);

    //
    // Update the maintain view mode.
    //
    clientAtts->SetMaintainView(window->GetMaintainViewMode());

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
// ****************************************************************************

void
ViewerWindowManager::UpdateViewAtts(int windowIndex, bool update2d,
    bool update3d)
{
    int index = (windowIndex == -1 ? activeWindow : windowIndex);
    const avtView3D &view3d = windows[index]->GetView3D();
    const avtView2D &view2d = windows[index]->GetView2D();

    if(index == activeWindow || windows[index]->GetViewIsLocked())
    {
        //
        // Set the 2D attributes from the window's view.
        //
        if(update2d)
        {
            view2d.SetToViewAttributes(view2DClientAtts);
            view2DClientAtts->Notify();
        }

        //
        // Set the 3D attributes from the window's view.
        //
        if(update3d)
        {
            view3d.SetToViewAttributes(view3DClientAtts);
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
                    windows[i]->SetView3D(view3d);
                    windows[i]->SetView2D(view2d);
                }
            }
        }
    }

    //
    // Stack the views.
    //
    if(viewStacking)
    {
        if(windows[index]->GetViewDimension() == 3)
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
        else
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
        renderAtts->SetDisplayLists(!win->GetImmediateModeRendering());
        renderAtts->SetStereoRendering(win->GetStereo());
        renderAtts->SetStereoType((RenderingAttributes::StereoTypes)
            win->GetStereoType());
        renderAtts->SetNotifyForEachRender(win->GetNotifyForEachRender());

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
// ****************************************************************************

void
ViewerWindowManager::RenderInformationCallback(void *data)
{
    int index = (int)((long)data);
    instance->UpdateWindowInformation(index, true);
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
//   Removed UpatePickAttributes. 
//
//   Brad Whitlock, Mon Apr 14 17:26:24 PST 2003
//   Factored some updates out of UpdateGlobalAtts.
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
        ViewerAnimation *animation = windows[activeWindow]->GetAnimation();
        ViewerPlotList  *plotList = animation->GetPlotList();
        plotList->UpdatePlotList();
        plotList->UpdatePlotAtts();
        plotList->UpdateSILRestrictionAtts();
        keyframeClientAtts->SetEnabled(plotList->GetKeyframeMode());
        keyframeClientAtts->Notify();
    }

    //
    // Send the new view info to the client.
    //
    UpdateViewAtts();

    //
    // Update the client animation attributes.
    //
    UpdateAnimationAtts();

    //
    // Update the client annotation attributes.
    //
    UpdateAnnotationAtts();

    //
    // Update the client's light list.
    //
    UpdateLightListAtts();

    //
    // Update the window information.
    //
    UpdateWindowInformation();

    //
    // Update the rendering attributes.
    //
    UpdateRenderingAtts();
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
// ****************************************************************************

ViewAttributes *
ViewerWindowManager::GetView2DClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (view2DClientAtts == 0)
    {
        view2DClientAtts = new ViewAttributes;
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
// ****************************************************************************

ViewAttributes *
ViewerWindowManager::GetView3DClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (view3DClientAtts == 0)
    {
        view3DClientAtts = new ViewAttributes;
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
// ****************************************************************************

void
ViewerWindowManager::SetAnimationAttsFromClient()
{
    windows[activeWindow]->GetAnimation()->SetPipelineCaching(
        animationClientAtts->GetPipelineCachingMode());

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
// ****************************************************************************
 
void
ViewerWindowManager::SetKeyframeAttsFromClient()
{
    windows[activeWindow]->GetAnimation()->GetPlotList()->SetKeyframeMode(
        keyframeClientAtts->GetEnabled());
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
// ****************************************************************************

void
ViewerWindowManager::SetFrameIndex(int frame, int windowIndex)
{
    if(windowIndex < -1 || windowIndex >= maxWindows)
        return;

    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    if(windows[index] != 0)
    {
        // Set the frame of the active window first.
        windows[index]->GetAnimation()->SetFrameIndex(frame);

        // If the active window is time-locked, update the other windows
        // that are also time locked.
        if(windows[index]->GetTimeLock())
        {
            for(int i = 0; i < maxWindows; ++i)
            {
                if(i != index && windows[i] != 0)
                {
                    if(windows[i]->GetTimeLock())
                        windows[i]->GetAnimation()->SetFrameIndex(frame);
                }
            }
        }
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
        windows[index]->GetAnimation()->Stop();
        windows[index]->GetAnimation()->NextFrame();

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
                        windows[i]->GetAnimation()->Stop();
                        windows[i]->GetAnimation()->NextFrame();
                    }
                }
            }
        }
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
        windows[index]->GetAnimation()->Stop();
        windows[index]->GetAnimation()->PrevFrame();

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
                        windows[i]->GetAnimation()->Stop();
                        windows[i]->GetAnimation()->PrevFrame();
                    }
                }
            }
        }
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
        windows[index]->GetAnimation()->Stop();

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
                        windows[i]->GetAnimation()->Stop();
                    }
                }
            }
        }
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
        windows[index]->GetAnimation()->Play();

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
                        windows[i]->GetAnimation()->Play();
                    }
                }
            }
        }
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
        windows[index]->GetAnimation()->ReversePlay();

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
                        windows[i]->GetAnimation()->ReversePlay();
                    }
                }
            }
        }
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
// ****************************************************************************

void
ViewerWindowManager::SetLightListFromClient()
{
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
// ****************************************************************************

void
ViewerWindowManager::UpdateWindowInformation(int windowIndex, bool reportTimes)
{
    GetWindowInformation();

    //
    // Set certain window mode information into the state object.
    //
    int index = (windowIndex == -1) ? activeWindow : windowIndex;
    ViewerWindow *win = windows[index];
    if(win != 0 && index == activeWindow)
    {
        // Set window mode, etc.
        windowInfo->SetWindowMode(int(win->GetInteractionMode()));
        windowInfo->SetBoundingBoxNavigate(win->GetBoundingBoxMode());
        windowInfo->SetSpin(win->GetSpinMode());
        windowInfo->SetLockView(win->GetViewIsLocked());
        windowInfo->SetViewExtentsType(int(win->GetViewExtentsType()));
        windowInfo->SetViewDimension(win->GetViewDimension());
        windowInfo->SetPerspective(win->GetPerspectiveProjection());
        windowInfo->SetLockTools(win->GetToolLock());
        windowInfo->SetLockTime(win->GetTimeLock());
        windowInfo->SetCameraViewMode(win->GetCameraViewMode());
        windowInfo->SetFullFrame(win->GetFullFrameMode());
        // Set the render times.
        if(reportTimes)
        {
            float times[3] = {0., 0., 0.};
            win->GetRenderTimes(times);
            windowInfo->SetLastRenderMin(times[0]);
            windowInfo->SetLastRenderAvg(times[1]);
            windowInfo->SetLastRenderMax(times[2]);
        }

        // Set the approximate number of triangles.
        windowInfo->SetNumTriangles(win->GetNumTriangles());

        // Set the bounding box.
        double extents[6] = {0., 0., 0., 0., 0., 0.};
        win->GetExtents(win->GetViewDimension(), extents);
        windowInfo->SetExtents(extents);

        windowInfo->Notify();
    }
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
// ****************************************************************************

void
ViewerWindowManager::InitWindowLimits()
{
    //
    // Create the window layouts.  The drawing areas for each window must
    // be square so the width and height are set to the minimum of the two.
    //
    int       i, j;
    int       borderWidth, borderHeight;
    int       tempWidth, tempHeight;
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
    windowLimits[0][0].x      = x;
    windowLimits[0][0].y      = y;
    windowLimits[0][0].width  = (tempWidth < tempHeight) ?
                                 tempWidth : tempHeight;
    windowLimits[0][0].height = windowLimits[0][0].width;

    //
    // The layout for a 2x2 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/2)  - borderWidth;
    tempHeight = (screenHeight/2) - borderHeight;
    for (i = 0; i < 2; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 2; j++)
        {
            windowLimits[1][cnt].x      = x;
            windowLimits[1][cnt].y      = y;
            windowLimits[1][cnt].width  = (tempWidth < tempHeight) ?
                                           tempWidth : tempHeight;
            windowLimits[1][cnt].height = windowLimits[1][cnt].width;
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
    for (i = 0; i < 3; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 3; j++)
        {
            windowLimits[2][cnt].x      = x;
            windowLimits[2][cnt].y      = y;
            windowLimits[2][cnt].width  = (tempWidth < tempHeight) ?
                                           tempWidth : tempHeight;
            windowLimits[2][cnt].height = windowLimits[2][cnt].width;
            x = x + windowLimits[2][cnt].width + borderWidth;
            cnt ++;
        }
        y = y + windowLimits[2][0].height + borderHeight;
    }

    //
    // The layout for a 4x4 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/4)  - borderWidth;
    tempHeight = (screenHeight/4) - borderHeight;
    for (i = 0; i < 4; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 4; j++)
        {
            windowLimits[3][cnt].x      = x;
            windowLimits[3][cnt].y      = y;
            windowLimits[3][cnt].width  = (tempWidth < tempHeight) ?
                                           tempWidth : tempHeight;
            windowLimits[3][cnt].height = windowLimits[3][cnt].width;
            x = x + windowLimits[3][cnt].width + borderWidth;
            cnt ++;
        }
        y = y + windowLimits[3][0].height + borderHeight;
    }

    //
    // The layout for a 2x1 grid.
    //
    cnt = 0;
    x          = screenX + borderLeft - shiftX;
    y          = screenY + borderTop  - shiftY;
    tempWidth  = (screenWidth/2) - borderWidth;
    tempHeight =  screenHeight   - borderHeight;
    for (i = 0; i < 2; i++)
    {
        windowLimits[4][cnt].x      = x;
        windowLimits[4][cnt].y      = y;
        windowLimits[4][cnt].width  = (tempWidth < tempHeight) ?
                                       tempWidth : tempHeight;
        windowLimits[4][cnt].height = windowLimits[4][cnt].width;
        x = x + windowLimits[4][cnt].width + borderWidth;
        cnt ++;
    }

    //
    // The layout for a 2x4 grid.
    //
    cnt = 0;
    y          = screenY + borderTop - shiftY;
    tempWidth  = (screenWidth/4)  - borderWidth;
    tempHeight = (screenHeight/2) - borderHeight;
    for (i = 0; i < 2; i++)
    {
        x = screenX + borderLeft - shiftX;
        for (j = 0; j < 4; j++)
        {
            windowLimits[5][cnt].x      = x;
            windowLimits[5][cnt].y      = y;
            windowLimits[5][cnt].width  = (tempWidth < tempHeight) ?
                                           tempWidth : tempHeight;
            windowLimits[5][cnt].height = windowLimits[5][cnt].width;
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
    int       size;

    if (windowIndex < layout)
    {
        x    = windowLimits[layoutIndex][windowIndex].x;
        y    = windowLimits[layoutIndex][windowIndex].y;
        size = windowLimits[layoutIndex][windowIndex].width;
    }
    else
    {
        x    = windowLimits[0][0].x + (nWindows - layout + 1) * 32;
        y    = windowLimits[0][0].y + (nWindows - layout + 1) * 32;
        size = windowLimits[layoutIndex][0].width;
    }

    //
    // Create the new window along with its animation.
    //
    CreateVisWindow(windowIndex, size, size, x, y);

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
    w->SetImmediateModeRendering(!renderAtts->GetDisplayLists());
    w->SetStereoRendering(renderAtts->GetStereoRendering(),
        (int)renderAtts->GetStereoType());
    w->SetNotifyForEachRender(renderAtts->GetNotifyForEachRender());
    w->SetScalableRendering(renderAtts->GetScalableRendering());
    w->SetScalableThreshold(renderAtts->GetScalableThreshold());
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
    // If the window dimension is 2, mark the view as having been modified.
    //
    if (instance->windows[index]->GetViewDimension() == 2)
    {
        instance->windows[index]->SetViewModified2d();
    }

    //
    // Update the view attributes in the client and any locked windows.
    //
    instance->UpdateViewAtts(index);

    //
    // For software rendering.  It looks like the size isn't being used.
    //
    WindowAttributes winAtts;
    winAtts.SetView(*GetView3DClientAtts());
    int size[2];
    vw->GetWindowSize(size[0], size[1]);
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
// ****************************************************************************

void
ViewerWindowManager::SetPlotColors(const AnnotationAttributes *atts)
{
    double bg[4];
    double fg[4];
    if (atts->GetBackgroundMode() == AnnotationAttributes::Solid)
    {
        atts->GetBackgroundColor().GetRgba(bg);
    }
    else
    {
        double gbg1[4], gbg2[4];
        atts->GetGradientColor1().GetRgba(gbg1);
        atts->GetGradientColor2().GetRgba(gbg2);
        bg[0] = (gbg1[0] + gbg2[0]) * 0.5;
        bg[1] = (gbg1[1] + gbg2[1]) * 0.5;
        bg[2] = (gbg1[2] + gbg2[2]) * 0.5;
    }

    atts->GetForegroundColor().GetRgba(fg);
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
                ViewerAnimation::AnimationMode mode =
                    windows[i]->GetAnimation()->GetMode();

                if (mode == ViewerAnimation::PlayMode ||
                    mode == ViewerAnimation::ReversePlayMode)
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
                windows[i]->GetAnimation()->Stop(false);
                ++numWindows;
            }
        }

        //
        // If there is only one window, update the global atts since there
        // will be no pending delete to update them.
        //
        if(numWindows < 2)
            UpdateGlobalAtts();
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
            ViewerAnimation::AnimationMode mode =
                windows[i]->GetAnimation()->GetMode();

            if (mode == ViewerAnimation::PlayMode ||
                mode == ViewerAnimation::ReversePlayMode)
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
        ViewerAnimation::AnimationMode mode =
            windows[lastAnimation]->GetAnimation()->GetMode();

        // Prevent the timer from emitting any signals since the
        // code to handle animation may get back to the Qt event
        // loop which makes it possible to get back here reentrantly.
        timer->blockSignals(true);

        if (mode == ViewerAnimation::PlayMode)
            windows[lastAnimation]->GetAnimation()->NextFrame();
        else
            windows[lastAnimation]->GetAnimation()->PrevFrame();

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
// ****************************************************************************

ViewerWindow *
ViewerWindowManager::GetLineoutWindow() 
{
    if (lineoutWindow == -1)
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
            if (windows[winIdx]->GetAnimation()->GetPlotList()->GetNumPlots() == 0)
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
        windows[lineoutWindow]->SetTypeIsCurve(true);
        windows[lineoutWindow]->SetInteractionMode(NAVIGATE);
        referenced[lineoutWindow] = true;
    }
    return windows[lineoutWindow];
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
        if (windows[lineoutWindow] != 0)
        {
            windows[lineoutWindow]->SetTypeIsCurve(false);
        }
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
// Method: ViewerWindowManager::BeginEngineExecute
//
// Purpose: 
//   Tells the client that an engine is executing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 15:00:11 PST 2002
//
// Modifications:
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

    //
    // Disable all of the popup menus.
    //
    for(int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if(windows[windowIndex] != 0)
            windows[windowIndex]->SetPopupEnabled(false);
    }
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

    //
    // Enable all of the popup menus.
    //
    for(int windowIndex = 0; windowIndex < maxWindows; ++windowIndex)
    {
        if(windows[windowIndex] != 0)
            windows[windowIndex]->SetPopupEnabled(true);
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
    mgrNode->AddNode(new DataNode("activeWindow", activeWindow));

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
    // Load information specific to ViewerWindowManager.
    //

    //
    // Create the right number of windows.
    //
    DataNode *windowsNode = parentNode->GetNode("Windows");
    if(windowsNode == 0)
        return;

    int i, c;
    int newNWindows = windowsNode->GetNumChildren();
    DataNode *sizeNode = 0;
    DataNode *locationNode = 0;
 
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
                w = sizeNode->AsIntArray()[0];
                h = sizeNode->AsIntArray()[1];
                x = locationNode->AsIntArray()[0];
                y = locationNode->AsIntArray()[1];

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
                w = sizeNode->AsIntArray()[0];
                h = sizeNode->AsIntArray()[1];
                x = locationNode->AsIntArray()[0];
                y = locationNode->AsIntArray()[1];

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
                AddWindow();
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
        if(windows[i] != 0 && childCount < newNWindows)
            windows[i]->SetFromNode(wNodes[childCount++]);
    }

    //
    // Set the active window.
    //
    DataNode *node;
    if((node = searchNode->GetNode("activeWindow")) != 0)
    {
        int n = node->AsInt();
        if(n >= 0 && n < nWindows && windows[n] != 0)
            SetActiveWindow(n + 1);
        else
            UpdateAllAtts();
    }
    else
        UpdateAllAtts();

    //
    // Set the lineout window.
    //
    for(i = 0; i < maxWindows; ++i)
    {
        if(windows[i] != 0 && windows[i]->GetTypeIsCurve())
        {
            lineoutWindow = i;
            break;
        }
    }
}
