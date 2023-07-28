# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(avtdbatts PRIVATE
    avtSIL.C
    avtSILArray.C
    avtSILCollection.C
    avtSILEnumeratedNamespace.C
    avtSILMatrix.C
    avtSILNamespace.C 
    avtSILRangeNamespace.C
    avtSILRestriction.C
    avtSILRestrictionTraverser.C
    avtSILSet.C)

target_include_directories(avtdbatts PUBLIC .)

