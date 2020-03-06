# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Remove if test that is handled by the parent dir's CMakeLists.txt
#  (VISIT_DBIO_ONLY, VISIT_ENGINE_ONLY, AND VISIT_SERVER_COMPONENTS_ONLY),
#  VISIT_ENABLE_AVTEXAMPLES.
#
#****************************************************************************

macro(VISIT_EXAMPLES_ADD_FOLDER type)
    if(${ARGC} GREATER 1)
        set_target_properties(${ARGN} PROPERTIES FOLDER tools/examples/${type})
    else()
        set_target_properties(${type} PROPERTIES FOLDER tools/examples/${type})
    endif()
endmacro()

if (NOT VISIT_STATIC)
    add_subdirectory(avtexamples)
    
    if(VISIT_PYTHON_SCRIPTING)
        add_subdirectory(dataserver)
    endif()
endif()

add_subdirectory(embedviewer)

if(NOT WIN32)
    add_subdirectory(mcurvit)
endif()
