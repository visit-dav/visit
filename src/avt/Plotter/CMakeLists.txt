#*****************************************************************************
#
# Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
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
#
#****************************************************************************/

SET(AVTPLOTTER_MESA_SOURCES
avtMesaSurfaceAndWireframeRenderer.C 
vtk/vtkMesaRectilinearGridMapper.C 
vtk/vtkMesaStructuredGridMapper.C 
vtk/vtkMesaTexturedBackgroundMapper.C 
vtk/vtkOSMesaRenderWindow.C 
vtk/vtkVisItMesaPolyDataMapper.C
)

SET_SOURCE_FILES_PROPERTIES(
    ${AVTPLOTTER_MESA_SOURCES}
    PROPERTIES
    COMPILE_FLAGS "-I${MESA_INCLUDE_DIR} -I${GLEW_INCLUDE_DIR}"
)

SET(AVTPLOTTER_OPENGL_SOURCES
avtGLEWInitializer.C 
avtOpenGLSurfaceAndWireframeRenderer.C 
vtk/vtkOpenGLRectilinearGridMapper.C 
vtk/vtkOpenGLStructuredGridMapper.C 
vtk/vtkOpenGLTexturedBackgroundMapper.C 
vtk/vtkVisItOpenGLPolyDataMapper.C 
vtk/vtkVisItOpenGLTexture.C
)

SET_SOURCE_FILES_PROPERTIES(
    ${AVTPLOTTER_OPENGL_SOURCES}
    PROPERTIES
#    COMPILE_FLAGS "-I${OPENGL_INCLUDE_DIR}{GLEW_INCLUDE_DIR}"
    COMPILE_FLAGS "-I${GLEW_INCLUDE_DIR}"
)

SET(RENDERING_SOURCES
vtk/InitVTKRendering.C 
vtk/vtkBackgroundActor.C 
vtk/vtkDashedXorGridMapper2D.C 
vtk/vtkLineLegend.C 
vtk/vtkMultiFontVectorText.C 
vtk/vtkParallelImageSpaceRedistributor.C 
vtk/vtkRectilinearGridMapper.C 
vtk/vtkRubberBandMapper2D.C 
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
)

IF(X11_FOUND)
    SET_SOURCE_FILES_PROPERTIES(
        vtk/vtkDashedXorGridMapper2D.C  vtk/vtkRubberBandMapper2D.C 
        PROPERTIES
        COMPILE_FLAGS "-DHAVE_XLIB"
    )
ENDIF(X11_FOUND)

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
${RENDERING_SOURCES}
${AVTPLOTTER_MESA_SOURCES}
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
)

IF(QT_MAC_USE_COCOA)
    INCLUDE_DIRECTORIES(${QT_INCLUDES})
ENDIF(QT_MAC_USE_COCOA)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} )

IF(QT_MAC_USE_COCOA)
    LINK_DIRECTORIES(${QT_LIBRARY_DIR})
ENDIF(QT_MAC_USE_COCOA)


#********************************* SERIAL ************************************
ADD_LIBRARY(avtplotter_ser ${AVTPLOTTER_SOURCES})
TARGET_LINK_LIBRARIES(avtplotter_ser visitcommon avtmath avtview avtfilters_ser avtpipeline_ser visit_vtk lightweight_visit_vtk vtkHybrid ${GLEW_LIB}) #${OPENGL_gl_LIBRARY} ${MESA_LIB})
IF(QT_MAC_USE_COCOA)
    TARGET_LINK_LIBRARIES(avtplotter_ser ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
ENDIF(QT_MAC_USE_COCOA)
VISIT_INSTALL_TARGETS(avtplotter_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtplotter_par ${AVTPLOTTER_SOURCES})
    TARGET_LINK_LIBRARIES(avtplotter_par visitcommon avtmath avtview avtfilters_par avtpipeline_par visit_vtk lightweight_visit_vtk vtkHybrid ${GLEW_LIB}) #${OPENGL_gl_LIBRARY} ${MESA_LIB})
    IF(QT_MAC_USE_COCOA)
        TARGET_LINK_LIBRARIES(avtplotter_par ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
    ENDIF(QT_MAC_USE_COCOA)
    VISIT_INSTALL_TARGETS(avtplotter_par)
ENDIF(VISIT_PARALLEL)
