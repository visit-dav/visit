# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu May 16, 2024
#   Using blt, so create lists for sources, headers, includes.
#
#****************************************************************************

# Store some configure information into a C file (like version, GIT revision
# and platform things that change).

configure_file(ConfigureInfo.C.in
               ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C @ONLY IMMEDIATE)

visit_append_list(
    NAME  visitcommon_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/DebugStreamFull.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Environment.C
          ${CMAKE_CURRENT_SOURCE_DIR}/FileFunctions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InstallationFunctions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PTY.C
          ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeSetting.C
          ${CMAKE_CURRENT_SOURCE_DIR}/StackTimer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/TimingsManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItInit.C
          ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C)

visit_append_list(
    NAME  visitcommon_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureInfo.h
          ${CMAKE_CURRENT_SOURCE_DIR}/DebugStream.h
          ${CMAKE_CURRENT_SOURCE_DIR}/DebugStreamFull.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Environment.h
          ${CMAKE_CURRENT_SOURCE_DIR}/FileFunctions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InstallationFunctions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/legend_defines.h
          ${CMAKE_CURRENT_SOURCE_DIR}/misc_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PTY.h
          ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeSetting.h
          ${CMAKE_CURRENT_SOURCE_DIR}/StackTimer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/TimingsManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItInit.h)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/misc>)

if(WIN32)
    # for GetProfilesDirectory used in FileFunctions
    visit_append_list(NAME visitcommon_DEPENDS ITEMS userenv)
    # for std::filesystem::path used in FileFunctions
    visit_append_list(NAME visitcommon_FEATURES ITEMS cxx_std_17)
endif()

