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

# List the files that we'll run moc on
SET(WINUTIL_MOC_SOURCES
QvisColorTableButton.h
QvisNoDefaultColorTableButton.h
QvisColorTableWidget.h
QvisVariableButton.h
QvisVariableButtonHelper.h
QvisVariablePopupMenu.h
)
QT_WRAP_CPP(WINUTIL WINUTIL_SOURCES ${WINUTIL_MOC_SOURCES})

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

# Specify the libraries to link against on Windows & Mac
TARGET_LINK_LIBRARIES(winutil visitcommon avtdbatts ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
if(LINUX)
    target_link_libraries(winutil ${QT_QTX11EXTRAS_LIBRARY} X11)
endif()

VISIT_INSTALL_TARGETS(winutil)
