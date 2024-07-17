# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Hank Childs, Sat Aug 21 11:53:13 PDT 2010
#   Rename directory DDF to DataBinning.
#
#   Brad Whitlock, Thu Jan 20 08:48:38 PST 2011
#   Do not build QtVisWindow on engine-only or server-components-only builds.
#
#****************************************************************************

if(VISIT_DBIO_ONLY)
    add_subdirectory(DBAtts)
    add_subdirectory(Pipeline)
    add_subdirectory(Database)
else()
    add_subdirectory(DBAtts)
    add_subdirectory(Math)
    add_subdirectory(IVP)
    add_subdirectory(Pipeline)
    add_subdirectory(DataBinning)
    add_subdirectory(MIR)
    add_subdirectory(Shapelets)
    add_subdirectory(View)
    add_subdirectory(Filters)
    add_subdirectory(Database)
    add_subdirectory(Plotter)
    add_subdirectory(Expressions)
    add_subdirectory(Queries)
    add_subdirectory(FileWriter)
    add_subdirectory(VisWindow)

    if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
        add_subdirectory(QtVisWindow)
    endif()

    if(VISIT_PYTHON_FILTERS)
        add_subdirectory(PythonFilters)
    endif()

    if(MFEM_FOUND)
        add_subdirectory(MFEM)
    endif()

    if(CONDUIT_FOUND AND MFEM_FOUND)
        add_subdirectory(Blueprint)
    endif()
endif()
