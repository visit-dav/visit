# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#    Kathleen Biagas, Tue Jan 24 11:05:25 PST 2017
#    Add support for Qt5.
#
#*****************************************************************************

set(pyside_hook_sources "")
set(pyside_hook_gen_sources
    ${CMAKE_CURRENT_BINARY_DIR}/pyside_hook/pysidehook_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pyside_hook/pyside_hook_module_wrapper.cpp)

set(pyside_hook_include_paths ${Qt5Core_INCLUDE_DIRS})

set(pyside_hook_link_libs ${QT_QTCORE_LIBRARY})

PYSIDE_ADD_MODULE(pyside_hook
                  site-packages/visit
                  pyside_hook_sources
                  pyside_hook_gen_sources
                  pyside_hook_include_paths
                  pyside_hook_link_libs
                  global.h typesystem.xml)

