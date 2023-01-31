# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Remove if test that is handled by the parent dir's CMakeLists.txt
#  (VISIT_DBIO_ONLY, VISIT_ENGINE_ONLY, AND VISIT_SERVER_COMPONENTS_ONLY)
#
#  Kathleen Biagas, Mon Jan 30, 2023
#  No longer build qtssh on unix.
#
#****************************************************************************

add_subdirectory(mpeg2encode)

if(WIN32)
    add_subdirectory(qtssh)
endif()
