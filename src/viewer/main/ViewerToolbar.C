/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ViewerToolbar.h>
#include <ViewerAction.h>
#include <ViewerWindow.h>
#include <qtoolbar.h>

// ****************************************************************************
// Method: ViewerToolbar::ViewerToolbar
//
// Purpose: 
//   Constructor for the ViewerToolbar class.
//
// Arguments:
//   win : A pointer to the ViewerWindow that owns this toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:48:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerToolbar::ViewerToolbar(ViewerWindow *win) : toolbars()
{    
    window = win;
    hidden = true;
}

// ****************************************************************************
// Method: ViewerToolbar::~ViewerToolbar
//
// Purpose: 
//   Destructor for the ViewerToolbar class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:48:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerToolbar::~ViewerToolbar()
{
}

// ****************************************************************************
// Method: ViewerToolbar::Show
//
// Purpose: 
//   Shows the named toolbar.
//
// Arguments:
//   toolBarName : The name of the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:41:58 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:14:20 PST 2004
//   I made it so the toolbar is not really shown unless the whole toolbar
//   is not hidden. The visibilty flag still gets set though so when the
//   toolbar is made active, the right toolbars are shown.
//
// ****************************************************************************

void
ViewerToolbar::Show(const std::string &toolBarName)
{
    ToolbarMap::iterator pos;
    if((pos = toolbars.find(toolBarName)) != toolbars.end())
    {
        if(pos->second.toolbar)
        {
            if(!hidden)
                pos->second.toolbar->show();
            pos->second.visible = true;
        }
    }
}

// ****************************************************************************
// Method: ViewerToolbar::Hide
//
// Purpose: 
//   Hides the named toolbar.
//
// Arguments:
//   toolBarName : The name of the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:41:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToolbar::Hide(const std::string &toolBarName)
{
    ToolbarMap::iterator pos;
    if((pos = toolbars.find(toolBarName)) != toolbars.end())
    {
        if(pos->second.toolbar)
        {
            pos->second.toolbar->hide();
            pos->second.visible = false;
        }
    }
}

// ****************************************************************************
// Method: ViewerToolbar::ShowAll
//
// Purpose: 
//   Shows all of the toolbars that have been marked as visible. This way we
//   can create some toolbars and have them not show up if they are not
//   enabled.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:41:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToolbar::ShowAll()
{
    hidden = false;
    // Iterate through the toolbars in the map and show them.
    ToolbarMap::iterator pos;
    for(pos = toolbars.begin(); pos != toolbars.end(); ++pos)
    {
        if(pos->second.visible && pos->second.toolbar)
            pos->second.toolbar->show();
    }
}

// ****************************************************************************
// Method: ViewerToolbar::HideAll
//
// Purpose: 
//   Hides all of the toolbars that have been marked as visible.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:41:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToolbar::HideAll()
{
    hidden = true;
    // Iterate through the toolbars in the map and hide them.
    ToolbarMap::iterator pos;
    for(pos = toolbars.begin(); pos != toolbars.end(); ++pos)
    {
        if(pos->second.visible && pos->second.toolbar)
            pos->second.toolbar->hide();
    }
}

// ****************************************************************************
// Method: ViewerToolbar::AddAction
//
// Purpose: 
//   Adds an action to the named toolbar. If the toolbar does not exist, it
//   is created.
//
// Arguments:
//   toolBarName : The name of the toolbar.
//   action          : The action that we're adding to the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:45:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToolbar::AddAction(const std::string &toolBarName,
    ViewerActionBase *action)
{
    // Create the named toolbar.
    ToolbarItem item(CreateToolbar(toolBarName));

    // If the toolbar was created then let the action add itself to the toolbar.
    if(item.toolbar)
    {
        // Let the action add its buttons to the toolbar.
        action->ConstructToolbar(item.toolbar);
    }
}

// ****************************************************************************
// Method: ViewerToolbar::RemoveAction
//
// Purpose: 
//   Lets an action remove itself from all toolbars.
//
// Arguments:
//   action : The action that we want to remove.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 10:27:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerToolbar::RemoveAction(ViewerActionBase *action)
{
    ToolbarMap::iterator pos;
    for(pos = toolbars.begin(); pos != toolbars.end(); ++pos)
        action->RemoveFromToolbar(pos->second.toolbar);
}

// ****************************************************************************
// Method: ViewerToolbar::CreateToolbar
//
// Purpose: 
//   Creates a toolbar with the specified name. If that toolbar already exists
//   then the existing toolbar is returned.
//
// Arguments:
//   toolBarName : The name of the toolbar to create.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 09:46:44 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 14:09:18 PST 2004
//   I added code to make sure that the new toolbar is hidden by default.
//
// ****************************************************************************

ViewerToolbar::ToolbarItem
ViewerToolbar::CreateToolbar(const std::string &toolBarName)
{
    ToolbarMap::iterator pos;
    if((pos = toolbars.find(toolBarName)) != toolbars.end())
        return pos->second;
    else
    {
        // Create a toolbar and make sure that it is hidden.
        QToolBar *tb = (QToolBar *)window->CreateToolbar(toolBarName);
        if(tb != 0)
            tb->hide();

        ToolbarItem retval(tb, false);
        toolbars[toolBarName] = retval;

        return retval;
    }
}

//
// ViewerToolbar::ToolbarItem class.
//

ViewerToolbar::ToolbarItem::ToolbarItem()
{
    toolbar = 0;
    visible = false;
}

ViewerToolbar::ToolbarItem::ToolbarItem(const ViewerToolbar::ToolbarItem &obj)
{
    toolbar = obj.toolbar;
    visible = obj.visible;
}

ViewerToolbar::ToolbarItem::ToolbarItem(QToolBar *tb, bool v)
{
    toolbar = tb;
    visible = v;
}

ViewerToolbar::ToolbarItem::~ToolbarItem()
{
}

void
ViewerToolbar::ToolbarItem::operator = (const ViewerToolbar::ToolbarItem &obj)
{
    toolbar = obj.toolbar;
    visible = obj.visible;
}
