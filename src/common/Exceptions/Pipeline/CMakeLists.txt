# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu May 16, 2024
#   Using blt, so create lists for sources, headers, includes.
#
#****************************************************************************

visit_append_list(
    NAME  visitcommon_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/AbortException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadCellException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadDomainException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadIndexException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadNodeException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadVectorException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ExpressionException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/GhostCellException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ImproperUseException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/IncompatibleDomainListsException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/IntervalTreeNotCalculatedException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidCategoryException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidCellTypeException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidConversionException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDimensionsException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidLimitsException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidMergeException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidSetException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/LogicalIndexException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NoCurveException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NoDefaultVariableException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NoInputException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NonQueryableInputException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/QueryArgumentException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/StubReferencedException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/UnexpectedValueException.C)

visit_append_list(
    NAME  visitcommon_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/AbortException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtexception_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadCellException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadDomainException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadIndexException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadNodeException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadVectorException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ExpressionException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/GhostCellException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ImproperUseException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/IncompatibleDomainListsException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/IntervalTreeNotCalculatedException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidCategoryException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidCellTypeException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidConversionException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDimensionsException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidLimitsException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidMergeException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidSetException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/LogicalIndexException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NoCurveException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NoDefaultVariableException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NoInputException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NonQueryableInputException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/QueryArgumentException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/StubReferencedException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/UnexpectedValueException.h)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/Exceptions/Pipeline>)

