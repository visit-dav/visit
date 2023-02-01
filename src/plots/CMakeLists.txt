# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Kathleen Biagas, Mon May 12 12:59:25 PDT 2014
#    Prefer VISIT_SELECTED_PLOT_PLUGINS if defined.  Test for existence of
#    subdir before adding it.
#
#****************************************************************************

include(${VISIT_SOURCE_DIR}/CMake/PluginMacros.cmake)

IF(NOT WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISIT_PLUGIN_DIR}/plots)
ELSE(NOT WIN32)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISIT_PLUGIN_DIR}/plots)
ENDIF(NOT WIN32)

SET(REQUIRED_PLOT_PLUGINS
Boundary
Contour
Curve
FilledBoundary
Histogram
Label
Mesh
Molecule
MultiCurve
ParallelCoordinates
Pseudocolor
Scatter
Spreadsheet
Subset
Surface
Tensor
Truecolor
Vector
WellBore
)

# Changes in Volume plot for VTK9 are too significant
# to handle within the source files with conditional compilations.
# Especially for the changes to the enums in the atts.
# So just use a different directory
if(VTK_VERSION VERSION_LESS_EQUAL "8.1.0")
    list(APPEND REQUIRED_PLOT_PLUGINS Volume)
else()
    list(APPEND REQUIRED_PLOT_PLUGINS VolumeVTK9)
endif()

SET(OPTIONAL_PLOT_PLUGINS
Topology
)

SET(MINIMAL_PLOT_PLUGINS
Curve
Mesh
Pseudocolor
)

# Select the plugins to build
IF(DEFINED VISIT_SELECTED_PLOT_PLUGINS)
    SET(BUILD_PLUGINS ${VISIT_SELECTED_PLOT_PLUGINS})
ELSEIF(VISIT_BUILD_ALL_PLUGINS)
    SET(BUILD_PLUGINS ${REQUIRED_PLOT_PLUGINS} ${OPTIONAL_PLOT_PLUGINS})
ELSE()
    IF(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${MINIMAL_PLOT_PLUGINS})
    ELSE(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${REQUIRED_PLOT_PLUGINS})
    ENDIF(VISIT_BUILD_MINIMAL_PLUGINS)
ENDIF()

# Add the plugin directories
FOREACH(plot ${BUILD_PLUGINS})
    # (EXISTS test requires full path)
    IF(EXISTS ${VISIT_SOURCE_DIR}/plots/${plot})
        ADD_SUBDIRECTORY(${plot})
    ELSE()
        IF(DEFINED VISIT_SELECTED_PLOT_PLUGINS)
            Message(FATAL_ERROR "Selected plot (${plot}) does not exist.")
        ELSEIF(VISIT_STATIC)
            # for code below, remove anything we aren't actually building
            Message("${plot} removed from build because it does not exist")
            LIST(REMOVE_ITEM BUILD_PLUGINS ${plot})
        ENDIF()
    ENDIF()
ENDFOREACH(plot)

# If we're building statically, make various main programs depend on plugins
# being built
IF(VISIT_STATIC)
    # Write a file containing the list of plugins such that we can include it
    # into C++ code.
    FOREACH(plot ${BUILD_PLUGINS})
        SET(plotCode "${plotCode}PLUGIN_VERB(${plot})\n")
    ENDFOREACH(plot)
    FILE(WRITE ${VISIT_BINARY_DIR}/include/enabled_plots.h "${plotCode}")

    IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
        CREATE_PLUGIN_DEPENDENCIES(gui_exe "I" "Plot" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(gui_exe "G" "Plot" ${BUILD_PLUGINS})

        CREATE_PLUGIN_DEPENDENCIES(viewer_exe "I" "Plot" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(viewer_exe "V" "Plot" ${BUILD_PLUGINS})

        IF(VISIT_PYTHON_SCRIPTING)
            CREATE_PLUGIN_DEPENDENCIES(cli "I" "Plot" ${BUILD_PLUGINS})
            CREATE_PLUGIN_DEPENDENCIES(cli "S" "Plot" ${BUILD_PLUGINS})
        ENDIF(VISIT_PYTHON_SCRIPTING)
    ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)

    CREATE_PLUGIN_DEPENDENCIES(engine_ser_exe "I" "Plot" ${BUILD_PLUGINS})
    CREATE_PLUGIN_DEPENDENCIES(engine_ser_exe "E" "Plot_ser" ${BUILD_PLUGINS})

    IF(VISIT_PARALLEL)
        CREATE_PLUGIN_DEPENDENCIES(engine_par_exe "I" "Plot" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(engine_par_exe "E" "Plot_par" ${BUILD_PLUGINS})
    ENDIF(VISIT_PARALLEL)
ENDIF(VISIT_STATIC)
