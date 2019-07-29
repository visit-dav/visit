# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#****************************************************************************/

# Use the FASTBIT_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET(FB_INCLUDE include)
if(VISIT_FASTBIT_DIR)
  IF(DEFINED FASTBIT_VERSION)
    IF(${FASTBIT_VERSION} VERSION_LESS "2.0.0")
      MESSAGE(STATUS "  FASTBIT_VERSION is set for FastBit 1.X")
      SET(FB_INCLUDE include)
    ELSE(${FASTBIT_VERSION} VERSION_LESS "2.0.0")
      MESSAGE(STATUS "  FASTBIT_VERSION is set for FastBit 2.X")
      SET(FB_INCLUDE include/fastbit)
      SET(VISIT_CXX_FLAGS "${VISIT_CXX_FLAGS} -std=c++11" CACHE STRING "CXX flags" FORCE)
    ENDIF(${FASTBIT_VERSION} VERSION_LESS "2.0.0")
  ELSE(DEFINED FASTBIT_VERSION)
     MESSAGE(WARNING "  FASTBIT_VERSION is not set assuming include path for FastBit 1.X")
  ENDIF(DEFINED FASTBIT_VERSION)
endif()

IF (WIN32)
    SET_UP_THIRD_PARTY(FASTBIT lib ${FB_INCLUDE} fastbit)
ELSE (WIN32)
    IF("${VISIT_CMAKE_PLATFORM}" STREQUAL "Linux")
        # Linux requires librt to resolve "clock_gettime"
        # add this as a general dep:
        #SET(FASTBIT_LIBDEP /usr/lib rt "${FASTBIT_LIBDEP}")
    ENDIF("${VISIT_CMAKE_PLATFORM}" STREQUAL "Linux")
    SET_UP_THIRD_PARTY(FASTBIT lib ${FB_INCLUDE} fastbit)
ENDIF (WIN32)

