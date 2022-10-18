# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Tue Nov 30 11:32:08 PST 2010
#   I added multiwindow example.
#
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Brad Whitlock, Thu Oct 20 15:42:10 PDT 2011
#   Fix static build.
#
#****************************************************************************

SET(SOURCES1
CommandParser.C
SimpleVisApp.C
Synchronizer.C
main.C
)

SET(SOURCES2
MultiWindowVisApp.C
main.C
)

# If the viewer is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES1 ${CMAKE_CURRENT_SOURCE_DIR}/../../../viewer/main/ViewerStaticSymbolLocator.C)
    SET(STATIC_SOURCES2 ${CMAKE_CURRENT_SOURCE_DIR}/../../../viewer/main/ViewerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

# The subset of the sources that have Q_OBJECT in their header.
SET(MOC_SOURCES1
CommandParser.h
SimpleVisApp.h
Synchronizer.h
)
QT_WRAP_CPP(embedded SOURCES1 ${MOC_SOURCES1})

# The subset of the sources that have Q_OBJECT in their header.
SET(MOC_SOURCES2
MultiWindowVisApp.h
)
QT_WRAP_CPP(multiwindow SOURCES2 ${MOC_SOURCES2})

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/viewer/core
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_SOURCE_DIR}/viewer/main/ui
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/vtkqt
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

IF(NOT APPLE)
    ADD_EXECUTABLE(embedded ${VISIT_APPLICATION_STYLE} ${SOURCES1} ${STATIC_SOURCES1})
    ADD_EXECUTABLE(multiwindow ${VISIT_APPLICATION_STYLE} ${SOURCES2} ${STATIC_SOURCES2})
ELSE(NOT APPLE)
    SET(GUI_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    ADD_EXECUTABLE(embedded MACOSX_BUNDLE ${SOURCES1}  ${STATIC_SOURCES1} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "Embedded viewer example ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Embedded viewer example")
    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "Embedded viewer example version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "Embedded viewer example ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2022, Lawrence Livermore National Security, LLC")

    ADD_EXECUTABLE(multiwindow MACOSX_BUNDLE ${SOURCES2}  ${STATIC_SOURCES2} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
#    SET(MACOSX_BUNDLE_BUNDLE_NAME          "Multiwindow embedded viewer example ${VISIT_VERSION}")
#    SET(MACOSX_BUNDLE_INFO_STRING          "Multiwindow embedded viewer example")
#    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
#    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
#    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "Multiwindow embedded viewer example version ${VISIT_VERSION}")
#    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "Multiwindow embedded viewer example ${VISIT_VERSION}")
#    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
#    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2022, Lawrence Livermore National Security, LLC")
ENDIF(NOT APPLE)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    MAC_NIB_INSTALL(embedded)
    MAC_NIB_INSTALL(multiwindow)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

SET(EMBEDDED_VIEWER_LIBS
${viewer_exe_IOperator}
${viewer_exe_VOperator}
${viewer_exe_IPlot}
${viewer_exe_VPlot}
avtdatabase_ser 
avtdbatts
avtfilters_ser 
avtmath
avtmir_ser 
avtpipeline_ser
avtplotter_ser 
avtqtviswindow
avtview 
avtviswindow_ser
avtwriter_ser 
engineproxy
enginerpc
lightweight_visit_vtk
mdserverproxy
mdserverrpc
vclproxy
vclrpc
viewer
viewerrpc 
visit_verdict 
visit_vtk
vtkqt
${VIEWER_QT_LIBS}
)

TARGET_LINK_LIBRARIES(embedded ${EMBEDDED_VIEWER_LIBS})
TARGET_LINK_LIBRARIES(multiwindow ${EMBEDDED_VIEWER_LIBS})

IF(APPLE)
    # Hey, do we need this on 10.6?
    SET_TARGET_PROPERTIES(embedded 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
    SET_TARGET_PROPERTIES(multiwindow 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
ENDIF(APPLE)

# If we're building statically then the viewer can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(embedded
        ${viewer_exe_IOperator}
        ${viewer_exe_VOperator}
        ${viewer_exe_IPlot}
        ${viewer_exe_VPlot}
    )

    ADD_DEPENDENCIES(multiwindow
        ${viewer_exe_IOperator}
        ${viewer_exe_VOperator}
        ${viewer_exe_IPlot}
        ${viewer_exe_VPlot}
    )
ENDIF(VISIT_STATIC)

VISIT_EXAMPLES_ADD_FOLDER(embedded embedded multiwindow)
