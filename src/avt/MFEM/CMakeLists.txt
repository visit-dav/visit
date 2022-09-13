# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

set(AVTMFEM_SOURCES
    avtMFEMDataAdaptor.C
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${VISIT_COMMON_INCLUDES}
                    ${VTK_INCLUDE_DIRS}
                    ${MFEM_INCLUDE_DIR}
                    )

# Add link directories
link_directories(${LIBRARY_OUTPUT_DIRECTORY} ${MFEM_LIBRARY_DIR} )

add_library(avtmfem ${AVTMFEM_SOURCES})

target_link_libraries(avtmfem visitcommon ${MFEM_LIB} vtkCommonCore vtkFiltersCore)

VISIT_INSTALL_TARGETS(avtmfem)
