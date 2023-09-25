# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
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

if(WIN32)
    visit_append_list(
       NAME  visitcommon_SOURCES
       ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/win32commhelpers.C)
endif()

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/comm>)

