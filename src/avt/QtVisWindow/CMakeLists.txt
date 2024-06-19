# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Kathleen Biagas, Tue Dec 13, 2022
#  Moved setting of vtk sources to subdir.
#
#  Kathleen Biagas, Tue May 5, 2023
#  Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

visit_add_library(
    NAME       avtqtviswindow
    SOURCES    QtVisWindow.C
               VisWinRenderingWithWindow.C
    HEADERS    QtVisWindow.h
               qtviswindow_exports.h
               VisWinRenderingWithWindow.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/QtVisWindow>
    DEPENDS_ON visitcommon
               avtviswindow_ser
               vtkqt
               winutil
               ${QT_QTCORE_LIBRARY}
               ${QT_QTGUI_LIBRARY})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(avtqtviswindow)
endif()

add_subdirectory(vtk)
