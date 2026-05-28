# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu May 16, 2024
#   Using blt, so create lists for sources, headers, includes.
#
#****************************************************************************


visit_append_list(
    NAME  visitcommon_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BadHostException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BufferConnection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/CommunicationHeader.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Connection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionGroup.C
          ${CMAKE_CURRENT_SOURCE_DIR}/DConvert.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ExistingRemoteProcess.C
          ${CMAKE_CURRENT_SOURCE_DIR}/FConvert.C
          ${CMAKE_CURRENT_SOURCE_DIR}/FileConnection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/IConvert.C
          ${CMAKE_CURRENT_SOURCE_DIR}/LaunchService.C
          ${CMAKE_CURRENT_SOURCE_DIR}/LConvert.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ParentProcess.C
          ${CMAKE_CURRENT_SOURCE_DIR}/RemoteProcess.C
          ${CMAKE_CURRENT_SOURCE_DIR}/SocketBridge.C
          ${CMAKE_CURRENT_SOURCE_DIR}/SocketConnection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/TypeRepresentation.C)

visit_append_list(
    NAME  visitcommon_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BadHostException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BufferConnection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CancelledConnectException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ChangeUsernameException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/comm_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CommunicationHeader.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ConnectCallback.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Connection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionGroup.h
          ${CMAKE_CURRENT_SOURCE_DIR}/convert.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CouldNotConnectException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ExistingRemoteProcess.h
          ${CMAKE_CURRENT_SOURCE_DIR}/FileConnection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/IncompatibleSecurityTokenException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/IncompatibleVersionException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/LaunchService.h
          ${CMAKE_CURRENT_SOURCE_DIR}/LostConnectionException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ParentProcess.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PipeConnection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/RemoteProcess.h
          ${CMAKE_CURRENT_SOURCE_DIR}/SocketBridge.h
          ${CMAKE_CURRENT_SOURCE_DIR}/SocketConnection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/TypeRepresentation.h)

if(WIN32)
    visit_append_list(
       NAME  visitcommon_SOURCES
       ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/win32commhelpers.C)
    visit_append_list(
       NAME  visitcommon_HEADERS
       ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/win32commhelpers.h)
endif()

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/comm>)

