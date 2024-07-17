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
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#   Kathleen Biagas, Wed Jul 17 2024
#   Utilize visit_add_executable.
#
#****************************************************************************

visit_add_executable(
    NAME       vcl
    SOURCES    LauncherApplication.C
               main.C
               ${VISIT_VCL_RESOURCE_FILE}
    HEADERS    LauncherApplication.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    DEPENDS_ON visitcommon
               vclrpc
               Threads::Threads
               ${DL_LIB})

if(NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    target_compile_definitions(vcl PRIVATE COMM_EXPORTS)

    visit_add_executable(
        NAME testvcl
        SOURCES testvcl.C
        DEPENDS_ON visitcommon
                   vclproxy
                   vclrpc
                   Threads::Threads
                   ${DL_LIB})

    set_target_properties(testvcl
        PROPERTIES EXCLUDE_FROM_ALL true)

    if(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE)
        visit_add_executable(
            NAME       visit_socket_relay
            SOURCES    SocketRelayMain.C
            DEPENDS_ON visitcommon
                       Threads::Threads
                       ${DL_LIB})
    endif()
endif()

