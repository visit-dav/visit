# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Cyrus Harrison,Tue Feb  2 16:35:18 PST 2010
#    Added the python filter query.
#
#    Cyrus Harrison, Wed Mar 10 14:28:33 PST 2010   
#    Use proper python include path ${PYTHON_INCLUDE_PATH}, instead of 
#    ${PYTHON_INCLUDE_DIRS}
#
#    Eric Brugger, Wed Jun 30 13:37:54 PDT 2010
#    Added avtXRayQuery.
#
#    Hank Childs, Thu May 12 15:37:21 PDT 2011
#    Add avtAverageValueQuery.
#
#    Kevin Griffin, Thu Aug 11 10:53:13 PDT 2016
#    Add avtGyRadiusQuery.
#
#    Alister Maguire, Tue Sep 24 13:46:56 MST 2019
#    Added avtDirectDatabaseQOTFilter and avtTimeLoopQOTFilter.
#
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#
#    Cyrus Harrison, Mon May  9 14:01:56 PDT 2022
#    Added logic to use and link conduit in queries
#
#    Kathleen Biagas, Fri Sep 9, 2022
#    Link with 'rt' on Linux.
#
#    Kathleen Biagas, Tue Dec 13, 2022
#    * Moved setting of sources to subdirs.
#    * Changed use of 'link_directories' to 'target_link_directories' for
#    CONDUIT_LIBRARY_DIR. 
#    * Removed use of PYTHON_LIBRARY_DIRS, no longer defined.
#
#****************************************************************************

add_subdirectory(Abstract)
add_subdirectory(Misc)
add_subdirectory(Pick)
add_subdirectory(Queries)

set(avtquery_ser_DEPENDS visitcommon avtmath avtshapelets visit_vtk)

if(CONDUIT_FOUND)
    # we will use conduit in queries
    list(APPEND avtquery_ser_DEPENDS conduit)
endif()


if(NOT WIN32 AND NOT APPLE)
    find_library(LIBRT rt)
    list(APPEND avtquery_ser_DEPENDS ${LIBRT})
endif()

if(VISIT_PYTHON_FILTERS)
    visit_append_list(
        NAME avtquery_ser_INCLUDES
        ITEMS $<BUILD_INTERFACE:>${PYTHON_INCLUDE_PATH})
endif()


set(avtquery_par_SOURCES  ${avtquery_ser_SOURCES})
set(avtquery_par_HEADERS  ${avtquery_ser_HEADERS})
set(avtquery_par_INCLUDES ${avtquery_ser_INCLUDES})
set(avtquery_par_DEPENDS  ${avtquery_ser_DEPENDS})

visit_add_library(
    NAME       avtquery_ser
    SOURCES    ${avtquery_ser_SOURCES}
    HEADERS    ${avtquery_ser_HEADERS}
    INCLUDES   ${avtquery_ser_INCLUDES}
    DEPENDS_ON ${avtquery_ser_DEPENDS}
               avtexpressions_ser
               avtfilters_ser)


if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtquery_par
        SOURCES    ${avtquery_par_SOURCES}
        HEADERS    ${avtquery_par_HEADERS}
        INCLUDES   ${avtquery_par_INCLUDES}
        DEPENDS_ON ${avtquery_par_DEPENDS}
                   avtexpressions_par
                   avtfilters_par)
        if(VISIT_PYTHON_FILTERS)
            target_link_libraries(avtquery_par PUBLIC avtpythonfilters_par)
        endif()
        if(CONDUIT_FOUND)
            target_link_libraries(avtquery_par PUBLIC conduit_mpi)
        endif()
endif()






