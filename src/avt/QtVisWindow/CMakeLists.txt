# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(AVTQTVISWINDOW_SOURCES
VisWinRenderingWithWindow.C
QtVisWindow.C
vtk/vtkDashedXorGridMapper2D.C
vtk/vtkRubberBandMapper2D.C
)

IF(NOT APPLE AND X11_FOUND)
    SET_SOURCE_FILES_PROPERTIES(
        vtk/vtkDashedXorGridMapper2D.C  vtk/vtkRubberBandMapper2D.C 
        PROPERTIES
        COMPILE_FLAGS "-DHAVE_XLIB"
    )
ENDIF(NOT APPLE AND X11_FOUND)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/vtk
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Exceptions
${VISIT_SOURCE_DIR}/avt/VisWindow/Interactors
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/vtkqt
${VISIT_SOURCE_DIR}/winutil
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(avtqtviswindow ${AVTQTVISWINDOW_SOURCES})
TARGET_LINK_LIBRARIES(avtqtviswindow visitcommon avtviswindow_ser vtkqt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

VISIT_INSTALL_TARGETS(avtqtviswindow)
