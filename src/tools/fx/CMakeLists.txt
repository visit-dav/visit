# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Remove if test that is handled by the parent dir's CMakeLists.txt
#  (VISIT_DBIO_ONLY, VISIT_ENGINE_ONLY, AND VISIT_SERVER_COMPONENTS_ONLY)
#  Add annotations only if VISIT_ENABLE_ANNOTATION_TOOLS is true (default).
#
#****************************************************************************

macro(VISIT_FX_ADD_FOLDER type)
    if(${ARGC} GREATER 1)
        set_target_properties(${ARGN} PROPERTIES FOLDER tools/fx/${type})
    else()
        set_target_properties(${type} PROPERTIES FOLDER tools/fx/${type})
    endif()
endmacro()

if(VISIT_ENABLE_ANNOTATION_TOOLS)
    ADD_SUBDIRECTORY(annotations)
endif()

ADD_SUBDIRECTORY(imagetools)

# These do not need to be built:
# antialiased_shadows
