# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

execute_process(
    COMMAND ${CMAKE_COMMAND}
        -S ${FIXTURE_SOURCE}
        -B ${FIXTURE_BINARY}
        -DTEST_CASE=${TEST_CASE}
        -DDEPENDENCY_MODULE=${DEPENDENCY_MODULE}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_stdout
    ERROR_VARIABLE configure_stderr)

set(configure_output "${configure_stdout}\n${configure_stderr}")

if(configure_result EQUAL 0)
    message(FATAL_ERROR
        "Expected ${TEST_CASE} configuration to fail, but it succeeded")
endif()

string(FIND "${configure_output}" "${EXPECTED_TEXT}" expected_position)
if(expected_position EQUAL -1)
    message(FATAL_ERROR
        "Expected configuration output to contain '${EXPECTED_TEXT}'.\n"
        "Actual output:\n${configure_output}")
endif()
