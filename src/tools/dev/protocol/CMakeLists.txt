# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
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
    set(protocol_STATIC_SOURCES ${VISIT_SOURCE_DIR}/engine/main/EngineStaticSymbolLocator.C)
endif()

set(protocol_DEPENDS
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
    ${vtktiff_LIBRARIES}
    ${vtkjpeg_LIBRARIES}
    ${vtkpng_LIBRARIES}
    ${ALL_THIRDPARTY_IO_LIB}
    ${MESA_LIB}
    ${OPENGL_LIBRARIES}
    Threads::Threads)

if(NOT WIN32)
    list(APPEND protocol_DEPENDS dl)
    if(NOT APPLE)
        list(APPEND protocol_DEPENDS ${X11_LIBRARIES})
    endif()
endif()

visit_add_executable(
    NAME        visitprotocol
    SOURCES     dump_protocol.C
                ${STATIC_SOURCES}
    DEPENDS_ON  ${protocol_DEPENDS}
    FOLDER      tools/visitprotocol)

