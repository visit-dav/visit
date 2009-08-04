/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                         VisWinRenderingWithWindow.C                       //
// ************************************************************************* //

#include <VisWinRenderingWithWindow.h>

#include <vtkQtRenderWindow.h>
#include <vtkQtRenderWindowInteractor.h>

#ifdef Q_WS_X11
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
// ****************************************************************************

VisWinRenderingWithWindow::VisWinRenderingWithWindow(
    vtkQtRenderWindow *rw, bool own, VisWindowColleagueProxy &p)
    : VisWinRendering(p)
{
    cursorIndex = 0;
    fullScreenMode = false;

    renWin = rw;
    ownRenderWindow = own;
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
    if (renWin != NULL && ownRenderWindow)
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
//  Modifications:
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support.
//
//    Brad Whitlock, Fri Aug 22 14:44:05 PST 2008
//    If we don't own the render window then it must be embedded. We should
//    not do any window manager grab in that case.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::RealizeRenderWindow(void)
{
    if (fullScreenMode)
        renWin->showFullScreen();
    else
        renWin->show();

#ifdef Q_WS_X11
    if(ownRenderWindow)
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
//    Eric Brugger, Thu Feb 19 11:14:17 PST 2004
//    I made the routine a noop if the window hasn't been realized.
//
// ****************************************************************************

void
VisWinRenderingWithWindow::Iconify(void)
{
    if (realized)
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
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
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
    renWin->SetLargeIcons(val);
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
// ****************************************************************************

void
VisWinRenderingWithWindow::SetCursorForMode(INTERACTION_MODE m)
{
    int newCursorIndex;

    //
    // Determine the new cursor index. The arrow cursor is 0 and the pick
    // cursor is 1.
    //
    if(m == ZONE_PICK || m == NODE_PICK || m == SPREADSHEET_PICK)
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
