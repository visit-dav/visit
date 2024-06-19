# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Mon Jan 17 17:42:15 MST 2011
#   Add QvisColorTableWidget.
#
#   Kathleen Biagas, Fri Aug 8 08:52:46 PDT 2014
#   Added QvisNoDefaultColorTableButton.
#
#   Kathleen Biagas, Mon Mar 14 13:51:16 PDT 2016
#   With QT 5 on linux, link with Qt's X11Extras library.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   Link with X11 library when linking with X11Extras library.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

visit_add_library(
    NAME       winutil
    SOURCES    Appearance.C
               QvisColorTableButton.C
               QvisColorTableWidget.C
               QvisNoDefaultColorTableButton.C
               QvisVariableButton.C
               QvisVariableButtonHelper.C
               QvisVariablePopupMenu.C
               UnicodeHelper.C
               VariableMenuPopulator.C
               WindowMetrics.C
    HEADERS    Appearance.h
               QvisColorTableButton.h
               QvisColorTableWidget.h
               QvisNoDefaultColorTableButton.h
               QvisVariableButton.h
               QvisVariableButtonHelper.h
               QvisVariablePopupMenu.h
               UnicodeHelper.h
               VariableMenuPopulator.h
               WindowMetrics.h
               winutil_exports.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/winutil>
    DEPENDS_ON visitcommon
               avtdbatts
               ${QT_QTCORE_LIBRARY}
               ${QT_QTGUI_LIBRARY}
               ${QT_QTWIDGETS_LIBRARY})

if(LINUX)
# Specify the libraries to link against on Windows & Mac
    target_link_libraries(winutil PRIVATE ${QT_QTX11EXTRAS_LIBRARY} X11)
endif()

set_target_properties(winutil PROPERTIES AUTOMOC ON)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(avtqtviswindow)
endif()

