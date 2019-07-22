// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
