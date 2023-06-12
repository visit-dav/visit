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
#****************************************************************************

set(VTKQT_SOURCES
    vtkQtRenderWindow.C
    vtkQtImagePrinter.C
)

if(APPLE)
    list(APPEND VTKQT_SOURCES osxHelper.mm)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VISIT_BINARY_DIR}/include
    ${VISIT_SOURCE_DIR}/include
    ${VISIT_SOURCE_DIR}/vtkqt
    ${VISIT_SOURCE_DIR}/common/utility
    ${VTK_INCLUDE_DIRS}
)

add_library(vtkqt ${VTKQT_SOURCES})

# Specify the libraries to link against on Windows & Mac
if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonCore vtkGUISupportQt vtkGUISupportQtOpenGL vtkIOImage
        vtkRenderingOpenGL2 vtkRenderingFreeType)
else()
    set(vtklibs VTK::CommonCore VTK::GUISupportQt VTK::IOImage
        VTK::RenderingOpenGL2 VTK::RenderingFreeType)
endif()

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(vtkqt)
endif()

target_link_libraries(vtkqt ${vtklibs} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY} ${QT_QTPRINTSUPPORT_LIBRARY} ${OPENGL_LIBRARIES})

if(HAVE_OSMESA)
    target_link_libraries(vtkqt ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(vtkqt ${EGL_LIBRARIES})
endif()

if(APPLE)
    set_target_properties(vtkqt PROPERTIES
        LINK_FLAGS "-framework Cocoa"
    )
endif()

VISIT_INSTALL_TARGETS(vtkqt)

