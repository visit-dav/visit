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

#include <ViewerWindow.h>
#include <ViewerToggleAction.h>
#include <QAction>

// ****************************************************************************
// Method: ViewerToggleAction::ViewerToggleAction
//
// Purpose: 
//   Constructor.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:10:26 PDT 2004
//   I turned the pixmaps into pointers to get rid a runtime warning about
//   creating pixmaps in -nowin mode.
//   
// ****************************************************************************

ViewerToggleAction::ViewerToggleAction(ViewerWindow *win) : 
    ViewerAction(win)
{
    toggled = false;
    regularIcon = 0;
    toggledIcon = 0;
    SetToggleAction(true);
}

// ****************************************************************************
// Method: ViewerToggleAction::~ViewerToggleAction
//
// Purpose: 
//   Destructor.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:12:11 PDT 2004
//   Added code to delete pixmaps.
//
// ****************************************************************************

ViewerToggleAction::~ViewerToggleAction()
{
    delete toggledIcon;
    delete regularIcon;
}

// ****************************************************************************
// Method: ViewerToggleAction::SetIcons
//
// Purpose: 
//   Sets the on/off toggle icons.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:12:25 PDT 2004
//   Added code to copy icons.
//
// ****************************************************************************

void
ViewerToggleAction::SetIcons(const QPixmap &p1, const QPixmap &p2)
{
    if(!window->GetNoWinMode())
    {
        toggledIcon = new QPixmap(p1);
        regularIcon = new QPixmap(p2);
        SetIcon(QIcon(*regularIcon));
    }
}

// ****************************************************************************
// Method: ViewerToggleAction::PreExecute
//
// Purpose: 
//   
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToggleAction::PreExecute()
{
    toggled = Checked();
}

// ****************************************************************************
// Method: ViewerToggleAction::Update
//
// Purpose: 
//   Updates the appearance of the action if it has a visual representation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003
//   Check for valid icons before attempting to set them.
// 
//   Brad Whitlock, Tue May 27 14:19:15 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerToggleAction::Update()
{
    // Update the action's enabled state.
    bool actionShouldBeEnabled = Enabled();
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the action's toggled state if it is a toggle action.
    if(action->isCheckable())
    {
        bool actionShouldBeToggled = Checked();
        if(toggled != actionShouldBeToggled)
        {
            // Set the appropriate icon into the action.
            if (!window->GetNoWinMode() &&
                !action->icon().isNull())
            {
                if(actionShouldBeToggled)
                    SetIcon(QIcon(*toggledIcon));
                else
                    SetIcon(QIcon(*regularIcon));
            }
            action->blockSignals(true);
            action->setChecked(actionShouldBeToggled);
            action->blockSignals(false);
        }
        toggled = actionShouldBeToggled;
    }
}
