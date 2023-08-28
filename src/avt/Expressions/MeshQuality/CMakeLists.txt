# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(MESHQUALITY_SOURCES
    avtCornerAngle.C
    avtEdgeLength.C
    avtFacePlanarity.C
    avtMinCornerArea.C
    avtMinSinCorner.C
    avtNeighborExpression.C
    avtNodeDegreeExpression.C
    avtRevolvedSurfaceArea.C
    avtRevolvedVolume.C
    avtSideVolume.C
    avtVerdictExpression.C
    avtVMetricArea.C
    avtVMetricAspectGamma.C
    avtVMetricAspectRatio.C
    avtVMetricCondition.C
    avtVMetricDiagonalRatio.C
    avtVMetricDimension.C
    avtVMetricJacobian.C
    avtVMetricMaxDiagonal.C
    avtVMetricMaximumAngle.C
    avtVMetricMinDiagonal.C
    avtVMetricMinimumAngle.C
    avtVMetricOddy.C
    avtVMetricRelativeSize.C
    avtVMetricScaledJacobian.C
    avtVMetricShape.C
    avtVMetricShapeAndSize.C
    avtVMetricShear.C
    avtVMetricSkew.C
    avtVMetricStretch.C
    avtVMetricTaper.C
    avtVMetricVolume.C
    avtVMetricWarpage.C)

target_sources(avtexpressions_ser PRIVATE ${MESHQUALITY_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${MESHQUALITY_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(MESHQUALITY_SOURCES)

