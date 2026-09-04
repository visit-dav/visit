# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************

visit_add_library(
    NAME        mdserverrpc
    SOURCES     ChangeDirectoryException.C
                ChangeDirectoryRPC.C
                CloseDatabaseRPC.C
                ConnectRPC.C
                CreateGroupListRPC.C
                ExpandPathRPC.C
                GetDBPluginInfoRPC.C
                GetDirectoryRPC.C
                GetFileListException.C
                GetFileListRPC.C
                GetMetaDataException.C
                GetMetaDataRPC.C
                GetPluginErrorsRPC.C
                GetSILRPC.C
                LoadPluginsRPC.C
                MDServerMethods.C
                MDServerState.C
                RestoreSessionRPC.C
                SaveSessionRPC.C
                SetMFileOpenOptionsRPC.C
    HEADERS     ChangeDirectoryException.h
                ChangeDirectoryRPC.h
                CloseDatabaseRPC.h
                ConnectRPC.h
                CreateGroupListRPC.h
                ExpandPathRPC.h
                GetDBPluginInfoRPC.h
                GetDirectoryRPC.h
                GetFileListException.h
                GetFileListRPC.h
                GetMetaDataException.h
                GetMetaDataRPC.h
                GetPluginErrorsRPC.h
                GetSILRPC.h
                LoadPluginsRPC.h
                MDServerMethods.h
                MDServerState.h
                mdsrpc_exports.h
                RestoreSessionRPC.h
                SaveSessionRPC.h
                SetMFileOpenOptionsRPC.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/mdserver/rpc>
    DEPENDS_ON  visitcommon avtdbatts)

