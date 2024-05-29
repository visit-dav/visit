# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kevin Griffin, Fri May 3 18:44:57 PDT 2019
#   Add logic to install libraries on OSX correctly.
#
#   Kathleen Biagas, Thu Jul 18 10:41:50 PDT 2019
#   Add special handling of tbb and embree installs for linux.
#
#   Kathleen Biagas, Wed Aug 17, 2022
#   Incorporate ARSanderson's OSPRay 2.8.0 work for VTK 9.
#
#   Kathleen Biagas, Tue Nov 29 12:29:40 PST 2022
#   Use cmake_path instead of GET_FILENAME_SHORTEXT and get_filename_component.
#
#   Kathleen Biagas, Fri Dec  2 20:16:38 PST 2022
#   Use cmake_path to get PARENT_PATH when handling tbb and embree.
#
#*****************************************************************************

if(NOT EXISTS ${VISIT_OSPRAY_DIR})
    message(STATUS "VISIT_OSPRAY_DIR is not specified or does not exits.  please check the value and re-run cmake. Otherwise ospray will not be used.")
    return()
endif()


if(EXISTS ${VISIT_OSPRAY_DIR}/lib64)
    set(LIB lib64)
else()
    set(LIB lib)
endif()

set(ospray_DIR ${VISIT_OSPRAY_DIR}/${LIB}/cmake/ospray-${OSPRAY_VERSION})
find_package(ospray ${OSPRAY_VERSION} REQUIRED
             PATHS ${VISIT_OSPRAY_DIR}
             PATH_SUFFIXES lib/cmake/opsray-${OSPRAY_VERSION}
                           lib64/cmake/ospray-${OSPRAY_VERSION}
             NO_MODULE
             NO_DEFAULT_PATH)

if(ospray_FOUND)
    set(HAVE_LIBOSPRAY true)
    add_definitions(-DHAVE_OSPRAY)
    if(VISIT_INSTALL_THIRD_PARTY)
        # since all the libs needed for VisIt at runtime aren't
        # enumerated in the ospray targets from find_package,
        # just install all of them
        set(LIB_SEARCH_PATH ${VISIT_OSPRAY_DIR}/lib)
        if(EXISTS ${VISIT_OSPRAY_DIR}/lib64)
            set(LIB_SEARCH_PATH ${VISIT_OSPRAY_DIR}/lib64)
        endif()
        if(NOT ospray_lib_libs)
            file(GLOB ospray_lib_libs
                 LIST_DIRECTORIES FALSE
                 ${LIB_SEARCH_PATH}/*)
        endif()
        unset(LIB_SEARCH_PATH)
        #  install libraries
        foreach(lib ${ospray_lib_libs})
            THIRD_PARTY_INSTALL_LIBRARY(${lib})
        endforeach()

        if(NOT VISIT_HEADERS_SKIP_INSTALL)
            THIRD_PARTY_INSTALL_INCLUDE(ospray ${OSPRAY_INCLUDE_DIR})
        endif()
    endif()
endif()

