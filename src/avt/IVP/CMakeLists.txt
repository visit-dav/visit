# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Hank Childs, Sat Jun  5 16:26:46 CDT 2010
#   Add avtStateRecorderIntegralCurve.
#
#   Kathleen Biagas, Tue Apr 16, 2024
#   Removed avtManagerWorkerICAlgorithm.
#
#****************************************************************************

set(avtivp_SOURCES
    avtCellLocator.C
    avtCellLocatorBIH.C
    avtCellLocatorClassic.C
    avtCellLocatorRect.C
    avtCellLocatorRectEdge.C
    avtCellLocatorRectFace.C
    avtICAlgorithm.C
    avtIntegralCurve.C
    avtIntegralCurveIC.C
    avtIVPAdamsBashforth.C
    avtIVPDopri5.C
    avtIVPEuler.C
    avtIVPFlashField.C
    avtIVPLeapfrog.C
    avtIVPM3DC1Field.C
    avtIVPM3DC1Integrator.C
    avtIVPNek5000Field.C
    avtIVPNek5000TimeVaryingField.C
    avtIVPRK4.C
    avtIVPSolver.C
    avtIVPVTKEdgeField.C
    avtIVPVTKFaceField.C
    avtIVPVTKField.C
    avtIVPVTKOffsetField.C
    avtIVPVTKTimeVaryingField.C
    avtLCSIC.C
    avtParICAlgorithm.C
    avtPICSFilter.C
    avtPODICAlgorithm.C
    avtPoincareIC.C
    avtSerialICAlgorithm.C
    avtStateRecorderIntegralCurve.C
    MemStream.C)

set(avtivp_HEADERS
    avtCellLocator.h
    avtCellLocatorBIH.h
    avtCellLocatorClassic.h
    avtCellLocatorRect.h
    avtCellLocatorRectEdge.h
    avtCellLocatorRectFace.h
    avtICAlgorithm.h
    avtIntegralCurve.h
    avtIntegralCurveIC.h
    avtIVPAdamsBashforth.h
    avtIVPDopri5.h
    avtIVPEuler.h
    avtIVPField.h
    avtIVPFlashField.h
    avtIVPLeapfrog.h
    avtIVPM3DC1Field.h
    avtIVPM3DC1Integrator.h
    avtIVPNek5000Field.h
    avtIVPNek5000TimeVaryingField.h
    avtIVPRK4.h
    avtIVPSolver.h
    avtIVPStateHelper.h
    avtIVPTimeVaryingField.h
    avtIVPVTKEdgeField.h
    avtIVPVTKFaceField.h
    avtIVPVTKField.h
    avtIVPVTKOffsetField.h
    avtIVPVTKTimeVaryingField.h
    avtLCSIC.h
    avtParICAlgorithm.h
    avtPICSFilter.h
    avtPODICAlgorithm.h
    avtPoincareIC.h
    avtSerialICAlgorithm.h
    avtStateRecorderIntegralCurve.h
    ivp_exports.h
    MemStream.h)

set(avtivp_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/IVP>
    ${ACCELERATE_FRAMEWORK_INCLUDE_DIR})

set(avtivp_DEPENDS
    visitcommon
    avtdbatts
    avtmath
    avtview
    lightweight_visit_vtk
    visit_vtk
    VTK::CommonCore
    VTK::FiltersCore
    nek5000_interp)

if(NEKTAR++_FOUND)
    list(APPEND avtivp_SOURCES
        avtIVPNektar++Field.C
        avtIVPNektar++TimeVaryingField.C)
    list(APPEND avtivp_HEADERS
        avtIVPNektar++Field.h
        avtIVPNektar++TimeVaryingField.h)
    set(avtivp_DEFINES
        NEKTAR_MEMORY_POOL_ENABLED
        NEKTAR_USING_LAPACK
        NEKTAR_USING_BLAS)
    foreach(inc ${NEKTAR++_INCLUDE_DIRS})
        list(APPEND avtivp_INCLUDES
            $<BUILD_INTERFACE:${inc}>)
    endforeach()
    foreach(inc ${NEKTAR++_TP_INCLUDE_DIRS})
        list(APPEND avtivp_INCLUDES
            $<BUILD_INTERFACE:${inc}>)
    endforeach()

    list(APPEND avtivp_DEPENDS
        ${NEKTAR++_LIBRARIES}
        ${NEKTAR++_TP_LIBRARIES})
endif()


#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtivp_ser
    SOURCES    ${avtivp_SOURCES}
    HEADERS    ${avtivp_HEADERS}
    DEFINES    ${avtivp_DEFINES}
    INCLUDES   ${avtivp_INCLUDES}
    DEPENDS_ON ${avtivp_DEPENDS}
               avtdatabase_ser
               avtfilters_ser
               avtpipeline_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtivp_par
        SOURCES    ${avtivp_SOURCES}
        HEADERS    ${avtivp_HEADERS}
        DEFINES    ${avtivp_DEFINES}
        INCLUDES   ${avtivp_INCLUDES}
        DEPENDS_ON ${avtivp_DEPENDS}
                   avtdatabase_par
                   avtfilters_par
                   avtpipeline_par)
endif()

