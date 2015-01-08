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

