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

add_library(avtviswindow_ser)

if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtviswindow_par)
endif()

add_subdirectory(Colleagues)
add_subdirectory(Exceptions)
add_subdirectory(Interactors)
add_subdirectory(Proxies)
add_subdirectory(Tools)
add_subdirectory(VisWindow)

include_directories(
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/Database/Database
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Data
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
    ${VISIT_SOURCE_DIR}/avt/Plotter
    ${VISIT_SOURCE_DIR}/avt/Plotter/vtk
    ${VISIT_SOURCE_DIR}/avt/View
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${VTK_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR})

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkFiltersModeling vtkImagingCore vtkInteractionStyle)
    if(VISIT_OSPRAY)
        list(APPEND vtklibs vtkRenderingOSPRay)
    endif()
else()
    set(vtklibs VTK::FiltersModeling VTK::ImagingCore VTK::InteractionStyle)
endif()

#********************************* SERIAL ************************************
target_link_libraries(avtviswindow_ser visitcommon avtview avtmath
    avtplotter_ser visit_vtk lightweight_visit_vtk ${vtklibs} ${OBJC_LIBS})
VISIT_INSTALL_TARGETS(avtviswindow_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    target_link_libraries(avtviswindow_par visitcommon avtview avtmath
       avtplotter_par visit_vtk lightweight_visit_vtk ${vtklibs} ${OBJC_LIBS})
    VISIT_INSTALL_TARGETS(avtviswindow_par)
endif()

