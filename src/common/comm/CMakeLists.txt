# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_include_directories(visitcommon PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${VISIT_COMMON_INCLUDES}>")

if(NOT VISIT_HEADERS_SKIP_INSTALL)
    target_include_directories(visitcommon PUBLIC
      $<INSTALL_INTERFACE:include/visit/common/comm>
      $<INSTALL_INTERFACE:include/visit/include>)
endif()

target_sources(visitcommon PRIVATE
  BadHostException.C
  BufferConnection.C
  CommunicationHeader.C
  Connection.C
  ConnectionGroup.C
  DConvert.C
  ExistingRemoteProcess.C
  FConvert.C
  FileConnection.C
  IConvert.C
  LaunchService.C
  LConvert.C
  ParentProcess.C
  RemoteProcess.C
  SocketBridge.C
  SocketConnection.C
  TypeRepresentation.C
)

if(WIN32)
    target_sources(visitcommon PRIVATE win32commhelpers.C)
endif()

