#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#   Brad Whitlock, Wed Aug 24 13:11:30 PDT 2011
#   Add vtkTexturedPointsPainter and remove some obsolete files.
#
#   Brad Whitlock, Wed Oct 12 22:42:12 PDT 2011
#   I moved some mappers to a different library since they may contain Qt. I
#   also cleaned up obsolete stuff in this file.
#
#   Carson Brownlee, Sun May  6 16:25:28 PDT 2012
#   Add support for Manta.
#
#   Kathleen Biagas, Tue Jul 31 12:08:32 MST 2012
#   Change CONFIGURE_FILE settings so things work correctly with 
#   out-of-source builds.
#
#   Kathleen Biagas, Tue Sep  3 11:08:03 PDT 2013
#   Swap avtGLEWInitializer for avtOpenGLExtensionManager.
#
#   Kathleen Biagas, Wed Jan  8 14:19:28 PST 2014
#   Add vtkAxesActor2D.
#
#   Kathleen Biagas, Tue Mar 15 08:26:33 MST 2016
#   OSPRay is only required if requested (VISIT_OSPRAY is set to true/on).
#   Change OSPRay messages type to STATUS.
#
#   Kathleen Biagas, Mon Jul 18 10:04:22 PDT 2016
#   Remove SurfaceAndWireframeMapper, and specialized mappers for
#   Structured/Rectilinear grids.
#
#   Kathleen Biagas, Tue Feb 6 14:15:43 PST 2018
#   Moved avtLookupTable from avt/Pipeline/Data to here, and
#   visit_vtk/full/vtkDepthSortPolyData2 to vtk, due to their dependence on
#   vtkRendering.
#
#   Kathleen Biagas, Thu June 7, 2018
#   No longer add 'VTK_DEFINITIONS' to target. Add vtkRenderingVolumeOpenGL2
#   to link line.  Needed because the module is now init'd from
#   InitVTKRendering.
#
#   Kathleen Biagas, Fri Jun  8 12:17:45 PDT 2018
#   Remove vtkRenderingVolumeOpenGL2 from link line. Moved module init back
#   into volume plot.
#
#   Qi WU, Sat Jun 10 22:21:27 MST 2018
#   Remove the old VISIT_OSPRAY configurations because they are too old. 
#   Additional Note: The FindOSPRay.cmake file is broken for a recent version
#                    of OSPRay. We should use the built-inosprayConfig.cmake
#                    file to find OSPRay instead.
#
#   Kathleen Biagas, Thu Sep 27 11:34:32 PDT 2018
#   Install InitVTKRenderingConfig.h when VISIT_INSTALL_THIRD_PARTY true.
#
#****************************************************************************/

# create configuration file to pass off cmake options
CONFIGURE_FILE(
  "${VISIT_SOURCE_DIR}/avt/Plotter/vtk/InitVTKRenderingConfig.h.in"
  "${VISIT_BINARY_DIR}/include/vtk/InitVTKRenderingConfig.h"
)

if(VISIT_INSTALL_THIRD_PARTY)
    # ensure the config file gets installed
    install(FILES ${VISIT_BINARY_DIR}/include/vtk/InitVTKRenderingConfig.h
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Plotter/vtk
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ)
endif()

SET(AVTPLOTTER_OPENGL_SOURCES
vtk/vtkOpenGLPointMapper.C
vtk/vtkOpenGLTexturedBackgroundMapper.C
)

SET(AVTPLOTTER_VTK_SOURCES
vtk/InitVTKRendering.C 
vtk/vtkAxesActor2D.C 
vtk/vtkBackgroundActor.C 
vtk/vtkDepthSortPolyData2.C
vtk/vtkLineLegend.C 
vtk/vtkMultiFontVectorText.C 
vtk/vtkParallelImageSpaceRedistributor.C 
vtk/vtkPointGlyphMapper.C 
vtk/vtkPointMapper.C 
vtk/vtkTexturedBackgroundActor.C 
vtk/vtkTimeSliderActor.C 
vtk/vtkTriad2D.C 
vtk/vtkVectorFontCharacterData.C 
vtk/vtkVisItAxisActor.C 
vtk/vtkVisItAxisActor2D.C 
vtk/vtkVisItCubeAxesActor.C 
vtk/vtkVisItScalarBarActor.C 
vtk/vtkVisItScalarBarWithOpacityActor.C 
vtk/vtkVisItTextActor.C
)

if(VISIT_OSPRAY)
    list(APPEND AVTPLOTTER_VTK_SOURCES
         vtk/vtkOSPRayVisItCubeAxesActorNode.C
         vtk/vtkOSPRayVisItAxisActorNode.C
         vtk/vtkVisItViewNodeFactory.C
        )
endif()

SET(AVTPLOTTER_SOURCES
avtActor.C
avtBehavior.C
avtCurveLegend.C
avtCustomRenderer.C
avtDecorationsDrawable.C
avtDecorationsMapper.C
avtDrawable.C
avtExternallyRenderedImagesActor.C
avtGeometryDrawable.C
avtImageDrawable.C
avtImageMapper.C
avtLabelActor.C
avtLabeledCurveMapper.C
avtLegend.C
avtLevelsLegend.C
avtLevelsMapper.C
avtLevelsPointGlyphMapper.C
avtLightList.C
avtLineoutActor.C
avtLookupTable.C
avtMapper.C
avtMapperBase.C
avtMapper2D.C
avtPickActor.C
avtPlot.C
avtPlotMetaData.C
avtPointMapper.C
avtSoftwareShader.C
avtTensorGlyphMapper.C
avtTransparencyActor.C
avtUserDefinedMapper.C
avtVariableLegend.C
avtVariableMapper.C
avtVariablePointGlyphMapper.C
avtVectorGlyphMapper.C
avtVolumeVariableLegend.C
RenderOrder.C
${AVTPLOTTER_VTK_SOURCES}
${AVTPLOTTER_OPENGL_SOURCES}
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/vtk
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${PROJECT_BINARY_DIR}/avt/Plotter/vtk/
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})


#********************************* SERIAL ************************************
ADD_LIBRARY(avtplotter_ser ${AVTPLOTTER_SOURCES})
TARGET_LINK_LIBRARIES(avtplotter_ser 
    visitcommon avtmath avtview avtfilters_ser avtpipeline_ser visit_vtk 
    lightweight_visit_vtk vtkCommonExecutionModel vtkFiltersHybrid
    vtkRenderingCore vtkRenderingFreeType vtkRenderingOpenGL2 
    ${OPENGL_LIBRARIES})
if(VISIT_OSPRAY)
    target_link_libraries(avtplotter_ser vtkRenderingOSPRay)
endif()
VISIT_INSTALL_TARGETS(avtplotter_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtplotter_par ${AVTPLOTTER_SOURCES})
    TARGET_LINK_LIBRARIES(avtplotter_par
        visitcommon avtmath avtview avtfilters_par avtpipeline_par visit_vtk
        lightweight_visit_vtk vtkCommonExecutionModel vtkFiltersHybrid
        vtkRenderingCore vtkRenderingFreeType vtkRenderingOpenGL2
        ${OPENGL_LIBRARIES})
    if(VISIT_OSPRAY)
        target_link_libraries(avtplotter_par vtkRenderingOSPRay)
    endif()
    VISIT_INSTALL_TARGETS(avtplotter_par)
ENDIF(VISIT_PARALLEL)

