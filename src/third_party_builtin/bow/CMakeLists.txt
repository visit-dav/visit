# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:22:08 PST 2010
#   Add folder property.
#
#   Brad Whitlock, Thu Apr 18 14:40:01 PDT 2013
#   Provide a macro name for zlib.
#
#   Kathleen Biagas, Tue Oct 25, 2022
#   Modernize CMake usage: add includes to target property, use BUILD and
#   INSTALL interfaces.  Install target with proper exports.
#
#****************************************************************************

add_library(bow bow.C fwav.C gzlib.C)

target_include_directories(bow PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  PRIVATE
  $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIR}>)

if(NOT VISIT_HEADERS_SKIP_INSTALL)
    target_include_directories(bow PUBLIC
      $<INSTALL_INTERFACE:include/visit/third_party_builtin/bow>)
endif()

target_link_libraries(bow PRIVATE ${ZLIB_LIBRARY})

visit_install_targets_export(bow)

set_target_properties(bow PROPERTIES FOLDER thirdparty_builtin/bow)

