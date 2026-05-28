# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

if(VISIT_STATIC)
    # Create a single static simV2 archive containing all relevant
    # visit libraries

    # create a library that depends on a dummy source file
    set(SIMV2_STATIC_SRC ${CMAKE_CURRENT_BINARY_DIR}/simv2_static_dummy.c)

    visit_add_library(
        NAME        simV2_static_ser
        SOURCES     ${SIMV2_STATIC_SRC}
        DEPENDS_ON  ${engine_ser_exe_IDatabase}
                    ${engine_ser_exe_EDatabase_ser}
                    ${engine_ser_exe_IOperator}
                    ${engine_ser_exe_EOperator_ser}
                    ${engine_ser_exe_IPlot}
                    ${engine_ser_exe_EPlot_ser}
                    viewercore_ser
                    viewerrpc)
        
    # create the dummy source file
    add_custom_command(OUTPUT ${SIMV2_STATIC_SRC} 
                       COMMAND ${CMAKE_COMMAND} -E touch ${SIMV2_STATIC_SRC})

    configure_file(${VISIT_SOURCE_DIR}/CMake/CreateStaticSimV2.cmake.in
                   ${CMAKE_CURRENT_BINARY_DIR}/CreateStaticSimV2.cmake
                   @ONLY)

    add_custom_command(TARGET simV2_static_ser POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -P
                       ${CMAKE_CURRENT_BINARY_DIR}/CreateStaticSimV2.cmake
                       COMMENT "CREATING SIMV2 STATIC ARCHIVE")

    if(VISIT_PARALLEL)
        visit_add_library(
            NAME        simV2_static_par
            SOURCES     ${SIMV2_STATIC_SRC}
            DEPENDS_ON  ${engine_ser_exe_IDatabase}
                        ${engine_ser_exe_EDatabase_par}
                        ${engine_ser_exe_IOperator}
                        ${engine_ser_exe_EOperator_par}
                        ${engine_ser_exe_IPlot}
                        ${engine_ser_exe_EPlot_par}
                        viewercore_par
                        viewerrpc)
    endif()
endif()

