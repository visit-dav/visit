# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the OSPRAY_DIR hint from the config-site .cmake file

#MESSAGE(STATUS "  OSPRAY_VERSION=${OSPRAY_VERSION}")
#MESSAGE(STATUS "  VISIT_OSPRAY_DIR=${VISIT_OSPRAY_DIR}")

# The libraries as of ospray 2.8
SET(OSPRAY_LIBRARIES ospray
                     ospray_imgui
                     ospray_module_ispc
		     embree3
		     openvkl
		     openvkl_module_cpu_device
		     openvkl_module_cpu_device_16
		     openvkl_module_cpu_device_8
		     openvkl_module_cpu_device_4
		     rkcommon
		     tbbmalloc)

if(WIN32)
    list(APPEND OSPRAY_LIBRARIES tbb12)
else()
    list(APPEND OSPRAY_LIBRARIES tbb)
endif()

If( EXISTS ${VISIT_OSPRAY_DIR} )

    IF(EXISTS ${VISIT_OSPRAY_DIR}/lib64)
        SET(LIB lib64)
    ELSE()
        SET(LIB lib)
    ENDIF()

    IF(EXISTS ${VISIT_OSPRAY_DIR}/${LIB}/cmake/ospray-${OSPRAY_VERSION}/osprayConfig.cmake)
        SET(OSPRAY_DIR ${VISIT_OSPRAY_DIR})
    ENDIF()

    MESSAGE(STATUS "Checking for ospray in ${OSPRAY_DIR}/${LIB}/cmake/ospray-${OSPRAY_VERSION}")

    # Find ospray
    set(CMAKE_PREFIX_PATH ${VISIT_OSPRAY_DIR}/${LIB}/cmake/ospray-${OSPRAY_VERSION} ${CMAKE_PREFIX_PATH})
    set(CMAKE_LIBRARY_PATH ${VISIT_OSPRAY_DIR}/${ospray}/LIB ${CMAKE_LIBRARY_PATH})

    FIND_PACKAGE(ospray)

#    MESSAGE(STATUS "  OSPRAY_INCLUDE_DIRS=${OSPRAY_INCLUDE_DIRS}")
#    MESSAGE(STATUS "  OSPRAY_LIBRARY_DIRS=${OSPRAY_LIBRARY_DIRS}")
#    MESSAGE(STATUS "  OSPRAY_DEFINITIONS=${OSPRAY_DEFINITIONS}")
ENDIF()

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(OSPRAY
    ${LIB}
    include/ospray
    ${OSPRAY_LIBRARIES}
    )

IF(OSPRAY_FOUND)
    SET(HAVE_OSPRAY true CACHE BOOL "Have ospray lib")
else()
    unset(OSPRAY_LIBRARIES)
ENDIF(OSPRAY_FOUND)

