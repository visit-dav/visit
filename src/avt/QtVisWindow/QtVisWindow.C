// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               QtVisWindow.C                               //
// ************************************************************************* //

#include <QtVisWindow.h>

#include <VisWinRenderingWithWindow.h>
#include <vtkQtRenderWindow.h>
#include <VisWinTools.h>

vtkQtRenderWindow* (*QtVisWindow::windowCreationCallback)(void *) = 0;
void                *QtVisWindow::windowCreationData = 0;
bool                QtVisWindow::ownsAllWindows = false;

// ****************************************************************************
//  Method: QtVisWindow constructor
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2004
//
//  Modifications:
//    Jeremy Meredith, Tue Jul 17 16:35:37 EDT 2007
//    Added fullscreen support.
//
//    Brad Whitlock, Fri Apr  6 11:43:24 PDT 2012
//    Pass stereo flag to vtkQtRenderWindow constructor so we can create it
//    with the right stereo flags set on the GL context.
//
// ****************************************************************************

QtVisWindow::QtVisWindow(bool fullScreenMode) : VisWindow(false)
{
    bool owns = true;
    vtkQtRenderWindow *renWin = 0;
    if(windowCreationCallback != 0)
    {
        renWin = windowCreationCallback(windowCreationData);
        owns = ownsAllWindows; //false;
    }
    else
        renWin = vtkQtRenderWindow::New(VisWinRendering::GetStereoEnabled());
    VisWinRenderingWithWindow *ren =
        new VisWinRenderingWithWindow(renWin, owns, colleagueProxy);
    ren->SetFullScreenMode(fullScreenMode);
    Initialize(ren);
}

// ****************************************************************************
// Method: QtVisWindow::SetWindowCreationCallback
//
// Purpose: 
//   Sets the window creation callback.
//
// Arguments:
//   wcc     : The window creation callback.
//   wccdata : The window creation callback arguments.
//
// Returns:    A new instance of vtkQtRenderWindow.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:24:18 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QtVisWindow::SetWindowCreationCallback(vtkQtRenderWindow *(*wcc)(void*), void *wccdata)
{
    windowCreationCallback = wcc;
    windowCreationData = wccdata;
}

// ****************************************************************************
// Method: QtVisWindow::SetOwnerShipOfWindows
//
// Purpose:
//   Sets the owner of the windows.
//
// Arguments:
//   isowner     : True: VisIt owns the windows, False: User owns the windows.
//
// Returns:    None.
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Mon Oct 25 16:24:18 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
QtVisWindow::SetOwnerShipOfAllWindows(bool isowner)
{
    ownsAllWindows = isowner;
}


// ****************************************************************************
// Method: QtVisWindow::CreateToolColleague
//
// Purpose: 
//   Create tools colleague with tool geometry.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 7 16:56:07 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QtVisWindow::CreateToolColleague()
{
    tools = new VisWinTools(colleagueProxy, true);
    AddColleague(tools);
}
