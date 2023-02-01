# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(avtmir_ser PRIVATE DiscreteMIR.C)
target_include_directories(avtmir_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtmir_par PRIVATE DiscreteMIR.C)
    target_include_directories(avtmir_par PUBLIC .)
endif()

