# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Fri Mar 17 10:27:23 PDT 2017
#   Set HAVE_NEKTAR_PP if NEKTAR++_FOUND.
#
#****************************************************************************/

# Use the NEKTAR++_DIR hint from the config-site .cmake file 

#MESSAGE(STATUS "  NEKTAR++_VERSION=${NEKTAR++_VERSION}")
#MESSAGE(STATUS "  VISIT_NEKTAR++_DIR=${VISIT_NEKTAR++_DIR}")

# The libraries as of Nektar++ 4.4
SET(NEKTAR++_LIBRARIES LibUtilities
		       StdRegions
		       SpatialDomains
		       LocalRegions
		       MultiRegions 
		       Collections
		       GlobalMapping
		       FieldUtils)

If( EXISTS ${VISIT_NEKTAR++_DIR} )

    IF(EXISTS ${VISIT_NEKTAR++_DIR}/lib64)
        SET(LIB lib64)
    ELSE()
        SET(LIB lib)
    ENDIF()

    IF(EXISTS ${VISIT_NEKTAR++_DIR}/${LIB}/nektar++/cmake/NEKTAR++Config.cmake)
        SET(NEKTAR++_DIR ${VISIT_NEKTAR++_DIR})
    ENDIF()

    MESSAGE(STATUS "Checking for Nektar++ in ${NEKTAR++_DIR}/${LIB}/nektar++/cmake")

    #  Find Nektar++
    set(CMAKE_PREFIX_PATH ${VISIT_NEKTAR++_DIR}/${LIB}/nektar++/cmake ${CMAKE_PREFIX_PATH})
    set(CMAKE_LIBRARY_PATH ${VISIT_NEKTAR++_DIR}/${LIB}/nektar++ ${CMAKE_LIBRARY_PATH})

    FIND_PACKAGE(Nektar++)
#    FIND_PACKAGE(NEKTAR++ 5.0.0 REQUIRED PATHS ${NEKTAR++_DIR})

#    MESSAGE(STATUS "  NEKTAR++_INCLUDE_DIRS=${NEKTAR++_INCLUDE_DIRS}")
#    MESSAGE(STATUS "  NEKTAR++_TP_INCLUDE_DIRS=${NEKTAR++_TP_INCLUDE_DIRS}")
#    MESSAGE(STATUS "  NEKTAR++_LIBRARY_DIRS=${NEKTAR++_LIBRARY_DIRS}")
#    MESSAGE(STATUS "  NEKTAR++_TP_LIBRARY_DIRS=${NEKTAR++_TP_LIBRARY_DIRS}")

#    MESSAGE(STATUS "  NEKTAR++_DEFINITIONS=${NEKTAR++_DEFINITIONS}")
ENDIF()

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(NEKTAR++
    ${LIB}
    include/nektar++
    ${NEKTAR++_LIBRARIES}
    )

IF(NEKTAR++_FOUND)
    SET(HAVE_NEKTAR_PP true CACHE BOOL "Have Nektar++ lib")
else()
    unset(NEKTAR++_LIBRARIES)
ENDIF(NEKTAR++_FOUND)

