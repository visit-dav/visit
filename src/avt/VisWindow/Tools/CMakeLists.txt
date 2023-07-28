# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(TOOLS_SOURCES
    avtAxisRestrictionToolInterface.C
    avtBoxToolInterface.C
    avtLineToolInterface.C
    avtPlaneToolInterface.C
    avtPointToolInterface.C
    avtSphereToolInterface.C
    avtToolInterface.C
    VisitAxisRestrictionTool.C
    VisitBoxTool.C
    VisitInteractiveTool.C
    VisitLineTool.C
    VisitPlaneTool.C
    VisitPointTool.C
    VisitSphereTool.C)

target_sources(avtviswindow_ser PRIVATE ${TOOLS_SOURCES})
target_include_directories(avtviswindow_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtviswindow_par PRIVATE ${TOOLS_SOURCES})
    target_include_directories(avtviswindow_par PUBLIC .)
endif()

unset(TOOLS_SOURCES)

