# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Kathleen Biagas, Tue Feb 5 15:15:27 PST 2013
#    Removed GUISupport, we use VTK's version now.
#
#    Kevin Griffin, Tue Apr 25 11:43:15 PDT 2017
#    Added Cocoa framework for OSX helper function.
#
#    Kathleen Biagas, Tue May 5, 2023
#    Disable qt6 forcing of defines for UNICODE.
#
#    Kathleen Biagas, Tue Sep 5, 2023 
#    Remove untested, unused EGL logic.
#
#    Kathleen Biagas, Mon Jun 3, 2024
#    Modified to use visit_add_library.
#
#****************************************************************************

set(vtkqt_SOURCES
    vtkQtRenderWindow.C
    vtkQtImagePrinter.C)

set(vtkqt_HEADERS
    vtkqt_exports.h
    vtkQtRenderWindow.h
    vtkQtImagePrinter.h)

if(APPLE)
    list(APPEND vtkqt_SOURCES osxHelper.mm)
    list(APPEND vtkqt_HEADERS osxHelper.h)
endif()

set(vtkqt_DEPENDS ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY} ${QT_QTPRINTSUPPORT_LIBRARY} ${OPENGL_LIBRARIES})

list(APPEND vtkqt_DEPENDS VTK::CommonCore VTK::GUISupportQt VTK::IOImage
        VTK::RenderingOpenGL2 VTK::RenderingFreeType)

if(HAVE_OSMESA)
    list(APPEND vtkqt_DEPENDS ${OSMESA_LIBRARIES})
endif()


visit_add_library(
    NAME        vtkqt
    SOURCES     ${vtkqt_SOURCES}
    HEADERS     ${vtkqt_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/vtkqt>
    DEPENDS_ON  ${vtkqt_DEPENDS})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(vtkqt)
endif()

if(APPLE)
    set_target_properties(vtkqt PROPERTIES LINK_FLAGS "-framework Cocoa")
endif()

