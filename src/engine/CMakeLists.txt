# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

add_subdirectory(rpc)

if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    add_subdirectory(proxy)
endif()

if(NOT VISIT_STATIC)
    # "main" is handled from the top level in the static case so it can be
    # created after we know which plugins are in use. This is needed for
    # static linking.
    add_subdirectory(main)
endif()

