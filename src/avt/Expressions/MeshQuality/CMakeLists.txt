# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCornerAngle.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEdgeLength.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFacePlanarity.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinCornerArea.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinSinCorner.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNeighborExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNodeDegreeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRevolvedSurfaceArea.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRevolvedVolume.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSideVolume.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVerdictExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricArea.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricAspectGamma.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricAspectRatio.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricCondition.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricDiagonalRatio.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricDimension.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricJacobian.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMaxDiagonal.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMaximumAngle.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMinDiagonal.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMinimumAngle.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricOddy.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricRelativeSize.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricScaledJacobian.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricShape.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricShapeAndSize.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricShear.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricSkew.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricStretch.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricTaper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricVolume.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricWarpage.C)

visit_append_list(
    NAME  avtexpressions_ser_Headers
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCornerAngle.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEdgeLength.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFacePlanarity.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinCornerArea.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinSinCorner.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNeighborExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNodeDegreeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRevolvedSurfaceArea.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRevolvedVolume.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSideVolume.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVerdictExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricArea.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricAspectGamma.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricAspectRatio.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricCondition.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricDiagonalRatio.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricDimension.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricJacobian.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMaxDiagonal.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMaximumAngle.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMinDiagonal.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricMinimumAngle.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricOddy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricRelativeSize.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetrics.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricScaledJacobian.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricShape.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricShapeAndSize.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricShear.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricSkew.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricStretch.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricTaper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricVolume.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVMetricWarpage.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/MeshQuality>)

