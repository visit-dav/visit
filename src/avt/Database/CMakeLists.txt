# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Cyrus Harrison, Thu Apr 19 16:07:11 PDT 2012
#  Added Ghost/avtLocalStructuredDomainBoundaries.C
#
#****************************************************************************

SET(DATABASE_SOURCES
Database/avtDatabase.C
Database/avtDatabaseFactory.C
Database/avtDatasetCollection.C
Database/avtDatasetDatabase.C
Database/avtFileDescriptorManager.C
Database/avtGenericDatabase.C
Database/avtIOInformation.C
Database/avtSILGenerator.C
Database/avtSourceFromDatabase.C
Database/avtVariableCache.C
Database/avtTransformManager.C
Database/avtPolygonToTrianglesTesselator.C
Database/avtPolyhedralSplit.C
)

SET(FORMATS_SOURCES
Formats/avtFileFormat.C
Formats/avtFileFormatInterface.C
Formats/avtMTSDFileFormat.C
Formats/avtMTSDFileFormatInterface.C
Formats/avtMTMDFileFormat.C
Formats/avtMTMDFileFormatInterface.C
Formats/avtSTMDFileFormat.C
Formats/avtSTMDFileFormatInterface.C
Formats/avtSTSDFileFormat.C
Formats/avtSTSDFileFormatInterface.C
)

SET(GHOST_SOURCES
Ghost/StructuredBoundary.C
Ghost/avtDomainBoundaries.C
Ghost/avtGhostNodeGenerator.C
Ghost/avtIsenburgSGG.C
Ghost/avtStreamingGhostGenerator.C
Ghost/avtStructuredDomainBoundaries.C
Ghost/avtLocalStructuredDomainBoundaries.C
Ghost/avtUnstructuredDomainBoundaries.C
Ghost/avtNekDomainBoundaries.C
Ghost/avtStructuredDomainNesting.C
Ghost/avtUnstructuredPointBoundaries.C
)

SET(AVTDATABASE_SOURCES
${DATABASE_SOURCES}
${FORMATS_SOURCES}
${GHOST_SOURCES}
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/Database
${CMAKE_CURRENT_SOURCE_DIR}/Formats
${CMAKE_CURRENT_SOURCE_DIR}/Ghost
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/MIR/Base
${VISIT_SOURCE_DIR}/avt/MIR/Tet
${VISIT_SOURCE_DIR}/avt/MIR/Youngs
${VISIT_SOURCE_DIR}/avt/MIR/Zoo
${VISIT_SOURCE_DIR}/avt/MIR/Discrete/
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/third_party_builtin/tess2
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

# Unless we're doing a VISIT_DBIO_ONLY build then we need avtmir
IF(NOT VISIT_DBIO_ONLY)
    SET(AVTMIR_SER avtmir_ser)
    SET(AVTMIR_PAR avtmir_par)
ENDIF(NOT VISIT_DBIO_ONLY)

#********************************* SERIAL ************************************
ADD_LIBRARY(avtdatabase_ser ${AVTDATABASE_SOURCES})
TARGET_LINK_LIBRARIES(avtdatabase_ser visitcommon avtdbatts ${AVTMIR_SER} avtpipeline_ser visit_vtk tess2)
VISIT_INSTALL_TARGETS(avtdatabase_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtdatabase_par ${AVTDATABASE_SOURCES})
    TARGET_LINK_LIBRARIES(avtdatabase_par visitcommon avtdbatts ${AVTMIR_PAR} avtpipeline_par visit_vtk tess2)
    VISIT_INSTALL_TARGETS(avtdatabase_par)
ENDIF(VISIT_PARALLEL)
