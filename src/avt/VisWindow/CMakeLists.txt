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

SET(COLLEAGUES_SOURCES
Colleagues/VisWinAnnotations.C
Colleagues/VisWinAxes.C
Colleagues/VisWinAxes3D.C
Colleagues/VisWinAxesArray.C
Colleagues/VisWinBackground.C
Colleagues/VisWinColleague.C
Colleagues/VisWinFrame.C
Colleagues/VisWinInteractions.C
Colleagues/VisWinLegends.C
Colleagues/VisWinLighting.C
Colleagues/VisWinAxesParallel.C
Colleagues/VisWinPathTracker.C
Colleagues/VisWinPlots.C
Colleagues/VisWinQuery.C
Colleagues/VisWinRendering.C
Colleagues/VisWinRenderingWithoutWindow.C
Colleagues/VisWinTools.C
Colleagues/VisWinTriad.C
Colleagues/VisWinUserInfo.C
Colleagues/VisWinView.C
Colleagues/avtAnnotationColleague.C
Colleagues/avtImageColleague.C
Colleagues/avtLegendAttributesColleague.C
Colleagues/avtLine2DColleague.C
Colleagues/avtText2DColleague.C
Colleagues/avtText3DColleague.C
Colleagues/avtTimeSliderColleague.C
)
IF (NOT WIN32)
SET_SOURCE_FILES_PROPERTIES(
    Colleagues/VisWinRendering.C
    PROPERTIES
    COMPILE_FLAGS "-I ${MESA_INCLUDE_DIR} -I ${OPENGL_INCLUDE_DIR} -I ${GLEW_INCLUDE_DIR}"
)
ELSE (NOT WIN32)
SET_SOURCE_FILES_PROPERTIES(
    Colleagues/VisWinRendering.C
    PROPERTIES
    COMPILE_FLAGS "-I ${MESA_INCLUDE_DIR} -I ${GLEW_INCLUDE_DIR}"
)
ENDIF (NOT WIN32)

SET(EXCEPTIONS_SOURCES
Exceptions/BadColleagueException.C
Exceptions/BadInteractorException.C
Exceptions/BadPlotException.C
Exceptions/BadWindowModeException.C
Exceptions/PlotDimensionalityException.C
)

SET(INTERACTORS_SOURCES
Interactors/Dolly3D.C
Interactors/FlyThrough.C
Interactors/Lineout2D.C
Interactors/Navigate2D.C
Interactors/Navigate3D.C
Interactors/NavigateAxisArray.C
Interactors/NavigateCurve.C
Interactors/NullInteractor.C
Interactors/Pick.C
Interactors/VisitHotPointInteractor.C
Interactors/VisitInteractor.C
Interactors/Zoom2D.C
Interactors/Zoom3D.C
Interactors/ZoomAxisArray.C
Interactors/ZoomCurve.C
Interactors/ZoomInteractor.C
)

SET(PROXIES_SOURCES
Proxies/VisWindowProtectionProxy.C
)

SET(TOOLS_SOURCES
Tools/VisitInteractiveTool.C
Tools/VisitAxisRestrictionTool.C
Tools/VisitBoxTool.C
Tools/VisitLineTool.C
Tools/VisitPlaneTool.C
Tools/VisitPointTool.C
Tools/VisitSphereTool.C
Tools/avtAxisRestrictionToolInterface.C
Tools/avtBoxToolInterface.C
Tools/avtLineToolInterface.C
Tools/avtPlaneToolInterface.C
Tools/avtPointToolInterface.C
Tools/avtSphereToolInterface.C
Tools/avtToolInterface.C
)

SET(VISWINDOW_SOURCES
VisWindow/VisWindow.C
VisWindow/VisWindowTypes.C
)

SET(AVTVISWINDOW_SOURCES
${COLLEAGUES_SOURCES}
${EXCEPTIONS_SOURCES}
${INTERACTORS_SOURCES}
${PROXIES_SOURCES}
${TOOLS_SOURCES}
${VISWINDOW_SOURCES}
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/Colleagues
${CMAKE_CURRENT_SOURCE_DIR}/Exceptions
${CMAKE_CURRENT_SOURCE_DIR}/Interactors
${CMAKE_CURRENT_SOURCE_DIR}/Proxies
${CMAKE_CURRENT_SOURCE_DIR}/Tools
${CMAKE_CURRENT_SOURCE_DIR}/VisWindow
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtviswindow_ser ${AVTVISWINDOW_SOURCES})
TARGET_LINK_LIBRARIES(avtviswindow_ser visitcommon avtview avtmath avtplotter_ser visit_vtk lightweight_visit_vtk)
VISIT_INSTALL_TARGETS(avtviswindow_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtviswindow_par ${AVTVISWINDOW_SOURCES})
    TARGET_LINK_LIBRARIES(avtviswindow_par visitcommon avtview avtmath avtplotter_par visit_vtk lightweight_visit_vtk)
    VISIT_INSTALL_TARGETS(avtviswindow_par)
ENDIF(VISIT_PARALLEL)
