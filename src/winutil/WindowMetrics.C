// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "WindowMetrics.h"
#include <visitstream.h>
#include <QApplication>
#include <QRect>
#include <QScreen>

#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_LINUX)
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    #include <QtX11Extras/QX11Info>
#endif
#include <X11/Xlib.h>
static Window GetParent(Display *dpy, Window win, Window *root_ret=NULL);
#endif

//
// Static data
//
WindowMetrics *WindowMetrics::instance = NULL;

// ****************************************************************************
//  Class:  TestWin
//
//  Purpose:
//    Window for testing sizing and positioning.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
// ****************************************************************************
class TestWin : public QWidget
{
  public:
     TestWin() : QWidget()   { resize(1,1); move(100,100); }
};


// ****************************************************************************
//  Method:  WindowMetrics::Instance
//
//  Purpose:
//    Return the global window metrics.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
// ****************************************************************************
WindowMetrics*
WindowMetrics::Instance()
{
    if (!instance)
        instance = new WindowMetrics();
    return instance;
}

// ****************************************************************************
//  Constructor:  WindowMetrics::WindowMetrics
//
//  Purpose:
//    Calculate all the window metrics.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Sep 26 10:19:46 PDT 2001
//    Made win be dynamic.  It caused crashes when created on the stack.
//
//    Jeremy Meredith, Mon Oct  8 11:22:36 PDT 2001
//    Added loop around WaitForWindowManagerToGrabWindow because we might
//    need to wait for more than one ConfigureNotify before the window has
//    actually moved.
//
//    Sean Ahern, Fri Apr  5 11:37:20 PST 2002
//    Worked around a stupid bug in the Mac OS X OroborOSX window manager
//    version 0.8 beta 2.
//
//    Brad Whitlock, Thu Apr 18 12:04:45 PDT 2002
//    We don't need to popup a window on MS Windows to determine the size
//    of the borders, etc. I made the code reflect this.
//
//    Brad Whitlock, Wed Sep 3 11:32:07 PDT 2003
//    I changed the non-X11 code so that it only sets the shift and preshift
//    if we're on Windows. This way, MacOS X is unaffected.
//
//    Brad Whitlock, Wed Jan 11 17:35:10 PST 2006
//    I moved most of the code into the MeasureScreen method.
//
//    Cyrus Harrison, Wed Mar 17 09:03:49 PDT 2010
//    X11 Only:
//    Hide the test window instead of deleting it. The timing of the screen
//    measure is strange on some linux systems and deleting this window can
//    cause Qt to belive there are no active windows and emit a single to
//    exit the main application loop.
//
// ****************************************************************************
bool WindowMetrics::embedded = false;
WindowMetrics::WindowMetrics()
#if defined(Q_OS_LINUX)
: testWindow(0)
#endif
{
    //
    // Default values
    //
    screenX   = 0;
    screenY   = 0;
    screenW   = 1280;
    screenH   = 1024;

    borderT   = 26;
    borderB   = 4;
    borderL   = 4;
    borderR   = 4;

    shiftX    = 0;
    shiftY    = 0;

    preshiftX = 0;
    preshiftY = 0;
}

// ****************************************************************************
//  Method:  WindowMetrics::CalculateScreen
//
//  Purpose:
//    Calculates the screen size to avoid toolbars/panels.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 23 11:26:33 PDT 2001
//    Made logic for toolbars a little better.  It handles a few more
//    configurations.
//
//    Jeremy Meredith, Tue Sep 25 15:28:38 PDT 2001
//    Made a standalone function.
//
//    Brad Whitlock, Tue Oct 25 16:51:36 PDT 2011
//    Use Qt to get the information since it does a better job with platform-
//    specific issues like the Mac dock and multiple screens.
//
//    Kathleen Biagas, Wed Apr  5 13:04:35 PDT 2023
//    Replace obosolete desktop() with primaryScreen().
//
// ****************************************************************************
void
WindowMetrics::CalculateScreen(QWidget *win,
                               int &screenX, int &screenY,
                               int &screenW, int &screenH)
{
    QRect rect = qApp->primaryScreen()->geometry();
    screenX = rect.x();
#if defined(Q_OS_MAC)
    screenY = 0;
#else
    screenY = rect.y();
#endif
    screenW = rect.width();
    screenH = rect.height();
}

// ****************************************************************************
// Method: WindowMetics::MeasureScreen
//
// Purpose: 
//   Measures the screen.
//
// Arguments:
//   waitForWM : Determines whether we want to wait for the window manager
//               to move the test widget. We provide this flag so the loop
//               can be bypassed on systems where it might hang.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 11 17:33:47 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Jul 26 17:02:12 PST 2011
//   Get the X11 extensions and look for 2 that are common on Macs and not
//   elsewhere. If we think we're displaying to a Mac then limit one of our
//   loops to once so we don't get stuck.
//
//   Kathleen Biagas, Mon Mar 14 13:45:14 PDT 2016
//   Re-enable this method for QT 5.
//
// ****************************************************************************

void
WindowMetrics::MeasureScreen(bool waitForWM)
{
#if defined(Q_OS_LINUX)
    //
    // Create the test window
    //
    testWindow = new TestWin;
    testWindow->show();

    //
    // Try and determine if we're displaying X11 to a Mac.
    //
    int nExt = 0, appleDisplay = 0;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    char **ext = XListExtensions(QX11Info::display() , &nExt);
#else
    auto x11NativeInterface = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if(x11NativeInterface == NULL)
        return;
  
    char **ext = XListExtensions(x11NativeInterface->display() , &nExt);
#endif
    for(int e = 0; e < nExt; ++e)
    {
        if(strcmp(ext[e], "Apple-DRI") == 0 ||
           strcmp(ext[e], "Apple-WM") == 0)
        {
            appleDisplay++;
        }
    }
    XFreeExtensionList(ext);

    //
    // Calculate the metrics
    //
    CalculateScreen(testWindow, screenX, screenY, screenW, screenH);

    // We need for the window manager to put borders on the window
    // before we can determine their sizes
    if(waitForWM)
        WaitForWindowManagerToGrabWindow(testWindow);

    // We tried to put the main window at 100,100...
    // ...see where it *really* started out
    preshiftX = 100;
    preshiftY = 100;
    CalculateTopLeft(testWindow, preshiftX, preshiftY);
    preshiftX -= 100;
    preshiftY -= 100;

    // Determine the border sizes of the main window.
    CalculateBorders(testWindow, borderT, borderB, borderL, borderR);

    // Move it to the top left (we think)
    testWindow->move(borderL,borderT);
    testWindow->resize(2,2);

    if(waitForWM)
    {
        do
        {
            // If it's not at 0,0, then we have a shift
            WaitForWindowManagerToMoveWindow(testWindow);
            CalculateTopLeft(testWindow, shiftX, shiftY);
        }
        while ((shiftX == preshiftX+100  &&  shiftY == preshiftY+100) &&
                appleDisplay == 0);
        // (sometimes we need to wait for more than one ConfigureNotify)
    }
    else
        CalculateTopLeft(testWindow, shiftX, shiftY);

    // Adjust preshift to account for the actual shift
    preshiftX += (borderL - shiftX);
    preshiftY += (borderT - shiftY);

    testWindow->hide();
#else
    //
    // Calculate the metrics
    //
    CalculateScreen(0, screenX, screenY, screenW, screenH);

    //
    // Determine the border sizes of the main window.
    //
    CalculateBorders(0, borderT, borderB, borderL, borderR);

#if defined(Q_OS_WIN)
    preshiftX = shiftX = borderL;
    preshiftY = shiftY = borderT;
#endif
#endif
}


// ****************************************************************************
// ****************************************************************************
//                           PLATFORM SPECIFIC CODE
// ****************************************************************************
// ****************************************************************************
#if defined(Q_OS_WIN)

//
// Win32 coding
//

// ****************************************************************************
// Method: WindowMetrics::CalculateBorders
//
// Purpose: 
//   Determines the size of the borders around the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 18 12:25:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
WindowMetrics::CalculateBorders(QWidget *, 
    int &borderT, int &borderB, int &borderL, int &borderR)
{
    borderT = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYSIZE);
    borderB = GetSystemMetrics(SM_CYFRAME);
    borderL = GetSystemMetrics(SM_CXFRAME);
    borderR = GetSystemMetrics(SM_CXFRAME);
}

void
WindowMetrics::WaitForWindowManagerToGrabWindow(QWidget *win)
{
    // nothing
}

void
WindowMetrics::WaitForWindowManagerToMoveWindow(QWidget *win)
{
    // nothing
}

// ****************************************************************************
// Method: WindowMetrics::CalculateTopLeft
//
// Purpose: 
//    Calculates the topleft-most position of a given window.
//
// Arguments:
//   w : The widget we're testing.
//   X : The return X value.
//   Y : The return Y value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 18 11:58:37 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
WindowMetrics::CalculateTopLeft(QWidget *w, int &X, int &Y)
{
    X = w->x();
    Y = w->y();
}

#elif defined(Q_OS_MAC)

//
// MacOS coding
//

// ****************************************************************************
// Method: WindowMetrics::CalculateBorders
//
// Purpose: 
//   Determines the size of the borders around the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 18 12:25:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
WindowMetrics::CalculateBorders(QWidget *, 
    int &borderT, int &borderB, int &borderL, int &borderR)
{
#if 1
    //cerr << "WindowMetrics::CalculateBorders: Mac version not implemented!" << endl;
    borderT = 20;
    borderB = 0;
    borderL = 0;
    borderR = 1;
#endif
}

void
WindowMetrics::WaitForWindowManagerToGrabWindow(QWidget *win)
{
    // nothing
}

void
WindowMetrics::WaitForWindowManagerToMoveWindow(QWidget *win)
{
    // nothing
}

// ****************************************************************************
// Method: WindowMetrics::CalculateTopLeft
//
// Purpose: 
//    Calculates the topleft-most position of a given window.
//
// Arguments:
//   w : The widget we're testing.
//   X : The return X value.
//   Y : The return Y value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 18 11:58:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
WindowMetrics::CalculateTopLeft(QWidget *w, int &X, int &Y)
{
    X = w->x();
    Y = w->y();
}

#elif defined(Q_OS_LINUX)

//
// X11 coding
//

// ****************************************************************************
// Method: CalculateBorders
//
// Purpose: 
//   This method calculates the border sizes of the window decorations.
//
// Note:
//   This method is platform specific. This is the X11 implementation.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 7 16:06:02 PST 2000
//
// Modifications:
//    Jeremy Meredith, Thu Apr  5 13:47:02 PDT 2001
//    Added a check -- sometimes there is a desktop-size window over the
//    root window, and we assumed this was part of the main window.
//
//    Jeremy Meredith, Fri Jul 20 11:18:50 PDT 2001
//    Added checks to make sure the borders are not negative.  This somehow
//    happened under twm.
//
//    Jeremy Meredith, Tue Sep 25 14:38:46 PDT 2001
//    Made standalone function.
//
//    Kathleen Biagas, Wed Apr  5 13:04:35 PDT 2023
//    Replace obosolete desktop() with primaryScreen().
//
// ****************************************************************************

void
WindowMetrics::CalculateBorders(QWidget *win,
                                int &borderT, int &borderB,
                                int &borderL, int &borderR)
{
    unsigned int nchildren;
    Window root, parent_window, *children=NULL;
    XWindowAttributes leaf_attributes;
    XWindowAttributes parent_attributes;

    // Get the display pointer and window Id from the main window.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    Display *dpy = QX11Info::display();
#else
    auto x11NativeInterface = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if(x11NativeInterface == NULL)
    {
         // ERROR - get out and use defaults.
         return;
    }

    Display *dpy = x11NativeInterface->display();
#endif
    Window main_window = win->winId();

    // There is a GUI window, so we must adjust the work area. 
    // First we must find the parent of the main window (which
    // will be a child of the window provided by the window manager.
    if(XQueryTree(dpy, main_window, &root, &parent_window,
                  &children, &nchildren ) == 0)
    {
         // ERROR - get out and use defaults.
         return;
    }
    // Deallocate the memory allocated by XQueryTree().
    if(nchildren > 0)
        XFree((char *)children);

    // Get the attributes of the main window and place them
    // into the leaf_attributes structure.
    if(XGetWindowAttributes(dpy, main_window, &leaf_attributes ) == 0)
    {
         // ERROR - get out and use defaults.
         return;
    }

    // At this point, we have the a leaf_window, either the
    // main_window or the Temp window.  Get the attributes.
    int x = leaf_attributes.x;
    int y = leaf_attributes.y;
    int height = leaf_attributes.height;
    int width  = leaf_attributes.width;
    int border_width = leaf_attributes.border_width;
    int big_height = 0, big_width = 0;

    int desktop_width  = qApp->primaryScreen()->geometry().width();
    int desktop_height = qApp->primaryScreen()->geometry().height();

    // Start progressing up the tree.
    int count = 0;
    while(parent_window != root)
    {
        Window grandparent_window;

        // Get the attributes for the parent window.
        if(XGetWindowAttributes(dpy, parent_window, &parent_attributes) == 0)
        {
            // ERROR - get out and use defaults.
            return;
        }

        if (desktop_width == parent_attributes.width &&
            desktop_height == parent_attributes.height)
        {
            break;
        }

        // Get the parent of the parent window, called the
        // grandparent here.
        if(XQueryTree(dpy, parent_window, &root,
                      &grandparent_window, &children, &nchildren ) == 0)
        {
            // ERROR - get out and use defaults.
            return;
        }
        // Deallocate the memory allocated by XQueryTree().
        if(nchildren > 0)
            XFree((char *)children);

        if(grandparent_window != root)
        {
            // The grandparent isn't the root window, so sum up
            // the x, y, and border_width values.
            x += parent_attributes.x;
            y += parent_attributes.y;
        }

        // The grandparent *is* the root window, so save
        // the height and width values of the parent.
        big_height = parent_attributes.height;
        big_width = parent_attributes.width;

        border_width += parent_attributes.border_width;
        parent_window = grandparent_window;

        ++count;
    } // end while

    // Store the calculated window border sizes.
    if(count != 0)
    {
        // Determine the border widths.
        borderT = y + border_width;
        borderB = big_height - borderT - height;
        borderL = x + border_width;
        borderR = big_width - borderL - width;
        // Sometimes they wind up negative.  Clamp them at 0.
        if (borderT < 0) borderT = 0;
        if (borderB < 0) borderB = 0;
        if (borderL < 0) borderL = 0;
        if (borderR < 0) borderR = 0;
    }
}

// ****************************************************************************
//  Method:  WaitForWindowManagerToGrabWindow
//
//  Purpose:
//    Waits for the window manager to grab a window
//
//  Arguments:
//    win        the window
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 13 15:38:26 PDT 2001
//    Rewrote this function.  Some window managers never put another 
//    X window between ours and the root, so the old way never worked.
//    This is actually taken from somewhere deep in Qt's source code.
//
// ****************************************************************************
void
WindowMetrics::WaitForWindowManagerToGrabWindow(QWidget *win)
{
    //if(embedded) return;
}

// ****************************************************************************
//  Method:  WaitForWindowManagerToMoveWindow
//
//  Purpose:
//    Waits for the window manager to move a window
//
//  Arguments:
//    win        the window
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 13, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:19:19 PDT 2001
//    Removed ProcessEvents.  It sometimes caused a crash on some platforms.
//
// ****************************************************************************
void
WindowMetrics::WaitForWindowManagerToMoveWindow(QWidget *win)
{
    //if(embedded) return;
}

// ****************************************************************************
//  Method:  CalculateTopLeft
//
//  Purpose:
//    Calculates the topleft-most position of a given window
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 14, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Apr 17 10:00:59 PDT 2002
//    Called GetParent with slightly different args.
//
// ****************************************************************************
void
WindowMetrics::CalculateTopLeft(QWidget *wid, int &X, int &Y)
{
    Window root, parent;
    XWindowAttributes atts;

    // Get the display pointer and window Id from the widget.
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    Display *dpy  = QX11Info::display();
#else
    auto x11NativeInterface = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if(x11NativeInterface == NULL)
    {
        // Error ?
        return;
    }
    Display *dpy  = x11NativeInterface->display();
#endif
    Window window = wid->winId();

    // Find the parent and the root.
    parent = GetParent(dpy, window, &root);

    // If the parent is the root, just return and use the default values
    if (parent == root)
        return;

    // Find the last parent before the root window
    while (parent != root)
    {
        window = parent;
        parent = GetParent(dpy, window);
    }

    // Get its X,Y position relative to the screen
    XGetWindowAttributes(dpy, window, &atts);
    X = atts.x;
    Y = atts.y;
}

// ****************************************************************************
//  Method:  GetParent
//
//  Purpose:
//    Returns the parent window id.  If a valid pointer is passed as
//    "root", we return the root window id well.
//
//  Programmer:  Sean Ahern
//  Creation:    Mon Apr 15 15:54:34 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
static Window
GetParent(Display *dpy, Window win, Window *root_ret)
{
    Window  root, parent, *children = NULL;
    unsigned int nchildren;

    XQueryTree(dpy, win, &root, &parent, &children, &nchildren);
    XFree(children);

    if (root_ret != NULL)
        *root_ret = root;

    return parent;
}
#endif
