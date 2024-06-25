# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#  Cyrus Harrison, Tue Aug 10 10:39:08 PDT 2010
#   Added rms operator.
#
#****************************************************************************

set(avtdbin_SOURCES
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
    avtUniformBinningScheme.C)

set(avtdbin_HEADERS
    avtBinningScheme.h
    avtDataBinning.h
    avtDataBinningConstructor.h
    avtDataBinningFunctionInfo.h
    avtR2Faverage.h
    avtR2Fcount.h
    avtR2Fmaximum.h
    avtR2Fminimum.h
    avtR2Foperator.h
    avtR2Fpdf.h
    avtR2Frms.h
    avtR2Fstddev.h
    avtR2Fsum.h
    avtR2Fvariance.h
    avtUniformBinningScheme.h
    dbin_exports.h)

set(avtdbin_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/DataBinning>)

set(avtdbin_DEPENDS
    visitcommon
    avtdbatts
    avtmath)

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtdbin_ser
    SOURCES    ${avtdbin_SOURCES}
    HEADERS    ${avtdbin_HEADERS}
    INCLUDES   ${avtdbin_INCLUDES}
    DEPENDS_ON ${avtdbin_DEPENDS}
               avtpipeline_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtdbin_par
        SOURCES    ${avtdbin_SOURCES}
        HEADERS    ${avtdbin_HEADERS}
        INCLUDES   ${avtdbin_INCLUDES}
        DEPENDS_ON ${avtdbin_DEPENDS}
                   avtpipeline_par)
endif()

