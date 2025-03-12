# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************


#********************************* SERIAL ************************************

visit_add_executable(
    NAME        mrgtree2dot
    SOURCES     mrgtree2dot.C
    INCLUDES    ${SILO_INCLUDE_DIR}
    DEPENDS_ON  ${SILO_LIB}
    FOLDER      tools/data)

visit_patch_target(
    NAME        mrgtree2dot
    LINKDIR     ${SILO_LIBRARY_DIR})

