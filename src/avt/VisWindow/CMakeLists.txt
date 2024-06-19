# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Thu Mar  1 10:56:34 PST 2012
#   I modified the CMakeList to remove the include path for the mesa header
#   files when building without Mesa. This caused a compile failure on some
#   systems because the include path was blank.
#
#   Brad Whitlock, Fri May 17 11:12:55 PDT 2013
#   Link with Objective C library on Mac.
#
#   Kathleen Biagas, Mon Jul 13 20:09:58 PDT 2015
#   Add Colleagues/avtText3DColleague.C
#
#   Eric Brugger, Fri Aug 30 11:07:45 PDT 2019
#   Add OPENGL_INCLUDE_DIR to INCLUDE_DIRECTORIES.
#
#   Kathleen Biagas, Tue Dec 13, 2022
#   Moved setting of sources to subdirs.
#
#****************************************************************************

add_subdirectory(Colleagues)
add_subdirectory(Exceptions)
add_subdirectory(Interactors)
add_subdirectory(Proxies)
add_subdirectory(Tools)
add_subdirectory(VisWindow)

set(avtviswindow_DEPENDS
    visitcommon
    avtview
    avtmath
    lightweight_visit_vtk
    visit_vtk
    VTK::FiltersModeling
    VTK::ImagingCore
    VTK::InteractionStyle
    ${OBJC_LIBS})

set(avtviswindow_par_SOURCES  ${avtviswindow_ser_SOURCES})
set(avtviswindow_par_HEADERS  ${avtviswindow_ser_HEADERS})
set(avtviswindow_par_INCLUDES ${avtviswindow_ser_INCLUDES})

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtviswindow_ser
    SOURCES    ${avtviswindow_ser_SOURCES}
    HEADERS    ${avtviswindow_ser_HEADERS}
    INCLUDES   ${avtviswindow_ser_INCLUDES}
    DEPENDS_ON ${avtviswindow_DEPENDS}
               avtplotter_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_library(
        NAME       avtviswindow_par
        SOURCES    ${avtviswindow_par_SOURCES}
        HEADERS    ${avtviswindow_par_HEADERS}
        INCLUDES   ${avtviswindow_par_INCLUDES}
        DEPENDS_ON ${avtviswindow_DEPENDS}
                   avtplotter_par)
endif()

