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

SET(AVTIVP_SOURCES
avtCellLocator.C
avtCellLocatorClassic.C
avtCellLocatorBIH.C
avtCellLocatorRect.C
avtCellLocatorRectFace.C
avtCellLocatorRectEdge.C
avtIntegralCurve.C
avtIVPAdamsBashforth.C
avtIVPDopri5.C
avtIVPEuler.C
avtIVPRK4.C
avtIVPLeapfrog.C
avtIVPSolver.C
avtIVPM3DC1Field.C
avtIVPM3DC1Integrator.C
avtIVPNek5000Field.C
avtIVPNek5000TimeVaryingField.C
avtIVPFlashField.C
avtIVPVTKField.C
avtIVPVTKOffsetField.C
avtIVPVTKFaceField.C
avtIVPVTKEdgeField.C
avtIVPVTKTimeVaryingField.C
avtLCSIC.C
avtPoincareIC.C
avtStateRecorderIntegralCurve.C
avtIntegralCurveIC.C
MemStream.C
avtICAlgorithm.C
avtParICAlgorithm.C
avtPICSFilter.C
avtPODICAlgorithm.C
avtSerialICAlgorithm.C
)

IF( NEKTAR++_FOUND )
    SET( AVTIVP_SOURCES ${AVTIVP_SOURCES}
        avtIVPNektar++Field.C
        avtIVPNektar++TimeVaryingField.C )

    ADD_DEFINITIONS("-DNEKTAR_MEMORY_POOL_ENABLED")
    ADD_DEFINITIONS("-DNEKTAR_USING_LAPACK")
    ADD_DEFINITIONS("-DNEKTAR_USING_BLAS")
ENDIF( NEKTAR++_FOUND )

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/third_party_builtin/nek5000/jl
${VTK_INCLUDE_DIRS}
${NEKTAR++_INCLUDE_DIR}
${NEKTAR++_INCLUDE_DIRS}
${NEKTAR++_TP_INCLUDE_DIRS}
${ACCELERATE_FRAMEWORK_INCLUDE_DIR}
)

set(vtklibs VTK::CommonCore VTK::FiltersCore)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${NEKTAR++_LIBRARY_DIR} ${NEKTAR++_LIBRARY_DIRS} ${NEKTAR++_TP_LIBRARY_DIRS} ${ACCELERATE_FRAMEWORK_LINK_FLAGS})


#********************************* SERIAL ************************************
ADD_LIBRARY(avtivp_ser ${AVTIVP_SOURCES})
TARGET_LINK_LIBRARIES(avtivp_ser avtfilters_ser lightweight_visit_vtk visit_vtk avtmath avtview avtpipeline_ser avtdatabase_ser avtdbatts visitcommon ${vtklibs} nek5000_interp ${NEKTAR++_LIB} ${NEKTAR++_LIBRARIES} ${NEKTAR++_TP_LIBRARIES})
VISIT_INSTALL_TARGETS(avtivp_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtivp_par ${AVTIVP_SOURCES})
    TARGET_LINK_LIBRARIES(avtivp_par avtfilters_par lightweight_visit_vtk visit_vtk avtmath avtview avtpipeline_par avtdatabase_par avtdbatts visitcommon ${vtklibs} nek5000_interp ${NEKTAR++_LIB} ${NEKTAR++_LIBRARIES} ${NEKTAR++_TP_LIBRARIES})
    VISIT_INSTALL_TARGETS(avtivp_par)
ENDIF(VISIT_PARALLEL)
