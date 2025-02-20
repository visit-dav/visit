# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Send 'diagnostics/osmesatest' type argument to VISIT_TOOLS_ADD_FOLDER.
#
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change PTHREAD to Threads::Threads.
#
#   Kathleen Biagas, Tue Oct 31 10:27:56 PDT 2023
#   Add support for VTK 9.
#
#****************************************************************************

set(REQUIRED_VTK_LIBS
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::FiltersGeometry
    VTK::FiltersHybrid
    VTK::FiltersSources
    VTK::IOImage
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::RenderingFreeType
    VTK::InteractionStyle)

if(NOT WIN32)
    set(LIBDL dl)
endif()

visit_add_executable(
    NAME        osmesatest_ser
    SOURCES     osmesatest.cpp
    INCLUDES    ${OSMESA_INCLUDE_DIR}
    DEPENDS_ON  ${OSMESA_LIBRARIES}
                ${LIBDL}
                Threads::Threads
    FOLDER      tools/diagnostics)

visit_add_executable(
    NAME        osmesavtktest_ser
    SOURCES     osmesavtktest.cpp
    INCLUDES    ${OSMESA_INCLUDE_DIR}
    DEPENDS_ON  visit_vtk_offscreen
                visitcommon
                ${REQUIRED_VTK_LIBS}
                ${LIBDL}
                Threads::Threads
    FOLDER      tools/diagnostics)

if(VISIT_PARALLEL)
    visit_add_parallel_executable(
        NAME        osmesatest_par
        SOURCES     osmesatest.cpp
        INCLUDES    ${OSMESA_INCLUDE_DIR}
        DEPENDS_ON  ${OSMESA_LIBRARIES}
                    ${LIBDL}
                    Threads::Threads
        FOLDER      tools/diagnostics)

    visit_add_parallel_executable(
        NAME        osmesavtktest_par
        SOURCES     osmesavtktest.cpp
        INCLUDES    ${OSMESA_INCLUDE_DIR}
        DEPENDS_ON  visit_vtk_offscreen
                    visitcommon
                    ${REQUIRED_VTK_LIBS}
                    ${LIBDL}
                    Threads::Threads
        FOLDER      tools/diagnostics)
endif()

