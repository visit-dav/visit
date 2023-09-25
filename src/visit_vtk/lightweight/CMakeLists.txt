# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkIOCore in support of building plugins vs install on Windows.
#   It is a direct depencency for this library, and CMake automagically pulls
#   it in when compiling VisIt, even if not listed, but not when building
#   the plugins.
#
#****************************************************************************/

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonCore vtkFiltersCore vtkIOCore vtkIOLegacy)
else()
    set(vtklibs VTK::CommonCore VTK::FiltersCore VTK::IOCore VTK::IOLegacy)
endif()

visit_add_library(
    NAME    lightweight_visit_vtk

    SOURCES  ClipCasesHex.C
             ClipCasesLin.C
             ClipCasesPix.C
             ClipCasesPyr.C
             ClipCasesQua.C
             ClipCasesTet.C
             ClipCasesTri.C
             ClipCasesWdg.C
             ClipCasesVox.C
             ClipCasesVtx.C
             ClipCasesPoly5.C
             ClipCasesPoly6.C
             ClipCasesPoly7.C
             ClipCasesPoly8.C
             InitVTKLite.C
             vtkCellIntersections.C
             vtkDataSetFromVolume.C
             vtkDebugStream.C
             vtkEnumThreshold.C
             vtkMultiLineSource.C
             vtkElementLabelArray.C
             vtkPolyDataRelevantPointsFilter.C
             vtkSkewLookupTable.C
             vtkTriangulationTables.C
             vtkUnstructuredGridBoundaryFilter.C
             vtkUnstructuredGridFacelistFilter.C
             vtkVisItCellLocator.C
             vtkVisItPointDataToCellData.C
             vtkVisItPointLocator.C
             vtkVisItUtility.C

    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
             $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/include/visit_vtk/lightweight>

    DEPENDS_ON visitcommon ${vtklibs})

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    target_include_directories(lightweight_visit_vtk PUBLIC "$<BUILD_INTERFACE:${VTK_INCLUDE_DIRS}>")
endif()

visit_install_export_targets(lightweight_visit_vtk)

