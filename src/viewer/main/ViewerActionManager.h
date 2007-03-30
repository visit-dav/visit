#ifndef VIEWER_ACTION_MANAGER_H
#define VIEWER_ACTION_MANAGER_H
#include <viewer_exports.h>
#include <ViewerRPC.h>
#include <string>
#include <vector>
#include <vectortypes.h>

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
// ****************************************************************************

class VIEWER_API ViewerActionManager
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
private:
    ViewerActionBase *GetAction(ActionIndex) const;
    int         GetNumberOfActionGroupMemberships(ActionIndex index) const;
    void        AddAction(ViewerActionBase *action, ActionIndex index);
    void        AddActionGroup(const ActionGroup &group);
    void        AddActionToActionGroup(int index, ActionIndex);
    void        UpdatePopup();
    void        RealizeActionGroups();

    ViewerActionBase **actions;
    ActionGroupVector  actionGroups;
};

#endif
