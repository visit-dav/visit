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

#ifndef VIEWER_ACTION_MANAGER_H
#define VIEWER_ACTION_MANAGER_H
#include <viewer_exports.h>
#include <ViewerBase.h>
#include <ViewerRPC.h>
#include <string>
#include <vector>
#include <vectortypes.h>

class DataNode;
class ViewerActionBase;
class ViewerWindow;
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
// Creation:   Wed Jan 29 12:38:01 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 10:24:00 PDT 2003
//   I added the UpdateActionConstruction method.
//
//   Brad Whitlock, Mon Jun 30 13:35:21 PST 2003
//   I added CreateNode and SetFromNode.
//
//   Brad Whitlock, Tue Feb 24 13:24:59 PST 2004
//   I added an argument to RealizeActionGroups.
//
//   Brad Whitlock, Tue Mar 16 14:25:11 PST 2004
//   I added an argument to RealizeActionGroups.
//
//   Brad Whitlock, Fri Apr 15 17:08:31 PST 2005
//   I added CopyFrom.
//
//   Brad Whitlock, Mon Feb 12 17:54:31 PST 2007
//   Added ViewerBase base class.
//
//   Brad Whitlock, Wed Feb 13 14:18:00 PST 2008
//   Added argument to SetFromNode.
//
// ****************************************************************************

class VIEWER_API ViewerActionManager : public ViewerBase
{
    typedef std::vector<ActionIndex> ActionIndexVector;

    struct ActionGroup
    {
        ActionGroup();
        ActionGroup(const std::string &n);
        ActionGroup(const ActionGroup &);
        virtual ~ActionGroup();
        void operator =(const ActionGroup &);
        void AddAction(ActionIndex);

        std::string       name;
        ActionIndexVector actions;
        bool              enabled;
        bool              canHaveToolbar;
    };

    typedef std::vector<ActionGroup> ActionGroupVector;

public:
    ViewerActionManager(ViewerWindow *window);
    virtual ~ViewerActionManager();

    void CopyFrom(const ViewerActionManager *);

    void HandleAction(const ViewerRPC &rpc);
    void Update();
    void UpdateSingleWindow();
    void EnableActions(ViewerWindowManagerAttributes *);
    void UpdateActionInformation(ViewerWindowManagerAttributes *);
    void UpdateActionConstruction(ViewerActionBase *);
    void SetActionGroupEnabled(int index, bool val, bool update=true);
    bool GetActionGroupEnabled(int index) const;
    bool GetActionGroupHasToolbar(int index) const;
    stringVector GetActionGroupNames(bool=true) const;

    void CreateNode(DataNode *parentNode);
    void SetFromNode(DataNode *parentNode, const std::string &);
private:
    ViewerActionBase *GetAction(ActionIndex) const;
    int         GetNumberOfActionGroupMemberships(ActionIndex index) const;
    void        AddAction(ViewerActionBase *action, ActionIndex index);
    void        AddActionGroup(const ActionGroup &group);
    void        AddActionToActionGroup(int index, ActionIndex);
    void        UpdatePopup();
    void        RealizeActionGroups(bool, bool);

    ViewerActionBase **actions;
    ActionGroupVector  actionGroups;
};

#endif
