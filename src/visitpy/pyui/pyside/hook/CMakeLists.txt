# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#    Kathleen Biagas, Tue Jan 24 11:05:25 PST 2017
#    Add support for Qt5.
#
#    Cyrus Harrison, Wed Feb 24 16:06:36 PST 2021
#    A few small changes for pyside2
#*****************************************************************************

# PYSIDE_ADD_MODULE handles the most of the qt include paths

set(pyside_hook_sources "")

set(pyside_hook_gen_sources
    ${CMAKE_CURRENT_BINARY_DIR}/pyside_hook/pysidehook_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pyside_hook/pyside_hook_module_wrapper.cpp)

# Note: 
# Cyrus Harrison, Wed Feb 24 16:05:47 PST 2021
# These include paths are still needed for the hook lib, which
# is a bit puzzling. It may be due to how we are using the includes
# like <QtCore/QtCore.  I tried a few combos to explore, but
# the basic attempts failed to compile. 
set(pyside_hook_include_paths ${Qt5Core_INCLUDE_DIRS})
set(pyside_hook_link_libs ${QT_QTCORE_LIBRARY})

PYSIDE_ADD_MODULE(pyside_hook
                  site-packages/visit_utils/builtin
                  pyside_hook_sources
                  pyside_hook_gen_sources
                  pyside_hook_include_paths
                  pyside_hook_link_libs
                  global.h typesystem.xml)

