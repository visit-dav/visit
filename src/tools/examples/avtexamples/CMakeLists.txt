# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Mon Jun 21 14:21:35 PDT 2021
#   Added qvtkopenglExample and qtvtkExample.
#
#   Kathleen Biagas, Mon Jun 28, 2021
#   Removed VTK_EXTRA_LIBS, it was never set.
#   Resolve runtime warnings and errors on Windows:
#      Add vtkInteractionStyle to qtvtkExample and qvtkopenglExample link.
#      Add vtkRenderingOpenGL2 to qvtkopenglExample link.
#  
#   Eric Brugger, Thu Jul 15 13:34:12 PDT 2021
#   Added qtviswinExample.C
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#   Kathleen Biagas, Tue Sep 5, 2023 
#   Remove untested, unused EGL logic.
#
#****************************************************************************


LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY}
${AVT_REQUIRED_LIB_DIR}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
)

set(avtexamples_COMMON_DEPENDS
    ${engine_ser_exe_IDatabase}
    ${engine_ser_exe_EDatabase_ser}
    avtdatabase_ser
    avtdbatts
    avtfilters_ser
    avtpipeline_ser
    lightweight_visit_vtk
    visitcommon
    visit_vtk
    ${ALL_THIRDPARTY_IO_LIB}
    ${OPENGL_LIBRARIES}
    Threads::Threads)

## avtFilterExample
visit_add_executable(
    NAME        avtFilterExample
    SOURCES     avtFilterExample.C
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${avtexamples_COMMON_DEPENDS}
                ${OSMESA_LIBRARIES}
    FOLDER      tools/examples/avtexamples)

## avtRenderExample
visit_add_executable(
    NAME        avtRenderExample
    SOURCES     avtRenderExample.C
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${avtexamples_COMMON_DEPENDS}
                avtplotter_ser
                avtview
                avtviswindow_ser
                avtwriter_ser
                ${OSMESA_LIBRARIES}
    FOLDER      tools/examples/avtexamples)

## avtPluginExample
visit_add_executable(
    NAME        avtPluginExample
    SOURCES     avtPluginExample.C
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_SOURCE_DIR}/avt/Plotter
    DEPENDS_ON  ${avtexamples_COMMON_DEPENDS}
                ${OSMESA_LIBRARIES}
    FOLDER      tools/examples/avtexamples)

## avtPlotPluginExample
visit_add_executable(
    NAME        avtPlotPluginExample
    SOURCES     avtPlotPluginExample.C
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${avtexamples_COMMON_DEPENDS}
                avtplotter_ser
                avtview
                avtviswindow_ser
                avtwriter_ser
                ${OSMESA_LIBRARIES}
    FOLDER      tools/examples/avtexamples)

## networkManagerExample
visit_add_executable(
    NAME        networkManagerExample
    SOURCES     networkManagerExample.C
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  ${avtexamples_COMMON_DEPENDS}
                avtplotter_ser
                avtview
                avtviswindow_ser
                avtwriter_ser
                engine_ser
    FOLDER      tools/examples/avtexamples)

## qt examples

# qvtkopenglExample
visit_add_executable(
    NAME        qvtkopenglExample
    SOURCES     qvtkopenglExample.C
                GUIWindow.C
    HEADERS     GUIWindow.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  vtkqt
                ${QT_QTGUI_LIBRARY}
                ${QT_QTOPENGL_LIBRARY}
                ${QT_QTNETWORK_LIBRARY}
                ${QT_QTCORE_LIBRARY}
                VTK::CommonColor
                VTK::FiltersSources
                VTK::InteractionStyle
                VTK::RenderingOpenGL2
                ${OPENGL_LIBRARIES}
    FOLDER      tools/examples/avtexamples)
 

# qtvtkExample
visit_add_executable(
    NAME        qtvtkExample
    SOURCES     qtvtkExample.C
                GUIWindow2.C
    HEADERS     GUIWindow2.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS_ON  vtkqt
                ${QT_QTGUI_LIBRARY}
                ${QT_QTOPENGL_LIBRARY}
                ${QT_QTNETWORK_LIBRARY}
                ${QT_QTCORE_LIBRARY}
                VTK::CommonColor
                VTK::FiltersSources
                VTK::InteractionStyle
                VTK::RenderingOpenGL2
                ${OPENGL_LIBRARIES}
    FOLDER      tools/examples/avtexamples)

set_target_properties(qvtkopenglExample qtvtkExample PROPERTIES AUTOMOC ON)

# qtviswinExample
visit_add_executable(
    NAME        qtviswinExample
    SOURCES     qtviswinExample.C
    DEPENDS_ON  visitcommon
                avtdbatts
                avtview
                avtmath
                avtpipeline_ser
                avtplotter_ser
                avtfilters_ser
                avtviswindow_ser
                avtqtviswindow
                winutil
                ${QT_QTGUI_LIBRARY}
                ${QT_QTOPENGL_LIBRARY}
                ${QT_QTNETWORK_LIBRARY}
                ${QT_QTCORE_LIBRARY}
                VTK::GUISupportQt
    FOLDER      tools/examples/avtexamples)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(qtviswinExample)
endif()

