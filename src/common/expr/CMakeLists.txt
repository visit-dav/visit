# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
  NAME  visitcommon_SOURCES
  ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/ExprGrammar.C
        ${CMAKE_CURRENT_SOURCE_DIR}/ExprInit.C
        ${CMAKE_CURRENT_SOURCE_DIR}/ExprNode.C
        ${CMAKE_CURRENT_SOURCE_DIR}/ExprParser.C
        ${CMAKE_CURRENT_SOURCE_DIR}/ExprParseTreeNode.C
        ${CMAKE_CURRENT_SOURCE_DIR}/ExprScanner.C
        ${CMAKE_CURRENT_SOURCE_DIR}/InvalidExpressionException.C
        ${CMAKE_CURRENT_SOURCE_DIR}/ParsingExprList.C
        ${CMAKE_CURRENT_SOURCE_DIR}/RecursiveExpressionException.C)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/expr>)


if(VISIT_ENABLE_UNIT_TESTS AND NOT WIN32)
    blt_add_executable(NAME exprconfig
        SOURCES    ExprConfig.C
        DEPENDS_ON visitcommon)

    blt_add_executable(NAME exprtest
        SOURCES test.C
        DEPENDS_ON visitcommon)

    if(DEFINED VISIT_EXE_LINKER_FLAGS)
        blt_patch_target(NAME exprconfig LINK_FLAGS ${VISIT_EXE_LINKER_FLAGS})
        blt_patch_target(NAME exprtest LINK_FLAGS ${VISIT_EXE_LINKER_FLAGS})
    endif()

    add_custom_target(init)
    add_dependencies(init exprconfig)
    add_custom_command(TARGET init POST_BUILD COMMAND exprconfig ExprInit.C
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

