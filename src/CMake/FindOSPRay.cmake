# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kevin Griffin, Fri May 3 18:44:57 PDT 2019
#   Add logic to install libraries on OSX correctly.
#
#*****************************************************************************

IF(VISIT_OSPRAY)

    # -- this is a hack for TBB_ROOT
    IF(NOT DEFINED TBB_ROOT)
      SET(TBB_ROOT ${VISIT_TBB_ROOT})
    ENDIF()
    SET(embree_DIR ${EMBREE_DIR})
    MARK_AS_ADVANCED(ospray_DIR)
    MARK_AS_ADVANCED(embree_DIR)

    # setup ospray
    # we have to search for the osprayConfig.cmake file in the ospray install
    # folder. Because ospray sometimes install its library in <...>/lib64
    # folder instead of the <...>/lib folder, we have to check both
    # possibilities
    FIND_PACKAGE(ospray REQUIRED
                 PATHS 
                     ${OSPRAY_DIR}/lib/cmake/ospray-${OSPRAY_VERSION}
                     ${OSPRAY_DIR}/lib64/cmake/ospray-${OSPRAY_VERSION}
                 NO_DEFAULT_PATH)
    ADD_DEFINITIONS(-DVISIT_OSPRAY)
    # append additional module libraries
    IF(NOT APPLE)
        list(APPEND OSPRAY_LIBRARIES 
            ${LIBRARY_PATH_PREFIX}ospray_module_ispc${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_visit${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_visit_common${LIBRARY_SUFFIX})
    ELSE()
        list(APPEND OSPRAY_LIBRARIES
            ${LIBRARY_PATH_PREFIX}ospray.0${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_common.0${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_ispc${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_ispc.0${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_visit${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_visit.0${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_visit_common${LIBRARY_SUFFIX}
            ${LIBRARY_PATH_PREFIX}ospray_module_visit_common.0${LIBRARY_SUFFIX})
    ENDIF()

    # ospray tries to dlopen the ispc libs at runtime
    # so we need ot make sure those libs exist in
    # ${VISIT_BINARY_DIR}/lib/
    # so developer builds can load them

    IF( NOT WIN32 )
        FOREACH(ospray_lib ${OSPRAY_LIBRARIES})
            IF( "${ospray_lib}" MATCHES "ispc")
                execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                        ${ospray_lib}
                                        ${VISIT_BINARY_DIR}/lib/)
            ENDIF()
        ENDFOREACH()
    ENDIF()


    # install ospray libs follow visit's standard
    #
    # -- OSPRAY_LIBRARIES contains also libtbb and libembree, so we dont
    #    have to handle embree and tbb seperately. As for ISPC, since ISPC
    #    is only a compiler and will only be needed when we are compiling
    #    ospray source, we dont have to install ISPC at all here for VisIt.
    #
    # on linux ospray have libraries in a form of:
    #      libospray[xxx].so
    #      libospray[xxx].so.0
    #      libospray[xxx].so.[version]
    #
    # on mac ospray have libraries in a form of:
    #      libospray[xxx].dylib
    #      libospray[xxx].0.dylib
    #      libospray[xxx].[version].dylib
    #
    # on windows
    #      libospray[xxx].lib
    #      libospray[xxx].dll
    #
    # -- installing headers
    IF(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
      INSTALL(DIRECTORY ${OSPRAY_INCLUDE_DIR}/ospray
        DESTINATION
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                         GROUP_WRITE GROUP_READ
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                              GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                          WORLD_READ WORLD_EXECUTE
        PATTERN ".svn" EXCLUDE)
    ENDIF()
    # -- installing libs
    IF(NOT VISIT_OSPRAY_SKIP_INSTALL)
      FOREACH(l ${OSPRAY_LIBRARIES})
        # here because the CMake variable OSPRAY_LIBRARIES contains words
        # like 'optimized' and 'debug', we should skip them
        IF( (NOT "${l}" STREQUAL "optimized") AND
            (NOT "${l}" STREQUAL "debug"))
          GET_FILENAME_COMPONENT(_name_ ${l} NAME_WE)
          IF( (NOT WIN32) AND (NOT APPLE) AND
              (NOT "${_name_}" STREQUAL "libtbb_debug") AND
              (NOT "${_name_}" STREQUAL "libtbbmalloc_debug") AND
              (NOT "${_name_}" STREQUAL "libtbb") AND
              (NOT "${_name_}" STREQUAL "libtbbmalloc") AND
              (NOT "${_name_}" STREQUAL "libembree3") )
            THIRD_PARTY_INSTALL_LIBRARY(${l}.0)
          ENDIF()
          THIRD_PARTY_INSTALL_LIBRARY(${l})
        ENDIF()
      ENDFOREACH()
    ENDIF()

    # debug
    MESSAGE(STATUS "OSPRAY_INCLUDE_DIR: " ${OSPRAY_INCLUDE_DIR})
    MESSAGE(STATUS "OSPRAY_LIBRARIES: " ${OSPRAY_LIBRARIES})
    MESSAGE(STATUS "OSPRay for VisIt: ON")

ELSE(VISIT_OSPRAY)
    MESSAGE(STATUS "OSPRay for VisIt: OFF")
ENDIF(VISIT_OSPRAY)


