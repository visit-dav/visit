# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(visitcommon PRIVATE
    InvalidColortableException.C
    LibraryNotFoundException.C)

target_include_directories(visitcommon PUBLIC .)

