#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#   Hank Childs, Sat Jun  5 16:26:46 CDT 2010
#   Add avtStateRecorderIntegralCurve.
#
#   Hank Childs, Sun Jun  6 12:01:50 PDT 2010
#   Delete avtStreamline.C and add its replacement, avtIntegralCurve.C.
#
#****************************************************************************/
SET(AVTIVP_SOURCES
avtCellLocator.C
avtCellLocatorClassic.C
avtCellLocatorBIH.C
avtCellLocatorRect.C
avtCellLocatorRectFace.C
avtCellLocatorRectEdge.C
avtIntegralCurve.C
avtIVPAdamsBashforth.C
avtIVPDopri5.C
avtIVPEuler.C
avtIVPRK4.C
avtIVPLeapfrog.C
avtIVPSolver.C
avtIVPM3DC1Field.C
avtIVPM3DC1Integrator.C
avtIVPNek5000Field.C
avtIVPNIMRODField.C
avtIVPFlashField.C
avtIVPVTKField.C
avtIVPVTKOffsetField.C
avtIVPVTKFaceField.C
avtIVPVTKEdgeField.C
avtIVPVTKTimeVaryingField.C
avtLCSIC.C
avtPoincareIC.C
avtStateRecorderIntegralCurve.C
avtStreamlineIC.C
MemStream.C
avtCommDSOnDemandICAlgorithm.C
avtICAlgorithm.C
avtMasterSlaveICAlgorithm.C
avtParDomICAlgorithm.C
avtParICAlgorithm.C
avtPICSFilter.C
avtPODICAlgorithm.C
avtPOSBalanceICAlgorithm.C
avtSerialICAlgorithm.C
avtStreamlineFilter.C
avtStreamlinePolyDataFilter.C
)

IF( NEKTAR++_FOUND )
    SET( AVTIVP_SOURCES ${AVTIVP_SOURCES} avtIVPNektar++Field.C )

    ADD_DEFINITIONS("-DNEKTAR_PLUS_PLUS_FOUND")
    ADD_DEFINITIONS("-DNEKTAR_MEMORY_POOL_ENABLED")
    ADD_DEFINITIONS("-DNEKTAR_USING_LAPACK")
    ADD_DEFINITIONS("-DNEKTAR_USING_BLAS")
ENDIF( NEKTAR++_FOUND )

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/third_party_builtin/nek5000/jl
${VTK_INCLUDE_DIRS}
${NEKTAR++_INCLUDE_DIR}
${NEKTAR++_INCLUDE_DIRS}
${NEKTAR++_TP_INCLUDE_DIRS}
${ACCELERATE_FRAMEWORK_INCLUDE_DIR}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${EAVL_LIBRARY_DIR} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${NEKTAR++_LIBRARY_DIR} ${NEKTAR++_LIBRARY_DIRS} ${NEKTAR++_TP_LIBRARY_DIRS} ${ACCELERATE_FRAMEWORK_LINK_FLAGS})

#ADD_LIBRARY(avtivp ${AVTIVP_SOURCES})
#TARGET_LINK_LIBRARIES(avtivp avtfilters avtmath visitcommon visit_vtk vtkCommonCore vtkFiltersCore ${NEKTAR++_LIB} ${NEKTAR++_LIBRARIES} ${NEKTAR++_TP_LIBRARIES})
#VISIT_INSTALL_TARGETS(avtivp)


#********************************* SERIAL ************************************
ADD_LIBRARY(avtivp_ser ${AVTIVP_SOURCES})
TARGET_LINK_LIBRARIES(avtivp_ser avtfilters_ser lightweight_visit_vtk visit_vtk avtmath avtview avtpipeline_ser avtdatabase_ser avtdbatts visitcommon visit_vtk vtkCommonCore vtkFiltersCore nek5000_interp ${NEKTAR++_LIB} ${NEKTAR++_LIBRARIES} ${NEKTAR++_TP_LIBRARIES})
VISIT_INSTALL_TARGETS(avtivp_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtivp_par ${AVTIVP_SOURCES})
    TARGET_LINK_LIBRARIES(avtivp_par avtfilters_par lightweight_visit_vtk visit_vtk avtmath avtview avtpipeline_par avtdatabase_par avtdbatts visitcommon visit_vtk vtkCommonCore vtkFiltersCore nek5000_interp ${NEKTAR++_LIB} ${NEKTAR++_LIBRARIES} ${NEKTAR++_TP_LIBRARIES})
    VISIT_INSTALL_TARGETS(avtivp_par)
ENDIF(VISIT_PARALLEL)
