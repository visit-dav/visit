// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MDSERVER_STATE_H
#define MDSERVER_STATE_H

#include <mdsrpc_exports.h>

#include <GetDirectoryRPC.h>
#include <ChangeDirectoryRPC.h>
#include <GetFileListRPC.h>
#include <GetMetaDataRPC.h>
#include <GetSILRPC.h>
#include <ConnectRPC.h>
#include <CreateGroupListRPC.h>
#include <ExpandPathRPC.h>
#include <CloseDatabaseRPC.h>
#include <LoadPluginsRPC.h>
#include <GetPluginErrorsRPC.h>
#include <GetDBPluginInfoRPC.h>
#include <SetMFileOpenOptionsRPC.h>
#include <SaveSessionRPC.h>
#include <RestoreSessionRPC.h>

#include <maptypes.h>
#include <vectortypes.h>

class Xfer;
class VisItRPC;

class MDSERVER_RPC_API MDServerState
{
public:
    MDServerState();
    ~MDServerState();
    size_t                     GetNumStateObjects();
    VisItRPC*                  GetStateObject(int i);

    void                       DetermineSeparator();
    char                       separator;

    void SetupComponentRPCs(Xfer* xfer);


    GetDirectoryRPC&            GetGetDirectoryRPC() { return getDirectoryRPC; }
    ChangeDirectoryRPC&         GetChangeDirectoryRPC() { return changeDirectoryRPC; }
    GetFileListRPC&             GetGetFileListRPC() { return getFileListRPC; }
    GetMetaDataRPC&             GetGetMetaDataRPC() { return getMetaDataRPC; }
    GetSILRPC&                  GetGetSILRPC() { return getSILRPC; }
    ConnectRPC&                 GetConnectRPC() { return connectRPC; }
    CreateGroupListRPC&         GetCreateGroupListRPC() { return createGroupListRPC; }
    ExpandPathRPC&              GetExpandPathRPC() { return expandPathRPC; }
    CloseDatabaseRPC&           GetCloseDatabaseRPC() { return closeDatabaseRPC; }
    LoadPluginsRPC&             GetLoadPluginsRPC() { return loadPluginsRPC; }
    GetPluginErrorsRPC&         GetGetPluginErrorsRPC() { return getPluginErrorsRPC; }
    GetDBPluginInfoRPC&         GetGetDBPluginInfoRPC() { return getDBPluginInfoRPC; }
    SetMFileOpenOptionsRPC&     GetSetMFileOpenOptionsRPC() { return setMFileOpenOptionsRPC; }
    SaveSessionRPC&             GetGetSaveSessionRPC() { return saveSessionRPC; }
    RestoreSessionRPC&          GetGetRestoreSessionRPC() { return restoreSessionRPC; }

private:
    GetDirectoryRPC            getDirectoryRPC;
    ChangeDirectoryRPC         changeDirectoryRPC;
    GetFileListRPC             getFileListRPC;
    GetMetaDataRPC             getMetaDataRPC;
    GetSILRPC                  getSILRPC;
    ConnectRPC                 connectRPC;
    CreateGroupListRPC         createGroupListRPC;
    ExpandPathRPC              expandPathRPC;
    CloseDatabaseRPC           closeDatabaseRPC;
    LoadPluginsRPC             loadPluginsRPC;
    GetPluginErrorsRPC         getPluginErrorsRPC;
    GetDBPluginInfoRPC         getDBPluginInfoRPC;
    SetMFileOpenOptionsRPC     setMFileOpenOptionsRPC;
    SaveSessionRPC             saveSessionRPC;
    RestoreSessionRPC          restoreSessionRPC;

    std::vector<VisItRPC*> objVector;
    friend class MDServerMethods;
};

#endif
