# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Dec 18, 2019
#   Added run_visit_test_suite.bat.in configure file for windows.
#
#   Kathleen Biagas, Tue Mar 5, 2024
#   For PYTHON_ADD_PIP_SETUP change DEST_DIR from lib/site-packages to
#   simply site-packages. The lib part handled in PYTHON_ADD_PIP_SETUP.
#
#****************************************************************************

if(WIN32)
    project(VISIT_TEST)
endif()

#-----------------------------------------------------------------------------
# If this directory exists the root CMakeLists.txt adds this directory as a
# subdirectory to the main VisIt project.
#
# Prevent users from running cmake directy in this directory.
#-----------------------------------------------------------------------------
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    message(FATAL_ERROR "VisIt's \"test\" directory cannot be configured "
                    "independently from the main \"src\" directory. Please "
                    "run cmake on VisIt's \"src\" directory.")
endif()


if(NOT PYTHON_FOUND)
    message(STATUS "Skipping build of VisIt Testing Python Module (PYTHON_FOUND=FALSE)")
else()
    message(STATUS "Configuring VisIt Testing Python Module")

    #
    # generate helper script that has params, like data dir, baseline dir, etc
    # populated to help run test suite out of source easily
    #
    if(NOT WIN32)
        # for pluginVsInstall tests, need path to MPI that VisIt is using.
        # It will be used to set CMAKE_PREFIX_PATH in the run_visit_test_suite
        # script so that CMake can find the right MPI.
        if(VISIT_MPI_COMPILER)
            cmake_path(GET VISIT_MPI_COMPILER PARENT_PATH MPI_ROOT)
            cmake_path(SET MPI_ROOT NORMALIZE ${MPI_ROOT}/../)
        elseif(VISIT_MPI_HOME)
            cmake_path(SET MPI_ROOT NORMALIZE ${VISIT_MPI_HOME})
        endif()

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

    # deps for visit_testing module
    SET(pytesting_sources setup.py
                          py_src/__init__.py
                          py_src/HtmlDiff.py
                          py_src/HtmlPython.py
                          py_src/visit_test_common.py
                          py_src/visit_test_common.py
                          py_src/visit_test_ctest.py
                          py_src/visit_test_main.py
                          py_src/visit_test_reports.py
                          py_src/visit_test_main.py
                          py_src/visit_test_reports.py
                          py_src/visit_test_suite.py)

    PYTHON_ADD_PIP_SETUP(NAME visit_testing_py_setup
                         DEST_DIR site-packages
                         PY_MODULE_DIR visit_testing
                         PY_SETUP_FILE setup.py
                         PY_SOURCES ${pytesting_sources})

    if(BUILD_CTEST_TESTING)
        add_subdirectory(tests)
    endif()


endif()
