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
#   tests, use --fuzzy --avgdiff=2 so tests more pass on
#   apple
#
# ****************************************************************************

MACRO(ADD_VISIT_PY_TESTS MODULE_NAME PY_TESTS)
    SET(TESTSUITE_PLATFORM_ARGS)
    SET(TESTSUITE_VISIT_EXEC "${CMAKE_BINARY_DIR}/exe/visit.exe")
    IF(WIN32)
        # windows needs skiplist to various prevent crashes
        # and msvc needs configuration specific paths
        IF(MSVC_IDE)
            SET(TESTSUITE_VISIT_EXEC ${CMAKE_BINARY_DIR}/exe/$<CONFIGURATION>/visit.exe)
        ENDIF()
    ELSE()
        # run all tests on unix like systems
        LIST(APPEND TESTSUITE_PLATFORM_ARGS "--no-skiplist")
    ENDIF()
    SET(N_PY_TESTS)
    LIST(LENGTH PY_TESTS N_PY_TESTS)
    MESSAGE(STATUS "Configuring ${N_PY_TESTS} python tests for ${MODULE_NAME}")
    FOREACH(PY_TEST ${PY_TESTS})
        ADD_TEST(
            NAME Test${MODULE_NAME}_${PY_TEST}
            COMMAND ${PYTHON_EXECUTABLE} "${VISIT_TEST_DIR}/visit_test_suite.py"
            ${TESTSUITE_PLATFORM_ARGS} --no-data-check --ctest -n 1
            --cleanup-delay=1 --avgdiff=2 --fuzzy --lessverbose
            -d ${VISIT_DATA_DIR} -b ${VISIT_BASELINE_DIR}
            -e ${CMAKE_BINARY_DIR}/bin/visit
            -o ${CMAKE_BINARY_DIR}/PyTestOutput
            ${CMAKE_CURRENT_SOURCE_DIR}/${PY_TEST}.py
            )
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
    FOREACH(PY3T_LIBNAME PY3T_LIBNAMES)
        SET(LOCATOR "${PY3T_LIBNAME}_FOUND")
        IF(NOT ${LOCATOR})
            SET(LIBS_FOUND FALSE)
        ENDIF()
    ENDFOREACH()
    IF(LIBS_FOUND)
        #MESSAGE(STATUS "adding tests for ${PY3T_LIBNAMES}")
        LIST(APPEND ${PY3T_TESTLIST} ${PY3T_TESTNAMES} ${ARGN})
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
    simulation)
