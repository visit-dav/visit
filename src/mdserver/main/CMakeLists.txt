# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Thu Feb 25 10:59 MST 2010
#   Exclude testmds on windows.
#
#   Kathleen Bonnell, Tue Mar 23 11:51:02 MST 2010
#   Put install of mdserver outside of NOT WIN32 loop so it will get installed
#   on windows.
#
#   Mark C. Miller, Thu Jul 29 13:09:44 PDT 2010
#   Removed LibPipelineStubs as mdserver is now linked with libvisit_vtk
#
#   Mark C. Miller, Tue Jan 11 11:14:08 PST 2011
#   Removed testmds. It is obsoleted by other testing.
#
#   Brad Whitlock, Fri May 18 16:09:34 PST 2012
#   Use different resource file.
#
#   Brad Whitlock, Tue Nov 11 20:11:26 PST 2014
#   Force inclusion of SimV2 plugin sources here for static builds. We don't
#   do it the normal way because we don't want that plugin in the engine.
#
#   David Camp, Thu Aug 27 09:25:33 PDT 2015
#   Added the SaveSessionRPCExecutor.C and RestoreSessionRPCExecutor.C files.
#   They are used for the remove save and load of session files.
# 
#   Justin Privitera, Wed Aug 24 11:08:51 PDT 2022
#   Added logic to make mdserver aware of avtBP lib.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   Replaced vtktiff with vtktiff_LIBRARIES, vtkpng with vtkpng_LIBRARIES,
#   and vtkjpeg with vtkjpeg_LIBRARIES.
#
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#  Kathleen Biagas, Wed Jul 17, 2024
#  Utilize visit_add_executable.
#
#****************************************************************************

set(mdserver_SOURCES
    ChangeDirectoryRPCExecutor.C
    CloseDatabaseRPCExecutor.C
    ConnectRPCExecutor.C
    ExpandPathRPCExecutor.C
    GetDBPluginInfoRPCExecutor.C
    GetDirectoryRPCExecutor.C
    GetFileListRPCExecutor.C
    GetMetaDataRPCExecutor.C
    GetPluginErrorsRPCExecutor.C
    GetSILRPCExecutor.C
    KeepAliveRPCExecutor.C
    LoadPluginsRPCExecutor.C
    main.C
    MDServerApplication.C
    MDServerConnection.C
    QuitRPCExecutor.C
    RestoreSessionRPCExecutor.C
    SaveSessionRPCExecutor.C
    SetMFileOpenOptionsRPCExecutor.C)

set(mdserver_HEADERS
    ChangeDirectoryRPCExecutor.h
    CloseDatabaseRPCExecutor.h
    ConnectRPCExecutor.h
    ExpandPathRPCExecutor.h
    GetDBPluginInfoRPCExecutor.h
    GetDirectoryRPCExecutor.h
    GetFileListRPCExecutor.h
    GetMetaDataRPCExecutor.h
    GetPluginErrorsRPCExecutor.h
    GetSILRPCExecutor.h
    KeepAliveRPCExecutor.h
    LoadPluginsRPCExecutor.h
    MDServerApplication.h
    MDServerConnection.h
    mdsmain_exports.h
    QuitRPCExecutor.h
    RestoreSessionRPCExecutor.h
    SaveSessionRPCExecutor.h
    SetMFileOpenOptionsRPCExecutor..)

# If the mdserver is static then we need some static symbol lookup functions
if(VISIT_STATIC)
    set(SIMV2_PLUGIN_DIR ${VISIT_SOURCE_DIR}/databases/SimV2)
    set(SIMV2_PLUGIN_SOURCES
        ${SIMV2_PLUGIN_DIR}/SimV2CommonPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2MDServerPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2PluginInfo.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2FileFormat.C)
    set(mdserver_STATIC_SOURCES ${SIMV2_PLUGIN_SOURCES} MDServerStaticSymbolLocator.C)
endif()

set(mdserver_DEFINES MDSERVER)

if(NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    list(APPEND mdserver_DEFINES COMM_EXPORTS)
endif()

set(mdserver_DEPENDS
    mdserverrpc
    visitcommon
    avtdbatts
    avtdatabase_ser
    lightweight_visit_vtk
    visit_verdict
    VTK::CommonCore
    VTK::IOCore
    VTK::tiff
    VTK::png
    VTK::jpeg
    ${vtklibs}
    ${vtktiff_LIBRARIES}
    ${vtkpng_LIBRARIES}
    ${vtkjpeg_LIBRARIES}
    zlib
    Threads::Threads
    ${DL_LIB})

if(TARGET avtblueprint)
    list(APPEND mdserver_DEPENDS avtblueprint)
endif()

if(VISIT_STATIC)
    list(APPEND mdserver_DEPENDS
        ${mdserver_MDatabase}
        ${mdserver_IDatabase})
endif()

visit_add_executable(
    NAME       mdserver
    SOURCES    ${mdserver_SOURCES}
               ${mdserver_STATIC_SOUCES}
               ${VISIT_MDSERVER_RESOURCE_FILE}
    HEADERS    ${mdserver_HEADERS}
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/mdserver/main>
    DEFINES    ${mdserver_DEFINES}
    DEPENDS_ON ${mdserver_DEPENDS})

if(VISIT_STATIC)
   target_link_directoris(mdserver PRIVATE ${ALL_THIRDPARTY_IO_LIBRARY_DIR})
endif()

if(WIN32 AND MSVC)
    set_target_properties(mdserver PROPERTIES LINK_FLAGS "/STACK:4000000")
endif()

