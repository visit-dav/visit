#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#  Cyrus Harrison, Tue Jan 10 14:51:38 PST 2012
#  Add link of ${CMAKE_THREAD_LIBS} to the viewer.
#
#  Brad Whitlock, Fri May 18 16:09:34 PST 2012
#  Use different resource file.
#
#  Brad Whitlock, Thu Sep 11 10:45:23 PDT 2014
#  Moved many files to viewercore.
#
#  Kathleen Biagas, Tue Apr 14 14:55:01 MST 2015
#  Only add ViewerSubmitParallEngineToWindowsHPC if we actually have the
#  HPC scheduler.
#
#****************************************************************************/
IF(VISIT_PARALLEL AND WIN32 AND HAVE_HPC_SCHEDULER)
    SET(VIEWERLIB_WIN32_SOURCES
        ViewerSubmitParallelEngineToWindowsHPC.cpp
    )
    SET_SOURCE_FILES_PROPERTIES(ViewerEngineManager.C PROPERTIES
        COMPILE_DEFINITIONS VISIT_SUPPORTS_WINDOWS_HPC
    )
ENDIF(VISIT_PARALLEL AND WIN32 AND HAVE_HPC_SCHEDULER)

IF(VISIT_DDT)
SET(VIEWER_DDT
ddt/DDTManager.C
ddt/DDTSession.C
ddt/DDTActions.C
ddt/DDTActionsUI.C
)

SET(VIEWER_DDT_MOC
ddt/DDTManager.h
ddt/DDTSession.h
)
ENDIF(VISIT_DDT)

SET(VIEWER_UI
ui/AnimationActionsUI.C
ui/LockActionsUI.C
ui/PlotAndOperatorActionsUI.C
ui/PrintWindowAction.C
ui/SetAppearanceAction.C
ui/ToolbarActionsUI.C
ui/ViewActionsUI.C
ui/ViewerActionManagerUI.C
ui/ViewerActionUI.C
ui/ViewerActionUIMultiple.C
ui/ViewerActionUISingle.C
ui/ViewerActionUIToggle.C
ui/ViewerChangeUsernameUI.C
ui/ViewerConnectionPrinterUI.C
ui/ViewerConnectionProgressDialog.C
ui/ViewerFactoryUI.C
ui/ViewerHostProfileSelectorWithWin.C
ui/ViewerMessagingUI.C
ui/ViewerPasswordWindow.C
ui/ViewerPopupMenu.C
ui/ViewerToolbar.C
ui/ViewerWindowUI.C
ui/VisItChangeUsernameWindow.C
ui/VisItPasswordWindow.C
ui/WindowActionsUI.C
)

SET(VIEWER_UI_MOC
ui/PlotAndOperatorActionsUI.h
ui/ViewerActionUI.h
ui/ViewerActionUIMultiple.h
ui/ViewerActionUISingle.h
ui/ViewerConnectionPrinterUI.h
ui/ViewerConnectionProgressDialog.h
ui/ViewerHostProfileSelectorWithWin.h
ui/ViewerPasswordWindow.h
ui/VisItChangeUsernameWindow.h
ui/VisItPasswordWindow.h
)

SET(VIEWERLIB_SOURCES
${VIEWER_UI}
${VIEWER_DDT}
#QWsSocket.C
SharedDaemon.C
ViewerBaseUI.C
ViewerClientConnection.C
ViewerCommandFromSimObserver.C
ViewerEngineManager.C
ViewerEngineManagerImplementation.C
ViewerFactoryMain.C
ViewerFileServer.C
ViewerFileServerImplementation.C
ViewerMasterXfer.C
ViewerMetaDataObserver.C
ViewerObserverToSignal.C
ViewerSILAttsObserver.C
ViewerServerManager.C
ViewerStateBuffered.C
ViewerSubject.C
VisItViewer.C
WebSocketConnection.C
${VIEWERLIB_WIN32_SOURCES}
)

# If the viewer is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(VIEWER_STATIC_SOURCES ViewerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

SET(VIEWERLIB_MOC_SOURCES
${VIEWER_UI_MOC}
${VIEWER_DDT_MOC}
#QWsSocket.h
SharedDaemon.h
ViewerBaseUI.h
ViewerClientConnection.h
ViewerCommandFromSimObserver.h
ViewerMetaDataObserver.h
ViewerObserverToSignal.h
ViewerSILAttsObserver.h
ViewerStateBuffered.h
ViewerSubject.h
WebSocketConnection.h
)
QT_WRAP_CPP(viewer VIEWERLIB_SOURCES ${VIEWERLIB_MOC_SOURCES})

IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    ADD_DEFINITIONS(-DCOMM_EXPORTS)
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/ui
${CMAKE_CURRENT_SOURCE_DIR}/ddt
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/viewer/core
${VISIT_SOURCE_DIR}/viewer/core/actions
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
${VISIT_SOURCE_DIR}/visit_vtk/osmesa
${VISIT_SOURCE_DIR}/vtkqt
${VISIT_SOURCE_DIR}/winutil
${QT_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
${QT_QTGUI_INCLUDE_DIR}
${QT_QTOPENGL_INCLUDE_DIR}
${QT_QTNETWORK_INCLUDE_DIR}
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${EAVL_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})


#*************************** The viewer library ******************************
ADD_LIBRARY(viewer ${VIEWERLIB_SOURCES})
IF (WIN32)
    SET_TARGET_PROPERTIES(viewer PROPERTIES OUTPUT_NAME viewerlib)
ENDIF (WIN32)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTCORE_LIBRARY}
${QT_QTNETWORK_LIBRARY}
)
IF(VISIT_STATIC)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

TARGET_LINK_LIBRARIES(viewer 
viewerrpc
viewercore_ser
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
${CMAKE_THREAD_LIBS}
)

IF(MESA_FOUND)
    TARGET_LINK_LIBRARIES(viewer visit_vtk_osmesa)
ENDIF(MESA_FOUND)

#*************************** The viewer executable ***************************
IF(NOT APPLE)
    ADD_EXECUTABLE(viewer_exe ${VISIT_APPLICATION_STYLE} viewer.C ${VIEWER_STATIC_SOURCES} ${VISIT_VIEWER_RESOURCE_FILE})
ELSE(NOT APPLE)
    SET(VIEWER_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${VIEWER_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    ADD_EXECUTABLE(viewer_exe MACOSX_BUNDLE viewer.C ${VIEWER_STATIC_SOURCES} ${VISIT_SOURCE_DIR}/common/icons/${VIEWER_ICON_FILE})
    
    SET_TARGET_PROPERTIES( viewer_exe PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${VISIT_SOURCE_DIR}/include/ViewerInfo.plist.in)
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Graphical viewer for VisIt")
    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "VisIt version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC")
    
    IF(VISIT_STATIC)
        MAC_NIB_INSTALL(viewer)
    ENDIF(VISIT_STATIC)
ENDIF(NOT APPLE)

SET_TARGET_PROPERTIES(viewer_exe PROPERTIES OUTPUT_NAME viewer)
TARGET_LINK_LIBRARIES(viewer_exe
    ${viewer_exe_IOperator}
    ${viewer_exe_VOperator}
    ${viewer_exe_IPlot}
    ${viewer_exe_VPlot}
    viewer
    vtkpng
    vtkjpeg
    ${VTKZLIB_LIB}
    ${VTK_FREETYPE_LIBRARIES}
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
