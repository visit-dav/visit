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

IF(VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(DBAtts)
    ADD_SUBDIRECTORY(Pipeline)
    ADD_SUBDIRECTORY(Database)
ELSE(VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(DBAtts)
    ADD_SUBDIRECTORY(Math)
    ADD_SUBDIRECTORY(IVP)
    ADD_SUBDIRECTORY(Pipeline)
    ADD_SUBDIRECTORY(DataBinning)
    ADD_SUBDIRECTORY(MIR)
    ADD_SUBDIRECTORY(Shapelets)
    ADD_SUBDIRECTORY(View)
    ADD_SUBDIRECTORY(Filters)
    ADD_SUBDIRECTORY(Database)
    ADD_SUBDIRECTORY(Plotter)
    ADD_SUBDIRECTORY(Expressions)
    ADD_SUBDIRECTORY(Queries)
    ADD_SUBDIRECTORY(FileWriter)
    ADD_SUBDIRECTORY(VisWindow)
    IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
        ADD_SUBDIRECTORY(QtVisWindow)
    ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
    IF(VISIT_PYTHON_FILTERS)
        ADD_SUBDIRECTORY(PythonFilters)
    ENDIF(VISIT_PYTHON_FILTERS)

    if(MFEM_FOUND)
        add_subdirectory(MFEM)
    endif()
    if(CONDUIT_FOUND AND MFEM_FOUND)
        add_subdirectory(Blueprint)
    endif()

ENDIF(VISIT_DBIO_ONLY)
