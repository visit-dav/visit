# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Gunther H. Weber, Thu Jan 14 11:36:21 PST 2010
#   Added conditional target for separate socket bridge executable.
#
#   Kathleen Bonnell, Thu Feb 25 11:00:43 MST 2010 
#   Exclude testvcl on windows.
#
#   Brad Whitlock, Tue Nov 29 11:15:24 PST 2011
#   I moved SocketBridge to common.
#
#   Brad Whitlock, Fri May 18 16:09:34 PST 2012
#   Use different resource file.
#
#   Kathleen Biagas, Thu Oct 27, 2022
#   On Windows, vcl has direct dependence on ws2_32.
#
#****************************************************************************

SET(VCL_SOURCES
LauncherApplication.C
main.C
)

IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    ADD_DEFINITIONS(-DCOMM_EXPORTS)
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/launcher/rpc
${VISIT_SOURCE_DIR}/launcher/proxy
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} )

ADD_EXECUTABLE(vcl ${VISIT_APPLICATION_STYLE} ${VCL_SOURCES} ${VISIT_VCL_RESOURCE_FILE})
TARGET_LINK_LIBRARIES(vcl
    ${VISIT_EXE_LINKER_FLAGS}
    vclrpc
    visitcommon
    ${CMAKE_THREAD_LIBS}
    ${DL_LIB}
)

if(WIN32)
    target_link_libraries(vcl ws2_32)
endif()

IF(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE AND NOT WIN32)
    ADD_EXECUTABLE(visit_socket_relay SocketRelayMain.C)
    TARGET_LINK_LIBRARIES(visit_socket_relay
        ${VISIT_EXE_LINKER_FLAGS}
        visitcommon
        ${CMAKE_THREAD_LIBS}
        ${DL_LIB}
    )
    VISIT_INSTALL_TARGETS(visit_socket_relay)
ENDIF()

IF (NOT WIN32)
    ADD_EXECUTABLE(testvcl EXCLUDE_FROM_ALL testvcl.C)
    TARGET_LINK_LIBRARIES(testvcl vclproxy vclrpc visitcommon ${CMAKE_THREAD_LIBS} ${DL_LIB})
ENDIF (NOT WIN32)
VISIT_INSTALL_TARGETS(vcl)
