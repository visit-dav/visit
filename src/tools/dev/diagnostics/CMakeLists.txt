# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

if(VISIT_PARALLEL)
    add_subdirectory(mpitest)
endif()

add_subdirectory(networktest)
add_subdirectory(exceptiontest)

if(HAVE_OSMESA)
    add_subdirectory(osmesatest)
endif()
