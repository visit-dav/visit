#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
# OUT OF THE  USE OF THIS SOFTWARE, EVEN if ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/

#
# NOTE: This library is not allowed to have *any* Qt dependencies!
#

set(VIEWERCORE_SOURCES
ActionGroupDescription.C
EngineKey.C
LineoutListItem.C
NoEngineException.C
ViewStack.C
ViewerBase.C
ViewerActionLogic.C
ViewerActionManager.C
ViewerChangeUsername.C
ViewerConfigManager.C
ViewerConnectionPrinter.C
ViewerConnectionProgress.C
ViewerDatabaseCorrelationMethods.C
ViewerEngineManagerInterface.C
ViewerFactory.C
ViewerFileServerInterface.C
ViewerHostProfileSelector.C
ViewerHostProfileSelectorNoWin.C
ViewerInternalCommands.C
ViewerMessaging.C
ViewerOperator.C
ViewerOperatorFactory.C
ViewerPlot.C
ViewerPlotFactory.C
ViewerPlotList.C
ViewerProperties.C
ViewerQuery.C
ViewerQueryManager.C
ViewerRemoteProcessChooser.C
ViewerStateManager.C
ViewerText.C
ViewerVariableMethods.C
ViewerWindow.C
ViewerWindowManager.C
ViewerWindowManagerAttributes.C
actions/AnimationActions.C
actions/CopyActions.C
actions/DatabaseActions.C
actions/DatabaseStateActions.C
actions/EngineActions.C
actions/IOActions.C
actions/LockActions.C
actions/MDServerActions.C
actions/PlotAndOperatorActions.C
actions/QueryActions.C
actions/SelectionActions.C
actions/StateActions.C
actions/ViewActions.C
actions/WindowActions.C
)

if(NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    add_definitions(-DCOMM_EXPORTS)
endif()

include_directories(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/actions
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Interactors
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/osmesa
${VTK_INCLUDE_DIRS}
)

link_directories(${LIBRARY_OUTPUT_DIRECTORY})


#******************** The viewercore_ser library ******************************
add_library(viewercore_ser ${VIEWERCORE_SOURCES})
set_target_properties(viewercore_ser PROPERTIES HAS_CXX ON)

target_link_libraries(viewercore_ser
viewerrpc
visitcommon
avtdbatts
avtview
avtmath
avtpipeline_ser
avtplotter_ser
avtfilters_ser
avtviswindow_ser
avtwriter_ser
${CMAKE_THREAD_LIBS}
)

if(HAVE_OSMESA OR HAVE_EGL)
    target_link_libraries(viewercore_ser visit_vtk_offscreen)
endif()

#******************** The viewercore_par library ******************************
# We build a _par version of viewercore because we can't allow it to have any _ser
# library dependencies when we use it in the simV2 parallel runtime.
if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(viewercore_par ${VIEWERCORE_SOURCES})
    set_target_properties(viewercore_par PROPERTIES HAS_CXX ON)

    target_link_libraries(viewercore_par
    viewerrpc
    visitcommon
    avtdbatts
    avtview
    avtmath
    avtpipeline_par
    avtplotter_par
    avtfilters_par
    avtviswindow_par
    avtwriter_par
    ${CMAKE_THREAD_LIBS}
    )

    if(HAVE_OSMESA OR HAVE_EGL)
        target_link_libraries(viewercore_par visit_vtk_offscreen)
    endif()

    set(VIEWERCORE_PAR viewercore_par)
endif(VISIT_PARALLEL)

VISIT_INSTALL_TARGETS(viewercore_ser ${VIEWERCORE_PAR})
