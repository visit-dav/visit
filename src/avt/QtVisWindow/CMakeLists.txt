# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Kathleen Biagas, Tue Dec 13, 2022
#  Moved setting of vtk sources to subdir.
#
#****************************************************************************

add_library(avtqtviswindow QtVisWindow.C VisWinRenderingWithWindow.C)

add_subdirectory(vtk)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
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
    ${VTK_INCLUDE_DIRS})

target_link_libraries(avtqtviswindow visitcommon avtviswindow_ser vtkqt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

VISIT_INSTALL_TARGETS(avtqtviswindow)

