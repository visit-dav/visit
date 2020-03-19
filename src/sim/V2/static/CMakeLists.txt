# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

IF(VISIT_STATIC)
    # Create a single static simV2 archive containing all relevant visit libraries

    # create a library that depends on a dummy source file
    SET(SIMV2_STATIC_SRC ${CMAKE_CURRENT_BINARY_DIR}/simv2_static_dummy.c)
    SET(SIMV2_STATIC_SER ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsimV2_static_ser.a)
    SET(SIMV2_STATIC_PAR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsimV2_static_par.a)
    ADD_LIBRARY(simV2_static_ser STATIC ${SIMV2_STATIC_SRC})
    SET(SIMV2_TARGETS simV2_static_ser)
    # Add dependencies to force the combined library generation to take place 
    # once its dependencies are in place.
    ADD_DEPENDENCIES(simV2_static_ser
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        ${engine_ser_exe_IOperator}
        ${engine_ser_exe_EOperator_ser}
        ${engine_ser_exe_IPlot}
        ${engine_ser_exe_EPlot_ser}
        viewercore_ser
        viewerrpc
    )

    # create the dummy source file
    ADD_CUSTOM_COMMAND(OUTPUT ${SIMV2_STATIC_SRC} 
                       COMMAND ${CMAKE_COMMAND} -E touch ${SIMV2_STATIC_SRC}
    )

    CONFIGURE_FILE(${VISIT_SOURCE_DIR}/CMake/CreateStaticSimV2.cmake.in
                   ${CMAKE_CURRENT_BINARY_DIR}/CreateStaticSimV2.cmake
                   @ONLY
    )
    ADD_CUSTOM_COMMAND(TARGET simV2_static_ser POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -P
                       ${CMAKE_CURRENT_BINARY_DIR}/CreateStaticSimV2.cmake
                       COMMENT "CREATING SIMV2 STATIC ARCHIVE"
    )

    IF(VISIT_PARALLEL)
        ADD_LIBRARY(simV2_static_par STATIC ${SIMV2_STATIC_SRC})
        # Add dependencies to force the combined library generation to take place 
        # once its dependencies are in place.
        ADD_DEPENDENCIES(simV2_static_par
            ${engine_par_exe_IDatabase}
            ${engine_par_exe_EDatabase_par}
            ${engine_par_exe_IOperator}
            ${engine_par_exe_EOperator_par}
            ${engine_par_exe_IPlot}
            ${engine_par_exe_EPlot_par}
            viewercore_par
            viewerrpc
        )
        SET(SIMV2_TARGETS ${SIMV2_TARGETS} simV2_static_par)
    ENDIF(VISIT_PARALLEL)

    VISIT_INSTALL_TARGETS(${SIMV2_TARGETS})
ENDIF(VISIT_STATIC)

