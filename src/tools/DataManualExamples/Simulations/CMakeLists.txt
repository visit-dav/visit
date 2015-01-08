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
#   Brad Whitlock, Thu Aug 11 16:05:07 PDT 2011
#   Added ghostcells example.
#
#   Brad Whitlock, Wed Feb  1 11:06:20 PST 2012
#   Added domainbounds example.
#
#   Kathleen Biagas, Thu Jun 21 11:25:43 MST 2012 
#   Use new ADD_TARGET_INCLUDE function to add includes to a target instead
#   of setting them via compile flags.  
#
#   Brad Whitlock, Wed Jul 18 17:03:01 PDT 2012
#   Added materialvar.c
#
#   Brad Whitlock, Mon Aug 13 11:37:49 PDT 2012
#   Added multiblock.c
#
#   Kathleen Biagas, Fri Jun  6 11:16:01 PDT 2014
#   Remove no-longer use SIMV1 functions.  Copy python examples when
#   doing out-of-source builds.
#
#   Kathleen Biagas, Thu Jun 12 13:24:08 PDT 2014
#   Added mechanism for creating a simV2_static archive containing all
#   visit symbols necessary for compiling these examples statically.
#
#   Brad Whitlock, Fri Jun 13 14:03:00 PDT 2014
#   Add globalids example.
#
#   Kathleen Biagas, Wed Jul 16 11:49:03 MST 2014
#   Use 'project' on Windows instead of 'all_simulations' when
#   VISIT_DATA_MANUAL_EXAMPLES not turned on.
#
#   Kathleen Biagas, Wed Sep 3 17:23:47 MST 2014
#   Change target location on Windows, to be relative to in the exe dir,
#   will aid running sim examples via test suite.
#
#****************************************************************************/

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
)

# This target depends on all the simulations so we make this target if we want
# to build any of the simulations.
IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    IF (WIN32)
        PROJECT(VISIT_SIMULATION_EXAMPLES)
    ELSE()
        ADD_CUSTOM_TARGET(all_simulations)
    ENDIF()
ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)

# The simulation will be built here but not added to the "all" target. This means
# it won't be built unless we explicitly build it or make the "all_simulations"
# target.
FUNCTION(SET_SIMULATION_PROPERTIES target)
    IF(NOT WIN32)
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
        ENDIF()
    ELSE()
        SET_TARGET_PROPERTIES(sim_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            RUNTIME_OUTPUT_DIRECTORY_REWITHDEBINFO ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            FOLDER tools/DataManualExamples/Simulations
        )
    ENDIF()
ENDFUNCTION(SET_SIMULATION_PROPERTIES)

# We need more libraries on Linux and Windows
IF(WIN32)
    SET(EXTRA_SIM_LIBS ws2_32)
ELSE(WIN32)
    IF(NOT APPLE)
        SET(EXTRA_SIM_LIBS dl m)
    ENDIF(NOT APPLE)
ENDIF(WIN32)

SET(VTK_LIBS vtkCommonCore vtkImagingCore vtkRenderingCore vtkIOLegacy vtkIOPLY
             vtkInteractionStyle vtkRenderingOpenGL vtkRenderingFreeTypeOpenGL
             vtkFiltersHybrid vtkFiltersModeling vtkGeovisCore)
IF(BLUEGENEQ)
    LINK_DIRECTORIES(${ICET_LIBRARY_DIR})
ENDIF(BLUEGENEQ)

##
## SimV2 functions
##

FUNCTION(ADD_SIMV2_SIMULATION target)
    ADD_EXECUTABLE(sim_${target} ${ARGN})
    SET_SIMULATION_PROPERTIES(${target})
    SET_TARGET_PROPERTIES(sim_${target} PROPERTIES HAS_CXX ON)
    IF(NOT VISIT_STATIC)
        TARGET_LINK_LIBRARIES(sim_${target} simV2 ${EXTRA_SIM_LIBS})
    ELSE()
        TARGET_LINK_LIBRARIES(sim_${target} simV2_static_ser ${VTK_LIBS} ${ZLIB_LIB}
                                                             ${EXTRA_SIM_LIBS})
    ENDIF()
    ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
ENDFUNCTION(ADD_SIMV2_SIMULATION)

FUNCTION(ADD_FORTRAN_SIMV2_SIMULATION target)
    IF(VISIT_FORTRAN)
        ADD_EXECUTABLE(sim_${target} ${ARGN} simulationexamplev2.f)
        SET_SIMULATION_PROPERTIES(${target})
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2 ${EXTRA_SIM_LIBS})
        ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
    ENDIF(VISIT_FORTRAN)
ENDFUNCTION(ADD_FORTRAN_SIMV2_SIMULATION)

FUNCTION(ADD_PARALLEL_SIMV2_SIMULATION target)
    IF(VISIT_PARALLEL)
        IF(BLUEGENEQ)
            SET(TMP_NOLINK ${VISIT_NOLINK_MPI_WITH_LIBRARIES})
            # Do this to try and control the order of MPI libraries on the command line
            SET(VISIT_NOLINK_MPI_WITH_LIBRARIES ON)
            SET(EXTRA_SIM_LIBS ${EXTRA_SIM_LIBS} ${ICET_LIB} ${VISIT_PARALLEL_LIBS})
        ENDIF(BLUEGENEQ)
        ADD_PARALLEL_EXECUTABLE(sim_${target} ${ARGN})
        SET_SIMULATION_PROPERTIES(${target})
        SET_TARGET_PROPERTIES(sim_${target} PROPERTIES HAS_CXX ON)
        IF(NOT VISIT_STATIC)
            TARGET_LINK_LIBRARIES(sim_${target} simV2 ${EXTRA_SIM_LIBS})
        ELSE()
            TARGET_LINK_LIBRARIES(sim_${target} simV2_static_par ${VTK_LIBS} ${ZLIB_LIB}
                                                                 ${EXTRA_SIM_LIBS})
        ENDIF()
        ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
        IF(BLUEGENEQ)
            SET(VISIT_NOLINK_MPI_WITH_LIBRARIES ${TMP_NOLINK})
        ENDIF(BLUEGENEQ)
    ENDIF(VISIT_PARALLEL)
ENDFUNCTION(ADD_PARALLEL_SIMV2_SIMULATION)

FUNCTION(ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION target)
    IF(VISIT_PARALLEL AND VISIT_FORTRAN)
        ADD_PARALLEL_FORTRAN_EXECUTABLE(sim_${target} ${ARGN} simulationexamplev2.f)
        SET_SIMULATION_PROPERTIES(${target})
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2 ${EXTRA_SIM_LIBS})
        ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
    ENDIF(VISIT_PARALLEL AND VISIT_FORTRAN)
ENDFUNCTION(ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION)

FUNCTION(ADD_PYTHON_SIMV2_SIMULATION target)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${target} ${CMAKE_CURRENT_BINARY_DIR})
ENDFUNCTION(ADD_PYTHON_SIMV2_SIMULATION)


##
## Simulation targets
##
ADD_SIMV2_SIMULATION(aresamr aresamr.c)

ADD_SIMV2_SIMULATION(amr amr.c)
ADD_FORTRAN_SIMV2_SIMULATION(famr famr.f)

ADD_SIMV2_SIMULATION(csg csg.c)

ADD_SIMV2_SIMULATION(curve curve.c)
ADD_FORTRAN_SIMV2_SIMULATION(fcurve fcurve.f)

ADD_SIMV2_SIMULATION(domainbounds domainbounds.c)
ADD_PARALLEL_SIMV2_SIMULATION(domainbounds_par domainbounds.c)

ADD_SIMV2_SIMULATION(ghostcells ghostcells.c)
ADD_PARALLEL_SIMV2_SIMULATION(ghostcells_par ghostcells.c)

ADD_SIMV2_SIMULATION(globalids globalids.c)
ADD_PARALLEL_SIMV2_SIMULATION(globalids_par globalids.c)

ADD_SIMV2_SIMULATION(interleave interleave.c)

ADD_SIMV2_SIMULATION(life life.c)
ADD_PARALLEL_SIMV2_SIMULATION(life_par life.c)

ADD_SIMV2_SIMULATION(mandelbrot mandelbrot.C patch.C)
ADD_PARALLEL_SIMV2_SIMULATION(mandelbrot_par mandelbrot.C patch.C)

ADD_SIMV2_SIMULATION(material material.c)
ADD_FORTRAN_SIMV2_SIMULATION(fmaterial fmaterial.f)

ADD_SIMV2_SIMULATION(materialvar materialvar.c)

ADD_SIMV2_SIMULATION(mesh mesh.c)
ADD_FORTRAN_SIMV2_SIMULATION(fmesh fmesh.f)

ADD_SIMV2_SIMULATION(multiblock multiblock.c)
ADD_PARALLEL_SIMV2_SIMULATION(multiblock_par multiblock.c)

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

ADD_SIMV2_SIMULATION(ucdcurve ucdcurve.c)
ADD_PARALLEL_SIMV2_SIMULATION(ucdcurve_par ucdcurve.c)

ADD_SIMV2_SIMULATION(unstructured unstructured.c)
ADD_FORTRAN_SIMV2_SIMULATION(funstructured funstructured.f)

ADD_SIMV2_SIMULATION(updateplots updateplots.c)
ADD_PARALLEL_SIMV2_SIMULATION(updateplots_par updateplots.c)
ADD_FORTRAN_SIMV2_SIMULATION(fupdateplots fupdateplots.f)

#usequeries.c

ADD_SIMV2_SIMULATION(var var.c)

ADD_SIMV2_SIMULATION(writeback writeback.c)
ADD_PARALLEL_SIMV2_SIMULATION(writeback_par writeback.c)

# copy python examples to simulation examples build dir
IF(VISIT_OUT_OF_SOURCE_BUILD)
    ADD_PYTHON_SIMV2_SIMULATION(material.py)
    ADD_PYTHON_SIMV2_SIMULATION(updateplots.py)
    ADD_PYTHON_SIMV2_SIMULATION(var.py)
ENDIF()
