# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#*****************************************************************************

#[=======================================================================[.rst:
FindANARI
---------

Finds the ANARI library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``anari::anari``
  The ANARI library
``anari::anari_utilities``
  The ANARI utilities library
``anari::anari_library_debug``
  Library target for the debug device
``anari::helium``
  Library target containing base device implementation abstractions

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``anari_FOUND``
  True if the system has the ANARI library.
``HAVE_ANARI_EXAMPLE``
  True if the ANARI example back-end library (helide) was found.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``anari_DIR``
  The directory containing the ANARI config files.
``ANARI_INCLUDE_DIRS``
  The directory containing ``anari/anari.h``.
``ANARI_LIBRARIES``
  The ANARI Libraries and third-party ANARI back-end libraries if found
``ANARI_Example_LIBRARY``
  The path for the ANARI Example back-end library

#]=======================================================================]

if(EXISTS ${VISIT_ANARI_DIR})
    message(STATUS "Checking for ANARI in ${VISIT_ANARI_DIR}/lib/cmake/anari-${ANARI_VERSION}")

    if(NOT DEFINED anari_DIR)
        set(anari_DIR ${VISIT_ANARI_DIR}/lib/cmake/anari-${ANARI_VERSION}
            CACHE PATH
            "The directory containing the ANARI config files."
            FORCE)
    endif(NOT DEFINED anari_DIR)

    find_package(anari)
endif()

if(anari_FOUND)
    add_definitions(-DVISIT_ANARI)

    # Include directories
    get_target_property(_INCLUDE_DIRS anari::anari INTERFACE_INCLUDE_DIRECTORIES)
    set(ANARI_INCLUDE_DIRS ${_INCLUDE_DIRS} CACHE PATH "ANARI include directories" FORCE)
    mark_as_advanced(ANARI_INCLUDE_DIRS)

    # Install Headers
    if(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
      install(DIRECTORY ${VISIT_ANARI_DIR}/include/anari
        DESTINATION
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                         GROUP_WRITE GROUP_READ
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                              GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                          WORLD_READ WORLD_EXECUTE
        PATTERN ".git" EXCLUDE)
    endif()

    # Just the .so libs. If the .so is a symlink to the full version
    # the install library logic will correctly install both the full
    # version and the .so symlink, so only the .so is needed to be
    # sent to the function.
    file(GLOB ANARI_LIBRARIES ${VISIT_ANARI_DIR}/lib/lib*)

    # Install libs
    foreach(l ${ANARI_LIBRARIES})
      get_filename_component(_name_ ${l} NAME_WE)
      THIRD_PARTY_INSTALL_LIBRARY(${l})
    endforeach()

    #================== ANARI Example Back-end Library
    set(DLOPEN_LIBS)

    file(TO_CMAKE_PATH "$ENV{ANARI_Example_DIR}" _Example_DIR)
    find_library(ANARI_Example_LIBRARY
     	  NAMES
          anari_library_helide
        PATHS
          ${VISIT_ANARI_DIR}/lib
          ${_Example_DIR}/lib
    	  DOC "ANARI Example back-end library")

    mark_as_advanced(ANARI_Example_LIBRARY)

    if(ANARI_Example_LIBRARY)
      list(APPEND DLOPEN_LIBS ${ANARI_Example_LIBRARY})
      add_definitions(-DHAVE_ANARI_EXAMPLE)
      message(STATUS "ANARI Example back-end library found.")
    endif()

    # ANARI tries to dlopen the back-end libs at runtime
    # so we need to make sure those libs exist in
    # ${VISIT_BINARY_DIR}/lib/
    # so developer builds can load them
    file(COPY ${DLOPEN_LIBS}
      DESTINATION ${VISIT_BINARY_DIR}/lib/
      FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                       GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                   WORLD_READ WORLD_EXECUTE
      FOLLOW_SYMLINK_CHAIN)
endif(anari_FOUND)