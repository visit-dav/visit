#*****************************************************************************
#
# Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/

SET(DATA_SOURCES
Data/avtAverageValueRF.C
Data/avtCallback.C
Data/avtCellList.C
Data/avtColorTables.C
Data/avtCommonDataFunctions.C
Data/avtCompositeRF.C
Data/avtDataAttributes.C
Data/avtDataObject.C
Data/avtDataObjectInformation.C
Data/avtDataRangeSelection.C
Data/avtDataRepresentation.C
Data/avtDataTree.C
Data/avtDataValidity.C
Data/avtDataset.C
Data/avtDatasetExaminer.C
Data/avtDatasetVerifier.C
Data/avtDistancePixelizer.C
Data/avtDistanceToValueRF.C
Data/avtDynamicAttribute.C
Data/avtExtents.C
Data/avtExtractor.C
Data/avtFacelist.C
Data/avtFlatLighting.C
Data/avtHexahedronExtractor.C
Data/avtHexahedron20Extractor.C
Data/avtHexahedron27Extractor.C
Data/avtHistogramSpecification.C
Data/avtIdentifierSelection.C
Data/avtImage.C
Data/avtImagePartition.C
Data/avtImageRepresentation.C
Data/avtIntegrationRF.C
Data/avtIntervalTree.C
Data/avtIsolevelsSelection.C
Data/avtLightingModel.C
Data/avtLogicalSelection.C
Data/avtLookupTable.C
Data/avtMIPRangeRF.C
Data/avtMaterial.C
Data/avtMaximumIntensityProjectionRF.C
Data/avtMetaData.C
Data/avtMixedVariable.C
Data/avtNullData.C
Data/avtOpacityMap.C
Data/avtOpacityMapSamplePointArbitrator.C
Data/avtPhong.C
Data/avtPixelizer.C
Data/avtPlaneSelection.C
Data/avtPointAttribute.C
Data/avtPointExtractor.C
Data/avtPointSelection.C
Data/avtPyramidExtractor.C
Data/avtRangeMaxTable.C
Data/avtRay.C
Data/avtRayFunction.C
Data/avtRelativeValueSamplePointArbitrator.C
Data/avtResampleSelection.C
Data/avtSamplePointArbitrator.C
Data/avtSamplePoints.C
Data/avtSpatialBoxSelection.C
Data/avtSpecies.C
Data/avtTetrahedronExtractor.C
Data/avtVariablePixelizer.C
Data/avtVolume.C
Data/avtWedgeExtractor.C
)

SET(PIPELINE_SOURCES
Pipeline/avtContract.C
Pipeline/avtDataObjectReader.C
Pipeline/avtDataObjectString.C
Pipeline/avtDataObjectWriter.C
Pipeline/avtDataRequest.C
Pipeline/avtDataSetReader.C
Pipeline/avtDataSetWriter.C
Pipeline/avtDebugDumpOptions.C
Pipeline/avtImageReader.C
Pipeline/avtImageTiler.C
Pipeline/avtImageWriter.C
Pipeline/avtNamedSelection.C
Pipeline/avtNamedSelectionManager.C
Pipeline/avtNullDataReader.C
Pipeline/avtNullDataWriter.C
Pipeline/avtParallel.C
Pipeline/avtWebpage.C
)

SET(ABSTRACTFILTERS_SOURCES
AbstractFilters/avtCompactTreeFilter.C
AbstractFilters/avtDataObjectToDatasetFilter.C
AbstractFilters/avtDataObjectToImageFilter.C
AbstractFilters/avtDataObjectToSamplePointsFilter.C
AbstractFilters/avtDataTreeIterator.C
AbstractFilters/avtDatasetOnDemandFilter.C
AbstractFilters/avtDatasetToDataObjectFilter.C
AbstractFilters/avtDatasetToDatasetFilter.C
AbstractFilters/avtDatasetToImageFilter.C
AbstractFilters/avtDatasetToSamplePointsFilter.C
AbstractFilters/avtFacadeFilter.C
AbstractFilters/avtFilter.C
AbstractFilters/avtImageToDataObjectFilter.C
AbstractFilters/avtImageToImageFilter.C
AbstractFilters/avtMultiResolutionPartitionStrategy.C
AbstractFilters/avtMultipleInputToDataObjectFilter.C
AbstractFilters/avtMultipleInputToDatasetFilter.C
AbstractFilters/avtPluginDataTreeIterator.C
AbstractFilters/avtPluginStructuredChunkDataTreeIterator.C
AbstractFilters/avtSIMODataTreeIterator.C
AbstractFilters/avtSamplePointsToDataObjectFilter.C
AbstractFilters/avtSamplePointsToDatasetFilter.C
AbstractFilters/avtSamplePointsToImageFilter.C
AbstractFilters/avtSamplePointsToSamplePointsFilter.C
AbstractFilters/avtSingleFilterFacade.C
AbstractFilters/avtStructuredChunkDataTreeIterator.C
AbstractFilters/avtStructuredMeshChunker.C
AbstractFilters/avtStructuredMeshPartitionStrategy.C
AbstractFilters/avtSweepPlanePartitionStrategy.C
AbstractFilters/avtTimeLoopCollectorFilter.C
AbstractFilters/avtTimeLoopFilter.C
)

SET(SINKS_SOURCES
Sinks/avtDataObjectSink.C
Sinks/avtDatabaseWriter.C
Sinks/avtDatasetSink.C
Sinks/avtImageSink.C
Sinks/avtMultipleInputSink.C
Sinks/avtNullDataSink.C
Sinks/avtSamplePointsSink.C
Sinks/avtTerminatingDatasetSink.C
Sinks/avtTerminatingImageSink.C
Sinks/avtTerminatingNullDataSink.C
Sinks/avtTerminatingSink.C
)

SET(SOURCES_SOURCES
Sources/avtDataObjectSource.C
Sources/avtDatasetSource.C
Sources/avtImageSource.C
Sources/avtInlinePipelineSource.C
Sources/avtNullDataSource.C
Sources/avtOriginatingDatasetSource.C
Sources/avtOriginatingImageSource.C
Sources/avtOriginatingNullDataSource.C
Sources/avtOriginatingSource.C
Sources/avtQueryableSource.C
Sources/avtSamplePointsSource.C
Sources/avtSourceFromAVTDataset.C
Sources/avtSourceFromDataset.C
Sources/avtSourceFromImage.C
Sources/avtSourceFromNullData.C
)

SET(ALL_SOURCES
${DATA_SOURCES}
${PIPELINE_SOURCES}
${ABSTRACTFILTERS_SOURCES}
${SINKS_SOURCES}
${SOURCES_SOURCES}
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${VTK_LIBRARY_DIRS})

# Unless we're doing a VISIT_DBIO_ONLY build then we need avtmath
IF(NOT VISIT_DBIO_ONLY)
    SET(AVTMATH avtmath)
ENDIF(NOT VISIT_DBIO_ONLY)

#********************************* SERIAL ************************************
ADD_LIBRARY(avtpipeline_ser ${ALL_SOURCES})

TARGET_LINK_LIBRARIES(avtpipeline_ser visitcommon ${AVTMATH} avtdbatts lightweight_visit_vtk vtkGraphics ${ZLIB_LIB})
VISIT_INSTALL_TARGETS(avtpipeline_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtpipeline_par ${ALL_SOURCES})
    TARGET_LINK_LIBRARIES(avtpipeline_par visitcommon ${AVTMATH} avtdbatts lightweight_visit_vtk vtkGraphics ${ZLIB_LIB})
    VISIT_INSTALL_TARGETS(avtpipeline_par)
ENDIF(VISIT_PARALLEL)
