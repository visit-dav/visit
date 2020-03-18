# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added VTK::IOCore in support of building plugins vs install on Windows.
#   It is a direct depencency for this library, and CMake automagically pulls
#   it in when compiling VisIt, even if not listed, but not when building
#   the plugins.
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
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/common/misc
${VISIT_SOURCE_DIR}/common/utility
)

# Add link directories needed to use our libraries and VTK.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(lightweight_visit_vtk
    ${LIGHTWEIGHT_VISIT_VTK_SOURCES}
)

TARGET_LINK_LIBRARIES(lightweight_visit_vtk visitcommon VTK::CommonCore VTK::FiltersCore VTK::IOCore VTK::IOLegacy)

VISIT_INSTALL_TARGETS(lightweight_visit_vtk)
