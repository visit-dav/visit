# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Wed Oct  8 16:08:33 PDT 2014
#   Always build rpc and core since the sim library needs them.
#
#****************************************************************************

ADD_SUBDIRECTORY(rpc)
ADD_SUBDIRECTORY(core)

IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(proxy)
    ADD_SUBDIRECTORY(subjectproxy)
    IF(NOT VISIT_STATIC)
        # "main" is handled from the top level in the static case so it can be
        # created after we know which plugins are in use. This is needed for 
        # static linking.
        ADD_SUBDIRECTORY(main)
    ENDIF(NOT VISIT_STATIC)
ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

