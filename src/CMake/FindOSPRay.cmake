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
#*****************************************************************************

if(NOT EXISTS ${VISIT_OSPRAY_DIR})
    message(STATUS "VISIT_OSPRAY_DIR is not specified or does not exits.  please check the value and re-run cmake. Otherwise ospray will not be used.")
    return()
endif()

if(OSPRAY_VERSION VERSION_LESS_EQUAL "1.6.1")
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
          IF(LINUX AND
              (NOT "${_name_}" STREQUAL "libtbb_debug") AND
              (NOT "${_name_}" STREQUAL "libtbbmalloc_debug") AND
              (NOT "${_name_}" STREQUAL "libtbb") AND
              (NOT "${_name_}" STREQUAL "libtbbmalloc") AND
              (NOT "${_name_}" STREQUAL "libembree3") )
            THIRD_PARTY_INSTALL_LIBRARY(${l}.0)
          ENDIF()

          if(LINUX AND ("${_name_}" MATCHES "embree" OR
                        "${_name_}" MATCHES "tbb"))
              # embree and tbb are listed with their full extensions of
              # .so.[version]. The simple .so is also needed in order for
              # plugins to link correctly against an install.  If the .so
              # is a symlink to the full version (as with embree) the install
              # library logic will correctly install both the full verison and
              # the .so symlink, so only the .so is needed to be sent to the
              # function.
              cmake_path(SET _tmp_path ${l})
              cmake_path(GET _tmp_path FILENAME _tmp_name)
              cmake_path(GET _tmp_path EXTENSION LAST_ONLY _tmp_ext)
              string(REPLACE "${_tmp_ext}" "" _tmp_name ${_tmp_name})
              THIRD_PARTY_INSTALL_LIBRARY(${_tmp_path}/${_tmp_name})
              # tbb's .so isn't a symlink, so install full version too
              if("${_name_}" MATCHES "tbb")
                  THIRD_PARTY_INSTALL_LIBRARY(${l})
              endif()
          else()
              THIRD_PARTY_INSTALL_LIBRARY(${l})
          endif()
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

else() # ospray > 1.6.1

  # Use the OSPRAY_DIR hint from the config-site .cmake file

  #message(STATUS "  OSPRAY_VERSION=${OSPRAY_VERSION}")
  #message(STATUS "  VISIT_OSPRAY_DIR=${VISIT_OSPRAY_DIR}")

  # The libraries as of ospray 2.8
  set(OSPRAY_LIBRARIES ospray
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

  if(EXISTS ${VISIT_OSPRAY_DIR})

    if(EXISTS ${VISIT_OSPRAY_DIR}/lib64)
        set(LIB lib64)
    else()
        set(LIB lib)
    endif()

    if(EXISTS ${VISIT_OSPRAY_DIR}/${LIB}/cmake/ospray-${OSPRAY_VERSION}/osprayConfig.cmake)
        # vtk's find for opsray needs this
        set(ospray_DIR ${VISIT_OSPRAY_DIR}/${LIB}/cmake/ospray-${OSPRAY_VERSION})
    endif()

    SET_UP_THIRD_PARTY(OSPRAY LIBS ${OSPRAY_LIBRARIES})

    add_definitions(-DHAVE_OSPRAY)
  endif()
endif()

