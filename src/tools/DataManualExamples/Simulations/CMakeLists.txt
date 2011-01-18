#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
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
#   Brad Whitlock, Wed May 26 09:54:12 PDT 2010
#   I added Jean Favre's life.c which simulates the "game of life".
#
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Brad Whitlock, Tue Jan 18 00:32:18 PST 2011
#   Force C++ linker for serial SimV2 examples. Otherwise I've noticed we
#   are exposed to exceptions on newer Linux versions.
#
#****************************************************************************/

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
)

# This target depends on all the simulations so we make this target if we want
# to build any of the simulations.
IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    ADD_CUSTOM_TARGET(all_simulations)
ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)

# The simulation will be built here but not added to the "all" target. This means
# it won't be built unless we explicitly build it or make the "all_simulations"
# target.
FUNCTION(SET_SIMULATION_PROPERTIES target)
    SET_TARGET_PROPERTIES(sim_${target} PROPERTIES
        OUTPUT_NAME ${target}
        RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/tools/DataManualExamples/Simulations
        FOLDER tools/DataManualExamples/Simulations
    )
    IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
        SET_TARGET_PROPERTIES(sim_${target} PROPERTIES
            EXCLUDE_FROM_ALL TRUE
        )
        ADD_DEPENDENCIES(all_simulations sim_${target})
    ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)
ENDFUNCTION(SET_SIMULATION_PROPERTIES)

# Add the --export-dynamic flag for Linux gcc so SimV1 simulations will expose 
# their callback functions.
FUNCTION(ADD_EXPORT_DYNAMIC_LINK_FLAGS target)
    SET_TARGET_PROPERTIES(sim_${target} PROPERTIES
        ENABLE_EXPORTS ON
    )
ENDFUNCTION(ADD_EXPORT_DYNAMIC_LINK_FLAGS)

# We need more libraries on Linux and Windows
IF(WIN32)
    SET(EXTRA_SIM_LIBS ws2_32)
ELSE(WIN32)
    IF(NOT APPLE)
        SET(EXTRA_SIM_LIBS dl m)
    ENDIF(NOT APPLE)
ENDIF(WIN32)

##
## SimV1 functions
##

FUNCTION(ADD_SIMV1_SIMULATION target)
    ADD_EXECUTABLE(sim_${target} ${ARGN})
    SET_SIMULATION_PROPERTIES(${target})
    TARGET_LINK_LIBRARIES(sim_${target} sim ${EXTRA_SIM_LIBS})
    ADD_TARGET_COMPILE_FLAGS(sim_${target}
        -I${VISIT_SOURCE_DIR}/sim/V1/lib
        -I${VISIT_SOURCE_DIR}/databases/SimV1
    )
    ADD_EXPORT_DYNAMIC_LINK_FLAGS(${target})
ENDFUNCTION(ADD_SIMV1_SIMULATION)

FUNCTION(ADD_FORTRAN_SIMV1_SIMULATION target)
    IF(VISIT_FORTRAN)
        ADD_EXECUTABLE(sim_${target} ${ARGN} simulationexample.f)
        SET_SIMULATION_PROPERTIES(${target})
        TARGET_LINK_LIBRARIES(sim_${target} simf sim ${EXTRA_SIM_LIBS})
        ADD_TARGET_COMPILE_FLAGS(sim_${target}
            -I${VISIT_SOURCE_DIR}/sim/V1/lib
            -I${VISIT_SOURCE_DIR}/databases/SimV1
        )
        ADD_EXPORT_DYNAMIC_LINK_FLAGS(${target})
    ENDIF(VISIT_FORTRAN)
ENDFUNCTION(ADD_FORTRAN_SIMV1_SIMULATION)

FUNCTION(ADD_PARALLEL_SIMV1_SIMULATION target)
    ADD_PARALLEL_EXECUTABLE(sim_${target} ${ARGN})
    SET_SIMULATION_PROPERTIES(${target})
    SET_TARGET_PROPERTIES(sim_${target} PROPERTIES HAS_CXX ON)
    TARGET_LINK_LIBRARIES(sim_${target} sim ${EXTRA_SIM_LIBS})
    ADD_TARGET_COMPILE_FLAGS(sim_${target}
        -I${VISIT_SOURCE_DIR}/sim/V1/lib
        -I${VISIT_SOURCE_DIR}/databases/SimV1
    )
    ADD_EXPORT_DYNAMIC_LINK_FLAGS(${target})
ENDFUNCTION(ADD_PARALLEL_SIMV1_SIMULATION)

FUNCTION(ADD_PARALLEL_FORTRAN_SIMV1_SIMULATION target)
    IF(VISIT_FORTRAN)
        ADD_PARALLEL_FORTRAN_EXECUTABLE(sim_${target} ${ARGN} simulationexample.f)
        SET_SIMULATION_PROPERTIES(${target})
        TARGET_LINK_LIBRARIES(sim_${target} simf sim ${EXTRA_SIM_LIBS})
        ADD_TARGET_COMPILE_FLAGS(sim_${target}
            -I${VISIT_SOURCE_DIR}/sim/V1/lib
            -I${VISIT_SOURCE_DIR}/databases/SimV1
        )
        ADD_EXPORT_DYNAMIC_LINK_FLAGS(${target})
    ENDIF(VISIT_FORTRAN)
ENDFUNCTION(ADD_PARALLEL_FORTRAN_SIMV1_SIMULATION)

##
## SimV2 functions
##

FUNCTION(ADD_SIMV2_SIMULATION target)
    ADD_EXECUTABLE(sim_${target} ${ARGN})
    SET_SIMULATION_PROPERTIES(${target})
    SET_TARGET_PROPERTIES(sim_${target} PROPERTIES HAS_CXX ON)
    TARGET_LINK_LIBRARIES(sim_${target} simV2 ${EXTRA_SIM_LIBS})
    ADD_TARGET_COMPILE_FLAGS(sim_${target}
        -I${VISIT_SOURCE_DIR}/sim/V2/lib
    )
ENDFUNCTION(ADD_SIMV2_SIMULATION)

FUNCTION(ADD_FORTRAN_SIMV2_SIMULATION target)
    IF(VISIT_FORTRAN)
        ADD_EXECUTABLE(sim_${target} ${ARGN} simulationexamplev2.f)
        SET_SIMULATION_PROPERTIES(${target})
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2 ${EXTRA_SIM_LIBS})
        ADD_TARGET_COMPILE_FLAGS(sim_${target}
            -I${VISIT_SOURCE_DIR}/sim/V2/lib
        )
    ENDIF(VISIT_FORTRAN)
ENDFUNCTION(ADD_FORTRAN_SIMV2_SIMULATION)

FUNCTION(ADD_PARALLEL_SIMV2_SIMULATION target)
    ADD_PARALLEL_EXECUTABLE(sim_${target} ${ARGN})
    SET_SIMULATION_PROPERTIES(${target})
    SET_TARGET_PROPERTIES(sim_${target} PROPERTIES HAS_CXX ON)
    TARGET_LINK_LIBRARIES(sim_${target} simV2 ${EXTRA_SIM_LIBS})
    ADD_TARGET_COMPILE_FLAGS(sim_${target}
        -I${VISIT_SOURCE_DIR}/sim/V2/lib
    )
ENDFUNCTION(ADD_PARALLEL_SIMV2_SIMULATION)

FUNCTION(ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION target)
    IF(VISIT_FORTRAN)
        ADD_PARALLEL_FORTRAN_EXECUTABLE(sim_${target} ${ARGN} simulationexamplev2.f)
        SET_SIMULATION_PROPERTIES(${target})
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2 ${EXTRA_SIM_LIBS})
        ADD_TARGET_COMPILE_FLAGS(sim_${target}
            -I${VISIT_SOURCE_DIR}/sim/V2/lib
        )
    ENDIF(VISIT_FORTRAN)
ENDFUNCTION(ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION)

##
## Simulation targets
##

ADD_SIMV2_SIMULATION(amr amr.c)
ADD_FORTRAN_SIMV2_SIMULATION(famr famr.f)

ADD_SIMV2_SIMULATION(csg csg.c)

ADD_SIMV2_SIMULATION(curve curve.c)
ADD_FORTRAN_SIMV2_SIMULATION(fcurve fcurve.f)

ADD_SIMV2_SIMULATION(interleave interleave.c)

ADD_SIMV2_SIMULATION(life life.c)
ADD_PARALLEL_SIMV2_SIMULATION(life_par life.c)

ADD_SIMV2_SIMULATION(mandelbrot mandelbrot.C patch.C)

ADD_SIMV2_SIMULATION(material material.c)
ADD_FORTRAN_SIMV2_SIMULATION(fmaterial fmaterial.f)

ADD_SIMV2_SIMULATION(mesh mesh.c)
ADD_FORTRAN_SIMV2_SIMULATION(fmesh fmesh.f)

ADD_SIMV2_SIMULATION(point point.c)
ADD_FORTRAN_SIMV2_SIMULATION(fpoint fpoint.f)

ADD_SIMV2_SIMULATION(polyhedral polyhedral.c)

ADD_SIMV2_SIMULATION(scalar scalar.c)
ADD_FORTRAN_SIMV2_SIMULATION(fscalar fscalar.f)
ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION(fscalarp fscalarp.f)

ADD_SIMV2_SIMULATION(sim1 sim1.c)

ADD_SIMV2_SIMULATION(sim2 sim2.c)
ADD_PARALLEL_SIMV2_SIMULATION(sim2p sim2p.c)

ADD_SIMV2_SIMULATION(sim3 sim3.c)
ADD_PARALLEL_SIMV2_SIMULATION(sim3p sim3p.c)

ADD_SIMV2_SIMULATION(sim4 sim4.c)
ADD_PARALLEL_SIMV2_SIMULATION(sim4p sim4p.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim4 fsim4.f)

ADD_SIMV2_SIMULATION(sim5 sim5.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim5 fsim5.f)

ADD_SIMV2_SIMULATION(sim6 sim6.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim6 fsim6.f)

ADD_SIMV2_SIMULATION(sim7 sim7.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim7 fsim7.f)

ADD_SIMV2_SIMULATION(sim8 sim8.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim8 fsim8.f)

ADD_SIMV2_SIMULATION(sim9 sim9.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim9 fsim9.f)

ADD_SIMV2_SIMULATION(sim10 sim10.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim10 fsim10.f)

ADD_SIMV2_SIMULATION(sim11 sim11.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim11 fsim11.f)

ADD_SIMV2_SIMULATION(sim12 sim12.c)
ADD_FORTRAN_SIMV2_SIMULATION(fsim12 fsim12.f)

ADD_SIMV2_SIMULATION(species species.c)

ADD_SIMV2_SIMULATION(unstructured unstructured.c)
ADD_FORTRAN_SIMV2_SIMULATION(funstructured funstructured.f)

ADD_SIMV2_SIMULATION(updateplots updateplots.c)
ADD_PARALLEL_SIMV2_SIMULATION(updateplots_par updateplots.c)
ADD_FORTRAN_SIMV2_SIMULATION(fupdateplots fupdateplots.f)

#usequeries.c

ADD_SIMV2_SIMULATION(var var.c)

ADD_SIMV2_SIMULATION(writeback writeback.c)
ADD_PARALLEL_SIMV2_SIMULATION(writeback_par writeback.c)
