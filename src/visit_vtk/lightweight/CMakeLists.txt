# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkIOCore in support of building plugins vs install on Windows.
#   It is a direct dependency for this library, and CMake automagically pulls
#   it in when compiling VisIt, even if not listed, but not when building
#   the plugins.
#
#   Kathleen Biagas, Thu May 2, 2024
#   Added vtksys in support of building plugins vs install on Windows.
#   It is a direct dependency for this library, and CMake automagically pulls
#   it in when compiling VisIt, even if not listed. This doesn't occur when
#   building plugins against install, since we don't use the cmake targets,
#   only lists of libraries.
#
#****************************************************************************/

set(lightweight_SOURCES
    ClipCasesHex.C
    ClipCasesLin.C
    ClipCasesPix.C
    ClipCasesPoly5.C
    ClipCasesPoly6.C
    ClipCasesPoly7.C
    ClipCasesPoly8.C
    ClipCasesPyr.C
    ClipCasesQua.C
    ClipCasesTet.C
    ClipCasesTri.C
    ClipCasesWdg.C
    ClipCasesVox.C
    ClipCasesVtx.C
    InitVTKLite.C
    vtkCellIntersections.C
    vtkDataSetFromVolume.C
    vtkDebugStream.C
    vtkElementLabelArray.C
    vtkEnumThreshold.C
    vtkMultiLineSource.C
    vtkPolyDataRelevantPointsFilter.C
    vtkSkewLookupTable.C
    vtkTriangulationTables.C
    vtkUnstructuredGridBoundaryFilter.C
    vtkUnstructuredGridFacelistFilter.C
    vtkVisItCellLocator.C
    vtkVisItPointDataToCellData.C
    vtkVisItPointLocator.C
    vtkVisItUtility.C)

set(lightweight_HEADERS
    ClipCases.h
    InitVTKlite.h
    visit_vtk_light_exports.h
    vtkCellIntersections.h
    vtkDataSetFromVolume.h
    vtkDebugStream.h
    vtkElementLabelArray.h
    vtkEnumThreshold.h
    vtkMultiLineSource.h
    vtkPolyDataRelevantPointsFilter.h
    vtkSkew.h
    vtkSkewLookupTable.h
    vtkTriangulationTables.h
    vtkUnstructuredGridBoundaryFilter.h
    vtkUnstructuredGridFacelistFilter.h
    vtkVisItCellLocator.h
    vtkVisItPointDataToCellData.h
    vtkVisItPointLocator.h
    vtkVisItUtility.h)


visit_add_library(
    NAME        lightweight_visit_vtk
    SOURCES     ${lightweight_SOURCES}
    HEADERS     ${lightweight_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/visit_vtk/lightweight>
    DEPENDS_ON  visitcommon
                VTK::CommonCore
                VTK::FiltersCore
                VTK::IOCore
                VTK::IOLegacy)

