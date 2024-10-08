# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#
#    Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#    Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#    Kathleen Biagas, Tue Sep 5, 2023
#    Remove untested, unused EGL logic.
#
#    Kathleen Biagas, Tue Sep 26, 2023
#    Replace ${CMAKE_THREAD_LIBS} with Threads::Threads.
#
#    Kathleen Biagas, Thu Oct 3, 2024
#    Utilize visit_add_library.
#
#****************************************************************************

#
# NOTE: This library is not allowed to have *any* Qt dependencies!
#

set(VIEWERCORE_SOURCES
    ActionGroupDescription.C
    EngineKey.C
    LineoutListItem.C
    NoEngineException.C
    ViewerActionLogic.C
    ViewerActionManager.C
    ViewerBase.C
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
    ViewStack.C)

set(VIEWERCORE_HEADERS
    ActionGroupDescription.h
    EngineKey.h
    ExternalRenderRequestInfo.h
    LineoutListItem.h
    NoEngineException.h
    ViewerActionLogic.h
    ViewerActionManager.h
    ViewerBase.h
    ViewerChangeUsername.h
    ViewerConfigManager.h
    ViewerConnectionPrinter.h
    ViewerConnectionProgress.h
    ViewerDatabaseCorrelationMethods.h
    ViewerEngineManagerInterface.h
    ViewerFactory.h
    ViewerFileServerInterface.h
    ViewerHostProfileSelector.h
    ViewerHostProfileSelectorNoWin.h
    ViewerInternalCommands.h
    ViewerMessaging.h
    ViewerOperator.h
    ViewerOperatorFactory.h
    ViewerPlot.h
    ViewerPlotFactory.h
    ViewerPlotList.h
    ViewerProperties.h
    ViewerQuery.h
    ViewerQueryManager.h
    ViewerRemoteProcessChooser.h
    ViewerStateManager.h
    ViewerText.h
    ViewerVariableMethods.h
    ViewerWindow.h
    ViewerWindowManager.h
    ViewerWindowManagerAttributes.h
    ViewStack.h
    viewercore_exports.h)

set(VIEWERCORE_DEPENDS
    viewerrpc
    visitcommon
    avtdbatts
    avtview
    avtmath
    Threads::Threads)

if(HAVE_OSMESA)
    list(APPEND VIEWERCORE_DEPENDS visit_vtk_offscreen)
endif()


#******************** The viewercore_ser library ******************************
visit_add_library(
    NAME        viewercore_ser
    SOURCES     ${VIEWERCORE_SOURCES}
    HEADERS     ${VIEWERCORE_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/core>
    DEPENDS_ON  ${VIEWERCORE_DEPENDS}
                avtpipeline_ser
                avtplotter_ser
                avtfilters_ser
                avtviswindow_ser
                avtwriter_ser)


if(NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    target_compile_definitions(viewercore_ser PRIVATE COMM_EXPORTS)
endif()

set_target_properties(viewercore_ser PROPERTIES HAS_CXX ON)



#******************** The viewercore_par library ******************************
# We build a _par version of viewercore because we can't allow it to have any _ser
# library dependencies when we use it in the simV2 parallel runtime.
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME        viewercore_par
        SOURCES     ${VIEWERCORE_SOURCES}
        HEADERS     ${VIEWERCORE_HEADERS}
        INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/core>
        DEPENDS_ON  ${VIEWERCORE_DEPENDS}
                    avtpipeline_par
                    avtplotter_par
                    avtfilters_par
                    avtviswindow_par
                    avtwriter_par)

    set_target_properties(viewercore_par PROPERTIES HAS_CXX ON)

    if(NOT WIN32)
        # This keeps comm's exceptions visible when using -fvisibility=hidden
        target_compile_definitions(viewercore_par PRIVATE COMM_EXPORTS)
    endif()
endif()

#******************** 

add_subdirectory(actions)

#******************** 
# Add XML Code Gen Targets

set(VIEWER_CORE_GEN_OBJECTS
    ViewerWindowManagerAttributes
    ViewerProperties
    ActionGroupDescription)

# Viewer Core Objects
foreach(obj_name ${VIEWER_CORE_GEN_OBJECTS})
    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

