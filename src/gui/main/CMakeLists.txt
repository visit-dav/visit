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

if(VISIT_STATIC)
    # If the gui is static then we need some static symbol lookup functions
    set(GUI_STATIC_SOURCES GUIStaticSymbolLocator.C)
endif()

#****************************** The gui executable ***************************
if(APPLE)
    set(GUI_ICON_FILE VisItIcon.icns)
    set(VISIT_GUI_RESOURCE_FILE ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
    set_source_files_properties(${VISIT_GUI_RESOURCE_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
endif()

visit_add_executable(
    NAME        gui_exe 
    SOURCES     main.C
                ${GUI_STATIC_SOURCES}
                ${VISIT_GUI_RESOURCE_FILE}
    DEPENDS_ON  gui
                ${gui_exe_IOperator}
                ${gui_exe_GOperator}
                ${gui_exe_IPlot}
                ${gui_exe_GPlot}
    OUTPUT_NAME gui)

if(APPLE)
    if(VISIT_STATIC)
        # Satisfy references in libQtNetwork.
        MAC_NIB_INSTALL(gui)
    endif()
    set_target_properties(gui_exe PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${VISIT_SOURCE_DIR}/include/GUIInfo.plist.in)
    if(VISIT_STATIC)
        # Satisfy references in libQtNetwork.
        set_target_properties(gui_exe PROPERTIES
            LINK_FLAGS "-framework SystemConfiguration")
    endif()

    set(MACOSX_BUNDLE_BUNDLE_NAME          "VisIt ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_INFO_STRING          "Graphical client for VisIt")
    set(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       "gov.llnl.visit.gui")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "VisIt version ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "VisIt ${VISIT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2025, Lawrence Livermore National Security, LLC")
endif()

