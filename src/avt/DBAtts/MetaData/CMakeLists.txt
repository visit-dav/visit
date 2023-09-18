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


# add to the sources list
visit_append_list(
    NAME  avtdbatts_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBaseVarMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabaseMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDefaultPlotMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionTypeConversions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLabelMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaterialMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMatSpeciesMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMeshMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtScalarMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSimulationCommandSpecification.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSimulationInformation.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpeciesMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSubsetsMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmetricTensorMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTypes.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVarMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorMetaData.C)

# add to the includes list
visit_append_list(
    NAME  avtdbatts_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/include/avt/DBatts/MetaData>)

