# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


# experimental!!
# ctest/cdash testing support
set(BUILD_CTEST_TESTING OFF CACHE BOOL "Enable ctest hooks into VisIt's regression suite")
if(BUILD_CTEST_TESTING)
    # enable native cmake testing
    include(CTest)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
        @ONLY)
    # notes:
    # * visit test sources aren't in the source tree
    # * and baseline images are currently with test sources
    # * test data is generated from a set of programs
    #   that need to be built, also not in the source tree,
    #   but configured by this cmake lists file
    set(VISIT_TEST_DIR "${VISIT_SOURCE_DIR}/test"
        CACHE PATH "Path to VisIt's regression suite")
    set(VISIT_BASELINE_DIR "${VISIT_SOURCE_DIR}/../test/baseline"
        CACHE PATH "Path to baseline images used by VisIt's regression suite")
    set(VISIT_DATA_DIR "${CMAKE_BINARY_DIR}/data"
        CACHE PATH "Path to data used by VisIt's regression suite")
    # make it obvous if we do not have any of these, as
    # this will likely be a source of confusion for the
    # uninitiated
    if((NOT EXISTS ${VISIT_TEST_DIR}) OR (NOT EXISTS ${VISIT_BASELINE_DIR}) OR
      ((NOT EXISTS ${VISIT_DATA_DIR}) AND (NOT EXISTS "${VISIT_SOURCE_DIR}/data")))
        message(FATAL_ERROR
           "CTest based testing requires a checkout of trunk/test and trunk/data")
    endif()
    # add a phony target to be sure that data for tests is
    # generated/updated during the build
    add_custom_target(testing ALL)
    add_dependencies(testing testdata)
    # configure tests
    subdirs(${VISIT_TEST_DIR})
endif()

