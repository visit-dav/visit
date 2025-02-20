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
    DEPENDS_ON ${SILO_LIB}
    FOLDER     tools/add_visit_searchpath) 

# until Silo is imported as a proper cmake target
visit_patch_target(
    NAME       add_visit_search_path
    INCLUDES   ${SILO_INCLUDE_DIR}
    LINKDIR    ${SILO_LIBRARY_DIR})

