#ifndef VIEWER_ACTION_MANAGER_H
#define VIEWER_ACTION_MANAGER_H
#include <viewer_exports.h>
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
    void SetFromNode(DataNode *parentNode);
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
