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
#
#****************************************************************************/

# This flag controls whether we try to build the simv2 runtime with 
# viewercore support.
SET(SIMV2_VIEWER_INTEGRATION ON)

IF(SIMV2_VIEWER_INTEGRATION)
    SET(LIBSIM_RUNTIME_VIEWER_SOURCES SimEngineManager.C SimFileServer.C)
    SET(LIBSIM_RUNTIME_VIEWER_LIBS_SER viewercore_ser)
    SET(LIBSIM_RUNTIME_VIEWER_LIBS_PAR viewercore_par)
    SET(VISIT_VIEWER_DIRS
        ${VISIT_SOURCE_DIR}/viewer/core
        ${VISIT_SOURCE_DIR}/viewer/rpc
    )
    ADD_DEFINITIONS(-DSIMV2_VIEWER_INTEGRATION)
ENDIF(SIMV2_VIEWER_INTEGRATION)

SET(LIBSIM_RUNTIME_SOURCES 
${LIBSIM_RUNTIME_VIEWER_SOURCES}
SimEngine.C
SimOperatorPluginManager.C
SimPlotPluginManager.C
VisItControlInterfaceRuntime.C 
VisItDataInterfaceRuntime.C 
VisItDataInterfaceRuntimeP.C 
simv2_CommandMetaData.C
simv2_CurveData.C
simv2_CurveMetaData.C
simv2_CurvilinearMesh.C
simv2_CSGMesh.C
simv2_DomainBoundaries.C
simv2_DomainList.C 
simv2_DomainNesting.C
simv2_ExpressionMetaData.C
simv2_MaterialData.C
simv2_MaterialMetaData.C
simv2_MeshMetaData.C
simv2_NameList.C
simv2_PointMesh.C
simv2_RectilinearMesh.C
simv2_SimulationMetaData.C
simv2_SpeciesData.C
simv2_SpeciesMetaData.C
simv2_UnstructuredMesh.C
simv2_VariableData.C
simv2_VariableMetaData.C
)

# If VisIt is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(SIMV2_PLUGIN_DIR ${VISIT_SOURCE_DIR}/databases/SimV2)

    SET(SIMV2_PLUGIN_SOURCES
        ${SIMV2_PLUGIN_DIR}/SimV2CommonPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2EnginePluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2GetMesh.C
        ${SIMV2_PLUGIN_DIR}/SimV2PluginInfo.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2FileFormat.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2Writer.C
     )

    SET(LIBSIM_STATIC_SOURCES ${SIMV2_PLUGIN_SOURCES} SimStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VTK_INCLUDE_DIRS}
${VISIT_VIEWER_DIRS}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/DataBinning
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Formats
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
${VISIT_SOURCE_DIR}/avt/Expressions/General
${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
${VISIT_SOURCE_DIR}/avt/Expressions/Management
${VISIT_SOURCE_DIR}/avt/Expressions/Math
${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/Queries/Abstract
${VISIT_SOURCE_DIR}/avt/Queries/Misc
${VISIT_SOURCE_DIR}/avt/Queries/Pick
${VISIT_SOURCE_DIR}/avt/Queries/Queries
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/main
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/sim/V2/lib
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${SIMV2_PLUGIN_DIR}
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
${MESA_LIBRARY_DIR}
${GLEW_LIBRARY_DIR}
${VTK_LIBRARY_DIRS}
${ICET_LIBRARY_DIR}
)

ADD_LIBRARY(simV2runtime_ser ${LIBSIM_RUNTIME_SOURCES} ${LIBSIM_STATIC_SOURCES})
TARGET_LINK_LIBRARIES(simV2runtime_ser engine_ser ${LIBSIM_RUNTIME_VIEWER_LIBS_SER})

IF(NOT APPLE)
    SET_TARGET_PROPERTIES(simV2runtime_ser PROPERTIES INSTALL_RPATH "$ORIGIN")
ENDIF(NOT APPLE)

VISIT_INSTALL_TARGETS(simV2runtime_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(simV2runtime_par ${LIBSIM_RUNTIME_SOURCES} ${LIBSIM_STATIC_SOURCES})
    TARGET_LINK_LIBRARIES(simV2runtime_par engine_par ${LIBSIM_RUNTIME_VIEWER_LIBS_PAR})
    IF(NOT APPLE)
        SET_TARGET_PROPERTIES(simV2runtime_par PROPERTIES INSTALL_RPATH "$ORIGIN")
    ENDIF(NOT APPLE)

    VISIT_INSTALL_TARGETS(simV2runtime_par)
ENDIF(VISIT_PARALLEL)

