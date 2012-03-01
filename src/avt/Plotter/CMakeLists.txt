#*****************************************************************************
#
# Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
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
#****************************************************************************/

SET(AVTPLOTTER_OPENGL_SOURCES
avtGLEWInitializer.C 
avtOpenGLSurfaceAndWireframeRenderer.C 
vtk/vtkOpenGLRectilinearGridMapper.C 
vtk/vtkOpenGLStructuredGridMapper.C 
vtk/vtkOpenGLTexturedBackgroundMapper.C 
vtk/vtkTexturedPointsPainter.C
)

SET_SOURCE_FILES_PROPERTIES(
    ${AVTPLOTTER_OPENGL_SOURCES}
    PROPERTIES
    COMPILE_FLAGS "-I${GLEW_INCLUDE_DIR}"
)

SET(AVTPLOTTER_VTK_SOURCES
vtk/InitVTKRendering.C 
vtk/vtkBackgroundActor.C 
vtk/vtkLineLegend.C 
vtk/vtkMultiFontVectorText.C 
vtk/vtkParallelImageSpaceRedistributor.C 
vtk/vtkRectilinearGridMapper.C 
vtk/vtkStructuredGridMapper.C 
vtk/vtkTexturedBackgroundActor.C 
vtk/vtkTimeSliderActor.C 
vtk/vtkTriad2D.C 
vtk/vtkVectorFontCharacterData.C 
vtk/vtkVisItAxisActor.C 
vtk/vtkVisItAxisActor2D.C 
vtk/vtkVisItCubeAxesActor.C 
vtk/vtkVisItDataSetMapper.C 
vtk/vtkVisItScalarBarActor.C 
vtk/vtkVisItScalarBarWithOpacityActor.C 
vtk/vtkVisItTextActor.C
)

SET(AVTPLOTTER_SOURCES
avtActor.C
avtBehavior.C
avtCurveLegend.C
avtCustomRenderer.C
avtDecorationsDrawable.C
avtDecorationsMapper.C
avtDrawable.C
avtDrawer.C
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
avtMapper.C
avtPickActor.C
avtPlot.C
avtPointGlypher.C
avtSoftwareShader.C
avtSurfaceAndWireframeRenderer.C
avtTensorGlyphMapper.C
avtTheater.C
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
${VTK_INCLUDE_DIRS}
${MESA_INCLUDE_DIR}
${GLEW_INCLUDE_DIR}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} )

#********************************* SERIAL ************************************
ADD_LIBRARY(avtplotter_ser ${AVTPLOTTER_SOURCES})
TARGET_LINK_LIBRARIES(avtplotter_ser visitcommon avtmath avtview avtfilters_ser avtpipeline_ser visit_vtk lightweight_visit_vtk vtkHybrid ${GLEW_LIB})
VISIT_INSTALL_TARGETS(avtplotter_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtplotter_par ${AVTPLOTTER_SOURCES})
    TARGET_LINK_LIBRARIES(avtplotter_par visitcommon avtmath avtview avtfilters_par avtpipeline_par visit_vtk lightweight_visit_vtk vtkHybrid ${GLEW_LIB})
    VISIT_INSTALL_TARGETS(avtplotter_par)
ENDIF(VISIT_PARALLEL)
