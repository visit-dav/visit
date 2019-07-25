# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas,  Mon Sep 26 07:21:44 MST 2011
#   Make cognomen always build static lib on windows.
#
#****************************************************************************

SET(COGNOMEN_SOURCES
src/cognomen.c
)

INCLUDE_DIRECTORIES(
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/src
${CMAKE_CURRENT_SOURCE_DIR}/src/cog
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_DEFINITIONS(-DIN_COGNOMEN)


IF(WIN32)
    SET(TMP ${BUILD_SHARED_LIBS})
    SET(BUILD_SHARED_LIBS 0)
    ADD_PARALLEL_LIBRARY(cognomen ${COGNOMEN_SOURCES})
    TARGET_LINK_LIBRARIES(cognomen ws2_32)
    SET_TARGET_PROPERTIES(cognomen PROPERTIES FOLDER thirdparty_builtin/cognomen)
    SET(BUILD_SHARED_LIBS ${TMP})
ELSE(WIN32)
    ADD_PARALLEL_LIBRARY(cognomen ${COGNOMEN_SOURCES})
ENDIF(WIN32)

VISIT_INSTALL_TARGETS(cognomen)
