// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_ACTIONS_H
#define ENGINE_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: ClearCacheForAllEnginesAction
//
// Purpose:
//   Handles ViewerRPC::ClearCacheForAllEnginesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ClearCacheForAllEnginesAction : public ViewerActionLogic
{
public:
    ClearCacheForAllEnginesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearCacheForAllEnginesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ClearCacheAction
//
// Purpose:
//   Handles ViewerRPC::ClearCacheRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ClearCacheAction : public ViewerActionLogic
{
public:
    ClearCacheAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearCacheAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CloseComputeEngineAction
//
// Purpose:
//   Handles ViewerRPC::CloseComputeEngineRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CloseComputeEngineAction : public ViewerActionLogic
{
public:
    CloseComputeEngineAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CloseComputeEngineAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: GetProcInfoAction
//
// Purpose:
//   Handles ViewerRPC::GetProcInfoRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API GetProcInfoAction : public ViewerActionLogic
{
public:
    GetProcInfoAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~GetProcInfoAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: OpenComputeEngineAction
//
// Purpose:
//   Handles ViewerRPC::OpenComputeEngineRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenComputeEngineAction : public ViewerActionLogic
{
public:
    OpenComputeEngineAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~OpenComputeEngineAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SendSimulationCommandAction
//
// Purpose:
//   Handles ViewerRPC::SendSimulationCommandRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SendSimulationCommandAction : public ViewerActionLogic
{
public:
    SendSimulationCommandAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SendSimulationCommandAction() {}

    virtual void Execute();
};

#endif
