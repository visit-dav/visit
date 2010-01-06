#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

SET(MCURVIT_SOURCES
main.C
MultiCurveProcessor.C
MultiCurveViewer.C
QvisAnnotationWidget.C
QvisAppearanceWidget.C
QvisColorButton.C
QvisColorGridWidget.C
QvisColorManagerWidget.C
QvisColorSelectionWidget.C
QvisGeneralWidget.C
QvisGridWidget.C
QvisLineStyleWidget.C
QvisLineWidthWidget.C
QvisMessageDialog.C
QvisOpacitySlider.C
Synchronizer.C
)

# If the viewer is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../viewer/main/ViewerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

SET(MCURVIT_MOC_SOURCES
MultiCurveProcessor.h
MultiCurveViewer.h
QvisAnnotationWidget.h
QvisAppearanceWidget.h
QvisColorButton.h
QvisColorGridWidget.h
QvisColorManagerWidget.h
QvisColorSelectionWidget.h
QvisGeneralWidget.h
QvisGridWidget.h
QvisLineStyleWidget.h
QvisLineWidthWidget.h
QvisMessageDialog.h
QvisOpacitySlider.h
Synchronizer.h
)
QT_WRAP_CPP(mcurvit MCURVIT_SOURCES ${MCURVIT_MOC_SOURCES})

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/gui
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/vtkqt
${VTK_INCLUDE_DIRS}
${QT_INCLUDES}
)

SET(VTK_EXTRA_LIBS
vtktiff
vtkjpeg 
vtkpng 
vtkexpat
vtkzlib 
vtkDICOMParser 
vtkMPEG2Encode 
vtkfreetype 
vtkftgl
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

ADD_EXECUTABLE(mcurvit MACOSX_BUNDLE ${MCURVIT_SOURCES} ${STATIC_SOURCES})
SET(MACOSX_BUNDLE_BUNDLE_NAME          "MCurveIt ${VISIT_VERSION}")
SET(MACOSX_BUNDLE_INFO_STRING          "Curve visualizer")
SET(MACOSX_BUNDLE_ICON_FILE            ${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns)
SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)
SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "MCurveIt version ${VISIT_VERSION}")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "MCurveIt ${VISIT_VERSION}")
SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC")

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

TARGET_LINK_LIBRARIES(mcurvit
${viewer_exe_IOperator}
${viewer_exe_VOperator}
${viewer_exe_IPlot}
${viewer_exe_VPlot}
viewer
viewerrpc
visitcommon
vtkqt
avtdbatts
${VIEWER_QT_LIBS}
${VTK_EXTRA_LIBS}
${MESA_LIBS}
)

# If we're building statically then the viewer can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(mcurvit
        ${viewer_exe_IOperator}
        ${viewer_exe_VOperator}
        ${viewer_exe_IPlot}
        ${viewer_exe_VPlot}
    )
ENDIF(VISIT_STATIC)

VISIT_INSTALL_TARGETS(mcurvit)
