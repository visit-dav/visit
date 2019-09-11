# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

macro(VISIT_FX_ADD_FOLDER type)
    if(${ARGC} GREATER 1)
        set_target_properties(${ARGN} PROPERTIES FOLDER tools/fx/${type})
    else()
        set_target_properties(${type} PROPERTIES FOLDER tools/fx/${type})
    endif()
endmacro()


IF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
ELSE(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
    ADD_SUBDIRECTORY(annotations) # fx
    ADD_SUBDIRECTORY(imagetools) # fx
ENDIF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)

# These do not need to be built:
# antialiased_shadows
