# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Dec 18, 2019
#   Added run_visit_test_suite.bat.in configure file for windows.
#
#****************************************************************************

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


#
# generate helper script that has params, like data dir, baseline dir, etc
# populated to help run test suite out of source easily
#
if(NOT WIN32)
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/run_visit_test_suite.sh.in"
                    "${CMAKE_CURRENT_BINARY_DIR}/tmp/run_visit_test_suite.sh")

    # make sure our script is executable
    file(
        COPY ${CMAKE_CURRENT_BINARY_DIR}/tmp/run_visit_test_suite.sh
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
    )
else()
    configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/run_visit_test_suite.bat.in"
                    "${CMAKE_CURRENT_BINARY_DIR}/run_visit_test_suite.bat")
endif()

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
