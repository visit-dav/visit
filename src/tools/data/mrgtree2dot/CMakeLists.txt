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
    DEPENDS_ON  silo
    FOLDER      tools/data)

