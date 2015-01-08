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

#ifndef VIEWER_ACTION_MANAGER_H
#define VIEWER_ACTION_MANAGER_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <ViewerRPC.h>
#include <string>
#include <vector>
#include <vectortypes.h>

class DataNode;
class ViewerActionLogic;
class ViewerWindowManager;
class ViewerWindowManagerAttributes;

typedef ViewerRPC::ViewerRPCType ActionIndex;

// ****************************************************************************
// Class: ViewerActionManager
//
// Purpose:
//   This class creates and manages the actions that the viewer knows how
//   to do.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 18:23:35 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerActionManager : public ViewerBase
{
public:
    ViewerActionManager();
    virtual ~ViewerActionManager();

    void SetLogic(ActionIndex index, ViewerActionLogic *L);
    ViewerActionLogic *GetLogic(ActionIndex index) const;

    // Handle an action.
    virtual void HandleAction(const ViewerRPC &rpc);

    // Update any GUI components in all windows.
    virtual void Update();

    // Update any GUI components for the action's window (no-op)
    virtual void UpdateSingleWindow();

    // Copy data from one action manager to another.
    virtual void CopyFrom(const ViewerActionManager *);

    // Encode options onto a data node.
    virtual void CreateNode(DataNode *parentNode);

    // Set options from the data node.
    virtual void SetFromNode(DataNode *parentNode, const std::string &);

public:
    //
    // PUBLIC GUI-RELATED METHODS (non-functional in this class)
    //
    virtual void         SetActionGroupEnabled(int index, bool val, bool update=true);
    virtual bool         GetActionGroupEnabled(int) const;
    virtual stringVector GetActionGroupNames(bool = true) const;
    virtual void         EnableActions(ViewerWindowManagerAttributes *);
    virtual void         SaveActionGroups(ViewerWindowManagerAttributes *);
    virtual void         UpdateActionConstruction(ViewerRPC::ViewerRPCType rpc);

protected:
    std::vector<ViewerActionLogic *> logic;
};

#endif
