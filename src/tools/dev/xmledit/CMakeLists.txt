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

set(xmledit_SOURCES
    XMLDocument.C
    XMLEdit.C
    XMLEditAttribute.C
    XMLEditCMake.C
    XMLEditCode.C
    XMLEditCodeGeneratorWindow.C
    XMLEditCodeSelectionDialog.C
    XMLEditConditional.C
    XMLEditConstants.C
    XMLEditEnums.C
    XMLEditFields.C
    XMLEditFunctions.C
    XMLEditIncludes.C
    XMLEditMain.C
    XMLEditPlugin.C)

set(xmledit_HEADERS
    XMLDocument.h
    XMLEdit.h
    XMLEditAttribute.h
    XMLEditCMake.h
    XMLEditCode.h
    XMLEditCodeGeneratorWindow.h
    XMLEditCodeSelectionDialog.h
    XMLEditConditional.h
    XMLEditConstants.h
    XMLEditEnums.h
    XMLEditFields.h
    XMLEditFunctions.h
    XMLEditIncludes.h
    XMLEditPlugin.h
    XMLEditStd.h
    XMLToolIds.h)

set(xmledit_DEPENDS 
    ${QT_QTGUI_LIBRARY}
    ${QT_QTWIDGETS_LIBRARY}
    ${QT_QTXML_LIBRARY})

if(VISIT_STATIC)
    list(APPEND xmledit_DEPENDS
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTXML_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES})
endif()

visit_add_executable(
    NAME        xmledit
    SOURCES     ${xmledit_SOURCES}
    HEADERS     ${xmledit_HEADERS}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_SOURCE_DIR}/common/utility
                ${VISIT_SOURCE_DIR}/tools/dev/xml
                ${VISIT_BINARY_DIR}/include
    DEPENDS_ON  ${xmledit_DEPENDS}
    FOLDER      tools/xml )

set_target_properties(xmledit PROPERTIES AUTOMOC ON)

qt6_disable_unicode_defines(xmledit)

if(APPLE)
    set_target_properties(xmledit PROPERTIES MACOSX_BUNDLE TRUE)

    set(MACOSX_BUNDLE_BUNDLE_NAME          "XmlEdit ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_INFO_STRING          "Editor for VisIt plugin XML files")
    set(MACOSX_BUNDLE_ICON_FILE            ${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns)

    set_source_files_properties(${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "XmlEdit version ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "XmlEdit ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2025, Lawrence Livermore National Security, LLC")
    if(VISIT_STATIC)
        MAC_NIB_INSTALL(xmledit)
    endif()
endif()

