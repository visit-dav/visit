# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#****************************************************************************

#ADD_DEPENDENCIES(rpc)
#ADD_DEPENDENCIES(proxy)
#ADD_DEPENDENCIES(viewer)

SET(VIEWERLIB_SOURCES
ViewerSubjectProxy.C
ViewerSubjectProxyFactory.C
)

# If the viewer is static then we need some static symbol lookup functions
#IF(VISIT_STATIC)
#    SET(VIEWER_STATIC_SOURCES ViewerStaticSymbolLocator.C)
#ENDIF(VISIT_STATIC)


#IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
#    ADD_DEFINITIONS(-DCOMM_EXPORTS)
#ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/core
${VISIT_SOURCE_DIR}/viewer/core/actions
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_SOURCE_DIR}/viewer/main/ui
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/QtVisWindow
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Interactors
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/launcher/proxy
${VISIT_SOURCE_DIR}/launcher/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/vtkqt
${VISIT_SOURCE_DIR}/winutil
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

#*************************** The viewer library ******************************
ADD_LIBRARY(viewersubjectproxy ${VIEWERLIB_SOURCES})

set_target_properties(viewersubjectproxy PROPERTIES AUTOMOC ON)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTCORE_LIBRARY}
)

IF(VISIT_STATIC)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

TARGET_LINK_LIBRARIES(viewersubjectproxy
viewer
viewerrpc
viewerproxy
visitcommon
mdserverrpc
mdserverproxy
vclrpc
vclproxy
enginerpc
engineproxy
avtdbatts
avtview
avtmath
avtpipeline_ser
avtplotter_ser
avtfilters_ser
avtviswindow_ser
avtwriter_ser
avtqtviswindow
winutil
${VIEWER_QT_LIBS}
)

VISIT_INSTALL_TARGETS(viewersubjectproxy)
