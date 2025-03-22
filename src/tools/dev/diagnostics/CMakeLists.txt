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


# OSMesa should always be available with VTK 9.4.1
if(HAVE_OSMESA OR VTK_VERSION VERSION_GREATER_EQUAL "9.4.1")
    add_subdirectory(osmesatest)
endif()

