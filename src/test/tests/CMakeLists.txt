# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************

# template for adding VisIt's python test cases
# takes a module name and a list of test names
# constructs the test commands
#
# Modifications:
#   Kathleen Biagas, Thu Feb 6 08:57:13 PST 2014
#   Account for differences on Windows and whether or not the IDE is used
#
#   Burlen Loring, Wed May 21 13:20:09 PDT 2014
#   compact special windows cases, make unix like use the
#   python as reported by visit cmake config, add libsim
#   tests, use --fuzzy --avgdiff=11 so more tests pass on
#   apple
#
#   Burlen Loring, Mon May 26 15:53:01 PDT 2014
#   Switch to threshold based diffing
#
#   Burlen Loring, Thu Jul 17 10:49:46 PDT 2014
#   Fix scoping bug in PY_3RDPARTIES_TEST function that
#   prevented tests from being added to the list.
#
#   Burlen Loring, Thu Jul 17 10:49:46 PDT 2014
#   Added support for testsuites modes.
#
#   Kathleen Biagas, Tue Sep 23 13:15:13 MST 2014
#   Use TESTSUITE_VISIT_EXEC on all platforms. (Fixes ctest issue on Windows).
#
# ****************************************************************************

# by default run the same modes as edge, these could be restricted on
# the ctest command line by using the -L option.
set(_MODE_FLAGS "serial")
if(VISIT_PARALLEL)
    list(APPEND _MODE_FLAGS "parallel")
    if(HAVE_ICET)
        list(APPEND _MODE_FLAGS "scalable,parallel,icet")
    endif()
endif()
set(TESTSUITE_MODE_FLAGS ${_MODE_FLAGS} CACHE STRING
    "testsuite modes eg. serial;parallel;scalable,parallel,icet")
set(TESTSUITE_OPTIONS "" CACHE STRING
    "additional arguments to pass to testsuite.")
mark_as_advanced(TESTSUITE_MODE_FLAGS TESTSUITE_OPTIONS)

set(TESTSUITE_PLATFORM_ARGS)
set(TESTSUITE_VISIT_EXEC "${CMAKE_BINARY_DIR}/bin/visit")
if(WIN32)
    # msvc needs configuration specific paths
    if(MSVC_IDE)
        set(TESTSUITE_VISIT_EXEC "${CMAKE_BINARY_DIR}/exe/$<CONFIGURATION>/visit.exe")
    else()
        set(TESTSUITE_VISIT_EXEC "${CMAKE_BINARY_DIR}/exe/visit.exe")
    endif()
endif()

# macro that takes a module name and list of tests and creates
# coresponding test commands to be invoked by ctest
macro(ADD_VISIT_PY_TESTS MODULE_NAME PY_TESTS)
    set(TEST_EXTRA_ARGS ${ARGN})
    set(N_PY_TESTS)
    list(LENGTH PY_TESTS N_PY_TESTS)
    message(STATUS "Configuring ${N_PY_TESTS} python tests for ${MODULE_NAME}")
    foreach(MODE_FLAGS ${TESTSUITE_MODE_FLAGS})
        # compute suffix
        string(REGEX REPLACE "([a-z])[a-z]*,*" "\\1" MODE_SUFFIX ${MODE_FLAGS})
        # add the test
        foreach(PY_TEST ${PY_TESTS})
            set(PY_TEST_NAME Test${MODULE_NAME}_${PY_TEST}_${MODE_SUFFIX})
            add_test(
                NAME ${PY_TEST_NAME}
                COMMAND ${PYTHON_EXECUTABLE} "${VISIT_TEST_DIR}/visit_test_suite.py"
                -n 1 --no-data-check --ctest --lessverbose --cleanup-delay=1 --threshold-diff
                ${TESTSUITE_PLATFORM_ARGS} ${TEST_EXTRA_ARGS} ${TESTSUITE_OPTIONS}
                -m ${MODE_FLAGS} -d ${VISIT_DATA_DIR} -b ${VISIT_BASELINE_DIR}
                -e ${TESTSUITE_VISIT_EXEC} -o ${CMAKE_BINARY_DIR}/PyTestOutput
                ${CMAKE_CURRENT_SOURCE_DIR}/${PY_TEST}.py
                )
        # set labels
        set(MODE_NAME)
        string(REPLACE "," "_" MODE_NAME ${MODE_FLAGS})
        set_tests_properties(${PY_TEST_NAME} PROPERTIES LABELS ${MODE_NAME})
        endforeach()
    endforeach()
endmacro()

# macro for tests that depend on third party libraries
# ie that built by build_visit
macro(PY_3RDPARTY_TEST TESTLIST LIBNAME TESTNAME)
    set(LOCATOR "HAVE_${LIBNAME}")
    if(${LOCATOR})
        #message(STATUS "adding tests for ${LIBNAME}")
        list(APPEND ${TESTLIST} ${TESTNAME} ${ARGN})
    endif()
endmacro()

# like the preceeding macro, but supports multiple 3rd party
# dependencies. has the following keywaord args: TESTLIST,
# LIBNAMES, TESTNAMES
function(PY_3RDPARTIES_TEST)
    set(OPTS)
    set(VALS TESTLIST)
    set(MVALS LIBNAMES TESTNAMES)
    cmake_parse_arguments(PY3T "${OPTS}" "${VALS}" "${MVALS}" ${ARGN})
    set(LIBS_FOUND TRUE)
    foreach(PY3T_LIBNAME ${PY3T_LIBNAMES})
        set(LOCATOR "HAVE_${PY3T_LIBNAME}")
        if(NOT ${LOCATOR})
            set(LIBS_FOUND FALSE)
        endif()
    endforeach()
    if(LIBS_FOUND)
        #message(STATUS "adding tests for ${PY3T_LIBNAMES}")
        set(${PY3T_TESTLIST} ${${PY3T_TESTLIST}} ${PY3T_TESTNAMES} PARENT_SCOPE)
    endif()
endfunction()

subdirs(
    databases
    faulttolerance
    hybrid
    #leaks
    meshtype
    operators
    plots
    queries
    rendering
    session
    unit
    simulation
    )
