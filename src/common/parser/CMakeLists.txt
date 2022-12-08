# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(visitcommon PRIVATE
    ConfiguratingItem.C
    ConfiguratingSet.C
    Dictionary.C
    Grammar.C
    Parser.C
    ParseTreeNode.C
    Rule.C
    Scanner.C
    Sequence.C
    Symbol.C
    Token.C)

target_include_directories(visitcommon PUBLIC .)

