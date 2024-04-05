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
#****************************************************************************

SET(MDSERVER_SOURCES
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
MDServerApplication.C
MDServerConnection.C
QuitRPCExecutor.C
SetMFileOpenOptionsRPCExecutor.C
SaveSessionRPCExecutor.C
RestoreSessionRPCExecutor.C
main.C
)

# If the mdserver is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(SIMV2_PLUGIN_DIR ${VISIT_SOURCE_DIR}/databases/SimV2)
    SET(SIMV2_PLUGIN_SOURCES
        ${SIMV2_PLUGIN_DIR}/SimV2CommonPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2MDServerPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2PluginInfo.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2FileFormat.C
     )
    SET(MDSERVER_STATIC_SOURCES ${SIMV2_PLUGIN_SOURCES} MDServerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

ADD_DEFINITIONS(-DMDSERVER)
IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    ADD_DEFINITIONS(-DCOMM_EXPORTS)
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VTK_INCLUDE_DIRS}
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Formats
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${SIMV2_PLUGIN_DIR}
)

set(vtklibs VTK::CommonCore VTK::IOCore VTK::tiff VTK::png VTK::jpeg)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
${AVT_REQUIRED_LIB_DIR}
)

ADD_EXECUTABLE(mdserver ${VISIT_APPLICATION_STYLE} ${MDSERVER_SOURCES} ${MDSERVER_STATIC_SOURCES} ${VISIT_MDSERVER_RESOURCE_FILE})
TARGET_LINK_LIBRARIES(mdserver
    ${VISIT_EXE_LINKER_FLAGS}
    ${mdserver_MDatabase}
    ${mdserver_IDatabase}
    mdserverrpc
    visitcommon
    avtdbatts
    avtdatabase_ser
    lightweight_visit_vtk
    visit_verdict
    ${vtklibs}
    ${vtktiff_LIBRARIES}
    ${vtkpng_LIBRARIES}
    ${vtkjpeg_LIBRARIES}
    ${ZLIB_LIBRARY}
    Threads::Threads
    ${DL_LIB}
)

if(CONDUIT_FOUND AND MFEM_FOUND)
    LINK_DIRECTORIES(
        ${MFEM_LIBRARY_DIR}
        ${CONDUIT_LIBRARY_DIR}
        )
    INCLUDE_DIRECTORIES(
        ${MFEM_INCLUDE_DIR}
        ${CONDUIT_INCLUDE_DIR}
        ${VISIT_SOURCE_DIR}/avt/Blueprint
        )
    TARGET_LINK_LIBRARIES(mdserver 
        ${MFEM_LIB} 
        avtblueprint
        )
endif()

if(WIN32 AND MSVC)
    set_target_properties(mdserver PROPERTIES LINK_FLAGS "/STACK:4000000")
endif()

# If we're building statically then the mdserver can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(mdserver
        ${mdserver_MDatabase}
        ${mdserver_IDatabase}
    )
ENDIF(VISIT_STATIC)

VISIT_INSTALL_TARGETS(mdserver)
