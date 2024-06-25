# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_add_library(
    NAME       avtshapelets
    SOURCES    avtShapeletBasis1D.C
               avtShapeletBasis2D.C
               avtShapeletBasisSet.C
               avtShapeletDecompose.C
               avtShapeletDecompResult.C
               avtShapeletReconstruct.C
    HEADERS    avtShapeletBasis1D.h
               avtShapeletBasis2D.h
               avtShapeletBasisSet.h
               avtShapeletDecompose.h
               avtShapeletDecompResult.h
               avtShapeletReconstruct.h
               shapelets_exports.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Shapelets>
    DEPENDS_ON visitcommon
               avtmath
               VTK::CommonCore
               VTK::FiltersCore)

