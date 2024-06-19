# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCurvatureExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDisplacementExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEffectiveTensorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalizedCompactnessExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPrincipalDeviatoricTensorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPrincipalTensorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRelativeVolumeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainAlmansiExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainGreenLagrangeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainInfinitesimalExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainRateExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainVolumetricExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorMaximumShearExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCurvatureExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDisplacementExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEffectiveTensorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalizedCompactnessExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPrincipalDeviatoricTensorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPrincipalTensorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRelativeVolumeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainAlmansiExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainGreenLagrangeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainInfinitesimalExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainRateExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStrainVolumetricExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorMaximumShearExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/Derivations>)

