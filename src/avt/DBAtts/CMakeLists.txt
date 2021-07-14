# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#
#    Kathleen Biagas, Tue Jul 13 14:58:28 PDT 2021
#    Remove avtScalarMetaData and avtSimulationCommandSpecification from
#    code gen targets list. They cannot be auto-gen'd.
#
#****************************************************************************


# Add XML Code Gen Targets

# Note: These names are in alphabetical order


#####################
# MetaData Objects that we can't Code-Gen, or have customizations:
#####################

# avtScalarMetaData: Header file has a field that cannot be part of the .xml file due to it not being
#                    a defined Field type in tools/dev/xml/Field.h
#
# avtSimulationCommandSpecification:  Header file has `using` statements that currently cannot be
#                                     specified via logic in a .code file

set(METADATA_GEN_OBJECTS 
    avtArrayMetaData
    avtBaseVarMetaData
    avtCurveMetaData
    avtDatabaseMetaData
    avtDefaultPlotMetaData
    avtLabelMetaData
    avtMatSpeciesMetaData
    avtMaterialMetaData
    avtMeshMetaData
    avtSimulationInformation
    avtSpeciesMetaData
    avtSubsetsMetaData
    avtSymmetricTensorMetaData
    avtTensorMetaData
    avtVarMetaData
    avtVectorMetaData)

# Metadata Objects
foreach(obj_name ${METADATA_GEN_OBJECTS})
    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}/MetaData/
                       ${CMAKE_CURRENT_SOURCE_DIR}/MetaData/)
endforeach()


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
