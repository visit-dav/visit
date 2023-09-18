# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Apr 19 14:38:36 PDT 2023
#   Add target_include_directories for each of the unit tests.
#
#   Kathleen Biagas, Wed Aug 2, 2023
#   Remove win32-regex.
#
#   Kathleen Biagas, Tue Sep 26, 2023
#   Replace ${CMAKE_THREAD_LIBS} with Threads::Threads.
#
#****************************************************************************

visit_append_list(
    NAME  visitcommon_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/AtomicProperties.C
          ${CMAKE_CURRENT_SOURCE_DIR}/CoordUtils.C
          ${CMAKE_CURRENT_SOURCE_DIR}/GeometricHelpers.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Namescheme.C
          ${CMAKE_CURRENT_SOURCE_DIR}/StringHelpers.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Utility.C
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_gzstream.C
          ${CMAKE_CURRENT_SOURCE_DIR}/void_ref_ptr.C)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/utility>)

if(VISIT_ENABLE_UNIT_TESTS)
    blt_add_executable(
        NAME       MRUCache_test
        SOURCES    MRUCache_test.C
        INCLUDES   ${CMAKE_CURRENT_SOURCE_DIR}
        FOLDER     unit_tests)

    blt_add_executable(
        NAME       Namescheme_test
        SOURCES    Namescheme_test.C
        INCLUDES   ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS_ON visitcommon ${ZLIB_LIBRARY} Threads::Threads ${DL_LIB}
        FOLDER     unit_tests)

    blt_add_executable(
        NAME       Utility_test
        SOURCES    Utility_test.C
        INCLUDES   ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS_ON visitcommon ${ZLIB_LIBRARY} Threads::Threads ${DL_LIB}
        FOLDER     unit_tests)

    blt_add_executable(
        NAME       StringHelpers_test
        SOURCES    StringHelpers_test.C
        INCLUDES   ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS_ON visitcommon ${ZLIB_LIBRARY} Threads::Threads ${DL_LIB}
        FOLDER     unit_tests)

    if(DEFINED VISIT_EXE_LINKER_FLAGS)
        blt_patch_target(NAME Namescheme_test LINK_FLAGS ${VISIT_EXE_LINKER_FLAGS})
        blt_patch_target(NAME StringHelpers_test LINK_FLAGS ${VISIT_EXE_LINKER_FLAGS})
    endif()
endif()

