#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
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
#****************************************************************************/

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

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
${MESA_LIBRARY_DIR}
${EAVL_LIBRARY_DIR}
${VTK_LIBRARY_DIRS}
${GLEW_LIBRARY_DIR}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
)

ADD_EXECUTABLE(mdserver ${VISIT_APPLICATION_STYLE} ${MDSERVER_SOURCES} ${MDSERVER_STATIC_SOURCES} ${VISIT_MDSERVER_RESOURCE_FILE})
TARGET_LINK_LIBRARIES(mdserver 
    ${mdserver_MDatabase}
    ${mdserver_IDatabase}
    mdserverrpc 
    visitcommon 
    avtdbatts 
    avtdatabase_ser 
    lightweight_visit_vtk 
    visit_verdict 
    vtkCommonCore
    vtkIOCore 
    vtktiff
    ${VTKZLIB_LIB}
    vtkpng
    vtkjpeg
    ${ZLIB_LIB} 
    ${CMAKE_THREAD_LIBS} 
    ${DL_LIB}
)

# If we're building statically then the mdserver can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(mdserver
        ${mdserver_MDatabase}
        ${mdserver_IDatabase}
    )
ENDIF(VISIT_STATIC)

VISIT_INSTALL_TARGETS(mdserver)
