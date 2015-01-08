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

#ifndef VIEWER_ACTION_MANAGER_UI_H
#define VIEWER_ACTION_MANAGER_UI_H
#include <viewer_exports.h>
#include <ViewerActionManager.h>

class ViewerActionUI;

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
// Creation:   Mon Aug 18 16:38:16 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerActionManagerUI : public ViewerActionManager
{
public:
    ViewerActionManagerUI();
    virtual ~ViewerActionManagerUI();

    void            SetUI(ActionIndex index, ViewerActionUI *ui);
    ViewerActionUI *GetUI(ActionIndex index);

    // Update any GUI components for the action's window (no-op)
    virtual void UpdateSingleWindow();

public:
    //
    // PUBLIC GUI-RELATED METHODS
    //
    virtual void         SetActionGroupEnabled(int index, bool val, bool update=true);
    virtual bool         GetActionGroupEnabled(int index) const;
    virtual stringVector GetActionGroupNames(bool = true) const;
    virtual void         EnableActions(ViewerWindowManagerAttributes *);
    virtual void         SaveActionGroups(ViewerWindowManagerAttributes *);
    virtual void         UpdateActionConstruction(ActionIndex index);
private:
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
private:
    //
    // PRIVATE GUI-RELATED METHODS
    void        AddActionGroup(const ActionGroup &group);
    void        RealizeActionGroups(bool, bool);
    int         GetNumberOfActionGroupMemberships(ActionIndex index) const;
    void        UpdatePopup();
private:
    ActionGroupVector             actionGroups;
    std::vector<ViewerActionUI *> ui;
    bool                          actionsCreated;
};

#endif
