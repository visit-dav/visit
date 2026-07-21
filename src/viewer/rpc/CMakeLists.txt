# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#
#    Kathleen Biagas, Thu Oct 3, 2024
#    Utilize visit_add_library.
#
#****************************************************************************

visit_add_library(
    NAME        viewerrpc
    SOURCES     PostponedAction.C
                ViewerMethods.C
                ViewerRPC.C
                ViewerState.C
    HEADERS     PostponedAction.h
                ViewerMethods.h
                ViewerRPC.h
                ViewerState.h
                viewerrpc_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/rpc>
    DEPENDS_ON  visitcommon avtdbatts)


# Add XML Code Gen Targets

set(VIEWER_RPC_GEN_OBJECTS
    ViewerRPC
    PostponedAction)

# Viewer Core Objects
foreach(obj_name ${VIEWER_RPC_GEN_OBJECTS})
    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

