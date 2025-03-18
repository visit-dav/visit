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
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

set(mcurvit_SOURCES
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
    Synchronizer.C)

set(mcurvit_HEADERS
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
    QvisLineWidthWidget.h
    QvisMessageDialog.h
    QvisOpacitySlider.h
    Synchronizer.h)

# If the viewer is static then we need some static symbol lookup functions
if(VISIT_STATIC)
    set(mcurvit_STATIC_SOURCES ${VISIT_SOURCE_DIR}/viewer/main/ViewerStaticSymbolLocator.C)
endif()

set(mcurvit_DEPENDS 
    ${viewer_exe_IOperator}
    ${viewer_exe_VOperator}
    ${viewer_exe_IPlot}
    ${viewer_exe_VPlot}
    viewer
    viewerrpc
    visitcommon
    vtkqt
    avtdbatts
    ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${vtktiff_LIBRARIES}
    ${vtkjpeg_LIBRARIES}
    ${vtkpng_LIBRARIES}
    ${ZLIB_LIBRARY})

if(VISIT_STATIC)
    list(APPEND mcurvit_DEPENDS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES})
endif()

visit_add_executable(
    NAME        mcurvit
    SOURCES     ${mcurvit_SOURCES}
                ${mcurvit_STATIC_SOURCES}
    HEADERS     ${mcurvit_HEADERS}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${mcurvit_DEPENDS}
    FOLDER      tools/mcurvit)

set_target_properties(mcurvit PROPERTIES AUTOMOC ON)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(mcurvit)
endif()

if(APPLE)
    set_target_properties(mcurvit PROPERTIES MACOSX_BUNDLE TRUE)

    set(MACOSX_BUNDLE_BUNDLE_NAME          "MCurveIt ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_INFO_STRING          "Curve visualizer")
    set(MACOSX_BUNDLE_ICON_FILE            ${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns)
    set_source_files_properties(${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "MCurveIt version ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "MCurveIt ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2025, Lawrence Livermore National Security, LLC")

   if (VISIT_STATIC)
       MAC_NIB_INSTALL(mcurvit)
   endif()
endif()

