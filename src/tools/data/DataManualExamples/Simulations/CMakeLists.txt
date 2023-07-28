# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
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
#   Kathleen Biagas, Thu Feb 27 2020
#   Remove all_simulations target, as this folder is not added to the build
#   unless VISIT_ENABLE_DATA_MANUAL_EXAMPLES is true.
#   Remove 'if(VISIT_PARALLEL AND VISIT_FORTRAN)', 'if(VISIT_FORTRAN)', and
#   'if(VISIT_PARALLEL)' from functions so if-tests are performed only once.
#
#   Kathleen Biagas, Wed Nov 30, 2022
#   Remove test for VISIT_OUT_OF_SOURCE_BUILD, as out-of-source is required.
#
#****************************************************************************

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FUNCTION(SET_SIMULATION_PROPERTIES target)
    IF(NOT WIN32)
        SET_TARGET_PROPERTIES(sim_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/tools/DataManualExamples/Simulations
            FOLDER tools/data/DataManualExamples/Simulations
        )
    ELSE()
        SET_TARGET_PROPERTIES(sim_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            RUNTIME_OUTPUT_DIRECTORY_REWITHDEBINFO ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${VISIT_EXECUTABLE_DIR}/tools/DataManualExamples/Simulations
            FOLDER tools/data/DataManualExamples/Simulations
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

# If we have XDB and we're static linking then we need to link with libXDBLib here.
IF(VISIT_STATIC AND VISIT_ENABLE_XDB)
    SET(EXTRA_SIM_LIBS ${EXTRA_SIM_LIBS} ${VISIT_LIBRARY_DIR}/libXDBLib.so) # Just Linux for now.
ENDIF(VISIT_STATIC AND VISIT_ENABLE_XDB)

SET(VTK_LIBS vtkCommonCore vtkImagingCore vtkRenderingCore vtkIOLegacy vtkIOPLY
             vtkInteractionStyle vtkRenderingOpenGL2 
             vtkFiltersHybrid vtkFiltersModeling vtkGeovisCore)
IF(ICET_FOUND)
    LINK_DIRECTORIES(${ICET_LIBRARY_DIR})
ENDIF(ICET_FOUND)

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
        TARGET_LINK_LIBRARIES(sim_${target} simV2_static_ser
           ${VTK_LIBS} ${ZLIB_LIB} ${EXTRA_SIM_LIBS})
    ENDIF()
    ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
ENDFUNCTION(ADD_SIMV2_SIMULATION)

FUNCTION(ADD_FORTRAN_SIMV2_SIMULATION target)
    ADD_EXECUTABLE(sim_${target} ${ARGN} simulationexamplev2.f)
    SET_SIMULATION_PROPERTIES(${target})
    IF(NOT VISIT_STATIC)
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2 ${EXTRA_SIM_LIBS})
    ELSE()
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2_static_ser 
            ${VTK_LIBS} ${ZLIB_LIB} ${EXTRA_SIM_LIBS})
    ENDIF()
    ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
ENDFUNCTION(ADD_FORTRAN_SIMV2_SIMULATION)

FUNCTION(ADD_PARALLEL_SIMV2_SIMULATION target)
    IF(BLUEGENEQ)
        SET(TMP_NOLINK ${VISIT_NOLINK_MPI_WITH_LIBRARIES})
        # Do this to try and control the order of MPI libraries on the command line
        SET(VISIT_NOLINK_MPI_WITH_LIBRARIES ON)
        SET(EXTRA_SIM_LIBS ${EXTRA_SIM_LIBS} ${ICET_LIB} ${VISIT_PARALLEL_LIBS})
    ELSE(BLUEGENEQ)
       IF(ICET_FOUND)
           SET(EXTRA_SIM_LIBS ${EXTRA_SIM_LIBS} ${ICET_LIB})
       ENDIF(ICET_FOUND)
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
ENDFUNCTION(ADD_PARALLEL_SIMV2_SIMULATION)

FUNCTION(ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION target)
    ADD_PARALLEL_FORTRAN_EXECUTABLE(sim_${target} ${ARGN} simulationexamplev2.f)
    SET_SIMULATION_PROPERTIES(${target})
    SET(FLIBS ${EXTRA_SIM_LIBS})
    IF(ICET_FOUND)
        SET(FLIBS ${FLIBS} ${ICET_LIB})
    ENDIF(ICET_FOUND)
    # HACK: Force the Fortran MPI libraries.
    SET(FLIBS ${FLIBS} ${MPI_Fortran_LIBRARIES})
    IF(NOT VISIT_STATIC)
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2 ${FLIBS})
    ELSE()
        TARGET_LINK_LIBRARIES(sim_${target} simV2f simV2_static_ser 
            ${VTK_LIBS} ${ZLIB_LIB} ${FLIBS})
    ENDIF()
    ADD_TARGET_INCLUDE(sim_${target} ${VISIT_SOURCE_DIR}/sim/V2/lib)
    # HACK:Force the Fortran include path. Why was it not already set?
    ADD_TARGET_INCLUDE(sim_${target} ${MPI_Fortran_INCLUDE_PATH})
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
ADD_SIMV2_SIMULATION(batch batch.c extract.c)
ADD_SIMV2_SIMULATION(csg csg.c)
ADD_SIMV2_SIMULATION(curve curve.c)
ADD_SIMV2_SIMULATION(domainbounds domainbounds.c)
ADD_SIMV2_SIMULATION(ghostcells ghostcells.c)
ADD_SIMV2_SIMULATION(globalids globalids.c)
ADD_SIMV2_SIMULATION(higherorder higherorder.c)
ADD_SIMV2_SIMULATION(interleave interleave.c)
ADD_SIMV2_SIMULATION(life life.c)
ADD_SIMV2_SIMULATION(mandelbrot mandelbrot.C patch.C)
ADD_SIMV2_SIMULATION(mandelbulbsim mandelbulbsim.C extract.c)
ADD_SIMV2_SIMULATION(material material.c)
ADD_SIMV2_SIMULATION(materialvar materialvar.c)
ADD_SIMV2_SIMULATION(mesh mesh.c)
ADD_SIMV2_SIMULATION(multiblock multiblock.c)
ADD_SIMV2_SIMULATION(point point.c)
ADD_SIMV2_SIMULATION(polyhedral polyhedral.c)
ADD_SIMV2_SIMULATION(scalar scalar.c)
ADD_SIMV2_SIMULATION(surface surface.c)
ADD_SIMV2_SIMULATION(sim1 sim1.c)
ADD_SIMV2_SIMULATION(sim2 sim2.c)
ADD_SIMV2_SIMULATION(sim3 sim3.c)
ADD_SIMV2_SIMULATION(sim4 sim4.c)
ADD_SIMV2_SIMULATION(sim5 sim5.c)
ADD_SIMV2_SIMULATION(sim6 sim6.c)
ADD_SIMV2_SIMULATION(sim7 sim7.c)
ADD_SIMV2_SIMULATION(sim8 sim8.c)
ADD_SIMV2_SIMULATION(sim9 sim9.c)
ADD_SIMV2_SIMULATION(sim10 sim10.c)
ADD_SIMV2_SIMULATION(sim11 sim11.c)
ADD_SIMV2_SIMULATION(sim12 sim12.c)
ADD_SIMV2_SIMULATION(species species.c)
ADD_SIMV2_SIMULATION(ucdcurve ucdcurve.c)
ADD_SIMV2_SIMULATION(unstructured unstructured.c)
ADD_SIMV2_SIMULATION(updateplots updateplots.c)
#usequeries.c
ADD_SIMV2_SIMULATION(var var.c)
ADD_SIMV2_SIMULATION(writeback writeback.c)
ADD_SIMV2_SIMULATION(zerocopy zerocopy.c extract.c)

IF(VISIT_FORTRAN)
    ADD_FORTRAN_SIMV2_SIMULATION(famr famr.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fcurve fcurve.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fmaterial fmaterial.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fmesh fmesh.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fpoint fpoint.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fscalar fscalar.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim4 fsim4.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim5 fsim5.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim6 fsim6.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim7 fsim7.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim8 fsim8.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim9 fsim9.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim10 fsim10.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim11 fsim11.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fsim12 fsim12.f)
    ADD_FORTRAN_SIMV2_SIMULATION(funstructured funstructured.f)
    ADD_FORTRAN_SIMV2_SIMULATION(fupdateplots fupdateplots.f)
ENDIF(VISIT_FORTRAN)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_SIMV2_SIMULATION(batch_par batch.c extract.c)
    ADD_PARALLEL_SIMV2_SIMULATION(domainbounds_par domainbounds.c)
    ADD_PARALLEL_SIMV2_SIMULATION(ghostcells_par ghostcells.c)
    ADD_PARALLEL_SIMV2_SIMULATION(globalids_par globalids.c)
    ADD_PARALLEL_SIMV2_SIMULATION(life_par life.c)
    ADD_PARALLEL_SIMV2_SIMULATION(mandelbrot_par mandelbrot.C patch.C)
    ADD_PARALLEL_SIMV2_SIMULATION(mandelbulbsim_par mandelbulbsim.C extract.c)
    ADD_PARALLEL_SIMV2_SIMULATION(multiblock_par multiblock.c)
    ADD_PARALLEL_SIMV2_SIMULATION(sim2p sim2p.c)
    ADD_PARALLEL_SIMV2_SIMULATION(sim3p sim3p.c)
    ADD_PARALLEL_SIMV2_SIMULATION(sim4p sim4p.c)
    ADD_PARALLEL_SIMV2_SIMULATION(ucdcurve_par ucdcurve.c)
    ADD_PARALLEL_SIMV2_SIMULATION(updateplots_par updateplots.c)
    ADD_PARALLEL_SIMV2_SIMULATION(writeback_par writeback.c)
    ADD_PARALLEL_SIMV2_SIMULATION(zerocopy_par zerocopy.c extract.c)
    if(VISIT_FORTRAN)
        ADD_PARALLEL_FORTRAN_SIMV2_SIMULATION(fscalarp fscalarp.f)
    endif() 
ENDIF(VISIT_PARALLEL)


# copy python examples to simulation examples build dir
ADD_PYTHON_SIMV2_SIMULATION(material.py)
ADD_PYTHON_SIMV2_SIMULATION(updateplots.py)
ADD_PYTHON_SIMV2_SIMULATION(var.py)

