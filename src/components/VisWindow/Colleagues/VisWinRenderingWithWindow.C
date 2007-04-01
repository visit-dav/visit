// ************************************************************************* //
//                         VisWinRenderingWithWindow.C                       //
// ************************************************************************* //

#include <VisWinRenderingWithWindow.h>

#include <vtkQtRenderWindow.h>
#include <vtkQtRenderWindowInteractor.h>

#include <WindowMetrics.h>


// ****************************************************************************
//  Method: VisWinRenderingWithWindow constructor
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
// ****************************************************************************

VisWinRenderingWithWindow::VisWinRenderingWithWindow(
                                                    VisWindowColleagueProxy &p)
    : VisWinRendering(p)
{
    cursorIndex = 0;

    renWin = vtkQtRenderWindow::New();
    InitializeRenderWindow(renWin);
 
    iren = vtkQtRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);
}


// ****************************************************************************
//  Method: VisWinRenderingWithWindow destructor
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
// ****************************************************************************

VisWinRenderingWithWindow::~VisWinRenderingWithWindow()
{
    if (renWin != NULL)
    {
        renWin->Delete();
        renWin = NULL;
    }
    if (iren != NULL)
    {
        iren->Delete();
        iren = NULL;
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
// ****************************************************************************

void
VisWinRenderingWithWindow::RealizeRenderWindow(void)
{
    renWin->show();
#ifdef Q_WS_X11
    WindowMetrics::WaitForWindowManagerToGrabWindow(renWin);
#endif
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
// ****************************************************************************

void
VisWinRenderingWithWindow::Iconify(void)
{
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
    renWin->hide();
#else
    if (! renWin->isMinimized())
    {
        renWin->showMinimized();
    }
#endif
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
//
//    Hank Childs, Fri Apr 19 09:21:46 PDT 2002
//    Moved from base class.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::Show()
{
    renWin->show();
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
//
// ****************************************************************************
void
VisWinRenderingWithWindow::Raise()
{
    renWin->raise();
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
//
// ****************************************************************************
void
VisWinRenderingWithWindow::Lower()
{
    renWin->lower();
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
//
//    Hank Childs, Fri Apr 19 09:21:46 PDT 2002
//    Moved from base class.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::Hide()
{
    renWin->hide();
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
// ****************************************************************************

void
VisWinRenderingWithWindow::DeIconify(void)
{
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
    renWin->show();
#else
    renWin->showNormal();
#endif
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
// ****************************************************************************

vtkRenderWindow *
VisWinRenderingWithWindow::GetRenderWindow(void)
{
    return renWin;
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
// ****************************************************************************

vtkRenderWindowInteractor *
VisWinRenderingWithWindow::GetRenderWindowInteractor(void)
{
    return iren;
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
//   
// ****************************************************************************

void *
VisWinRenderingWithWindow::CreateToolbar(const char *name)
{
    return renWin->CreateToolbar(name);
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
    if(m == ZONE_PICK || m == NODE_PICK)
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
            renWin->setCursor(ArrowCursor);
            break;
        case 1:
            renWin->setCursor(CrossCursor);
            break;
        }
    }
}


