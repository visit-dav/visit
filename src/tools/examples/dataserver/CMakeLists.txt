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
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#****************************************************************************

# If static then we need some static symbol lookup functions
if(VISIT_STATIC)
    set(dataserver_STATIC_SOURCES ${VISIT_SOURCE_DIR}/visitpy/cli/ScriptingStaticSymbolLocator.C)
endif()


visit_add_executable(
    NAME        usedataserver
    SOURCES     usedataserver.cpp
                VisItDataServer.cpp
                ${STATIC_SOURCES}
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${PYTHON_INCLUDE_PATH}
    DEPENDS_ON  ${cli_IOperator}
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
                ${vtktiff_LIBRARIES}
                ${vtkjpeg_LIBRARIES}
                ${vtkpng_LIBRARIES}
                Threads::Threads
                ${ZLIB_LIBRARY}
    FOLDER      examples
    SKIP_INSTALL)

if(NOT WIN32)
    set(usedataserver_extra_libs dl)
    if(NOT APPLE)
        list(APPEND usedataserver_extra_libs ${X11_LIBRARIES})
    endif()
    visit_patch_target(
        NAME       usedataserver
        DEPENDS_ON ${usedataserver_extra_libs})
endif()

