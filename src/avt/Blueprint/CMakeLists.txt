# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
#
# Modifications:
#    Justin Privitera, Wed Mar 22 16:09:52 PDT 2023
#    Added visit_vtk to include dirs and target link libs.
#
#    Ryan Krattiger, Fri Oct 31 13:37:05 CST 2025
#    Blueprints with optional MFEM dependency
#
#****************************************************************************

set(AVTBLUEPRINT_SOURCES
    avtConduitBlueprintDataAdaptor.C
    avtConduitBlueprintInfoWarningHandler.C
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${VISIT_COMMON_INCLUDES}
                    ${VISIT_SOURCE_DIR}/avt/MFEM
                    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
                    ${VTK_INCLUDE_DIRS}
                    ${CONDUIT_INCLUDE_DIR}
                    ${MFEM_INCLUDE_DIR}
                    )

# Add link directories
link_directories(${LIBRARY_OUTPUT_DIRECTORY} ${CONDUIT_LIBRARY_DIR})

add_library(avtblueprint ${AVTBLUEPRINT_SOURCES})

set(vtklibs lightweight_visit_vtk VTK::CommonCore VTK::FiltersCore)

target_link_libraries(avtblueprint visitcommon ${CONDUIT_LIB} ${vtklibs})
if (MFEM_FOUND)
    target_link_libraries(avtblueprint avtmfem ${MFEM_LIB})
    link_directories(${LIBRARY_OUTPUT_DIRECTORY} ${MFEM_LIBRARY_DIR})
endif ()
target_include_directories(avtblueprint PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

VISIT_INSTALL_TARGETS(avtblueprint)
