# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(VISIT_DEPENDENCY_TESTS NONE)

include(CTest)

set(dependency_fixture ${CMAKE_CURRENT_LIST_DIR}/dependency-fixture)
set(dependency_module ${CMAKE_CURRENT_LIST_DIR}/../VisitDependency.cmake)

foreach(test_case IN ITEMS all_targets all_prebuilt mixed)
    add_test(
        NAME dependency_${test_case}
        COMMAND ${CMAKE_COMMAND}
            -S ${dependency_fixture}
            -B ${CMAKE_CURRENT_BINARY_DIR}/${test_case}
            -DTEST_CASE=${test_case}
            -DDEPENDENCY_MODULE=${dependency_module})
endforeach()

foreach(test_case IN ITEMS missing include_order)
    if(test_case STREQUAL "missing")
        set(expected_text "Disabled or not installed: TIFF")
    else()
        set(expected_text "Enabled but target not yet defined: TIFF")
    endif()
    add_test(
        NAME dependency_${test_case}
        COMMAND ${CMAKE_COMMAND}
            -DFIXTURE_SOURCE=${dependency_fixture}
            -DFIXTURE_BINARY=${CMAKE_CURRENT_BINARY_DIR}/${test_case}
            -DTEST_CASE=${test_case}
            -DDEPENDENCY_MODULE=${dependency_module}
            -DEXPECTED_TEXT=${expected_text}
            -P ${CMAKE_CURRENT_LIST_DIR}/expect-configure-failure.cmake)
endforeach()
