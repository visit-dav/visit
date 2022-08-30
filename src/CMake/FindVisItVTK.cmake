# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#****************************************************************************/

if(VTK_VERSION VERSION_EQUAL "9.1.0")
    include(${VISIT_SOURCE_DIR}/CMake/FindVTK9.cmake)
elseif(VTK_VERSION VERSION_EQUAL "8.1.0")
    include(${VISIT_SOURCE_DIR}/CMake/FindVTK8.cmake)
else()
    message(FATAL_ERROR "Only VTK versions 8.1.0 and 9.1.0 are supported")
endif()

if(VTK_FOUND)
    add_definitions("-DVTK_VERSION_HEX=${VTK_VERSION_HEX}")
endif()
