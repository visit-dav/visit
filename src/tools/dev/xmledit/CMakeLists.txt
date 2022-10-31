# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Kathleen Biagas, Mon Oct 31, 2022
#   Modernize CMake usage: don't create unnecessary variables, use automoc,
#   use BUILD_INTERFACE for includes.
#
#****************************************************************************

add_executable(xmledit
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

target_include_directories(xmledit PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/tools/dev/xml>"
    "$<BUILD_INTERFACE:${VISIT_COMMON_INCLUDES}>")

if(APPLE)
    set_target_properties(xmledit PROPERTIES MACOSX_BUNDLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME          "XmlEdit ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_INFO_STRING          "Editor for VisIt plugin XML files")
    set(MACOSX_BUNDLE_ICON_FILE            ${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns)
    set_source_files_properties(${VISIT_SOURCE_DIR}/common/icons/VisItIcon.icns
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "XmlEdit version ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "XmlEdit ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2022, Lawrence Livermore National Security, LLC")
elseif(WIN32)
    set_target_properties(xmledit PROPERTIES ${VISIT_APPLICATION_STYLE}
                                              Qt5_NO_LINK_QTMAIN ON)
endif()

set_target_properties(xmledit PROPERTIES AUTOMOC ON)

if(VISIT_STATIC)
    MAC_NIB_INSTALL(xmledit)
    target_link_libraries(xmledit PRIVATE
        ${RELEVANT_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTXML_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES})
else()
    target_link_libraries(xmledit PRIVATE
        ${QT_QTGUI_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${QT_QTCORE_LIBRARY})
endif()

visit_install_targets_export(xmledit)

VISIT_TOOLS_ADD_FOLDER(xml xmledit)

