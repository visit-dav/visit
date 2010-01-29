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

SET(GUILIB_SOURCES
AccessViewerSession.C
ColorTableObserver.C
FileServerList.C
GUIBase.C
KeyframeData.C
KeyframeDataModel.C
MovieSequence.C
MovieSequenceCompositeTransition.C
MovieSequenceCurrentPlots.C
MovieSequenceFactory.C
MovieSequenceRotate.C
MovieSequenceScripting.C
MovieSequenceTransition.C
MovieTemplateConfig.C
MovieUtility.C
NameSimplifier.C
QualifiedFilename.C
QvisAbstractOpacityBar.C
QvisAnimationSlider.C
QvisAnimationWindow.C
QvisAnnotationObjectInterface.C
QvisAnnotationObjectInterfaceFactory.C
QvisAnnotationWindow.C
QvisAppearanceWindow.C
QvisApplication.C
QvisAxisAttributesWidget.C
QvisColorButton.C
QvisColorGridWidget.C
QvisColorManagerWidget.C
QvisColorSelectionWidget.C
QvisColorSwatchListWidget.C
QvisColorTableWindow.C
QvisCommandWindow.C
QvisCompositeTransition.C
QvisDBOptionsDialog.C
QvisDatabaseCorrelationListWindow.C
QvisDatabaseCorrelationWindow.C
QvisDelayedWindow.C
QvisDelayedWindowObserver.C
QvisDelayedWindowSimpleObserver.C
QvisDialogLineEdit.C
QvisDownloader.C
QvisElementButton.C
QvisElementSelectionWidget.C
QvisEngineWindow.C
QvisExportDBWindow.C
QvisExpressionsWindow.C
QvisFileInformationWindow.C
QvisFileOpenDialog.C
QvisFileOpenWindow.C
QvisFilePanel.C
QvisFilePanelItem.C
QvisFileSelectionWindow.C
QvisFileWindowBase.C
QvisFontAttributesWidget.C
QvisGUIApplication.C
QvisGaussianOpacityBar.C
QvisGlobalLineoutWindow.C
QvisGridWidget.C
QvisHelpWindow.C
QvisHostProfileWindow.C
QvisImageAnnotationInterface.C
QvisInteractorWindow.C
QvisInterpreter.C
QvisKeyframeDelegate.C
QvisKeyframeDrawer.C
QvisKeyframePlotRangeWidget.C
QvisKeyframeWidget.C
QvisKeyframeWindow.C
QvisLegendAttributesInterface.C
QvisLightingWidget.C
QvisLightingWindow.C
QvisLine2DInterface.C
QvisLineStyleWidget.C
QvisLineWidthWidget.C
QvisMacroWindow.C
QvisMainWindow.C
QvisMaterialWindow.C
QvisMeshManagementWindow.C
QvisMessageWindow.C
QvisMovieProgressDialog.C
QvisNotepadArea.C
QvisOpacitySlider.C
QvisOperatorWindow.C
QvisOutputWindow.C
QvisPeriodicTableWidget.C
QvisPickWindow.C
QvisPlotListBox.C
QvisPlotListBoxItem.C
QvisPlotManagerWidget.C
QvisPluginWindow.C
QvisPointControl.C
QvisPostableMainWindow.C
QvisPostableWindow.C
QvisPostableWindowObserver.C
QvisPostableWindowSimpleObserver.C
QvisPredefinedViewports.C
QvisPreferencesWindow.C
QvisQueryOverTimeWindow.C
QvisQueryWindow.C
QvisRecentPathRemovalWindow.C
QvisRenderingWindow.C
QvisRotationTransition.C
QvisSILSetSelector.C
QvisSaveMovieWizard.C
QvisSaveWindow.C
QvisScreenPositionEdit.C
QvisScreenPositioner.C
QvisScribbleOpacityBar.C
QvisSequenceButton.C
QvisSequenceMappingModel.C
QvisSequenceTransition.C
QvisSequenceView.C
QvisSessionFileDatabaseLoader.C
QvisSessionSourceChanger.C
QvisSessionSourceChangerDialog.C
QvisSimulationCommandWindow.C
QvisSimulationMessageWindow.C
QvisSimulationWindow.C
QvisSpectrumBar.C
QvisStripChart.C
QvisStripChartMgr.C
QvisStripChartTabWidget.C
QvisSubsetPanelItem.C
QvisSubsetPanelWidget.C
QvisSubsetWindow.C
QvisText2DInterface.C
QvisText3DInterface.C
QvisTimeSliderInterface.C
QvisTurnDownButton.C
QvisUiLoader.C
QvisVCRControl.C
QvisViewWindow.C
QvisViewportWidget.C
QvisVisItUpdate.C
QvisWindowBase.C
QvisWizard.C
SimCommandSlots.C
SplashScreen.C
WidgetDataNode.C
mini3D.C
)

# The subset of the sources that have Q_OBJECT in their header.
SET(GUILIB_MOC_SOURCES
QvisAbstractOpacityBar.h
QvisAnimationSlider.h
QvisAnimationWindow.h
QvisAnnotationObjectInterface.h
QvisAnnotationWindow.h
QvisAppearanceWindow.h
QvisApplication.h
QvisAxisAttributesWidget.h
QvisColorButton.h
QvisColorGridWidget.h
QvisColorSelectionWidget.h
QvisColorTableWindow.h
QvisCommandWindow.h
QvisDBOptionsDialog.h
QvisDatabaseCorrelationListWindow.h
QvisDatabaseCorrelationWindow.h
QvisDelayedWindow.h
QvisDelayedWindowObserver.h
QvisDelayedWindowSimpleObserver.h
QvisDialogLineEdit.h
QvisDownloader.h
QvisMeshManagementWindow.h
QvisElementButton.h
QvisElementSelectionWidget.h
QvisEngineWindow.h
QvisExportDBWindow.h
QvisExpressionsWindow.h
QvisFileInformationWindow.h
QvisFilePanel.h
QvisFileSelectionWindow.h
QvisFileOpenWindow.h
QvisFileWindowBase.h
QvisFontAttributesWidget.h
QvisGaussianOpacityBar.h
QvisGlobalLineoutWindow.h
QvisGridWidget.h
QvisGUIApplication.h
QvisHelpWindow.h
QvisHostProfileWindow.h
QvisImageAnnotationInterface.h
QvisInteractorWindow.h
QvisInterpreter.h
QvisLightingWidget.h
QvisLightingWindow.h
QvisLineStyleWidget.h
QvisLineWidthWidget.h
QvisMacroWindow.h
QvisMainWindow.h
QvisMaterialWindow.h
QvisMessageWindow.h
QvisMovieProgressDialog.h
QvisNotepadArea.h
QvisOperatorWindow.h
QvisOpacitySlider.h
QvisOutputWindow.h
QvisPeriodicTableWidget.h
QvisPickWindow.h
QvisPlotListBox.h
QvisPointControl.h
QvisPlotManagerWidget.h
QvisPluginWindow.h
QvisPostableMainWindow.h
QvisPostableWindow.h
QvisPostableWindowObserver.h
QvisPostableWindowSimpleObserver.h
QvisPreferencesWindow.h
QvisQueryOverTimeWindow.h
QvisQueryWindow.h
QvisRecentPathRemovalWindow.h
QvisRenderingWindow.h
QvisSaveWindow.h
QvisScreenPositionEdit.h
QvisScreenPositioner.h
QvisScribbleOpacityBar.h
QvisSessionFileDatabaseLoader.h
QvisSessionSourceChanger.h
QvisSessionSourceChangerDialog.h
QvisSpectrumBar.h
QvisTurnDownButton.h
QvisVCRControl.h
QvisViewWindow.h
QvisWindowBase.h
QvisWizard.h
SplashScreen.h
QvisSimulationWindow.h
QvisStripChartMgr.h
QvisStripChart.h
QvisStripChartTabWidget.h
SimCommandSlots.h
QvisSimulationMessageWindow.h
QvisSimulationCommandWindow.h
QvisFileOpenDialog.h
QvisColorManagerWidget.h
QvisColorSwatchListWidget.h
QvisSILSetSelector.h
QvisSubsetWindow.h
QvisSubsetPanelWidget.h
QvisTimeSliderInterface.h
QvisLegendAttributesInterface.h
QvisLine2DInterface.h
QvisText3DInterface.h
QvisText2DInterface.h
QvisVisItUpdate.h
QvisSequenceButton.h
QvisCompositeTransition.h
QvisSequenceTransition.h
QvisPredefinedViewports.h
QvisSaveMovieWizard.h
QvisSequenceView.h
QvisSequenceMappingModel.h
QvisKeyframeWindow.h
QvisKeyframeWidget.h
QvisKeyframePlotRangeWidget.h
QvisViewportWidget.h
)
QT_WRAP_CPP(gui GUILIB_SOURCES ${GUILIB_MOC_SOURCES})

IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    ADD_DEFINITIONS(-DCOMM_EXPORTS)
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/common
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/winutil
${QT_INCLUDE_DIR} 
${QT_QTCORE_INCLUDE_DIR} 
${QT_QTGUI_INCLUDE_DIR} 
${QT_QTOPENGL_INCLUDE_DIR} 
${QT_QTNETWORK_INCLUDE_DIR} 
${QT_QTXML_INCLUDE_DIR} 
${QT_QTUITOOLS_INCLUDE_DIR}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR})

SET(GUI_QT_LIBS
${QT_QTUITOOLS_LIBRARY}
${QT_QTOPENGL_LIBRARY} 
${QT_QTGUI_LIBRARY} 
${QT_QTNETWORK_LIBRARY} 
${QT_QTXML_LIBRARY} 
${QT_QTCORE_LIBRARY} 
)
IF(VISIT_STATIC)
    SET(GUI_QT_LIBS
        ${GUI_QT_LIBS}
        ${QT_QTCORE_LIB_DEPENDENCIES} 
        ${QT_QTGUI_LIB_DEPENDENCIES} 
        ${QT_QTOPENGL_LIB_DEPENDENCIES} 
        ${QT_QTNETWORK_LIB_DEPENDENCIES} 
        ${QT_QTXML_LIB_DEPENDENCIES} 
        ${QT_QTUITOOLS_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

#***************************** The gui library *******************************
ADD_LIBRARY(gui ${GUILIB_SOURCES})

IF (WIN32)
    SET_TARGET_PROPERTIES(gui PROPERTIES OUTPUT_NAME guilib)
ENDIF (WIN32)
TARGET_LINK_LIBRARIES(gui 
viewerrpc
viewerproxy
visitcommon
mdserverrpc
mdserverproxy
avtdbatts
winutil
${GUI_QT_LIBS}
)

IF(APPLE)
    SET_TARGET_PROPERTIES(gui 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
ENDIF(APPLE)

# Install these targets
VISIT_INSTALL_TARGETS(gui)

IF(NOT VISIT_STATIC)
    ADD_SUBDIRECTORY(main)
ENDIF(NOT VISIT_STATIC)
