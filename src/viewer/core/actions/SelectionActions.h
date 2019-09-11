// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SELECTION_ACTIONS_H
#define SELECTION_ACTIONS_H
#include <ViewerActionLogic.h>

class EngineKey;

// ****************************************************************************
// Class: SelectionActionBase
//
// Purpose:
//   Base class VIEWERCORE_API for selection actions.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SelectionActionBase : public ViewerActionLogic
{
public:
    SelectionActionBase(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SelectionActionBase() {}
protected:
    bool GetNamedSelectionEngineKey(const std::string &selName, EngineKey &ek);
    void ReplaceNamedSelection(const EngineKey &engineKey,
                               const std::string &selName,
                               const std::string &newSelName);
};

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: ApplyNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::ApplyNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ApplyNamedSelectionAction : public SelectionActionBase
{
public:
    ApplyNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~ApplyNamedSelectionAction() {}

    virtual void Execute();

    virtual void ClearNamedSelection();
};

// ****************************************************************************
// Class: CreateNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::CreateNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CreateNamedSelectionAction : public SelectionActionBase
{
public:
    CreateNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~CreateNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DeleteNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::DeleteNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeleteNamedSelectionAction : public SelectionActionBase
{
public:
    DeleteNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~DeleteNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: InitializeNamedSelectionVariablesAction
//
// Purpose:
//   Handles ViewerRPC::InitializeNamedSelectionVariablesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API InitializeNamedSelectionVariablesAction : public SelectionActionBase
{
public:
    InitializeNamedSelectionVariablesAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~InitializeNamedSelectionVariablesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: LoadNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::LoadNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API LoadNamedSelectionAction : public SelectionActionBase
{
public:
    LoadNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~LoadNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SaveNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::SaveNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SaveNamedSelectionAction : public SelectionActionBase
{
public:
    SaveNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~SaveNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetNamedSelectionAutoApplyAction
//
// Purpose:
//   Handles ViewerRPC::SetNamedSelectionAutoApplyRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetNamedSelectionAutoApplyAction : public SelectionActionBase
{
public:
    SetNamedSelectionAutoApplyAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~SetNamedSelectionAutoApplyAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: UpdateNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::UpdateNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API UpdateNamedSelectionAction : public SelectionActionBase
{
public:
    UpdateNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~UpdateNamedSelectionAction() {}

    virtual void Execute();
private:
    void UpdateNamedSelection(const std::string &selName, bool updatePlots,
                              bool allowCache);
};

#endif
