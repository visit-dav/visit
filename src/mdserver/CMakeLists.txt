# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

add_subdirectory(rpc)
add_subdirectory(proxy)
if(NOT VISIT_STATIC)
    # "main" is handled from the top level in the static case so it can be
    # created after we know which plugins are in use. This is needed for
    # static linking.
    add_subdirectory(main)
endif()

