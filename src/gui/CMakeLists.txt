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
#   Kathleen Biagas, Tue Sep 17, 2024
#   Add QvisStripChartMgr.C QvisStripChartTabWidget.C back to the build
#   even if QWT isn't found. This will preserve the ability for simulations
#   to use the StripChart callbacks and not crash.
#
#****************************************************************************

set(GUILIB_SOURCES
    AccessViewerSession.C
    ColorTableObserver.C
    FileServerList.C
    GUIBase.C
    KeyframeData.C
    KeyframeDataModel.C
    mini3D.C
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
    QvisDatabaseCorrelationListWindow.C
    QvisDatabaseCorrelationWindow.C
    QvisDBOptionsDialog.C
    QvisDBOptionsHelpWindow.C
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
    QvisGaussianOpacityBar.C
    QvisGlobalLineoutWindow.C
    QvisGridWidget.C
    QvisGUIApplication.C
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
    QvisSaveMovieWizard.C
    QvisSaveWindow.C
    QvisScreenPositionEdit.C
    QvisScreenPositioner.C
    QvisScribbleOpacityBar.C
    QvisSelectionsDialog.C
    QvisSelectionsWindow.C
    QvisSequenceButton.C
    QvisSequenceMappingModel.C
    QvisSequenceTransition.C
    QvisSequenceView.C
    QvisSessionFileDatabaseLoader.C
    QvisSessionSourceChanger.C
    QvisSessionSourceChangerDialog.C
    QvisSetupHostProfilesAndConfigWindow.C
    QvisSILSetSelector.C
    QvisSimulationCommandWindow.C
    QvisSimulationMessageWindow.C
    QvisSimulationWindow.C
    QvisSourceManagerWidget.C
    QvisSpectrumBar.C
    QvisStripChartMgr.C
    QvisStripChartTabWidget.C
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
    QvisViewportWidget.C
    QvisViewWindow.C
    QvisVisItUpdate.C
    QvisWindowBase.C
    QvisWizard.C
    QvisXRayImageQueryWidget.C
    SimCommandSlots.C
    SplashScreen.C
    WidgetDataNode.C)

set(GUILIB_HEADERS
    AccessViewerSession.h
    ColorTableObserver.h
    FileServerList.h
    GUIBase.h
    KeyframeData.h
    KeyframeDataModel.h
    mini3D.h
    MovieSequence.h
    MovieSequenceCompositeTransition.h
    MovieSequenceCurrentPlots.h
    MovieSequenceFactory.h
    MovieSequenceRotate.h
    MovieSequenceScripting.h
    MovieSequenceTransition.h
    MovieTemplateConfig.h
    MovieUtility.h
    NameSimplifier.h
    QNarrowLineEdit.h
    QualifiedFilename.h
    QvisAbstractOpacityBar.h
    QvisAnimationSlider.h
    QvisAnimationWindow.h
    QvisAnnotationObjectInterface.h
    QvisAnnotationObjectInterfaceFactory.h
    QvisAnnotationWindow.h
    QvisAppearanceWindow.h
    QvisApplication.h
    QvisAxisAttributesWidget.h
    QvisCinemaWizard.h
    QvisCMFEWizard.h
    QvisCollapsibleFrame.h
    QvisColorButton.h
    QvisColorGridWidget.h
    QvisColorManagerWidget.h
    QvisColorSelectionWidget.h
    QvisColorSwatchListWidget.h
    QvisColorTableWindow.h
    QvisCommandWindow.h
    QvisCompositeTransition.h
    QvisDatabaseCorrelationListWindow.h
    QvisDatabaseCorrelationWindow.h
    QvisDBOptionsDialog.h
    QvisDBOptionsHelpWindow.h
    QvisDelayedWindow.h
    QvisDelayedWindowObserver.h
    QvisDelayedWindowSimpleObserver.h
    QvisDialogLineEdit.h
    QvisDownloader.h
    QvisElementButton.h
    QvisElementSelectionWidget.h
    QvisEngineWindow.h
    QvisExportDBWindow.h
    QvisExpressionsWindow.h
    QvisFileInformationWindow.h
    QvisFileOpenDialog.h
    QvisFileOpenWindow.h
    QvisSessionFileDialog.h
    QvisFilePanel.h
    QvisFilePanelItem.h
    QvisFileSelectionWindow.h
    QvisFileWindowBase.h
    QvisFontAttributesWidget.h
    QvisGaussianOpacityBar.h
    QvisGlobalLineoutWindow.h
    QvisGridWidget.h
    QvisGUIApplication.h
    QvisHelpWindow.h
    QvisHistogram.h
    QvisHistogramLimits.h
    QvisHohlraumFluxQueryWidget.h
    QvisHostProfileWindow.h
    QvisImageAnnotationInterface.h
    QvisInteractorWindow.h
    QvisInterpreter.h
    QvisKeyframeDelegate.h
    QvisKeyframeDrawer.h
    QvisKeyframePlotRangeWidget.h
    QvisKeyframeWidget.h
    QvisKeyframeWindow.h
    QvisLegendAttributesInterface.h
    QvisLightingWidget.h
    QvisLightingWindow.h
    QvisLine2DInterface.h
    QvisLine3DInterface.h
    QvisLineoutQueryWidget.h
    QvisLineWidthWidget.h
    QvisMacroWindow.h
    QvisMainWindow.h
    QvisMaterialWindow.h
    QvisMeshManagementWindow.h
    QvisMessageWindow.h
    QvisMovieProgressDialog.h
    QvisNotepadArea.h
    QvisOpacitySlider.h
    QvisOperatorWindow.h
    QvisOutputWindow.h
    QvisPeriodicTableWidget.h
    QvisPickQueryWidget.h
    QvisPickWindow.h
    QvisPlotListBox.h
    QvisPlotListBoxItem.h
    QvisPlotManagerWidget.h
    QvisPluginManagerAttributesDataModel.h
    QvisPluginWindow.h
    QvisPointControl.h
    QvisPostableMainWindow.h
    QvisPostableWindow.h
    QvisPostableWindowObserver.h
    QvisPostableWindowSimpleObserver.h
    QvisPredefinedViewports.h
    QvisPreferencesWindow.h
    QvisPythonFilterEditor.h
    QvisPythonSyntaxHighlighter.h
    QvisQueryOverTimeWindow.h
    QvisQueryWindow.h
    QvisRecentPathRemovalWindow.h
    QvisRenderingWindow.h
    QvisRotationTransition.h
    QvisSaveMovieWizard.h
    QvisSaveWindow.h
    QvisScreenPositionEdit.h
    QvisScreenPositioner.h
    QvisScribbleOpacityBar.h
    QvisSelectionsDialog.h
    QvisSelectionsWindow.h
    QvisSequenceButton.h
    QvisSequenceMappingModel.h
    QvisSequenceTransition.h
    QvisSequenceView.C
    QvisSessionFileDatabaseLoader.h
    QvisSessionSourceChanger.h
    QvisSessionSourceChangerDialog.h
    QvisSetupHostProfilesAndConfigWindow.h
    QvisSILSetSelector.h
    QvisSimulationCommandWindow.h
    QvisSimulationMessageWindow.h
    QvisSimulationWindow.h
    QvisSourceManagerWidget.h
    QvisSpectrumBar.h
    QvisStripChartMgr.h
    QvisStripChartTabWidget.h
    QvisSubsetPanelItem.h
    QvisSubsetPanelWidget.h
    QvisSubsetWindow.h
    QvisText2DInterface.h
    QvisText3DInterface.h
    QvisTimeQueryOptionsWidget.h
    QvisTimeSliderControlWidget.h
    QvisTimeSliderInterface.h
    QvisTurnDownButton.h
    QvisUiLoader.h
    QvisVariableListLimiter.h
    QvisVCRControl.h
    QvisViewportWidget.h
    QvisViewWindow.h
    QvisVisItUpdate.h
    QvisWindowBase.h
    QvisWizard.h
    QvisXRayImageQueryWidget.h
    SimCommandSlots.h
    SplashScreen.h
    StatusSubject.h
    WidgetDataNode.h
    gui_exports.h)

if(QWT_FOUND)
    list(APPEND GUILIB_SOURCES QvisStripChart.C)
    list(APPEND GUILIB_HEADERS QvisStripChart.h)
endif()



set(GUI_QT_LIBS
    ${QT_QTUITOOLS_LIBRARY}
    ${QT_QTOPENGL_LIBRARY} 
    ${QT_QTGUI_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTXML_LIBRARY} 
    ${QT_QTCORE_LIBRARY}
    ${QT_QTPRINTSUPPORT_LIBRARY})

if(VISIT_STATIC)
    list(APPEND GUI_QT_LIBS
        ${QT_QTCORE_LIB_DEPENDENCIES} 
        ${QT_QTGUI_LIB_DEPENDENCIES} 
        ${QT_QTOPENGL_LIB_DEPENDENCIES} 
        ${QT_QTNETWORK_LIB_DEPENDENCIES} 
        ${QT_QTXML_LIB_DEPENDENCIES} 
        ${QT_QTUITOOLS_LIB_DEPENDENCIES})
endif()

#***************************** The gui library *******************************

visit_add_library(
    NAME        gui
    SOURCES     ${GUILIB_SOURCES}
    HEADERS     ${GUILIB_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/common>
                $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/third_party_builtin>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/gui>
    DEPENDS_ON  viewerrpc
                viewerproxy
                visitcommon
                mdserverrpc
                mdserverproxy
                avtdbatts
                winutil
                ${GUI_QT_LIBS})

if (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    target_compile_definitions(gui PRIVATE COMM_EXPORTS)
endif()

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(gui)
endif()

set_target_properties(gui PROPERTIES AUTOMOC ON)

if(WIN32)
    set_target_properties(gui PROPERTIES OUTPUT_NAME guilib)
    if(QWT_FOUND)
        set_target_properties(gui PROPERTIES COMPILE_DEFINITIONS QWT_DLL)
    endif()
endif()

if(QWT_FOUND)
    target_link_libraries(gui PUBLIC qwt)
endif()

if(APPLE)
    set_target_properties(gui PROPERTIES LINK_FLAGS "-framework Carbon")
elseif(WIN32)
    # Needed because Qt has one library, QT_QTUITOOLS_LIBRARY,
    # with a default link library in it.
    set_target_properties(gui PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt")
elseif(UNIX)
    target_link_libraries(gui PUBLIC X11)
endif()


if(NOT VISIT_STATIC)
    add_subdirectory(main)
endif()

