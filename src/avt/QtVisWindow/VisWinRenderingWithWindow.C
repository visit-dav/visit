// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         VisWinRenderingWithWindow.C                       //
// ************************************************************************* //

#include <VisWinRenderingWithWindow.h>
#include <DebugStream.h>

#include <vtkQtRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRubberBandMapper2D.h>
#include <vtkDashedXorGridMapper2D.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtk_glew.h>

#if defined(Q_OS_LINUX)
// We only need WindowMetrics here if we're on X11.
#include <WindowMetrics.h>
#endif

#include <QCursor>

// ****************************************************************************
//  Method: VisWinRenderingWithWindow constructor
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support.
//
//    Brad Whitlock, Mon Aug 18 14:45:18 PDT 2008
//    Pass in the vtkQtRenderWindow that we'll be using.
//
//    Brad Whitlock, Tue Feb 22 15:06:20 PST 2011
//    Delete code to create an interactor.
//
// ****************************************************************************

VisWinRenderingWithWindow::VisWinRenderingWithWindow(
    vtkQtRenderWindow *rw, bool own, VisWindowColleagueProxy &p)
    : VisWinRendering(p)
{
    cursorIndex = 0;
    fullScreenMode = false;
    useLargeIcons = false;

    renWin = rw;
    ownRenderWindow = own;
    InitializeRenderWindow(renWin->GetRenderWindow());
}


// ****************************************************************************
//  Method: VisWinRenderingWithWindow destructor
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Feb 22 15:06:40 PST 2011
//    Remove interactor deletion.
//
// ****************************************************************************

VisWinRenderingWithWindow::~VisWinRenderingWithWindow()
{
    if (renWin != NULL && ownRenderWindow)
    {
        renWin->Delete();
        renWin = NULL;
    }
}

// ****************************************************************************
//  Method: VisWinRenderingWithWindow::RealizeRenderWindow
//
//  Purpose:
//      Makes the window "realize" itself (come onto the screen).
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support.
//
//    Brad Whitlock, Fri Aug 22 14:44:05 PST 2008
//    If we don't own the render window then it must be embedded. We should
//    not do any window manager grab in that case.
//
//    Brad Whitlock, Thu May 3 11:52 PDT 2012
//    Force a render on Windows so the new window will use the right background
//    color.
//
//    Cyrus Harrison, Sat Sep  8 15:22:28 PDT 2012
//    Force render on all platforms to make sure background is cleared at
//    startup.
//
//    Burlen Loring, Thu Oct  8 12:44:58 PDT 2015
//    Report some capabilities in the debug log.
//
// ****************************************************************************
#define safes(arg) (arg?((const char *)arg):"")

void
VisWinRenderingWithWindow::RealizeRenderWindow(void)
{
    if (fullScreenMode)
        renWin->showFullScreen();
    else
        renWin->show();


    renWin->GetRenderWindow()->Render();

    debug2 << "render window is a vtkQtRenderWindow" << endl;
    vtkOpenGLRenderWindow *glrw = dynamic_cast<vtkOpenGLRenderWindow*>(renWin->GetRenderWindow());
    if (!glrw) return;

    const char *glvers = safes(glGetString(GL_VERSION));
    debug2 << "  GLVersion: " << glvers << endl;
    // if we want more information than just Version, perhaps should call
    // vtkOpenGLRenderWindow::ReportCapabilities
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetSize
//
// Purpose: 
//   Set the size of the render window.
//
// Arguments:
//   w : the new width
//   h : The new height.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 15:15:44 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRenderingWithWindow::SetSize(int w, int h)
{
    if(ownRenderWindow)
        renWin->resize(w, h);
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::GetWindowSize
//
// Purpose: 
//   Return the size of the vtkQtRenderWindow instead of getting the size
//   with the Size() method because that returns the size of the OpenGL part
//   of the window as opposed to the whole thing.
//
// Arguments:
//   w : The return variable for the width.
//   h : The return variable for the height.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 21 12:13:25 PDT 2003
//
// Modifications:
//
//    Mark C. Miller, 07Jul03
//    Renamed to GetWindowSize
//   
// ****************************************************************************

void
VisWinRenderingWithWindow::GetWindowSize(int &w, int &h)
{
    w = renWin->width();
    h = renWin->height();
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetLocation
//
// Purpose: 
//   Set the window's location.
//
// Arguments:
//   x,y : The new location
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 15:17:41 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRenderingWithWindow::SetLocation(int x, int y)
{
    if(ownRenderWindow)
        renWin->move(x, y);
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::GetLocation
//
// Purpose: 
//   Get the window's current location.
//
// Arguments:
//   x,y : the window's location.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 15:18:11 PST 2011
//
// Modifications:
//   
// ****************************************************************************
void
VisWinRenderingWithWindow::GetLocation(int &x, int &y)
{
    x = renWin->pos().x();
    y = renWin->pos().y();
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetTitle
//
// Purpose: 
//   Set the window's title.
//
// Arguments:
//   title : The new window title.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 22 15:18:37 PST 2011
//
// Modifications:
//   
// ****************************************************************************
void
VisWinRenderingWithWindow::SetTitle(const char *title)
{
    renWin->setWindowTitle(title);
}

// ****************************************************************************
//  Method: VisWinRenderingWithWindow::Iconify
//
//  Purpose:
//      Iconify the render window.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Sep 25 11:07:21 PDT 2002
//    I made the window hide itself on the Windows platform.
//
//    Brad Whitlock, Thu Sep 4 10:43:02 PDT 2003
//    I made the window hide itself on MacOS X.
//
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::Iconify(void)
{
    if (realized && ownRenderWindow)
    {
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        renWin->hide();
#else
        if (! renWin->isMinimized())
        {
            renWin->showMinimized();
        }
#endif
    }
}


// ****************************************************************************
//  Method: VisWinRendering:WithWindow:Show
//
//  Purpose: 
//      Shows the window.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Apr 16 12:45:50 PDT 2002
//
//  Modifications:
//    Hank Childs, Fri Apr 19 09:21:46 PDT 2002
//    Moved from base class.
//
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::Show()
{
    if (realized)
    {
        renWin->show();
    }
}

// ****************************************************************************
//  Method: VisWinRendering:WithWindow:Raise
//
//  Purpose: 
//      Raises the window.
//
//  Programmer: Sean Ahern
//  Creation:   Mon May 20 13:34:07 PDT 2002
//
//  Modifications:
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************
void
VisWinRenderingWithWindow::Raise()
{
    if (realized)
    {
        renWin->raise();
    }
}

// ****************************************************************************
//  Method: VisWinRendering:WithWindow:Lower
//
//  Purpose: 
//      Lowers the window.
//
//  Programmer: Sean Ahern
//  Creation:   Mon May 20 13:34:07 PDT 2002
//
//  Modifications:
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************
void
VisWinRenderingWithWindow::Lower()
{
    if (realized)
    {
        renWin->lower();
    }
}

// ****************************************************************************
//  Method: VisWinRenderingWithWindow::ActivateWindow
//
//  Purpose: 
//      Activates the window.
//
//  Programmer: Gunther H. Weber
//  Creation:   Mon Jul 18 16:16:16 PDT 2011
//
//  Modifications:
//
// ****************************************************************************
void
VisWinRenderingWithWindow::ActivateWindow()
{
    if (realized)
    {
        renWin->activateWindow();
    }
}


// ****************************************************************************
//  Method: VisWinRenderingWithWindow::Hide
//
//  Purpose: 
//      Hides the window.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Apr 16 12:46:06 PDT 2002
//
//  Modifications:
//    Hank Childs, Fri Apr 19 09:21:46 PDT 2002
//    Moved from base class.
//
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::Hide()
{
    if (realized)
    {
        renWin->hide();
    }
}


// ****************************************************************************
//  Method: VisWinRenderingWithWindow::DeIconify
//
//  Purpose:
//      De-Iconify the render window.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Sep 25 11:08:26 PDT 2002
//    I made the window show itself on the Windows platform.
//
//    Brad Whitlock, Thu Sep 4 10:43:38 PDT 2003
//    I made the window show itself on MacOS X.
//
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::DeIconify(void)
{
    if (realized)
    {
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        renWin->show();
#else
        renWin->showNormal();
#endif
    }
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::IsVisible
//
// Purpose: 
//   Returns whether the renderwindow is visible.
//
// Returns:    Whether the renderwindow is visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 09:26:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
VisWinRenderingWithWindow::IsVisible() const
{
    return renWin->isVisible();
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetResizeEvent
//
// Purpose:
//   Set a callback for a window resize event.
//
// Arguments:
//   callback  The callback routine.
//   data      A pointer to the data to pass the callback routine.
//
// Programmer: Eric Brugger
// Creation:   August 17, 2001
//
// ****************************************************************************
 
void
VisWinRenderingWithWindow::SetResizeEvent(void (*callback)(void *), void *data)
{
    renWin->SetResizeCallback(callback, data);
}


// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetCloseCallback
//
// Purpose:
//   Sets the callback function that is called by the render window when it
//   is asked to close by the window manager.
//
// Arguments:
//   cb        The callback function.
//   data      The data to be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 12:03:31 PDT 2001
//
// ****************************************************************************
 
void
VisWinRenderingWithWindow::SetCloseCallback(void (*cb)(void *), void *data)
{
    renWin->SetCloseCallback(cb, data);
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetHideCallback
//
// Purpose:
//   Sets the callback function that is called by the render window when it
//   is asked to hide by the window manager.
//
// Arguments:
//   cb        The callback function.
//   data      The data to be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:00:46 PDT 2003
//
// ****************************************************************************
 
void
VisWinRenderingWithWindow::SetHideCallback(void (*cb)(void *), void *data)
{
    renWin->SetHideCallback(cb, data);
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetShowCallback
//
// Purpose:
//   Sets the callback function that is called by the render window when it
//   is asked to show by the window manager.
//
// Arguments:
//   cb        The callback function.
//   data      The data to be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 10:00:46 PDT 2003
//
// ****************************************************************************
 
void
VisWinRenderingWithWindow::SetShowCallback(void (*cb)(void *), void *data)
{
    renWin->SetShowCallback(cb, data);
}

// ****************************************************************************
//  Method: VisWinRenderingWithWindow::GetRenderWindow
//
//  Purpose:
//      Gets the Qt render window.  This is a virtual function that allows the
//      base type to operate on the window, even though it is not aware that
//      it is a Qt window.
//
//  Returns:    The render window typed as the base class.
// 
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Feb 22 15:05:24 PST 2011
//    Return the real VTK render window now that renWin is not a subclass of
//    vtkRenderWindow.
//
// ****************************************************************************

vtkRenderWindow *
VisWinRenderingWithWindow::GetRenderWindow(void)
{
    return renWin->GetRenderWindow();
}


// ****************************************************************************
//  Method: VisWinRenderingWithWindow::GetRenderWindowInteractor
//
//  Purpose:
//      Gets the Qt render window interactor.  This is a virtual function that
//      allows the base type to operate on the window, even though it is not
//      aware that it is a Qt window.
//
//  Returns:    The render window typed as the base class.
// 
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Feb 22 15:04:51 PST 2011
//    Return the interactor from the render window since we no longer create one.
//
// ****************************************************************************

vtkRenderWindowInteractor *
VisWinRenderingWithWindow::GetRenderWindowInteractor(void)
{
    return renWin->GetInteractor();
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::CreateToolbar
//
// Purpose: 
//   Creates a toolbar and returns a pointer to it.
//
// Arguments:
//   name : The name of the toolbar.
//
// Returns:    A pointer to the newly created toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 14:40:30 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 22 14:38:04 PST 2011
//   I moved the implementation from vtkQtRenderWindow.
//
// ****************************************************************************

void *
VisWinRenderingWithWindow::CreateToolbar(const char *name)
{
    void *t = (void *)renWin->addToolBar(name);
    if(strcmp(name, "Plots") == 0 || strcmp(name, "Operators") == 0) // hack for now
    {
        renWin->addToolBar(Qt::LeftToolBarArea, (QToolBar *)t);
        renWin->addToolBarBreak(Qt::LeftToolBarArea);
    }
    if(strcmp(name, "Lock") == 0) // hack
        renWin->addToolBarBreak();
    return t;
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetLargeIcons
//
// Purpose: 
//   Tells the vis window to use large icons.
//
// Arguments:
//   val : Whether to use large icons.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:47:08 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRenderingWithWindow::SetLargeIcons(bool val)
{
    useLargeIcons = val;
    if(useLargeIcons)
        renWin->setIconSize(QSize(32,32));
    else
        renWin->setIconSize(QSize(20,20));
}

// ****************************************************************************
// Method: VisWinRenderingWithWindow::SetCursorForMode
//
// Purpose: 
//   Sets the appropriate cursor for the interaction mode.
//
// Arguments:
//   m : The interaction mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:49:10 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Feb 27 16:22:40 PST 2004
//   I made the code work with Qt 3.
//
//   Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//   Added SPREADSHEET_PICK 
//
//   Brad Whitlock, Fri May  9 14:22:17 PDT 2008
//   Qt 4.
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
// ****************************************************************************

void
VisWinRenderingWithWindow::SetCursorForMode(INTERACTION_MODE m)
{
    int newCursorIndex;

    //
    // Determine the new cursor index. The arrow cursor is 0 and the pick
    // cursor is 1.
    //
    if(m == ZONE_PICK || m == NODE_PICK || m == SPREADSHEET_PICK || m == DDT_PICK)
        newCursorIndex = 1;
    else
        newCursorIndex = 0;

    //
    // If the cursor is different, then use the new cursor.
    //
    if(newCursorIndex != cursorIndex)
    {
        cursorIndex = newCursorIndex;

        switch(cursorIndex)
        {
        default:
        case 0:
            renWin->setCursor(QCursor(Qt::ArrowCursor));
            break;
        case 1:
            renWin->setCursor(QCursor(Qt::CrossCursor));
            break;
        }
    }
}

// ****************************************************************************
//  Method:  VisWinRenderingWithWindow::SetFullScreenMode
//
//  Purpose:
//    Set the full screen mode.
//
//  Arguments:
//    fs         the full screen mode
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 17, 2007
//
// ****************************************************************************

void
VisWinRenderingWithWindow::SetFullScreenMode(bool fs)
{
    fullScreenMode = fs;
}

// ****************************************************************************
//  Method:  VisWinRenderingWithWindow::CreateRubberbandMapper
//
//  Purpose:
//    Create a mapper to use for rubber band rendering.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Oct 14 14:53:31 PDT 2011
//
// ****************************************************************************

vtkPolyDataMapper2D *
VisWinRenderingWithWindow::CreateRubberbandMapper()
{
    vtkRubberBandMapper2D *mapper = vtkRubberBandMapper2D::New();
    mapper->SetWidget(renWin->GetGLWidget());
    return mapper;
}

// ****************************************************************************
//  Method:  VisWinRenderingWithWindow::CreateXorGridMapper
//
//  Purpose:
//    Create a mapper to use for rubber band guideline rendering.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Oct 14 14:53:31 PDT 2011
//
// ****************************************************************************

vtkPolyDataMapper2D *
VisWinRenderingWithWindow::CreateXorGridMapper()
{
    vtkDashedXorGridMapper2D *mapper = vtkDashedXorGridMapper2D::New();
    mapper->SetWidget(renWin->GetGLWidget());
    return mapper;
}
