# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#****************************************************************************/
#[=[
# If the config-site file did not set a VTK version, then set a
# default VTK version of the minimum required.
#]=]
if(NOT VTK_VERSION)
  if(VISIT_VTK_VERSION)
      if(${VISIT_VTK_VERSION} VERSION_LESS ${VTK_MINIMUM_VERSION})
          message(FATAL_ERROR "VTK version must be at least ${VTK_MINIMUM_VERSION}")
      endif()
      SETUP_APP_VERSION(VTK ${VISIT_VTK_VERSION})
  else()
      message(STATUS "VISIT_VTK_VERSION NOT FOUND assuming ${VTK_MINIMUM_VERSION}")
      SETUP_APP_VERSION(VTK ${VTK_MINIMUM_VERSION})
  endif()
endif()

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

