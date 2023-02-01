# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(DATA_SOURCES
    avtAverageValueRF.C
    avtCallback.C
    avtCellList.C
    avtColorTables.C
    avtCommonDataFunctions.C
    avtCompositeRF.C
    avtDataAttributes.C
    avtDataObject.C
    avtDataObjectInformation.C
    avtDataRangeSelection.C
    avtDataRepresentation.C
    avtDataTree.C
    avtDataValidity.C
    avtDataset.C
    avtDatasetExaminer.C
    avtDatasetVerifier.C
    avtDistancePixelizer.C
    avtDistanceToValueRF.C
    avtDynamicAttribute.C
    avtExtents.C
    avtExtractor.C
    avtFacelist.C
    avtFlatLighting.C
    avtHexahedronExtractor.C
    avtHexahedron20Extractor.C
    avtHexahedron27Extractor.C
    avtHistogramSpecification.C
    avtIdentifierSelection.C
    avtImage.C
    avtImagePartition.C
    avtImageRepresentation.C
    avtIntegrationRF.C
    avtIntervalTree.C
    avtIsolevelsSelection.C
    avtLightingModel.C
    avtLogicalSelection.C
    avtMaterial.C
    avtMaximumIntensityProjectionRF.C
    avtMetaData.C
    avtMIPRangeRF.C
    avtMixedVariable.C
    avtMultiresSelection.C
    avtNullData.C
    avtOpacityMap.C
    avtOpacityMapSamplePointArbitrator.C
    avtPhong.C
    avtPixelizer.C
    avtPlaneSelection.C
    avtPointExtractor.C
    avtPointSelection.C
    avtPyramidExtractor.C
    avtRangeMaxTable.C
    avtRay.C
    avtRayFunction.C
    avtRelativeValueSamplePointArbitrator.C
    avtResampleSelection.C
    avtResolutionSelection.C
    avtSamplePointArbitrator.C
    avtSamplePoints.C
    avtSpatialBoxSelection.C
    avtSpecies.C
    avtTetrahedronExtractor.C
    avtVariablePixelizer.C
    avtVolume.C
    avtWedgeExtractor.C)

target_sources(avtpipeline_ser PRIVATE ${DATA_SOURCES})
target_include_directories(avtpipeline_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtpipeline_par PRIVATE ${DATA_SOURCES})
    target_include_directories(avtpipeline_par PUBLIC .)
endif()

unset(DATA_SOURCES)

