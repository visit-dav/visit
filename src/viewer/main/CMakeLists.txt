# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
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
#  Kevin Griffin, Mon Oct 19 09:43:37 PDT 2020
#  Updated the bundle name, icon file name, identifier and copyright
#
#  Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#  Replaced vtkpng with vtkpng_LIBRARIES and vtkjpeg with vtkjpeg_LIBRARIES.
#
#  Kathleen Biagas, Thu Mar 30, 2023
#  Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#  Kathleen Biagas, Tue May 5, 2023
#  Disable qt6 forcing of defines for UNICODE.
#
#  Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#  Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#   Kathleen Biagas, Tue Sep 5, 2023
#   Remove untested, unused EGL logic.
#
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************

set(VIEWERLIB_SOURCES
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
    ViewerServerManager.C
    ViewerSILAttsObserver.C
    ViewerStateBuffered.C
    ViewerSubject.C
    VisItViewer.C
    WebSocketConnection.C)

set(VIEWERLIB_HEADERS
    SharedDaemon.h
    viewer_exports.h
    ViewerBaseUI.h
    ViewerClientConnection.h
    ViewerCommandFromSimObserver.h
    ViewerEngineManager.h
    ViewerEngineManagerImplementation.h
    ViewerEngineManagerImplementation_macros.h
    ViewerFactoryMain.h
    ViewerFileServer.h
    ViewerFileServerImplementation.h
    ViewerMasterXfer.h
    ViewerMetaDataObserver.h
    ViewerObserverToSignal.h
    ViewerServerManager.h
    ViewerSILAttsObserver.h
    ViewerStateBuffered.h
    ViewerSubject.h
    VisItViewer.h
    WebSocketConnection.h)

if(WIN32 AND VISIT_PARALLEL AND HAVE_HPC_SCHEDULER)
    list(APPEND VIEWERLIB_SOURCES
        ViewerSubmitParallelEngineToWindowsHPC.cpp)
    set_source_files_properties(ViewerEngineManager.C PROPERTIES
        COMPILE_DEFINITIONS VISIT_SUPPORTS_WINDOWS_HPC)
endif()

# If the viewer is static then we need some static symbol lookup functions
if(VISIT_STATIC)
    list(APPEND VIEWERLIB_SOURCES ViewerStaticSymbolLocator.C)
endif()

set(VIEWERLIB_DEPENDS
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
    Threads::Threads
    ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    VTK::GUISupportQt)
    
if(VTK_VERSION VERSION_LESS "9.4.0")
    if(VISIT_HAS_OSMESA)
        list(APPEND VIEWERLIB_DEPENDS visit_vtk_offscreen)
    endif()
endif()

if(VISIT_STATIC)
    list(APPEND VIEWERLIB_DEPENDS
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES})
endif()

#*************************** The viewer library ******************************
visit_add_library(
    NAME        viewer
    SOURCES     ${VIEWERLIB_SOURCES}
    HEADERS     ${VIEWERLIB_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/main>
    DEPENDS_ON  ${VIEWERLIB_DEPENDS})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(viewer)
endif()

set_target_properties(viewer PROPERTIES AUTOMOC ON)

if(WIN32)
    set_target_properties(viewer PROPERTIES OUTPUT_NAME viewerlib)
else()
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    target_compile_definitions(viewer PRIVATE COMM_EXPORTS)
endif()

if(VISIT_DDT)
    add_subdirectory(ddt)
endif()

add_subdirectory(ui)


#*************************** The viewer executable ***************************


if(APPLE)
    set(VIEWER_ICON_FILE VisItIcon.icns)
    set(VISIT_VIEWER_RESOURCE_FILE ${VISIT_SOURCE_DIR}/common/icons/${VIEWER_ICON_FILE})
    set_source_file_properties(${VISIT_VIEWER_RESOURCE_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
endif()

visit_add_executable(
    NAME        viewer_exe
    SOURCES     viewer.C
                ${VIEWER_STATIC_SOURCES}
                ${VISIT_VIEWER_RESOURCE_FILE}
    DEPENDS_ON  ${viewer_exe_IOperator}
                ${viewer_exe_VOperator}
                ${viewer_exe_IPlot}
                ${viewer_exe_VPlot}
                viewer
                ${vtkpng_LIBRARIES}
                ${vtkjpeg_LIBRARIES}
    OUTPUT_NAME viewer)
    



if(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME          "VisIt Viewer ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_INFO_STRING          "Graphical viewer for VisIt")
    set(MACOSX_BUNDLE_ICON_FILE            ${VIEWER_ICON_FILE})
    set(MACOSX_BUNDLE_VIEWER_IDENTIFIER       "gov.llnl.visit.viewer")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "VisIt version ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "VisIt ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2025, Lawrence Livermore National Security, LLC")
    set_target_properties(viewer_exe PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST
            ${VISIT_SOURCE_DIR}/include/ViewerInfo.plist.in)

    if(VISIT_STATIC)
        MAC_NIB_INSTALL(viewer)
    endif()
endif()

