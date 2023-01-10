# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Thu Oct 20 16:23:59 PDT 2011
#   Update static linking for a newer Qt which has an additional framework
#   dependency on Mac.
#
#   Brad Whitlock, Wed Nov 23 13:17:14 PST 2011
#   Allow for no console windows apps.
#
#   Brad Whitlock, Fri May 18 16:09:34 PST 2012
#   Use different resource file.
#
#   Kathleen Biagas, Tue May 24 16:17:22 MST 2016
#   Add qwt lib.
#
#   Kevin Griffin, Mon Oct 19 09:43:37 PDT 2020
#   Updated GUI bundle name, identifier and copyright
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/common
${VISIT_SOURCE_DIR}/gui
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/winutil
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QWT_LIBRARY_DIR})

IF(VISIT_STATIC)
    # If the gui is static then we need some static symbol lookup functions
    SET(GUI_STATIC_SOURCES GUIStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

#****************************** The gui executable ***************************
IF(NOT APPLE)
    ADD_EXECUTABLE(gui_exe ${VISIT_APPLICATION_STYLE} main.C ${GUI_STATIC_SOURCES} ${VISIT_GUI_RESOURCE_FILE})
ELSE(NOT APPLE)
    SET(GUI_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    IF(VISIT_STATIC)
        # Satisfy references in libQtNetwork.
        SET(APPLE_QT_DEPENDENCIES "-framework SystemConfiguration")
        MAC_NIB_INSTALL(gui)
    ENDIF(VISIT_STATIC)

    ADD_EXECUTABLE(gui_exe MACOSX_BUNDLE main.C ${GUI_STATIC_SOURCES} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})

    SET_TARGET_PROPERTIES( gui_exe PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${VISIT_SOURCE_DIR}/include/GUIInfo.plist.in)
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Graphical client for VisIt")
    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       "gov.llnl.visit.gui")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "VisIt version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2023, Lawrence Livermore National Security, LLC")

ENDIF(NOT APPLE)

SET_TARGET_PROPERTIES(gui_exe PROPERTIES OUTPUT_NAME gui)
TARGET_LINK_LIBRARIES(gui_exe 
gui
${gui_exe_IOperator}
${gui_exe_GOperator}
${gui_exe_IPlot}
${gui_exe_GPlot}
${APPLE_QT_DEPENDENCIES}
)

# If we're building statically then the gui can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(gui_exe
        ${gui_exe_IOperator}
        ${gui_exe_GOperator}
        ${gui_exe_IPlot}
        ${gui_exe_GPlot}
    )
ENDIF(VISIT_STATIC)

# Install these targets
VISIT_INSTALL_TARGETS(gui_exe)
