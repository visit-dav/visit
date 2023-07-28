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

SET(WINUTIL_SOURCES
Appearance.C
QvisColorTableButton.C
QvisNoDefaultColorTableButton.C
QvisColorTableWidget.C
QvisVariableButton.C
QvisVariableButtonHelper.C
QvisVariablePopupMenu.C
UnicodeHelper.C
VariableMenuPopulator.C
WindowMetrics.C
)


INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/winutil
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VTK_INCLUDE_DIRS}
)

# Add link directories needed to use Qt.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(winutil ${WINUTIL_SOURCES})

set_target_properties(winutil PROPERTIES AUTOMOC ON)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(avtqtviswindow)
endif()

# Specify the libraries to link against on Windows & Mac
TARGET_LINK_LIBRARIES(winutil visitcommon avtdbatts ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTWIDGETS_LIBRARY})
if(LINUX)
    target_link_libraries(winutil ${QT_QTX11EXTRAS_LIBRARY} X11)
endif()

VISIT_INSTALL_TARGETS(winutil)
