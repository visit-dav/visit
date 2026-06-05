# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Wed Feb 19, 2025
#  Utilize visit_add_executable.
#
#****************************************************************************

visit_add_executable(
    NAME       add_visit_searchpath
    SOURCES    add_visit_searchpath.C
    DEPENDS_ON silo
    FOLDER     tools/add_visit_searchpath) 

