# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Kathleen Biagas, Tue Dec 13, 2022
#  Moved setting of sources to subdirs.
#
#****************************************************************************

add_subdirectory(Base)
add_subdirectory(Discrete)
add_subdirectory(Tet)
add_subdirectory(Youngs)
add_subdirectory(Zoo)

set(avtmir_ser_DEPENDS
    visitcommon
    lightweight_visit_vtk
    visit_verdict)

set(avtmir_par_SOURCES  ${avtmir_ser_SOURCES})
set(avtmir_par_HEADERS  ${avtmir_ser_HEADERS})
set(avtmir_par_INCLUDES ${avtmir_ser_INCLUDES})
set(avtmir_par_DEPENDS  ${avtmir_ser_DEPENDS})

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtmir_ser
    SOURCES    ${avtmir_ser_SOURCES}
    HEADERS    ${avtmir_ser_HEADERS}
    INCLUDES   ${avtmir_ser_INCLUDES}
    DEPENDS_ON ${avtmir_ser_DEPENDS}
               avtpipeline_ser) 

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtmir_par
        SOURCES    ${avtmir_par_SOURCES}
        HEADERS    ${avtmir_par_HEADERS}
        INCLUDES   ${avtmir_par_INCLUDES}
        DEPENDS_ON ${avtmir_par_DEPENDS}
                   avtpipeline_par) 
endif()

