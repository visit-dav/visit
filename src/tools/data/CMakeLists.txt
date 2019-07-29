# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

macro(VISIT_DATA_ADD_FOLDER type)
    if(${ARGC} GREATER 1)
        set_target_properties(${ARGN} PROPERTIES FOLDER tools/data/${type})
    else()
        set_target_properties(${type} PROPERTIES FOLDER tools/data/${type})
    endif()
endmacro()


if(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
    add_subdirectory(convert)
else()
    add_subdirectory(datagen)
    # We always want these to be built
    if(SILO_FOUND)
        add_subdirectory(mrgtree2dot)
        if(NOT WIN32)
            add_subdirectory(add_visit_searchpath)
        endif()
    endif()
    add_subdirectory(convert)
    if(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
        add_subdirectory(writer)
        add_subdirectory(ultrawrapper)
    endif()
    if(HDF5_FOUND AND NOT WIN32)
        add_subdirectory(xdmf_writer)
    endif()
    add_subdirectory(DataManualExamples)
endif()

