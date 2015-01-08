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

#include <ViewerActionManager.h>
#include <ViewerActionLogic.h>
#include <ViewerProperties.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <DataNode.h>

// ****************************************************************************
// Method: ViewerActionManager::ViewerActionManager
//
// Purpose:
//   Constructor for ViewerActionManager.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 18:39:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionManager::ViewerActionManager()
{
    int n = (int)ViewerRPC::MaxRPC;
    logic.resize(n);
    for(int i = 0; i < n; ++i)
        logic[i] = NULL;
}

// ****************************************************************************
// Method: ViewerActionManager::~ViewerActionManager
//
// Purpose:
//   Destructor for ViewerActionManager.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 18:39:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionManager::~ViewerActionManager()
{
    for(size_t i = 0; i < logic.size(); ++i)
        delete logic[i];
}

// ****************************************************************************
// Method: ViewerActionManager::SetLogic
//
// Purpose:
//   Set the logic part of the actions for a specific viewer rpc.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 18:39:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionManager::SetLogic(ActionIndex index, ViewerActionLogic *L)
{
    int idx = (int)index;
    if(logic[idx] != NULL)
        delete logic[idx];

    L->SetRPCType(index);
    logic[idx] = L;
}

// ****************************************************************************
// Method: ViewerActionManager::GetLogic
//
// Purpose:
//   Return the logic part of the actions for a specific viewer rpc.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 18:39:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionLogic *
ViewerActionManager::GetLogic(ActionIndex index) const
{
    return logic[(int)index];
}

// ****************************************************************************
// Method: ViewerActionManager::HandleAction
//
// Purpose: 
//   This method calls the action prescribed by the rpc.
//
// Arguments:
//   rpc : The rpc to execute.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:05:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::HandleAction(const ViewerRPC &rpc)
{
    int idx = (int)rpc.GetRPCType();
    if(logic[idx] != NULL)
    {
        // Store the action's arguments.
        ViewerActionLogic::SetArgs(rpc);

        // Proces the action.
        logic[idx]->Execute();

        Update();
    }
}

// ****************************************************************************
// Method: ViewerActionManager::Update
//
// Purpose: 
//   Updates all of the actions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 12:22:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::Update()
{
    if(!GetViewerProperties()->GetNowin())
    {
        std::vector<ViewerWindow *> windows(ViewerWindowManager::Instance()->
                                            GetWindows());
        for(size_t i = 0; i < windows.size(); ++i)
            windows[i]->GetActionManager()->UpdateSingleWindow();
    }
}

// ****************************************************************************
// Method: ViewerActionManager::UpdateSingleWindow
//
// Purpose: 
//   Updates the actions in the window that owns the action manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 13:15:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::UpdateSingleWindow()
{
}

void
ViewerActionManager::SetActionGroupEnabled(int index, bool val, bool update)
{
}

bool
ViewerActionManager::GetActionGroupEnabled(int) const
{
    return true;
}

stringVector
ViewerActionManager::GetActionGroupNames(bool) const
{
    return stringVector();
}

// ****************************************************************************
// Method: ViewerActionManager::EnableActions
//
// Purpose: 
//   This method is called when a new window is created. Its job is to
//   process the settings to customize the actions and actiongroups so that
//   the toolbar and popup menu will be customized to user-settings.
//
// Arguments:
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:07:03 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionManager::EnableActions(ViewerWindowManagerAttributes *)
{
}

// ****************************************************************************
// Method: ViewerActionManager::SaveActionGroups
//
// Purpose: 
//   This method is called when we want to save some information about the
//   action groups to the window manager attributes.
//
// Arguments:
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:07:03 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionManager::SaveActionGroups(ViewerWindowManagerAttributes *)
{
}

// ****************************************************************************
// Method: ViewerActionManager::UpdateActionConstruction
//
// Purpose: 
//   Removes an action from all of the menus and toolbars to which it belongs
//   and then adds it back so it has the right number of sub actions.
//
// Arguments:
//   action : The action that we're updating in the menu and the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 09:54:06 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionManager::UpdateActionConstruction(ViewerRPC::ViewerRPCType rpc)
{
}

// ****************************************************************************
// Method: ViewerActionManager::CopyFrom
//
// Purpose: 
//   Copies action attributes from 1 action manager to this action manager.
//
// Arguments:
//   mgr : The source action manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 15 17:27:05 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::CopyFrom(const ViewerActionManager *mgr)
{
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        if(logic[i] != NULL && mgr->logic[i] != NULL)
            logic[i]->CopyFrom(mgr->logic[i]);
    }
}

// ****************************************************************************
// Method: ViewerActionManager::CreateNode
//
// Purpose: 
//   Lets each action add its information to the config file node.
//
// Arguments:
//   parentNode : The node to which data will be added.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 10:14:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = new DataNode("ViewerActionManager");

    // Let each action have the chance to add itself to the node.
    bool addToNode = false;
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        if(logic[i] != NULL && logic[i]->CreateNode(mgrNode))
            addToNode = true;
    }

    if(addToNode)
        parentNode->AddNode(mgrNode);
    else
        delete mgrNode;
}

// ****************************************************************************
// Method: ViewerActionManager::SetFromNode
//
// Purpose: 
//   Lets each action initialize itself from data in the node.
//
// Arguments:
//   parentNode    : The node from which config information is read.
//   configVersion : The version from the config file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 10:15:23 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Feb 13 14:03:06 PST 2008
//   Added configVersion.
//
// ****************************************************************************

void
ViewerActionManager::SetFromNode(DataNode *parentNode, const std::string &configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = parentNode->GetNode("ViewerActionManager");
    if(mgrNode == 0)
        return;

    // Let each action have the chance to initialize itself using the node.
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        if(logic[i] != NULL)
            logic[i]->SetFromNode(mgrNode, configVersion);
    }
}
