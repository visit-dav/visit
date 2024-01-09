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

SET(TARGETS osmesatest_ser osmesavtktest_ser)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${OSMESA_INCLUDE_DIR}
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
)

if(VTK_VERSION VERSION_LESS_EQUAL "8.1.0")
    set(REQUIRED_VTK_LIBS
        vtkCommonCore
        vtkCommonDataModel
        vtkCommonExecutionModel
        vtkFiltersGeometry
        vtkFiltersHybrid
        vtkFiltersSources
        vtkIOImage
        vtkRenderingCore
        vtkRenderingOpenGL2
        vtkRenderingFreeType
        vtkInteractionStyle)
else()
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
endif()

IF(NOT WIN32)
    SET(LIBDL dl)
ENDIF(NOT WIN32)

ADD_EXECUTABLE(osmesatest_ser osmesatest.cpp)
TARGET_LINK_LIBRARIES(osmesatest_ser ${VISIT_EXE_LINKER_FLAGS} ${OSMESA_LIBRARIES} ${LIBDL} Threads::Threads)

ADD_EXECUTABLE(osmesavtktest_ser osmesavtktest.cpp)
TARGET_LINK_LIBRARIES(osmesavtktest_ser ${VISIT_EXE_LINKER_FLAGS} visit_vtk_offscreen visitcommon ${REQUIRED_VTK_LIBS} ${LIBDL} Threads::Threads)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(osmesatest_par osmesatest.cpp)
    PARALLEL_EXECUTABLE_LINK_LIBRARIES(osmesatest_par ${OSMESA_LIBRARIES} ${LIBDL} Threads::Threads)

    ADD_PARALLEL_EXECUTABLE(osmesavtktest_par osmesavtktest.cpp)
    PARALLEL_EXECUTABLE_LINK_LIBRARIES(osmesavtktest_par visit_vtk_offscreen visitcommon ${REQUIRED_VTK_LIBS} ${LIBDL} Threads::Threads)

    SET(TARGETS ${TARGETS} osmesatest_par osmesavtktest_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_TARGETS(${TARGETS})
VISIT_TOOLS_ADD_FOLDER(diagnostics/osmesatest ${TARGETS})
