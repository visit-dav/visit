# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Fri Mar 17 10:27:23 PDT 2017
#   Set HAVE_NETKAR_PP if NEKTAR++_FOUND.
#
#   Kathleen Biagas, Mon Sep 12, 2022
#   Support Nektar 5.0.0 (version used in build_visit).
#   There are fewer libraries built.
#   Cleaned up find_package call that is necessary to pick up the nektar++
#   defined CMakeVariables. 
#
#****************************************************************************/

# Use the NEKTAR++_DIR hint from the config-site .cmake file 

#MESSAGE(STATUS "  NEKTAR++_VERSION=${NEKTAR++_VERSION}")
#MESSAGE(STATUS "  VISIT_NEKTAR++_DIR=${VISIT_NEKTAR++_DIR}")

set(NEKTAR++_LIBRARIES
       Collections
       LibUtilities
       LocalRegions
       MultiRegions
       SpatialDomains
       StdRegions)

# if NEKTAR++_VERSION not defined, will assume latest from build_visit, which is 5.0.0
# otherwise check the version number
if(DEFINED NEKTAR++_VERSION AND NEKTAR++_VERSION VERSION_LESS "5.0.0")
    # Additional libraries for Nektar++ 4.4
    list(APPEND NEKTAR++_LIBRARIES
            FieldUtils
            GlobalMapping)
endif()

if(EXISTS ${VISIT_NEKTAR++_DIR})
    # need to call find_package to pick up CMake vars defined in Nektar++'s config .cmake files
    find_package(Nektar++ NO_MODULE PATHS ${VISIT_NEKTAR++_DIR})

    # message(STATUS "  NEKTAR++_INCLUDE_DIRS=${NEKTAR++_INCLUDE_DIRS}")
    # message(STATUS "  NEKTAR++_TP_INCLUDE_DIRS=${NEKTAR++_TP_INCLUDE_DIRS}")
    # message(STATUS "  NEKTAR++_LIBRARY_DIRS=${NEKTAR++_LIBRARY_DIRS}")
    # message(STATUS "  NEKTAR++_TP_LIBRARY_DIRS=${NEKTAR++_TP_LIBRARY_DIRS}")
    # message(STATUS "  NEKTAR++_DEFINITIONS=${NEKTAR++_DEFINITIONS}")
endif()

include(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

# this will 'find' the Nektar++ libraries and also set them up for installation
SET_UP_THIRD_PARTY(NEKTAR++
    INCDIR include/nektar++
    LIBS ${NEKTAR++_LIBRARIES}
    )

if(NEKTAR++_FOUND)
    set(HAVE_NEKTAR_PP true CACHE BOOL "Have Nektar++ lib")
else()
    unset(NEKTAR++_LIBRARIES)
endif(NEKTAR++_FOUND)

