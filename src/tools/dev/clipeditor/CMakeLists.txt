# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Wed Apr 10 18:00:15 PDT 2013
#   Fix for static on Mac.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************


visit_add_executable(
    NAME        genhexcases
    SOURCES     genhexcases.C
    SKIP_INSTALL)
visit_add_executable(
    NAME        gentestcases
    SOURCES     gentetcases.C
    SKIP_INSTALL)
visit_add_executable(
    NAME        genvoxcases
    SOURCES     genvoxcases.C
    SKIP_INSTALL)
visit_add_executable(
    NAME        genpolycases
    SOURCES     genpolycases.C
    SKIP_INSTALL)

set(clipeditor_SOURCES
    ClipEditor.C
    DataSet.C
    Font.C
    main.C
    Matrix.C
    Shape.C
    Transforms.C
    Vector.C
    Viewer.C)

set(clipeditor_HEADERS
    ClipEditor.h
    DataSet.h
    Font.h
    Matrix.h
    Shape.h
    Transforms.h
    Vector.h
    Viewer.h)

set(RELEVANT_QT_LIBS 
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTWIDGETS_LIBRARY}
    ${QT_QTOPENGLWIDGETS_LIBRARY})

if(VISIT_STATIC)
    MAC_NIB_INSTALL(clipeditor)
    list(APPEND RELEVANT_QT_LIBS
        ${QT_QTCORE_LIB_DEPENDENCIES}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES})
endif()

visit_add_executable(
    NAME        clipeditor
    SOURCES     ${clipeditor_SOURCES}
    HEADERS     ${clipeditor_HEADERS}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${RELEVANT_QT_LIBS}
                ${OPENGL_LIBRARIES})

set_target_properties(clipeditor PROPERTIES AUTOMOC ON)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(clipeditor)
endif()

