# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jun 2 15:11:22 PST 2010
#   Disable MakeMili on windows.
#
#   Kathleen Bonnell, Fri Jun  4 14:46:56 PDT 2010
#   Change ${MILI_FOUND} to MILI_FOUND.
#
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Cyrus Harrison, Fri Apr 29 15:33:38 PDT 2011
#   Added mrgtree2dot utility.
#
#   Brad Whitlock, Thu Oct 20 12:38:14 PDT 2011
#   Do not build writer when we're doing a static build.
#
#   Brad Whitlock, Wed May 23 17:10:05 PDT 2012
#   Add usedataserver example program.
#
#   Eric Brugger, Wed Dec  5 13:17:20 PST 2012
#   Add xmf_writer.
#
#   Eric Brugger, Wed Apr 17 12:00:47 PDT 2013
#   Also build qtssh on unix.
#
#   Eric Brugger, Wed Dec 17 12:37:22 PST 2014
#   Add avtexamples.
#
#   Kathleen Biagas, Dec 22, 2014
#   Add ability to turn off building of avt examples.
#
#   Eric Brugger, Thu Oct  1 08:34:37 PDT 2015
#   Add add_visit_searchpath.
#
#   Kathleen Biagas, Thu Feb 27 2020
#   Restructured logic and only add examples if VISIT_ENABLE_EXAMPLES is
#   true.
#
#****************************************************************************

MACRO(VISIT_TOOLS_ADD_FOLDER type)
    IF(${ARGC} GREATER 1)
        SET_TARGET_PROPERTIES(${ARGN} PROPERTIES FOLDER tools/${type})
    ELSE(${ARGC} GREATER 1)
        SET_TARGET_PROPERTIES(${type} PROPERTIES FOLDER tools/${type})
    ENDIF(${ARGC} GREATER 1)
ENDMACRO(VISIT_TOOLS_ADD_FOLDER)

ADD_SUBDIRECTORY(data)

IF(NOT (VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY))
    ADD_SUBDIRECTORY(dev)
    if(VISIT_ENABLE_EXAMPLES)
        ADD_SUBDIRECTORY(examples)
    endif()
    ADD_SUBDIRECTORY(fx)
    ADD_SUBDIRECTORY(third_party)
ENDIF()
