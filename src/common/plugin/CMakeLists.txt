# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  visitcommon_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/DatabasePluginInfo.C
          ${CMAKE_CURRENT_SOURCE_DIR}/DatabasePluginManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDirectoryException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidPluginException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/OperatorPluginManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PlotPluginManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PluginBroadcaster.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.C)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/plugin>)

