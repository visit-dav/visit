# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(visitcommon PRIVATE
  ExprGrammar.C
  ExprInit.C
  ExprNode.C
  ExprParser.C
  ExprParseTreeNode.C
  ExprScanner.C
  InvalidExpressionException.C
  ParsingExprList.C
  RecursiveExpressionException.C)

target_include_directories(visitcommon PUBLIC . )

if(VISIT_ENABLE_UNIT_TESTS AND NOT WIN32)
    add_executable(exprconfig ExprConfig.C)
    target_link_libraries(exprconfig visitcommon)
    if(DEFINED VISIT_EXE_LINKER_FLAGS)
        target_link_options(exprconfig ${VISIT_EXE_LINKER_FLAGS})
    endif()

    add_executable(exprtest test.C)
    target_link_libraries(exprtest visitcommon)
    if(DEFINED VISIT_EXE_LINKER_FLAGS)
        target_link_options(exprtest ${VISIT_EXE_LINKER_FLAGS})
    endif()

    add_custom_target(init)
    add_dependencies(init exprconfig)
    add_custom_command(TARGET init POST_BUILD COMMAND exprconfig ExprInit.C
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

