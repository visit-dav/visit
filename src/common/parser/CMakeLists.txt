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
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/ConfiguratingItem.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ConfiguratingSet.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Dictionary.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Grammar.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Parser.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ParseTreeNode.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Rule.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Scanner.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Sequence.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Symbol.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Token.C)

visit_append_list(
    NAME  visitcommon_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/Colors.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ConfiguratingItem.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ConfiguratingSet.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Dictionary.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ParseException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/parser_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ParseTreeNode.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Pos.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Rule.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Scanner.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Sequence.h
          ${CMAKE_CURRENT_SOURCE_DIR}/State.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Symbol.h
          ${CMAKE_CURRENT_SOURCE_DIR}/SymbolSet.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItGrammar.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItParser.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItToken.h)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/parser>)

