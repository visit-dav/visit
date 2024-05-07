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

SET(LIGHTWEIGHT_VISIT_VTK_SOURCES
ClipCasesHex.C  
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
)

INCLUDE_DIRECTORIES(
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/common/misc
${VISIT_SOURCE_DIR}/common/utility
${VTK_INCLUDE_DIRS}
)

# Add link directories needed to use our libraries and VTK.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(lightweight_visit_vtk
    ${LIGHTWEIGHT_VISIT_VTK_SOURCES}
)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonCore vtkFiltersCore vtkIOCore vtkIOLegacy)
else()
    set(vtklibs VTK::CommonCore VTK::FiltersCore VTK::IOCore VTK::IOLegacy VTK::vtksys)
endif()

target_link_libraries(lightweight_visit_vtk visitcommon ${vtklibs})

VISIT_INSTALL_TARGETS(lightweight_visit_vtk)
