#*****************************************************************************
#
# Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
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
# ****************************************************************************

# by default run the same modes as edge, these could be restricted on
# the ctest command line by using the -L option.
SET(_MODE_FLAGS "serial")
IF(VISIT_PARALLEL)
    LIST(APPEND _MODE_FLAGS "parallel")
    IF(ICET_FOUND)
        LIST(APPEND _MODE_FLAGS "scalable,parallel,icet")
    ENDIF()
ENDIF()
SET(TESTSUITE_MODE_FLAGS ${_MODE_FLAGS} CACHE STRING
    "testsuite modes eg. serial;parallel;scalable,parallel,icet")
SET(TESTSUITE_OPTIONS "" CACHE STRING
    "additional arguments to pass to testsuite.")
MARK_AS_ADVANCED(TESTSUITE_MODE_FLAGS TESTSUITE_OPTIONS)

SET(TESTSUITE_PLATFORM_ARGS)
SET(TESTSUITE_VISIT_EXEC "${CMAKE_BINARY_DIR}/exe/visit.exe")
IF(WIN32)
    # windows needs skiplist to various prevent crashes
    # and msvc needs configuration specific paths
    IF(MSVC_IDE)
        SET(TESTSUITE_VISIT_EXEC ${CMAKE_BINARY_DIR}/exe/$<CONFIGURATION>/visit.exe)
    ENDIF()
ELSE()
    # TODO -- verify we get it right with the skiplist on.
    # run all tests on unix like systems
    LIST(APPEND TESTSUITE_PLATFORM_ARGS "--no-skiplist")
ENDIF()

# macro that takes a module name and list of tests and creates
# coresponding test commands to be invoked by ctest
MACRO(ADD_VISIT_PY_TESTS MODULE_NAME PY_TESTS)
    SET(TEST_EXTRA_ARGS ${ARGN})
    SET(N_PY_TESTS)
    LIST(LENGTH PY_TESTS N_PY_TESTS)
    MESSAGE(STATUS "Configuring ${N_PY_TESTS} python tests for ${MODULE_NAME}")
    FOREACH(MODE_FLAGS ${TESTSUITE_MODE_FLAGS})
        # compute suffix
        STRING(REGEX REPLACE "([a-z])[a-z]*,*" "\\1" MODE_SUFFIX ${MODE_FLAGS})
        # add the test
        FOREACH(PY_TEST ${PY_TESTS})
            SET(PY_TEST_NAME Test${MODULE_NAME}_${PY_TEST}_${MODE_SUFFIX})
            ADD_TEST(
                NAME ${PY_TEST_NAME}
                COMMAND ${PYTHON_EXECUTABLE} "${VISIT_TEST_DIR}/visit_test_suite.py"
                -n 1 --no-data-check --ctest --lessverbose --cleanup-delay=1 --threshold-diff
                ${TESTSUITE_PLATFORM_ARGS} ${TEST_EXTRA_ARGS} ${TESTSUITE_OPTIONS}
                -m ${MODE_FLAGS} -d ${VISIT_DATA_DIR} -b ${VISIT_BASELINE_DIR}
                -e ${CMAKE_BINARY_DIR}/bin/visit -o ${CMAKE_BINARY_DIR}/PyTestOutput
                ${CMAKE_CURRENT_SOURCE_DIR}/${PY_TEST}.py
                )
        # set labels
        SET(MODE_NAME)
        STRING(REPLACE "," "_" MODE_NAME ${MODE_FLAGS})
        SET_TESTS_PROPERTIES(${PY_TEST_NAME} PROPERTIES LABELS ${MODE_NAME})
        ENDFOREACH()
    ENDFOREACH()
ENDMACRO()

# macro for tests that depend on third party libraries
# ie that built by build_visit
MACRO(PY_3RDPARTY_TEST TESTLIST LIBNAME TESTNAME)
    SET(LOCATOR "${LIBNAME}_FOUND")
    IF(${LOCATOR})
        #MESSAGE(STATUS "adding tests for ${LIBNAME}")
        LIST(APPEND ${TESTLIST} ${TESTNAME} ${ARGN})
    ENDIF()
ENDMACRO()

# like the preceeding macro, but supports multiple 3rd party
# dependencies. has the following keywaord args: TESTLIST,
# LIBNAMES, TESTNAMES
FUNCTION(PY_3RDPARTIES_TEST)
    SET(OPTS)
    SET(VALS TESTLIST)
    SET(MVALS LIBNAMES TESTNAMES)
    CMAKE_PARSE_ARGUMENTS(PY3T "${OPTS}" "${VALS}" "${MVALS}" ${ARGN})
    SET(LIBS_FOUND TRUE)
    FOREACH(PY3T_LIBNAME ${PY3T_LIBNAMES})
        SET(LOCATOR "${PY3T_LIBNAME}_FOUND")
        IF(NOT ${LOCATOR})
            SET(LIBS_FOUND FALSE)
        ENDIF()
    ENDFOREACH()
    IF(LIBS_FOUND)
        #MESSAGE(STATUS "adding tests for ${PY3T_LIBNAMES}")
        SET(${PY3T_TESTLIST} ${${PY3T_TESTLIST}} ${PY3T_TESTNAMES} PARENT_SCOPE)
    ENDIF()
ENDFUNCTION()

SUBDIRS(
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
