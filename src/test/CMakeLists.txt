#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
# Modifications:
#
#****************************************************************************/

cmake_minimum_required(VERSION 2.8)

IF(WIN32)
    PROJECT(VISIT_TEST)
ENDIF(WIN32)

#-----------------------------------------------------------------------------
# If this directory exists the root CMakeLists.txt adds this directory as a
# subdirectory to the main VisIt project.
#
# Prevent users from running cmake directy in this directory.
#-----------------------------------------------------------------------------
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    MESSAGE(FATAL_ERROR "VisIt's \"test\" directory cannot be configured "
                    "independently from the main \"src\" directory. Please "
                    "run cmake on VisIt's \"src\" directory.")
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")


MESSAGE(STATUS "Configuring VisIt Testing Python Module")

if(NOT WIN32)
    PYTHON_ADD_DISTUTILS_SETUP(visit_testing_py_setup
                               site-packages
                               setup.py)
endif()

if(BUILD_CTEST_TESTING)
    add_subdirectory(tests)
endif()



if(WIN32)
    # copy over to the Build dir those test files necessary for running
    # test suite on windows, while keeping src dir clean.
    if (NOT EXISTS ${VISIT_BINARY_DIR}/test)
        make_directory(${VISIT_BINARY_DIR}/test)
    endif()
    if (NOT EXISTS ${VISIT_BINARY_DIR}/test/report_templates)
        make_directory(${VISIT_BINARY_DIR}/test/report_templates)
        make_directory(${VISIT_BINARY_DIR}/test/report_templates/css)
        make_directory(${VISIT_BINARY_DIR}/test/report_templates/js)
    endif()
    foreach(f  visit_test_suite.py
               visit_test_reports.py
               visit_test_main.py
               visit_test_common.py
               visit_test_ctest.py
               summarizeResults.py
               nobaseline.pnm
               skip.json
               HtmlPython.py
               HtmlDiff.py)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different 
             "${CMAKE_CURRENT_SOURCE_DIR}/${f}"
             "${VISIT_BINARY_DIR}/test/")
    endforeach()
    file(GLOB hf "${CMAKE_CURRENT_SOURCE_DIR}/report_templates/*.html")
    foreach(h ${hf})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different 
             ${h} "${VISIT_BINARY_DIR}/test/report_templates")
    endforeach()
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different 
             "${CMAKE_CURRENT_SOURCE_DIR}/report_templates/css/styles.css"
             "${VISIT_BINARY_DIR}/test/report_templates/css")
    file(GLOB jf "${CMAKE_CURRENT_SOURCE_DIR}/report_templates/js/*.js")
    foreach(j ${jf})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different 
             ${j} "${VISIT_BINARY_DIR}/test/report_templates/js")
    endforeach()
endif()
