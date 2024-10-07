# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas,  Mon Sep 26 07:21:44 MST 2011
#   Make cognomen always build static lib on windows.
#
#   Kathleen Biagas, Mon Oct 7, 2024
#   Utilize visit_add_parallel_library.
#
#****************************************************************************

if(WIN32)
    set(is_shared FALSE)
else()
    set(is_shared ${BUILD_SHARED_LIBS})
endif()

visit_add_parallel_library(
    NAME        cognomen
    SOURCES     ${CMAKE_CURRENT_SOURCE_DIR}/src/cognomen.c
    HEADERS     ${CMAKE_CURRENT_SOURCE_DIR}/src/cognomen.h
                ${CMAKE_CURRENT_SOURCE_DIR}/src/cog/bool.h
                ${CMAKE_CURRENT_SOURCE_DIR}/src/cog/types.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cog>
                $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/thirdparty_builtin/src>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/thirdparty_builtin/src/cog>
    DEFINES     IN_COGNOMEN
    SHARED      ${is_shared}
    FOLDER      thirdparty_builtin/cognomen)

if(WIN32)
    target_link_libraries(cognomen PUBLIC ws2_32)
endif()

