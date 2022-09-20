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
                    ${VTK_INCLUDE_DIRS}
                    ${CONDUIT_INCLUDE_DIR}
                    ${MFEM_INCLUDE_DIR}
                    )

# Add link directories
link_directories(${LIBRARY_OUTPUT_DIRECTORY} ${CONDUIT_LIBRARY_DIR} ${MFEM_LIBRARY_DIR})

add_library(avtblueprint ${AVTBLUEPRINT_SOURCES})

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonCore vtkFiltersCore)
else()
    set(vtklibs VTK::CommonCore VTK::FiltersCore)
endif()

target_link_libraries(avtblueprint visitcommon ${CONDUIT_LIB} ${MFEM_LIB} ${vtklibs}) 

VISIT_INSTALL_TARGETS(avtblueprint)
