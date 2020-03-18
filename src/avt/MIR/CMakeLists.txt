# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(BASE_SOURCES
Base/MIR.C
Base/MIRConnectivity.C
Base/MIROptions.C
)

SET(DISCRETE_SOURCES
Discrete/DiscreteMIR.C
)

SET(TET_SOURCES
Tet/Edge.C
Tet/Face.C
Tet/MaterialTetrahedron.C
Tet/MaterialTriangle.C
Tet/QuadraticHash.C
Tet/Tetrahedralizer.C
Tet/TetMIR.C
Tet/Triangulator.C
Tet/mat3d_tet.C
)

SET(YOUNGS_SOURCES
Youngs/CellInterface.C
Youngs/CellInterface2D.C
Youngs/CellInterface3D.C
Youngs/YoungsMIR.C
Youngs/vtkCQS.C
Youngs/vtkDataSetGradient.C
Youngs/vtkYoungsMaterialInterface.C
)

SET(ZOO_SOURCES
Zoo/BitUtils.C
Zoo/CellReconstructor.C
Zoo/IsovolumeCellReconstructor.C
Zoo/RecursiveCellReconstructor.C
Zoo/ResampledMat.C
Zoo/ZooMIR.C
)

SET(AVTMIR_SOURCES
${BASE_SOURCES}
${DISCRETE_SOURCES}
${TET_SOURCES}
${YOUNGS_SOURCES}
${ZOO_SOURCES}
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}/Base
${CMAKE_CURRENT_SOURCE_DIR}/Tet
${CMAKE_CURRENT_SOURCE_DIR}/Discrete
${CMAKE_CURRENT_SOURCE_DIR}/Youngs
${CMAKE_CURRENT_SOURCE_DIR}/Zoo
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/third_party_builtin/verdict
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtmir_ser ${AVTMIR_SOURCES})
TARGET_LINK_LIBRARIES(avtmir_ser visitcommon lightweight_visit_vtk  visit_verdict avtpipeline_ser)
VISIT_INSTALL_TARGETS(avtmir_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtmir_par ${AVTMIR_SOURCES})
    TARGET_LINK_LIBRARIES(avtmir_par visitcommon lightweight_visit_vtk visit_verdict avtpipeline_par)
    VISIT_INSTALL_TARGETS(avtmir_par)
ENDIF(VISIT_PARALLEL)
