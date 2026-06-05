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
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/DatabasePluginInfo.C
          ${CMAKE_CURRENT_SOURCE_DIR}/DatabasePluginManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDirectoryException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidPluginException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/OperatorPluginManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PlotPluginManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PluginBroadcaster.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.C)

visit_append_list(
    NAME  visitcommon_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/database_plugin_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/DatabasePluginInfo.h
          ${CMAKE_CURRENT_SOURCE_DIR}/DatabasePluginManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDirectoryException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidPluginException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/operator_plugin_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/OperatorPluginInfo.h
          ${CMAKE_CURRENT_SOURCE_DIR}/OperatorPluginManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/plot_plugin_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PlotPluginInfo.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PlotPluginManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/plugin_entry_point.h
          ${CMAKE_CURRENT_SOURCE_DIR}/plugin_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/plugin_vartypes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PluginBroadcaster.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.h)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/plugin>)

