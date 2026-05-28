# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtpipeline_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageValueRF.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCallback.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCellList.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtColorTables.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCommonDataFunctions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCompositeRF.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataAttributes.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObject.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectInformation.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataRangeSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataRepresentation.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataset.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetExaminer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetVerifier.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataTree.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataValidity.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistancePixelizer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistanceToValueRF.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDynamicAttribute.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExtents.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExtractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFacelist.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFlatLighting.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHexahedron20Extractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHexahedron27Extractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHexahedronExtractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHistogramSpecification.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIdentifierSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImage.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImagePartition.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageRepresentation.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntegrationRF.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntervalTree.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIsolevelsSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLightingModel.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaterial.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaximumIntensityProjectionRF.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMIPRangeRF.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMixedVariable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultiresSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOpacityMap.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOpacityMapSamplePointArbitrator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPhong.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPixelizer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlaneSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointExtractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPyramidExtractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRangeMaxTable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRay.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRayFunction.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRelativeValueSamplePointArbitrator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResampleSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResolutionSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointArbitrator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePoints.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpatialBoxSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpecies.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTetrahedronExtractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariablePixelizer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVolume.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWedgeExtractor.C)

visit_append_list(
    NAME  avtpipeline_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAccessor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageValueRF.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCallback.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCellList.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCellTypes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtColorTables.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCommonDataFunctions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCompositeRF.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataAttributes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObject.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectInformation.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataRangeSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataRepresentation.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataset.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetExaminer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetVerifier.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataTree.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataValidity.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistancePixelizer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistanceToValueRF.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDynamicAttribute.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExtents.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExtractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFacelist.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFlatLighting.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGhostData.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHexahedron20Extractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHexahedron27Extractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHexahedronExtractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHistogramSpecification.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIdentifierSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImage.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImagePartition.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageRepresentation.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageType.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntegrationRF.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntervalTree.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIsolevelsSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLightingModel.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaterial.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaximumIntensityProjectionRF.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMetaData.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMIPRangeRF.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMixedVariable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultiresSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullData.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOpacityMap.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOpacityMapSamplePointArbitrator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPhong.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPixelizer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlaneSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointExtractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPyramidExtractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRangeMaxTable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRay.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRayFunction.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRelativeValueSamplePointArbitrator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResampleSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResolutionSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointArbitrator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePoints.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpatialBoxSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpecies.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTetrahedronExtractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariablePixelizer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVolume.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVtkmDataSet.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWedgeExtractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/pipeline_exports.h)

visit_append_list(
    NAME  avtpipeline_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Pipeline/Data>)

