# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#  Kathleen Biagas, Tue May 5, 2023
#  Disable qt6 forcing of defines for UNICODE.
#
#*****************************************************************************

set(guiwrapper_include_paths
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VISIT_SOURCE_DIR}/gui/
    ${VISIT_SOURCE_DIR}/gui/main
    ${VISIT_SOURCE_DIR}/viewer/subjectproxy
    ${VISIT_SOURCE_DIR}/viewer/core 
    ${VISIT_SOURCE_DIR}/viewer/main 
    ${VISIT_SOURCE_DIR}/viewer/main/ui
    ${VISIT_SOURCE_DIR}/viewer/proxy
    ${VISIT_SOURCE_DIR}/viewer/rpc
    ${VISIT_SOURCE_DIR}/vtkqt
    ${VISIT_SOURCE_DIR}/common/utility
    ${VISIT_SOURCE_DIR}/common/state
    ${VISIT_SOURCE_DIR}/common/misc
    ${VISIT_SOURCE_DIR}/common/comm
    ${VISIT_SOURCE_DIR}/common/plugin
    ${VISIT_SOURCE_DIR}/common/Exceptions/Pipeline
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData/
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VTK_INSTALL_PREFIX}/include/vtk-5.0
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/winutil
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/avt/Plotter/vtk
    ${VISIT_SOURCE_DIR}/avt/Plotter/
    ${VTK_INCLUDE_DIRS}
)

include_directories(${guiwrapper_include_paths})

link_directories(${LIBRARY_OUTPUT_DIRECTORY} ${QWT_LIBRARY_DIR})
set(guiwrapper_link_libs 
    gui
    viewer
    viewersubjectproxy
${QT_QTUITOOLS_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTGUI_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTXML_LIBRARY}
${QT_QTCORE_LIBRARY}
${QWT_LIBRARY}
    )

set(guiwrapper_sources ${CMAKE_CURRENT_SOURCE_DIR}/guiwrapper.C)

#
# For dev build, we may need to link to mesa on Linux.
##
if(OSMESA_FOUND)
    set(guiwrapper_link_libs ${guiwrapper_link_libs} ${OSMESA_LIBRARIES})
endif()

add_library(guiwrapper SHARED ${guiwrapper_sources})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(guiwrapper)
endif()

target_link_libraries(guiwrapper ${guiwrapper_link_libs})

VISIT_INSTALL_TARGETS(guiwrapper)
