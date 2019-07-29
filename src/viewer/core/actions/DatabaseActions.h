// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DATABASE_ACTIONS_H
#define DATABASE_ACTIONS_H
#include <ViewerActionLogic.h>

class DataNode;
class EngineKey;
class avtDatabaseMetaData;
class avtDefaultPlotMetaData;

// ****************************************************************************
// Class: DatabaseActionBase
//
// Purpose:
//   Base class VIEWERCORE_API for database actions.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 16:33:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API DatabaseActionBase : public ViewerActionLogic
{
public:
    DatabaseActionBase(ViewerWindow *win);
    virtual ~DatabaseActionBase();

    static void SetSimConnectCallback(void (*cb)(EngineKey &, void *), void *cbdata);
    static void SetUpdateExpressionCallback(void (*cb)(const avtDatabaseMetaData *, void *), void *cbdata);
protected:
    int OpenDatabaseHelper(const std::string &entireDBName,
                           int timeState,
                           bool addDefaultPlots,
                           bool updateWindowInfo,
                           const std::string &forcedFileType);
    DataNode *CreateAttributesDataNode(const avtDefaultPlotMetaData *dp) const;

private:
    static void (*simConnectCB)(EngineKey &, void *);
    static void *simConnectCBData;
    static void (*updateExpressionCB)(const avtDatabaseMetaData *, void*);
    static void *updateExpressionCBData;
};

// ****************************************************************************
// Class: ActivateDatabaseAction
//
// Purpose:
//   Handles ViewerRPC::ActivateDatabaseRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ActivateDatabaseAction : public DatabaseActionBase
{
public:
    ActivateDatabaseAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~ActivateDatabaseAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: AlterDatabaseCorrelationAction
//
// Purpose:
//   Handles ViewerRPC::AlterDatabaseCorrelationRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AlterDatabaseCorrelationAction : public DatabaseActionBase
{
public:
    AlterDatabaseCorrelationAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~AlterDatabaseCorrelationAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CheckForNewStatesAction
//
// Purpose:
//   Handles ViewerRPC::CheckForNewStatesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CheckForNewStatesAction : public DatabaseActionBase
{
public:
    CheckForNewStatesAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~CheckForNewStatesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CloseDatabaseAction
//
// Purpose:
//   Handles ViewerRPC::CloseDatabaseRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CloseDatabaseAction : public DatabaseActionBase
{
public:
    CloseDatabaseAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~CloseDatabaseAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CreateDatabaseCorrelationAction
//
// Purpose:
//   Handles ViewerRPC::CreateDatabaseCorrelationRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CreateDatabaseCorrelationAction : public DatabaseActionBase
{
public:
    CreateDatabaseCorrelationAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~CreateDatabaseCorrelationAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DeleteDatabaseCorrelationAction
//
// Purpose:
//   Handles ViewerRPC::DeleteDatabaseCorrelationRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeleteDatabaseCorrelationAction : public DatabaseActionBase
{
public:
    DeleteDatabaseCorrelationAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~DeleteDatabaseCorrelationAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: OpenDatabaseAction
//
// Purpose:
//   Handles ViewerRPC::OpenDatabaseRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenDatabaseAction : public DatabaseActionBase
{
public:
    OpenDatabaseAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~OpenDatabaseAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: OverlayDatabaseAction
//
// Purpose:
//   Handles ViewerRPC::OverlayDatabaseRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OverlayDatabaseAction : public DatabaseActionBase
{
public:
    OverlayDatabaseAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~OverlayDatabaseAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ReOpenDatabaseAction
//
// Purpose:
//   Handles ViewerRPC::ReOpenDatabaseRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ReOpenDatabaseAction : public DatabaseActionBase
{
public:
    ReOpenDatabaseAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~ReOpenDatabaseAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ReplaceDatabaseAction
//
// Purpose:
//   Handles ViewerRPC::ReplaceDatabaseRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ReplaceDatabaseAction : public DatabaseActionBase
{
public:
    ReplaceDatabaseAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~ReplaceDatabaseAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: RequestMetaDataAction
//
// Purpose:
//   Handles ViewerRPC::RequestMetaDataRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RequestMetaDataAction : public DatabaseActionBase
{
public:
    RequestMetaDataAction(ViewerWindow *win) : DatabaseActionBase(win) {}
    virtual ~RequestMetaDataAction() {}

    virtual void Execute();
};

#endif
