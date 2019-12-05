# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Cyrus Harrison, Tue Mar  9 08:28:53 PST 2010
#   Define __USE_ISOC99 if we are using the boost interval lib to make sure
#   a rounding scheme is available.
#
#   Eric Brugger, Wed Jul 25 08:30:40 PDT 2012
#   I added the classes vtkMultiSplitter and vtkVolumeFromCSGVolume.
#
#   Kathleen Biagas, Wed Aug  8 08:15:26 PDT 2012
#   Removed vtkDisjointCubesFacelistFilter.
#
#   Burlen Loring, Sun Oct 18 00:47:57 PDT 2015
#   Added vtkDepthSortPolyData2
#
#   Kathleen Biagas, Mon Jul 18 09:57:06 PDT 2016
#   Remove vtkVisItTIFFWriter (in favor of using VTK's directly).
#
#   Brad Whitlock, Tue Sep 19 14:35:20 PDT 2017
#   OpenEXR support.
#
#   Kathleen Biagas, Mon Feb 5 17:22:22 PST 2018
#   Move vtkDepthSortPolyData2 to avt/Plotter/vtk since it depends on the
#   vtk rendering libs.  Remove vtk rendering libs from linker.
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkCommonExecutionModel, vtkCommonMisc in support of building plugins
#   vs install on Windows. They are direct depencencies for this library,
#   and CMake automagically pulls them in when compiling VisIt, even if not
#   listed, but not when building the plugins.
#
#   Alister Maguire, Mon Dec  2 14:18:39 MST 2019
#   Removed vtkPPMWriter. It's been replaced with vtkPNMWriter (not custom).
#
#****************************************************************************/

IF(OPENEXR_FOUND)
    SET(OPENEXR_SRC vtkOpenEXRWriter.C)
ENDIF(OPENEXR_FOUND)

SET(VISIT_VTK_SOURCES
InitVTK.C
vtkAxisDepthSort.C
vtkBinaryPartitionVolumeFromVolume.C
vtkCSGCell.C
vtkCSGGrid.C
vtkConnectedTubeFilter.C
vtkDataSetRemoveGhostCells.C
vtkLineoutFilter.C
vtkLinesFromOriginalCells.C
vtkMergeTree.C
vtkMultiSplitter.C
vtkOBJWriter.C
vtkOnionPeelFilter.C
vtkPolyDataOnionPeelFilter.C
vtkRGBWriter.C
vtkRectilinearGridFacelistFilter.C
vtkRectilinearLinesNoDataFilter.C
vtkSlicer.C
vtkStructuredGridFacelistFilter.C
vtkSurfaceFilter.C
vtkSurfaceFromVolume.C
vtkTensorReduceFilter.C
vtkUniqueFeatureEdges.C
vtkUnstructuredGridRelevantPointsFilter.C
vtkVectorGlyph.C
vtkVectorReduceFilter.C
vtkVertexFilter.C
vtkVisItCellDataToPointData.C
vtkVisItClipper.C
vtkVisItContourFilter.C
vtkVisItCutter.C
vtkVisItExtractGrid.C
vtkVisItExtractRectilinearGrid.C
vtkVisItFeatureEdges.C
vtkVisItGlyph3D.C
vtkVisItInterpolatedVelocityField.C
vtkVisItPolyDataNormals.C
vtkVisItPolyDataNormals2D.C
vtkVisItStructuredGridNormals.C
vtkVisItProbeFilter.C
vtkVisItRectilinearGrid.C
vtkVisItSTLWriter.C
vtkVisItScalarTree.C
vtkVisItSplitter.C
vtkVisItStructuredGrid.C
vtkVisItTubeFilter.C
vtkVisItTensorGlyph.C
vtkVolumeFromCSGVolume.C
vtkVolumeFromVolume.C
${OPENEXR_SRC}
)


INCLUDE_DIRECTORIES(
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/common/Exceptions/Pipeline
${VISIT_SOURCE_DIR}/common/misc
${VISIT_SOURCE_DIR}/common/utility
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS}
${BOOST_INCLUDE_DIR}
${OPENEXR_INCLUDE_DIR}
)

IF(HAVE_BOOST)
    ADD_DEFINITIONS(-D__USE_ISOC99)
ENDIF(HAVE_BOOST)

# Add link directories needed to use our libraries and VTK.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${OPENEXR_LIBRARY_DIR})

ADD_LIBRARY(visit_vtk ${VISIT_VTK_SOURCES})

TARGET_LINK_LIBRARIES(visit_vtk lightweight_visit_vtk vtkCommonExecutionModel vtkCommonMisc vtkFiltersExtraction vtkFiltersFlowPaths vtkFiltersSources vtkImagingHybrid vtkIOImage vtktiff ${OPENEXR_LIB})


VISIT_INSTALL_TARGETS(visit_vtk)
