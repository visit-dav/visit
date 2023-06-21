# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Mon Jun 21 14:21:35 PDT 2021
#   Added qvtkopenglExample and qtvtkExample.
#
#   Kathleen Biagas, Mon Jun 28, 2021
#   Removed VTK_EXTRA_LIBS, it was never set.
#   Resolve runtime warnings and errors on Windows:
#      Add vtkInteractionStyle to qtvtkExample and qvtkopenglExample link.
#      Add vtkRenderingOpenGL2 to qvtkopenglExample link.
#  
#   Eric Brugger, Thu Jul 15 13:34:12 PDT 2021
#   Added qtviswinExample.C
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/DataBinning
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
${VISIT_SOURCE_DIR}/avt/Expressions/General
${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
${VISIT_SOURCE_DIR}/avt/Expressions/Management
${VISIT_SOURCE_DIR}/avt/Expressions/Math
${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/QtVisWindow
${VISIT_SOURCE_DIR}/avt/Queries/Abstract
${VISIT_SOURCE_DIR}/avt/Queries/Misc
${VISIT_SOURCE_DIR}/avt/Queries/Pick
${VISIT_SOURCE_DIR}/avt/Queries/Queries
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/main
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/vtkqt
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY}
${AVT_REQUIRED_LIB_DIR}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
${ICET_LIBRARY_DIR}
)

ADD_EXECUTABLE(avtFilterExample avtFilterExample.C)
VISIT_INSTALL_TARGETS(avtFilterExample)
VISIT_EXAMPLES_ADD_FOLDER(avtexamples avtFilterExample)

TARGET_LINK_LIBRARIES(avtFilterExample
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
avtdatabase_ser
avtdbatts
avtfilters_ser
avtpipeline_ser
lightweight_visit_vtk
visitcommon
visit_vtk
${ALL_THIRDPARTY_IO_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
${ZLIB_LIBRARY}
)

if(HAVE_OSMESA)
    target_link_libraries(avtFilterExample ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(avtFilterExample ${EGL_LIBRARIES})
endif()

ADD_EXECUTABLE(avtRenderExample avtRenderExample.C)
VISIT_INSTALL_TARGETS(avtRenderExample)
VISIT_EXAMPLES_ADD_FOLDER(avtexamples avtRenderExample)

TARGET_LINK_LIBRARIES(avtRenderExample
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
avtdatabase_ser
avtdbatts
avtfilters_ser
avtpipeline_ser
avtplotter_ser
avtview
avtviswindow_ser
avtwriter_ser
lightweight_visit_vtk
visitcommon
visit_vtk
${ALL_THIRDPARTY_IO_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
${ZLIB_LIBRARY}
)

if(HAVE_OSMESA)
    target_link_libraries(avtRenderExample ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(avtRenderExample ${EGL_LIBRARIES})
endif()


ADD_EXECUTABLE(avtPluginExample avtPluginExample.C)
VISIT_INSTALL_TARGETS(avtPluginExample)
VISIT_EXAMPLES_ADD_FOLDER(avtexamples avtPluginExample)

TARGET_LINK_LIBRARIES(avtPluginExample
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
avtdatabase_ser
avtdbatts
avtfilters_ser
avtpipeline_ser
lightweight_visit_vtk
visitcommon
visit_vtk
${ALL_THIRDPARTY_IO_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
${ZLIB_LIBRARY}
)

if(HAVE_OSMESA)
    target_link_libraries(avtPluginExample ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(avtPluginExample ${EGL_LIBRARIES})
endif()

ADD_EXECUTABLE(avtPlotPluginExample avtPlotPluginExample.C)
VISIT_INSTALL_TARGETS(avtPlotPluginExample)
VISIT_EXAMPLES_ADD_FOLDER(avtexamples avtPlotPluginExample)

TARGET_LINK_LIBRARIES(avtPlotPluginExample
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
avtdatabase_ser
avtdbatts
avtfilters_ser
avtpipeline_ser
avtplotter_ser
avtview
avtviswindow_ser
avtwriter_ser
lightweight_visit_vtk
visitcommon
visit_vtk
${ALL_THIRDPARTY_IO_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
${ZLIB_LIBRARY}
)

if(HAVE_OSMESA)
    target_link_libraries(avtPlotPluginExample ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(avtPlotPluginExample ${EGL_LIBRARIES})
endif()

ADD_EXECUTABLE(networkManagerExample networkManagerExample.C)
VISIT_INSTALL_TARGETS(networkManagerExample)
VISIT_EXAMPLES_ADD_FOLDER(avtexamples networkManagerExample)

TARGET_LINK_LIBRARIES(networkManagerExample
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
avtdatabase_ser
avtdbatts
avtfilters_ser
avtpipeline_ser
avtplotter_ser
avtview
avtviswindow_ser
avtwriter_ser
engine_ser
lightweight_visit_vtk
visitcommon
visit_vtk
${ALL_THIRDPARTY_IO_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
${ZLIB_LIBRARY}
)

SET(QVTKOPENGLEXAMPLE_SOURCES
qvtkopenglExample.C
GUIWindow.C
)

ADD_EXECUTABLE(qvtkopenglExample ${QVTKOPENGLEXAMPLE_SOURCES})
set_target_properties(qvtkopenglExample PROPERTIES AUTOMOC ON)
VISIT_INSTALL_TARGETS(qvtkopenglExample)
VISIT_EXAMPLES_ADD_FOLDER(qvtkopenglExample qvtkopenglExample)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
  set(vtklibs vtkCommonColor vtkFiltersSources vtkInteractionStyle vtkRenderingOpenGL2)
else()
  set(vtklibs VTK::CommonColor VTK::FiltersSources VTK::InteractionStyle VTK::RenderingOpenGL2)
endif()

TARGET_LINK_LIBRARIES(qvtkopenglExample
vtkqt
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
${vtklibs}
${OPENGL_LIBRARIES}
${ZLIB_LIBRARY}
)

SET(QTVTKEXAMPLE_SOURCES
qtvtkExample.C
GUIWindow2.C
)


ADD_EXECUTABLE(qtvtkExample ${QTVTKEXAMPLE_SOURCES})
set_target_properties(qtvtkExample PROPERTIES AUTOMOC ON)
VISIT_INSTALL_TARGETS(qtvtkExample)
VISIT_EXAMPLES_ADD_FOLDER(qtvtkExample qtvtkExample)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
  set(vtklibs vtkCommonColor vtkFiltersSources vtkInteractionStyle)
else()
  set(vtklibs VTK::CommonColor VTK::FiltersSources VTK::InteractionStyle VTK::RenderingVolumeOpenGL2)
endif()

TARGET_LINK_LIBRARIES(qtvtkExample
vtkqt
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
${vtklibs}
${OPENGL_LIBRARIES}
${ZLIB_LIBRARY}
)

SET(QTVISWINEXAMPLE_SOURCES
qtviswinExample.C
)

ADD_EXECUTABLE(qtviswinExample ${QTVISWINEXAMPLE_SOURCES})
VISIT_INSTALL_TARGETS(qtviswinExample)
VISIT_EXAMPLES_ADD_FOLDER(qtviswinExample qtviswinExample)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
  set(vtkqtlib vtkGUISupportQt)
else()
  set(vtkqtlib VTK::GUISupportQt)
endif()
TARGET_LINK_LIBRARIES(qtviswinExample
visitcommon
avtdbatts
avtview
avtmath
avtpipeline_ser
avtplotter_ser
avtfilters_ser
avtviswindow_ser
avtqtviswindow
winutil
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
${vtkqtlib}
)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(qtviswinExample)
endif()

