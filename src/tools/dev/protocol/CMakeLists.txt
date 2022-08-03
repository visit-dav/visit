# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/launcher/rpc
${VISIT_SOURCE_DIR}/launcher/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
)

# If static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES ../../../engine/main/EngineStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(VTK_EXTRA_LIBS vtktiff vtkjpeg vtkpng)
else()
    set(VTK_EXTRA_LIBS VTK::tiff VTK::jpeg VTK::png)
endif()

ADD_EXECUTABLE(visitprotocol dump_protocol.C ${STATIC_SOURCES})
TARGET_LINK_LIBRARIES(visitprotocol 
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
${engine_ser_exe_IOperator}
${engine_ser_exe_EOperator_ser}
${engine_ser_exe_IPlot}
${engine_ser_exe_EPlot_ser}
avtdatabase_ser
viewerrpc
viewerproxy
enginerpc
engineproxy
vclrpc
vclproxy
mdserverrpc
mdserverproxy
visitcommon
${VTK_EXTRA_LIBS}
${ALL_THIRDPARTY_IO_LIB}
${MESA_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
)
IF(NOT WIN32)
    TARGET_LINK_LIBRARIES(visitprotocol dl)
    IF(NOT APPLE)
        TARGET_LINK_LIBRARIES(visitprotocol ${X11_LIBRARIES})
    ENDIF(NOT APPLE)
ENDIF(NOT WIN32)

# If we're building statically then we can't be link until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(visitprotocol
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        ${engine_ser_exe_IOperator}
        ${engine_ser_exe_EOperator_ser}
        ${engine_ser_exe_IPlot}
        ${engine_ser_exe_EPlot_ser}
    )
ENDIF(VISIT_STATIC)

VISIT_INSTALL_TARGETS(visitprotocol)
VISIT_TOOLS_ADD_FOLDER(visitprotocol)
