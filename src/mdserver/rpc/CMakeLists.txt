# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(MDSERVERRPC_SOURCES
ChangeDirectoryException.C
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
SetMFileOpenOptionsRPC.C
SaveSessionRPC.C
RestoreSessionRPC.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(mdserverrpc ${MDSERVERRPC_SOURCES})
TARGET_LINK_LIBRARIES(mdserverrpc visitcommon avtdbatts)

VISIT_INSTALL_TARGETS(mdserverrpc)
