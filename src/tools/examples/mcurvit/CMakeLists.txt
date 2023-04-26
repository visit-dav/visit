# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   Replaced vtktiff with vtktiff_LIBRARIES, vtkjpeg with vtkjpeg_LIBRARIES,
#   and vtkpng with vtkpng_LIBRARIES.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#****************************************************************************

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
QvisLineWidthWidget.C
QvisMessageDialog.C
QvisOpacitySlider.C
Synchronizer.C
)

# If the viewer is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../../viewer/main/ViewerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

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
${VISIT_SOURCE_DIR}/viewer/core
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_SOURCE_DIR}/viewer/main/ui
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/vtkqt
${VTK_INCLUDE_DIRS}
)

SET(VTK_EXTRA_LIBS
${vtktiff_LIBRARIES}
${vtkjpeg_LIBRARIES}
${vtkpng_LIBRARIES}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

IF(APPLE)
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
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2023, Lawrence Livermore National Security, LLC")
ELSE(APPLE)
    ADD_EXECUTABLE(mcurvit ${VISIT_APPLICATION_STYLE} ${MCURVIT_SOURCES} ${STATIC_SOURCES})
ENDIF(APPLE)

set_target_properties(mcurvit PROPERTIES AUTOMOC ON)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    MAC_NIB_INSTALL(mcurvit)
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
${ZLIB_LIBRARY}
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
VISIT_TOOLS_ADD_FOLDER(mcurvit)
