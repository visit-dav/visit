# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(SOURCES add_visit_searchpath.C)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${SILO_INCLUDE_DIR}
)

LINK_DIRECTORIES(${SILO_LIBRARY_DIR})

ADD_EXECUTABLE(add_visit_searchpath ${SOURCES})

TARGET_LINK_LIBRARIES(add_visit_searchpath
${SILO_LIB}
) 

VISIT_INSTALL_TARGETS(add_visit_searchpath)
VISIT_TOOLS_ADD_FOLDER(add_visit_searchpath)
