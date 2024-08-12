# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Cyrus Harrison, Mon Feb  8 15:40:54 PST 2010
#   Added QvisPythonSyntaxHighlighter
#
#   Cyrus Harrison, Thu Feb 11 09:37:20 PST 2010
#   Added QvisPythonFilterEditor
#
#   Cyrus Harrison, Mon Mar 15 13:27:16 PDT 2010
#   Added QvisTimeSliderControlWidget & QvisSourceManagerWidget.
#
#   Hank Childs, Sun Aug  1 14:41:50 PDT 2010
#   Add QvisCMFEWizard.
#
#   Brad Whitlock, Fri Aug  6 16:34:54 PDT 2010
#   Added QvisSelectionsWindow.
#
#   Brad Whitlock, Mon Dec 27 16:15:50 PST 2010
#   Added QvisHistogram, QvisHistogramLimits, QvisVariableListLimiter.
#
#   Brad Whitlock, Thu Jun  9 15:22:21 PDT 2011
#   Added QvisSelectionsDialog.
#
#   Kathleen Biagas, Fri Jun 10 08:57:36 PDT 2011
#   Added widgets for PickQuery, LineoutQuery, HohlraumFluxQuery,
#   XRayImageQuery,  TimeQueryOtions.
#
#   Brad Whitlock, Thu Sep 14 16:53:41 PDT 2017
#   Cinema support.
#
#   Eric Brugger, Thu Aug  5 11:21:21 PDT 2021
#   Removed support for SeedMe.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#   Kathleen Biagas, Thu Jul 18, 2024
#   QWT is now optional, so make StripChart classes dependent upon
#   it being found.
#
#****************************************************************************

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
QvisCinemaWizard.C
QvisCMFEWizard.C
QvisCollapsibleFrame.C
QvisColorButton.C
QvisColorGridWidget.C
QvisColorManagerWidget.C
QvisColorSelectionWidget.C
QvisColorSwatchListWidget.C
QvisColorTableWindow.C
QvisCommandWindow.C
QvisCompositeTransition.C
QvisDBOptionsDialog.C
QvisDBOptionsHelpWindow.C
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
QvisSessionFileDialog.C
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
QvisHistogram.C
QvisHistogramLimits.C
QvisHohlraumFluxQueryWidget.C
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
QvisLine3DInterface.C
QvisLineoutQueryWidget.C
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
QvisPickQueryWidget.C
QvisPickWindow.C
QvisPlotListBox.C
QvisPlotListBoxItem.C
QvisPlotManagerWidget.C
QvisPluginManagerAttributesDataModel.C
QvisPluginWindow.C
QvisPointControl.C
QvisPostableMainWindow.C
QvisPostableWindow.C
QvisPostableWindowObserver.C
QvisPostableWindowSimpleObserver.C
QvisPredefinedViewports.C
QvisPreferencesWindow.C
QvisPythonFilterEditor.C
QvisPythonSyntaxHighlighter.C
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
QvisSelectionsDialog.C
QvisSelectionsWindow.C
QvisSetupHostProfilesAndConfigWindow.C
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
QvisSourceManagerWidget.C
QvisSpectrumBar.C
QvisSubsetPanelItem.C
QvisSubsetPanelWidget.C
QvisSubsetWindow.C
QvisText2DInterface.C
QvisText3DInterface.C
QvisTimeQueryOptionsWidget.C
QvisTimeSliderControlWidget.C
QvisTimeSliderInterface.C
QvisTurnDownButton.C
QvisUiLoader.C
QvisVariableListLimiter.C
QvisVCRControl.C
QvisViewWindow.C
QvisViewportWidget.C
QvisVisItUpdate.C
QvisWindowBase.C
QvisWizard.C
QvisXRayImageQueryWidget.C
SimCommandSlots.C
SplashScreen.C
WidgetDataNode.C
mini3D.C
)

if(QWT_FOUND)
    list(APPEND GUILIB_SOURCES
        QvisStripChart.C
        QvisStripChartMgr.C
        QvisStripChartTabWidget.C)
endif()

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
${VISIT_SOURCE_DIR}/third_party_builtin
)

SET(GUI_QT_LIBS
${QT_QTUITOOLS_LIBRARY}
${QT_QTOPENGL_LIBRARY} 
${QT_QTGUI_LIBRARY} 
${QT_QTNETWORK_LIBRARY} 
${QT_QTXML_LIBRARY} 
${QT_QTCORE_LIBRARY}
${QT_QTPRINTSUPPORT_LIBRARY}
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

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(gui)
endif()

set_target_properties(gui PROPERTIES AUTOMOC ON)

IF (WIN32)
    SET_TARGET_PROPERTIES(gui PROPERTIES OUTPUT_NAME guilib)
    if(QWT_FOUND)
        SET_TARGET_PROPERTIES(gui PROPERTIES COMPILE_DEFINITIONS QWT_DLL)
    endif()
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

if(QWT_FOUND)
    target_compile_definitions(gui PUBLIC HAVE_QWT)
    target_include_directories(gui PUBLIC ${QWT_INCLUDE_DIR})
    target_link_libraries(gui ${QWT_LIBRARY})
endif()

IF(APPLE)
    SET_TARGET_PROPERTIES(gui 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
ELSEIF(WIN32)
# Needed because QT-4.6.3 has one library, QT_QTUITOOLS_LIBRARY,
# with a default link library in it.
    SET_TARGET_PROPERTIES(gui
        PROPERTIES
        LINK_FLAGS "/NODEFAULTLIB:libcmt"
    )
ELSEIF(UNIX)
    TARGET_LINK_LIBRARIES(gui X11)
ENDIF(APPLE)

# Install these targets
VISIT_INSTALL_TARGETS(gui)

IF(NOT VISIT_STATIC)
    ADD_SUBDIRECTORY(main)
ENDIF(NOT VISIT_STATIC)
