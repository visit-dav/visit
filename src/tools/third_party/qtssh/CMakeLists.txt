# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed May 5 12:07:54 PST 2010
#   Disable optimization.
# 
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Brad Whitlock, Mon May 21 14:27:34 PST 2012
#   Turn qtssh back into a console application.
#
#   Brad Whitlock, Tue Jun 12 16:14:32 PST 2012
#   Redo qtssh using new sources and refactored viewer windows.
#
#   Eric Brugger, Wed Apr 17 12:02:02 PDT 2013
#   Create a linux version of qtssh.
#
#   Brad Whitlock, Fri Jun  7 22:03:52 PDT 2013
#   Static build fixes.
#
#   Kevin Griffin, Tue Jan 17 10:29:26 PST 2017
#   Upgraded source code to version 0.67
#
#   Kathleen Biagas, Tue Jan 24, 2023
#   Upgraded source code to version 0.78.
#   Utilize putty's native CMake build system with modifications
#   to only build our qtssh version of plink and necessary libs.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

project(putty)

include(cmake/setup.cmake)
add_compile_definitions(HAVE_CMAKE_H)

add_library(utils STATIC)
target_include_directories(utils PRIVATE
    terminal
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh utils)
add_subdirectory(utils)
add_subdirectory(stubs)

add_library(logging OBJECT logging.c)
target_include_directories(logging PRIVATE
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh logging)

add_library(eventloop STATIC callback.c timing.c)
target_include_directories(eventloop PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh eventloop)

add_library(console STATIC clicons.c console.c)
target_include_directories(console PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh console)

add_library(settings STATIC cmdline.c settings.c)
target_include_directories(settings PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh settings)

add_library(crypto STATIC proxy/cproxy.c proxy/sshproxy.c)
target_include_directories(crypto PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
add_subdirectory(crypto)
VISIT_TOOLS_ADD_FOLDER(qtssh crypto)

add_library(network STATIC
  errsock.c logging.c x11disp.c
  proxy/proxy.c
  proxy/http.c
  proxy/socks4.c
  proxy/socks5.c
  proxy/telnet.c
  proxy/local.c
  proxy/interactor.c)
target_include_directories(network PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh network)

add_library(noterminal STATIC stubs/no-term.c ldisc.c)
target_include_directories(noterminal PRIVATE
     terminal
     ${putty_SOURCE_DIR}
     ${putty_SOURCE_DIR}/${platform}
     ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh noterminal)

add_library(all-backends OBJECT pinger.c)
target_include_directories(all-backends PRIVATE
     ${putty_SOURCE_DIR}/${platform}
     ${GENERATED_SOURCES_DIR})
VISIT_TOOLS_ADD_FOLDER(qtssh all-backends)

add_subdirectory(ssh)

add_library(otherbackends STATIC
    $<TARGET_OBJECTS:all-backends>
    $<TARGET_OBJECTS:logging>)
target_include_directories(otherbackends PRIVATE
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})
add_subdirectory(otherbackends)
VISIT_TOOLS_ADD_FOLDER(qtssh otherbackends)

add_executable(qtssh
  ${platform}/qtsshmain.c qtssh.cpp
  ${VISIT_SOURCE_DIR}/viewer/main/ui/VisItChangeUsernameWindow.C
  ${VISIT_SOURCE_DIR}/viewer/main/ui/VisItPasswordWindow.C)
set_target_properties(qtssh PROPERTIES AUTOMOC ON)
be_list(qtssh QTSSH SSH SERIAL OTHERBACKENDS)


if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(qtssh)
endif()

target_link_libraries(qtssh
  eventloop noterminal console sshclient otherbackends settings network crypto
  utils ${platform_libraries}
  ${QT_QTGUI_LIBRARY} ${QT_QTWIDGETS_LIBRARY} ${QT_QTCORE_LIBRARY})

target_include_directories(qtssh PRIVATE
    ${VISIT_SOURCE_DIR}/viewer/main/ui
    ${putty_SOURCE_DIR}
    ${putty_SOURCE_DIR}/${platform}
    ${GENERATED_SOURCES_DIR})

if(VISIT_STATIC)
    MAC_NIB_INSTALL(qtssh)
    target_link_libraries(qtssh ${QT_QTCORE_LIB_DEPENDENCIES}
                                ${QT_QTGUI_LIB_DEPENDENCIES})
endif()

VISIT_INSTALL_TARGETS(qtssh)
VISIT_TOOLS_ADD_FOLDER(qtssh qtssh)

add_subdirectory(windows)

configure_file(cmake/cmake.h.in ${GENERATED_SOURCES_DIR}/cmake.h)

