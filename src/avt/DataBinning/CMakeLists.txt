# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#  Cyrus Harrison, Tue Aug 10 10:39:08 PDT 2010
#   Added rms operator.
#
#****************************************************************************

SET(AVTDBIN_SOURCES
avtBinningScheme.C
avtDataBinning.C
avtDataBinningConstructor.C
avtDataBinningFunctionInfo.C
avtR2Faverage.C
avtR2Fcount.C
avtR2Fmaximum.C
avtR2Fminimum.C
avtR2Foperator.C
avtR2Fpdf.C
avtR2Frms.C
avtR2Fstddev.C
avtR2Fsum.C
avtR2Fvariance.C
avtUniformBinningScheme.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtdbin_ser ${AVTDBIN_SOURCES})
TARGET_LINK_LIBRARIES(avtdbin_ser visitcommon avtdbatts avtmath avtpipeline_ser)
VISIT_INSTALL_TARGETS(avtdbin_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtdbin_par ${AVTDBIN_SOURCES})
    TARGET_LINK_LIBRARIES(avtdbin_par visitcommon avtdbatts avtmath avtpipeline_par)
    VISIT_INSTALL_TARGETS(avtdbin_par)
ENDIF(VISIT_PARALLEL)
