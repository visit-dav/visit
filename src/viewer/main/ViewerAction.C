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

#include <ViewerAction.h>
#include <qaction.h>
#include <qiconset.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>

// ****************************************************************************
// Method: ViewerAction::ViewerAction
//
// Purpose: 
//   Constructor for the ViewerAction class.
//
// Arguments:
//   win  : The window that owns the action.
//   name : The name of the action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:02:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerAction::ViewerAction(ViewerWindow *win, const char *name) : 
    ViewerActionBase(win, name)
{
    // Create a new QAction and make it call our Activate method when
    // it is activated.
    QString n; n.sprintf("%s_action", name);
    action = new QAction(0, name);
    connect(action, SIGNAL(activated()),
            this, SLOT(Activate()));
    connect(action, SIGNAL(toggled(bool)),
            this, SLOT(HandleToggle(bool)));
}

// ****************************************************************************
// Method: ViewerAction::~ViewerAction
//
// Purpose: 
//   Destructor for the ViewerAction class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerAction::~ViewerAction()
{
    if(action)
        delete action;
}

// ****************************************************************************
// Method: ViewerAction::Setup
//
// Purpose: 
//   This method is invoked when the action is activated through a menu or a
//   toolbar. Actions that need to set the args ViewerRPC do it here.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::Setup()
{
}

// ****************************************************************************
// Method: ViewerAction::Update
//
// Purpose: 
//   Updates the action's state which affects how it looks in the toolbar and
//   the menu.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************
  
void
ViewerAction::Update()
{
    // Update the action's enabled state.
    bool actionShouldBeEnabled = Enabled();
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the action's toggled state if it is a toggle action.
    if(action->isToggleAction())
    {
        bool actionShouldBeToggled = Toggled();
        if(action->isOn() != actionShouldBeToggled)
        {
            action->blockSignals(true);
            action->setOn(actionShouldBeToggled);
            action->blockSignals(false);
        }
    }
}

void
ViewerAction::SetAllText(const char *text)
{
    SetText(text);
    SetMenuText(text);
    SetToolTip(text);
}

void
ViewerAction::SetText(const char *text)
{
    action->setText(text);
}

void
ViewerAction::SetMenuText(const char *text)
{
    action->setMenuText(text);
}

void
ViewerAction::SetToolTip(const char *text)
{
    action->setToolTip(text);
}

void
ViewerAction::SetIconSet(const QIconSet &icons)
{
    action->setIconSet(icons);
}

void
ViewerAction::SetToggleAction(bool val)
{
    action->setToggleAction(val);
}

// ****************************************************************************
// Method: ViewerAction::ConstructMenu
//
// Purpose: 
//   Adds the action to a menu.
//
// Arguments:
//   menu : The menu to which the action will be added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::ConstructMenu(QPopupMenu *menu)
{
    // simplest case
    action->addTo(menu);
}

// ****************************************************************************
// Method: ViewerAction::RemoveFromMenu
//
// Purpose: 
//   Removes the action from a menu.
//
// Arguments:
//   menu : The menu from which to remove the action.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 10:04:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::RemoveFromMenu(QPopupMenu *menu)
{
    // simplest case
    action->removeFrom(menu);
}

// ****************************************************************************
// Method: ViewerAction::ConstructToolbar
//
// Purpose: 
//   Adds the action to a toolbar.
//
// Arguments:
//   toolbar : The toolbar to which the action will be added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Mar 20 14:02:33 PST 2003
//   I added code to prevent actions that have no icons from being added
//   to the toolbar.
//
// ****************************************************************************

void
ViewerAction::ConstructToolbar(QToolBar *toolbar)
{
    // simplest case
    if(!action->iconSet().pixmap().isNull())
        action->addTo(toolbar);
}

// ****************************************************************************
// Method: ViewerAction::RemoveFromToolbar
//
// Purpose: 
//   Removes the action from a toolbar.
//
// Arguments:
//   toolbar : The toolbar from which the action will be removed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Mar 20 14:02:33 PST 2003
//   I added code to prevent actions that have no icons from being added
//   to the toolbar.
//
// ****************************************************************************

void
ViewerAction::RemoveFromToolbar(QToolBar *toolbar)
{
    // simplest case
    if(!action->iconSet().pixmap().isNull())
        action->removeFrom(toolbar);
}

// ****************************************************************************
// Method: ViewerAction::HandleToggle
//
// Purpose: 
//   Called when the button is toggled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::HandleToggle(bool)
{
}
