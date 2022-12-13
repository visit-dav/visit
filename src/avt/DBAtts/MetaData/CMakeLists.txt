# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
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
    avtMaterialMetaData
    avtMatSpeciesMetaData
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
                       ${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

target_sources(avtdbatts PRIVATE
    avtArrayMetaData.C
    avtBaseVarMetaData.C
    avtCurveMetaData.C
    avtDatabaseMetaData.C
    avtDefaultPlotMetaData.C
    avtExpressionTypeConversions.C
    avtLabelMetaData.C
    avtMaterialMetaData.C
    avtMatSpeciesMetaData.C
    avtMeshMetaData.C
    avtScalarMetaData.C
    avtSimulationCommandSpecification.C
    avtSimulationInformation.C
    avtSpeciesMetaData.C
    avtSubsetsMetaData.C
    avtSymmetricTensorMetaData.C
    avtTensorMetaData.C
    avtTypes.C
    avtVarMetaData.C
    avtVectorMetaData.C)

target_include_directories(avtdbatts PUBLIC .)

