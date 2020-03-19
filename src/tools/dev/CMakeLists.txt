# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Remove if test that is handled by the parent dir's CMakeLists.txt
#  (VISIT_DBIO_ONLY, VISIT_ENGINE_ONLY, AND VISIT_SERVER_COMPONENTS_ONLY)
#  VISIT_BUILD_DIAGNOSTICS renamed to VISIT_ENABLE_DIAGNOSTICS.
#
#****************************************************************************

if(VISIT_ENABLE_DIAGNOSTICS)
    add_subdirectory(diagnostics)
endif()

if(NOT WIN32)
    add_subdirectory(clipeditor)
endif()

add_subdirectory(protocol)
add_subdirectory(xml)
add_subdirectory(xmledit)

# These do not need to be built:
# avt_do_add
# doxygenate
# escan
# vtk_debug_parser
