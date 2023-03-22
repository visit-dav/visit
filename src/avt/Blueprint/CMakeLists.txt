# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
#
# Modifications:
#
#****************************************************************************

set(AVTBLUEPRINT_SOURCES
    avtConduitBlueprintDataAdaptor.C
    avtConduitBlueprintInfoWarningHandler.C
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${VISIT_COMMON_INCLUDES}
                    ${VISIT_SOURCE_DIR}/visit_vtk/full
                    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
                    ${VTK_INCLUDE_DIRS}
                    ${CONDUIT_INCLUDE_DIR}
                    ${MFEM_INCLUDE_DIR}
                    )

# Add link directories
link_directories(${LIBRARY_OUTPUT_DIRECTORY} ${CONDUIT_LIBRARY_DIR} ${MFEM_LIBRARY_DIR})

add_library(avtblueprint ${AVTBLUEPRINT_SOURCES})

target_link_libraries(avtblueprint visitcommon ${CONDUIT_LIB} ${MFEM_LIB} visit_vtk vtkCommonCore vtkFiltersCore)

VISIT_INSTALL_TARGETS(avtblueprint)
