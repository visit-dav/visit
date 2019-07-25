# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(METADATA_SOURCES
MetaData/avtArrayMetaData.C
MetaData/avtBaseVarMetaData.C
MetaData/avtCurveMetaData.C
MetaData/avtDatabaseMetaData.C
MetaData/avtDefaultPlotMetaData.C
MetaData/avtExpressionTypeConversions.C
MetaData/avtLabelMetaData.C
MetaData/avtMatSpeciesMetaData.C
MetaData/avtMaterialMetaData.C
MetaData/avtMeshMetaData.C
MetaData/avtScalarMetaData.C
MetaData/avtSimulationCommandSpecification.C
MetaData/avtSimulationInformation.C
MetaData/avtSpeciesMetaData.C
MetaData/avtSubsetsMetaData.C
MetaData/avtSymmetricTensorMetaData.C
MetaData/avtTensorMetaData.C
MetaData/avtTypes.C
MetaData/avtVarMetaData.C
MetaData/avtVectorMetaData.C
)

SET(SIL_SOURCES
SIL/avtSIL.C 
SIL/avtSILArray.C 
SIL/avtSILCollection.C 
SIL/avtSILEnumeratedNamespace.C 
SIL/avtSILMatrix.C 
SIL/avtSILNamespace.C 
SIL/avtSILRangeNamespace.C 
SIL/avtSILRestriction.C 
SIL/avtSILRestrictionTraverser.C 
SIL/avtSILSet.C
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
)

# Add link directories needed.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(avtdbatts ${METADATA_SOURCES} ${SIL_SOURCES})

TARGET_LINK_LIBRARIES(avtdbatts visitcommon)

VISIT_INSTALL_TARGETS(avtdbatts)
