// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
