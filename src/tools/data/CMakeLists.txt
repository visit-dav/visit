# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Remove if test that is handled by the parent dir's CMakeLists.txt
#  (VISIT_DBIO_ONLY, VISIT_ENGINE_ONLY, AND VISIT_SERVER_COMPONENTS_ONLY)
#  Only add datagen if VISIT_ENABLE_DATAGEN is true.
#  Only add mrgtree2dot and add_visit_searchpath if VISIT_ENABLE_SILO_TOOLS
#  is true.
#  Only add DataManualExamples if VISIT_ENABLE_DATA_MANUAL_EXAMPLES is
#  true.
#
#****************************************************************************

macro(VISIT_DATA_ADD_FOLDER type)
    if(${ARGC} GREATER 1)
        set_target_properties(${ARGN} PROPERTIES FOLDER tools/data/${type})
    else()
        set_target_properties(${type} PROPERTIES FOLDER tools/data/${type})
    endif()
endmacro()

add_subdirectory(convert)

if(VISIT_ENABLE_DATAGEN)
    add_subdirectory(datagen)
endif()

if(SILO_FOUND AND VISIT_ENABLE_SILO_TOOLS)
    add_subdirectory(mrgtree2dot)
    if(NOT WIN32)
        add_subdirectory(add_visit_searchpath)
    endif()
endif()

if(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
    add_subdirectory(writer)
    add_subdirectory(ultrawrapper)
endif()

if(HDF5_FOUND AND NOT WIN32)
    add_subdirectory(xdmf_writer)
endif()

if(VISIT_ENABLE_DATA_MANUAL_EXAMPLES)
    add_subdirectory(DataManualExamples)
endif()

