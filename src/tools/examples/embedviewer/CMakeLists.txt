# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Tue Nov 30 11:32:08 PST 2010
#   I added multiwindow example.
#
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Brad Whitlock, Thu Oct 20 15:42:10 PDT 2011
#   Fix static build.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

set(embedded_SOURCES
    CommandParser.C
    SimpleVisApp.C
    Synchronizer.C
    main.C)

set(embedded_HEADERS
    CommandParser.h
    SimpleVisApp.h
    Synchronizer.h
    main.C)

set(multiwindow_SOURCES
    MultiWindowVisApp.C
    main.C)

set(multiwindow_HEADERS
    MultiWindowVisApp.h
    main.C)

# If the viewer is static then we need some static symbol lookup functions
if(VISIT_STATIC)
    list(APPEND embedded_SOURCES ${VISIT_SOURCE_DIR}/viewer/main/ViewerStaticSymbolLocator.C)
    list(APPEND multiwindow_SOURCES ${VISIT_SOURCE_DIR}/viewer/main/ViewerStaticSymbolLocator.C)
endif()

set(embedded_DEPENDS 
    ${viewer_exe_IOperator}
    ${viewer_exe_VOperator}
    ${viewer_exe_IPlot}
    ${viewer_exe_VPlot}
    avtdatabase_ser 
    avtdbatts
    avtfilters_ser 
    avtmath
    avtmir_ser 
    avtpipeline_ser
    avtplotter_ser 
    avtqtviswindow
    avtview 
    avtviswindow_ser
    avtwriter_ser 
    engineproxy
    enginerpc
    lightweight_visit_vtk
    mdserverproxy
    mdserverrpc
    vclproxy
    vclrpc
    viewer
    viewerrpc 
    visit_verdict 
    visit_vtk
    vtkqt
    ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTCORE_LIBRARY})

if(VISIT_STATIC)
    list(APPEND embedded_DEPENDS
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES})
endif()


if(APPLE)
    set(GUI_ICON_FILE VisItIcon.icns)
    set(RESOURCE_FILE ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
    set_source_files_properties(${RESOURCE_FILE}
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set(MACOSX_BUNDLE_BUNDLE_NAME          "Embedded viewer example ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_INFO_STRING          "Embedded viewer example")
    set(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "Embedded viewer example version ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "Embedded viewer example ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2024, Lawrence Livermore National Security, LLC")

    set(CMAKE_MACOSX_BUNDLE ON)
endif()

visit_add_executable(
    NAME        embedded 
    SOURCES     ${embedded_SOURCES} ${RESOURCE_FILE}
    HEADERS     ${embedded_HEADERS}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${embedded_DEPENDS}
    FOLDER      tools/examples/embedded
    SKIP_INSTALL)

visit_add_executable(
    NAME        multiwindow 
    SOURCES     ${multiwindow_SOURCES} ${RESOURCE_FILE}
    HEADERS     ${multiwindow_HEADERS}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${embedded_DEPENDS}
    FOLDER      tools/examples/embedded
    SKIP_INSTALL)

set_target_properties(embedded multiwindow PROPERTIES AUTOMOC ON)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(embedded)
    qt6_disable_unicode_defines(multiwindow)
endif()

if(APPLE)
    if(VISIT_STATIC)
        MAC_NIB_INSTALL(embedded)
        MAC_NIB_INSTALL(multiwindow)
    endif()

    # Hey, do we need "-framework Carbon" on 10.6?
    set_target_properties(embedded multiwindow PROPERTIES
        MACOSX_BUNDLE TRUE
        LINK_FLAGS "-framework Carbon")
endif()



