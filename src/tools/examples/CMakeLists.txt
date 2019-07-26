# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

macro(VISIT_EXAMPLES_ADD_FOLDER type)
    if(${ARGC} GREATER 1)
        set_target_properties(${ARGN} PROPERTIES FOLDER tools/examples/${type})
    else()
        set_target_properties(${type} PROPERTIES FOLDER tools/examples/${type})
    endif()
endmacro()


IF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
ELSE(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
    IF(VISIT_BUILD_AVTEXAMPLES AND NOT VISIT_STATIC)
        ADD_SUBDIRECTORY(avtexamples)
    ENDIF(VISIT_BUILD_AVTEXAMPLES AND NOT VISIT_STATIC)
    ADD_SUBDIRECTORY(embedviewer)
    IF(NOT WIN32)
        ADD_SUBDIRECTORY(mcurvit)
    ENDIF(NOT WIN32)
    IF(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
        ADD_SUBDIRECTORY(dataserver)
    ENDIF(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
ENDIF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
