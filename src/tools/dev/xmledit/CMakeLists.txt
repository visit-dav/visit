# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

SET(XMLEDIT_SOURCES
XMLEditMain.C
XMLDocument.C
XMLEdit.C
XMLEditAttribute.C
XMLEditPlugin.C
XMLEditEnums.C
XMLEditFields.C
XMLEditFunctions.C
XMLEditCode.C
XMLEditCodeGeneratorWindow.C
XMLEditCodeSelectionDialog.C
XMLEditConstants.C
XMLEditIncludes.C
XMLEditMakefile.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/tools/dev/xml
)

IF(APPLE)
    ADD_EXECUTABLE(xmledit MACOSX_BUNDLE ${XMLEDIT_SOURCES})
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "XmlEdit ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Editor for VisIt plugin XML files")
    SET(MACOSX_BUNDLE_ICON_FILE            ${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "XmlEdit version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "XmlEdit ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2023, Lawrence Livermore National Security, LLC")
ELSE(APPLE)
    ADD_EXECUTABLE(xmledit ${VISIT_APPLICATION_STYLE} ${XMLEDIT_SOURCES})
ENDIF(APPLE)

set_target_properties(xmledit PROPERTIES AUTOMOC ON)

SET(RELEVANT_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTWIDGETS_LIBRARY}
${QT_QTXML_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    MAC_NIB_INSTALL(xmledit)
    SET(RELEVANT_QT_LIBS
        ${RELEVANT_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTXML_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

TARGET_LINK_LIBRARIES(xmledit ${RELEVANT_QT_LIBS})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(xmledit)
endif()

VISIT_INSTALL_TARGETS(xmledit)
VISIT_TOOLS_ADD_FOLDER(xml xmledit)
