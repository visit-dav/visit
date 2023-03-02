# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Sep 27 11:35:34 PDT 2018
#   Change PYTHON_LIBRARIES to PYTHON_LIBRARY for consistency throughout visit.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   Replaced vtktiff with vtktiff_LIBRARIES, vtkjpeg with vtkjpeg_LIBRARIES,
#   and vtkpng with vtkpng_LIBRARIES.
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${PYTHON_INCLUDE_PATH}
${VTK_INCLUDE_DIRS}
)

# If static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES ../../visitpy/cli/ScriptingStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

SET(VTK_EXTRA_LIBS
${vtktiff_LIBRARIES}
${vtkjpeg_LIBRARIES}
${vtkpng_LIBRARIES}
)

ADD_EXECUTABLE(usedataserver usedataserver.cpp VisItDataServer.cpp ${STATIC_SOURCES})
TARGET_LINK_LIBRARIES(usedataserver 
${cli_IOperator}
${cli_SOperator}
${cli_IPlot}
${cli_SPlot}
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
${PYTHON_LIBRARY}
${VTK_EXTRA_LIBS}
${CMAKE_THREAD_LIBS}
${ZLIB_LIBRARY}
)
IF(NOT WIN32)
    TARGET_LINK_LIBRARIES(usedataserver dl)
    IF(NOT APPLE)
        TARGET_LINK_LIBRARIES(usedataserver ${X11_LIBRARIES})
    ENDIF(NOT APPLE)
ENDIF(NOT WIN32)

# If we're building statically then we can't be link until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(usedataserver
        ${cli_IOperator}
        ${cli_EOperator}
        ${cli_IPlot}
        ${cli_EPlot}
    )
ENDIF(VISIT_STATIC)

VISIT_EXAMPLES_ADD_FOLDER(usedataserver)
