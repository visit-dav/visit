/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <ViewerWindowUI.h>
#include <VisWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerPopupMenu.h>
#include <ViewerToolbar.h>

ViewerWindowUI *
ViewerWindowUI::SafeDownCast(ViewerWindow *win)
{
    return dynamic_cast<ViewerWindowUI *>(win);
}

ViewerWindowUI::ViewerWindowUI(int windowIndex) : ViewerWindow(windowIndex)
{
    // Create the popup menu and the toolbar.
    popupMenu = new ViewerPopupMenu(this);
    toolbar = new ViewerToolbar(this);
}

ViewerWindowUI::~ViewerWindowUI()
{
    delete popupMenu;
    delete toolbar;
}

void
ViewerWindowUI::SetVisWindow(VisWindow *vw)
{
    ViewerWindow::SetVisWindow(vw);

    // Now, set up callbacks on the vis window.
    visWindow->SetCloseCallback(CloseCallback, (void *)this);
    visWindow->SetHideCallback(HideCallback, (void *)this);
    visWindow->SetShowCallback(ShowCallback, (void *)this);

    //
    // Callbacks to show the menu when the right mouse button is
    // clicked in the VisWindow.
    //
    visWindow->SetShowMenu(ShowMenuCallback, this);
    visWindow->SetHideMenu(HideMenuCallback, this);
}

// ****************************************************************************
//  Method: ViewerWindowUI::ShowMenu
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
ViewerWindowUI::ShowMenu()
{
    visWindow->SetSpinModeSuspended(true);
    popupMenu->ShowMenu();
}

// ****************************************************************************
//  Method: ViewerWindowUI::HideMenu
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
ViewerWindowUI::HideMenu()
{
    popupMenu->HideMenu();
    visWindow->SetSpinModeSuspended(false);
}

// ****************************************************************************
//  Method: ViewerWindowUI::ShowMenuCallback
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

void ViewerWindowUI::ShowMenuCallback(void *data)
{
    if(data == 0)
        return;

    ViewerWindowUI *win = (ViewerWindowUI *)data;
    win->ShowMenu();
}

// ****************************************************************************
//  Method: ViewerWindowUI::HideMenuCallback
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
ViewerWindowUI::HideMenuCallback(void *data)
{
    if(data == 0)
        return;

    ViewerWindowUI *win = (ViewerWindowUI *)data;
    win->HideMenu();
}

// ****************************************************************************
// Method: ViewerWindowUI::CloseCallback
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
ViewerWindowUI::CloseCallback(void *data)
{
    if(data == 0)
        return;

    //
    // Send a message to the rendering thread to delete this window.
    //
    ViewerWindowUI *win = (ViewerWindowUI *)data;
    ViewerWindowManager::Instance()->StopTimer();
    win->SendDeleteMessage();
}

// ****************************************************************************
// Method: ViewerWindowUI::HideCallback
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
ViewerWindowUI::HideCallback(void *data)
{
    ViewerWindowUI *win = (ViewerWindowUI *)data;
    win->SetVisible(false);
    ViewerWindowManager::Instance()->UpdateAnimationTimer();
}

// ****************************************************************************
// Method: ViewerWindowUI::ShowCallback
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
ViewerWindowUI::ShowCallback(void *data)
{
    ViewerWindowUI *win = (ViewerWindowUI *)data;
    win->SetVisible(true);
    ViewerWindowManager::Instance()->UpdateAnimationTimer();
}

// ****************************************************************************
// Method: ViewerWindowUI::SetLargeIcons
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
ViewerWindowUI::SetLargeIcons(bool val)
{
    visWindow->SetLargeIcons(val);
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
ViewerWindowUI::CreateToolbar(const std::string &name)
{
    return visWindow->CreateToolbar(name.c_str());
}
