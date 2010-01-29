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

SET(VIEWERLIB_SOURCES
ActionGroupDescription.C
AnimationActions.C
EngineKey.C
LineoutListItem.C
LockActions.C
NoEngineException.C
PlotAndOperatorActions.C
ViewActions.C
ViewStack.C
ViewerAction.C
ViewerActionBase.C
ViewerActionManager.C
ViewerBase.C
ViewerChangeUsernameWindow.C
ViewerClientConnection.C
ViewerCommandFromSimObserver.C
ViewerConfigManager.C
ViewerConnectionProgressDialog.C
ViewerEngineManager.C
ViewerFileServer.C
ViewerHostProfileSelector.C
ViewerHostProfileSelectorNoWin.C
ViewerHostProfileSelectorWithWin.C
ViewerMasterXfer.C
ViewerMessageBuffer.C
ViewerMetaDataObserver.C
ViewerMultipleAction.C
ViewerObserverToSignal.C
ViewerOperator.C
ViewerOperatorFactory.C
ViewerPasswordWindow.C
ViewerPlot.C
ViewerPlotFactory.C
ViewerPlotList.C
ViewerPopupMenu.C
ViewerProperties.C
ViewerQuery.C
ViewerQueryManager.C
ViewerRemoteProcessChooser.C
ViewerSILAttsObserver.C
ViewerServerManager.C
ViewerStateBuffered.C
ViewerSubject.C
ViewerToggleAction.C
ViewerToolbar.C
ViewerWindow.C
ViewerWindowManager.C
ViewerWindowManagerAttributes.C
VisItViewer.C
WindowActions.C
)

# If the viewer is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(VIEWER_STATIC_SOURCES ViewerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

SET(VIEWERLIB_MOC_SOURCES
PlotAndOperatorActions.h
ViewerAction.h
ViewerActionBase.h
ViewerChangeUsernameWindow.h
ViewerClientConnection.h
ViewerCommandFromSimObserver.h
ViewerConnectionProgressDialog.h
ViewerHostProfileSelectorWithWin.h
ViewerMetaDataObserver.h
ViewerMultipleAction.h
ViewerObserverToSignal.h
ViewerPasswordWindow.h
ViewerPlot.h
ViewerPlotList.h
ViewerSILAttsObserver.h
ViewerServerManager.h
ViewerStateBuffered.h
ViewerSubject.h
ViewerWindowManager.h
)
QT_WRAP_CPP(viewer VIEWERLIB_SOURCES ${VIEWERLIB_MOC_SOURCES})

IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    ADD_DEFINITIONS(-DCOMM_EXPORTS)
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/QtVisWindow
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Interactors
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/launcher/proxy
${VISIT_SOURCE_DIR}/launcher/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/vtkqt
${VISIT_SOURCE_DIR}/winutil
${QT_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
${QT_QTGUI_INCLUDE_DIR}
${QT_QTOPENGL_INCLUDE_DIR}
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

#*************************** The viewer library ******************************
ADD_LIBRARY(viewer ${VIEWERLIB_SOURCES})
IF (WIN32)
    SET_TARGET_PROPERTIES(viewer PROPERTIES OUTPUT_NAME viewerlib)
ENDIF (WIN32)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

TARGET_LINK_LIBRARIES(viewer 
viewerrpc
visitcommon
mdserverrpc
mdserverproxy
vclrpc
vclproxy
enginerpc
engineproxy
avtdbatts
avtview
avtmath
avtpipeline_ser
avtplotter_ser
avtfilters_ser
avtviswindow_ser
avtwriter_ser
avtqtviswindow
winutil
${VIEWER_QT_LIBS}
)

#*************************** The viewer executable ***************************
IF(NOT APPLE)
    ADD_EXECUTABLE(viewer_exe viewer.C ${VIEWER_STATIC_SOURCES})
ELSE(NOT APPLE)
    SET(VIEWER_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${VIEWER_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    ADD_EXECUTABLE(viewer_exe MACOSX_BUNDLE viewer.C ${VIEWER_STATIC_SOURCES} ${VISIT_SOURCE_DIR}/common/icons/${VIEWER_ICON_FILE})
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "VisIt Viewer ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Graphical viewer for VisIt")
    SET(MACOSX_BUNDLE_ICON_FILE            ${VIEWER_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISV)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "VisIt version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC")
ENDIF(NOT APPLE)

SET_TARGET_PROPERTIES(viewer_exe PROPERTIES OUTPUT_NAME viewer)
TARGET_LINK_LIBRARIES(viewer_exe 
    ${viewer_exe_IOperator}
    ${viewer_exe_VOperator}
    ${viewer_exe_IPlot}
    ${viewer_exe_VPlot}
    viewer
    vtkexpat
    vtkpng
    vtkjpeg
    vtkzlib
    vtkMPEG2Encode
    vtkDICOMParser
    vtkfreetype
    vtkftgl
)

# If we're building statically then the viewer can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(viewer_exe
        ${viewer_exe_IOperator}
        ${viewer_exe_VOperator}
        ${viewer_exe_IPlot}
        ${viewer_exe_VPlot}
    )
ENDIF(VISIT_STATIC)

VISIT_INSTALL_TARGETS(viewer_exe viewer)
