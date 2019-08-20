# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

IF(VISIT_BUILD_DIAGNOSTICS)
    ADD_SUBDIRECTORY(diagnostics) # dev
ENDIF()

IF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
ELSE(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
    IF(NOT WIN32)
        ADD_SUBDIRECTORY(clipeditor) # dev
    ENDIF(NOT WIN32)
    ADD_SUBDIRECTORY(protocol) # dev
    ADD_SUBDIRECTORY(xml) # dev
    ADD_SUBDIRECTORY(xmledit) # dev
ENDIF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)

# These do not need to be built:
# avt_do_add
# doxygenate
# escan
# vtk_debug_parser
