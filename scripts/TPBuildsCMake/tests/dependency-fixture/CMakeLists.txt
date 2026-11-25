# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(VISIT_DEPENDENCY_FIXTURE NONE)

if(NOT TEST_CASE)
    message(FATAL_ERROR "TEST_CASE is required")
endif()
if(NOT DEPENDENCY_MODULE)
    message(FATAL_ERROR "DEPENDENCY_MODULE is required")
endif()

set(visit_install_root "test-install-root")
include(${DEPENDENCY_MODULE})

set(ENABLE_JPEG ON)
set(ENABLE_ZLIB ON)
set(ENABLE_TIFF ON)
set(JPEG_PREBUILT FALSE)
set(ZLIB_PREBUILT FALSE)
set(TIFF_PREBUILT FALSE)

if(TEST_CASE STREQUAL "all_targets")
    add_custom_target(JPEG)
    add_custom_target(ZLIB)
    add_custom_target(TIFF)
    set(expected_dependencies JPEG ZLIB TIFF)
elseif(TEST_CASE STREQUAL "all_prebuilt")
    set(JPEG_PREBUILT TRUE)
    set(ZLIB_PREBUILT TRUE)
    set(TIFF_PREBUILT TRUE)
    set(expected_dependencies)
elseif(TEST_CASE STREQUAL "mixed")
    set(JPEG_PREBUILT TRUE)
    add_custom_target(ZLIB)
    add_custom_target(TIFF)
    set(expected_dependencies ZLIB TIFF)
elseif(TEST_CASE STREQUAL "missing")
    set(JPEG_PREBUILT TRUE)
    add_custom_target(ZLIB)
    set(ENABLE_TIFF OFF)
elseif(TEST_CASE STREQUAL "include_order")
    set(JPEG_PREBUILT TRUE)
    set(ZLIB_PREBUILT TRUE)
else()
    message(FATAL_ERROR "Unknown TEST_CASE: ${TEST_CASE}")
endif()

visit_resolve_dependencies(
    PACKAGE PYTHON
    DEPENDENCIES JPEG ZLIB TIFF
    OUT_VAR resolved_dependencies)

if(NOT "${resolved_dependencies}" STREQUAL "${expected_dependencies}")
    message(FATAL_ERROR
        "Expected dependencies '${expected_dependencies}', got "
        "'${resolved_dependencies}'")
endif()
