# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Hank Childs, Thu Jul 22 09:55:03 PDT 2010
#   Added avtMultiWindowSaver.
#
#   Tom Fogal,  Tue Aug 10 12:55:19 MDT 2010
#   Added include directory.
#
#   Brad Whitlock, Mon Dec 13 16:13:43 PST 2010
#   I added avtNamedSelectionExtension.
#
#   Hank Childs, Tue Mar 13 12:59:07 PDT 2012
#   Move avtDatasetOnDemandFilter to Filters directory.
#
#   Kathleen Biagas, Tue May 28 17:02:54 MST 2013
#   Move avtTimeLoopFilter and avtTimeLoopCollectorFilter to Filters directory.
#
#   Cyrus Harrison, on Oct 28 14:39:05 PDT 2013
#   Link vtkCommonDataModel (it contains the vtkDataSet types)
#
#   Brad Whitlock, Mon Apr  7 15:24:19 PDT 2014
#   Link to visit_vtk.
#
#   Eric Brugger, Thu Jan 14 10:44:28 PST 2016
#   Add linking of vtkm library.
#
#   Kathleen Biagas, Tue Feb 6 14:12:32 PST 2018
#   Removed vtkRendering* from link libs, moved avtLookupTable to avt/Plotter
#   due to its dependence on vtkRendering.  Removed VTK_DEFINITIONS.
#
#   Eric Brugger, Tue Aug 28 14:35:30 PDT 2018
#   I replaced support for vtkm with vtkh.
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkCommonMath, vtkCommonTransforms in support of building plugins
#   vs install on Windows. They are direct depencencies for this library,
#   and CMake automagically pulls them in when compiling VisIt, even if not
#   listed, but not when building the plugins.
#
#   Kathleen Biagas, Tue Dec 13, 2022
#   Moved setting of sources to subdirs.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   I replaced vtkh with vtkm.
#
#   Cyrus Harrison, Thu Mar  9 11:30:26 PST 2023
#   Fixed typo in vtk-m link for avtpipeline_par
#
#****************************************************************************/

add_library(avtpipeline_ser)

if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtpipeline_par)
endif()

add_subdirectory(AbstractFilters)
add_subdirectory(Data)
add_subdirectory(Pipeline)
add_subdirectory(Sinks)
add_subdirectory(Sources)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/Database/Database
    ${VISIT_SOURCE_DIR}/avt/Database/Ghost
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${VTKm_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR})

set(vtklibs VTK::CommonCore VTK::CommonDataModel VTK::CommonMath
            VTK::CommonTransforms VTK::FiltersGeometry)

# Unless we're doing a VISIT_DBIO_ONLY build then we need avtmath
if(NOT VISIT_DBIO_ONLY)
    set(AVTMATH avtmath)
endif()

#********************************* SERIAL ************************************
target_link_libraries(avtpipeline_ser
                      visitcommon ${AVTMATH} avtdbatts
                      visit_vtk lightweight_visit_vtk ${vtklibs}
                      ${ZLIB_LIBRARY})
if(HAVE_LIBVTKM)
    target_link_libraries(avtpipeline_ser vtkm_cont vtkm_filter_contour vtkm_filter_entity_extraction)
    if(VTKm_ENABLE_KOKKOS)
        target_link_libraries(avtpipeline_ser amdhip64)
    endif()
endif()

VISIT_INSTALL_TARGETS(avtpipeline_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    target_link_libraries(avtpipeline_par
                          visitcommon ${AVTMATH} avtdbatts
                          visit_vtk lightweight_visit_vtk ${vtklibs}
                          ${ZLIB_LIBRARY})
    if(HAVE_LIBVTKM)
        target_link_libraries(avtpipeline_par vtkm_cont vtkm_filter_contour vtkm_filter_entity_extraction)
        if(VTKm_ENABLE_KOKKOS)
            target_link_libraries(avtpipeline_par amdhip64)
        endif()
    endif()
    VISIT_INSTALL_TARGETS(avtpipeline_par)
endif()

