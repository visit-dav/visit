# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#****************************************************************************/

if(VTK_MAJOR_VERSION EQUAL 9)
    include(${VISIT_SOURCE_DIR}/CMake/FindVTK9.cmake)
elseif(VTK_MAJOR_VERSION EQUAL 8)
    include(${VISIT_SOURCE_DIR}/CMake/FindVTK8.cmake)
else()
    message(FATAL_ERROR "Only VTK versions 8.1 and 9.1 are supported")
endif()

if(VTK_FOUND)
    add_definitions("-DVTK_VERSION_HEX=${VTK_VERSION_HEX}")
endif()
