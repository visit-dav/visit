#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
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
#   Hank Childs, Sat Jun  5 19:03:22 CDT 2010
#   Add avtPICSFilter.
#
#   Hank Childs, Sun Jun  6 14:54:08 CDT 2010
#   Renamed avt*SLAlgorithm to avt*ICAlgorithm.
#
#   Eric Brugger, Wed Jun 30 12:59:41 PDT 2010
#   I added avtXRayFilter.
#
#   Hank Childs, Thu Aug 26 13:47:30 PDT 2010
#   Renamed avtCurrentExtentFilter to avtActualExtentsFilter.
#
#   Brad Whitlock, Fri Dec 10 16:21:23 PST 2010
#   I moved avtThresholdFilter to here from the Threshold plugin.
#
#   Eric Brugger, Tue Oct 25 13:55:27 PDT 2011
#   Add avtMultiresFilter.
#
#   Hank Childs, Tue Mar 13 12:59:07 PDT 2012
#   Move avtDatasetOnDemandFilter from Pipeline/AbstractFilters directory.
#
#   Kathleen Biagas, Tue May 28 17:02:23 MST 2013 
#   Move avtTimeLoopFilter and avtTimeLoopCollectorFilter from 
#   Pipeline/AbstractFilters directory.
#
#   Kevin Griffin, Mon Nov 3 12:41:11 PDT 2014
#   Add avtSubsetBlockMergeFilter.
#
#   Eric Brugger, Thu Jan 14 08:44:32 PST 2016
#   Add linking of vtkm library.
#
#   Cyrus Harrison, Mon Nov  7 10:03:35 PST 2016
#   Add avtPolylineCleanupFilter
#
#   Eric Brugger, Tue Aug 28 14:35:30 PDT 2018
#   I replaced support for vtkm with vtkh.
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkCommonMath, vtkCommonTransforms, vtkFiltersGeneral in support
#   of building plugins vs install on Windows. They are direct depencencies
#   for this library, and CMake automagically pulls them in when compiling
#   VisIt even if not listed, but not when building the plugins.
#
#****************************************************************************/

SET(AVTFILTERS_SOURCES
avtActualExtentsFilter.C
avtCondenseDatasetFilter.C
avtContourFilter.C
avtCoordSystemConvert.C
avtCurveConstructorFilter.C
avtCustomTransform.C
avtDatasetOnDemandFilter.C
avtDisplaceFilter.C
avtFacelistFilter.C
avtFeatureEdgesFilter.C
avtGhostZoneAndFacelistFilter.C
avtGhostZoneFilter.C
avtImageCommunicator.C
avtImageCompositer.C
avtLinearTransformFilter.C
avtLineoutFilter.C
avtLineScanFilter.C
avtLineToPolylineFilter.C
avtMassVoxelExtractor.C
avtMeshLogFilter.C
avtMissingDataFilter.C
avtMultiresFilter.C
avtNamedSelectionFilter.C
avtPointToGlyphFilter.C
avtPolylineAddEndPointsFilter.C
avtPolylineCleanupFilter.C
avtPolylineToRibbonFilter.C
avtPolylineToTubeFilter.C
avtRayCompositer.C
avtRayTracerBase.C
avtRayTracer.C
avtResampleFilter.C
avtSamplePointCommunicator.C
avtSamplePointExtractorBase.C
avtSamplePointExtractor.C
avtSamplePointToSurfaceFilter.C
avtShiftCenteringFilter.C
avtSimilarityTransformFilter.C
avtSmoothPolyDataFilter.C
avtStaggeringFilter.C
avtSubsetBlockMergeFilter.C
avtSummationFilter.C
avtSurfaceFilter.C
avtThresholdFilter.C
avtTiledImageCompositor.C
avtTimeLoopCollectorFilter.C
avtTimeLoopFilter.C
avtTransform.C
avtValueImageCompositer.C
avtVertexNormalsFilter.C
avtVoxelExtractor.C
avtWarpFilter.C
avtWholeImageCompositerNoZ.C
avtWholeImageCompositerWithZ.C
avtWorldSpaceToImageSpaceTransform.C
avtXRayFilter.C
)

IF(VISIT_SLIVR)
    LIST(APPEND AVTFILTERS_SOURCES
         avtSLIVRCommon.C
         avtSLIVRImageCompositor.C
         avtSLIVRRayTracer.C
         avtSLIVRSamplePointExtractor.C
         avtSLIVRVoxelExtractor.C)
    ADD_DEFINITIONS(-DVISIT_SLIVR)
ENDIF()

IF(VISIT_OSPRAY)
    LIST(APPEND AVTFILTERS_SOURCES
         avtOSPRayCommon.C
         avtOSPRayImageCompositor.C
         avtOSPRayRayTracer.C
         avtOSPRaySamplePointExtractor.C
         avtOSPRayVoxelExtractor.C
         )
    IF(ICET_FOUND)
        # If we have Ice-T then add -DHAVE_ICET to the compilation flags.
        # We need to link with OpenGL on Mac when we have IceT.
        IF(APPLE)
            SET(ICET_OPENGL ${OPENGL_gl_LIBRARY})
        ENDIF(APPLE)
        # Link icet libraries using absolute pathes
        SET(ICET_FULLPATH_LIB ${ICET_OPENGL})
        FOREACH(X ${ICET_LIB})
            LIST(APPEND ICET_FULLPATH_LIB "${ICET_LIBRARY_DIR}/${X}")
        ENDFOREACH()
        # Other setups
        INCLUDE_DIRECTORIES(${ICET_INCLUDE_DIR})
        LINK_DIRECTORIES(${ICET_LIBRARY_DIR})
        ADD_DEFINITIONS(-DVISIT_OSPRAY_ICET)
    ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/IVP
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/main
${VTKh_INCLUDE_DIRS}
${VTKM_DIR}/include/vtkm-1.2
${VTKM_DIR}/include/vtkm-1.2/vtkm/thirdparty/taotuple
${VTK_INCLUDE_DIRS}
)

if(VISIT_OSPRAY)
    include_directories(${OSPRAY_INCLUDE_DIR})
endif()

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtfilters_ser ${AVTFILTERS_SOURCES})
TARGET_LINK_LIBRARIES(avtfilters_ser visitcommon lightweight_visit_vtk visit_vtk avtmath avtview avtpipeline_ser avtdatabase_ser avtdbatts vtkCommonMath vtkCommonTransforms vtkFiltersGeneral vtkFiltersModeling)
IF(VISIT_OSPRAY) 
    TARGET_LINK_LIBRARIES(avtfilters_ser ${OSPRAY_LIBRARIES})
    IF(ICET_FOUND)
        TARGET_LINK_LIBRARIES(avtfilters_ser ${ICET_FULLPATH_LIB})
    ENDIF()
ENDIF() 
VISIT_INSTALL_TARGETS(avtfilters_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtfilters_par ${AVTFILTERS_SOURCES})
    TARGET_LINK_LIBRARIES(avtfilters_par visitcommon lightweight_visit_vtk visit_vtk avtmath avtview avtpipeline_par avtdatabase_par avtdbatts vtkCommonMath vtkCommonTransforms vtkFiltersGeneral vtkFiltersModeling)
    IF(VISIT_OSPRAY) 
        TARGET_LINK_LIBRARIES(avtfilters_par ${OSPRAY_LIBRARIES})
        IF(ICET_FOUND)
            TARGET_LINK_LIBRARIES(avtfilters_par ${ICET_FULLPATH_LIB})
        ENDIF()
    ENDIF()
    VISIT_INSTALL_TARGETS(avtfilters_par)
ENDIF(VISIT_PARALLEL)
