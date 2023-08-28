# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(visitcommon PRIVATE
    AbortException.C
    BadCellException.C
    BadDomainException.C
    BadIndexException.C
    BadNodeException.C
    BadVectorException.C
    ExpressionException.C
    GhostCellException.C
    ImproperUseException.C
    IncompatibleDomainListsException.C
    IntervalTreeNotCalculatedException.C
    InvalidCategoryException.C
    InvalidCellTypeException.C
    InvalidDimensionsException.C
    InvalidLimitsException.C
    InvalidMergeException.C
    InvalidSetException.C
    InvalidConversionException.C
    LogicalIndexException.C
    NoCurveException.C
    NoDefaultVariableException.C
    NoInputException.C
    NonQueryableInputException.C
    QueryArgumentException.C
    StubReferencedException.C
    UnexpectedValueException.C)

target_include_directories(visitcommon PUBLIC .)

