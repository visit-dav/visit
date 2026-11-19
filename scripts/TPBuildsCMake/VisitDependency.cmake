# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

include_guard(GLOBAL)

# Resolve package dependencies to ExternalProject targets. A dependency is
# satisfied when its package module either found the expected artifacts below
# visit_install_root or created a target in the current build.
function(visit_resolve_dependencies)
    set(one_value_args PACKAGE OUT_VAR)
    set(multi_value_args DEPENDENCIES)
    cmake_parse_arguments(VRD
        ""
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    if(VRD_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "visit_resolve_dependencies received unexpected arguments: "
            "${VRD_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT VRD_PACKAGE)
        message(FATAL_ERROR
            "visit_resolve_dependencies requires PACKAGE")
    endif()
    if(NOT VRD_OUT_VAR)
        message(FATAL_ERROR
            "visit_resolve_dependencies requires OUT_VAR")
    endif()

    set(build_dependencies)
    set(disabled_dependencies)
    set(ordering_dependencies)

    foreach(dependency IN LISTS VRD_DEPENDENCIES)
        string(TOUPPER "${dependency}" dependency)
        set(prebuilt_var "${dependency}_PREBUILT")
        set(enable_var "ENABLE_${dependency}")

        if(DEFINED ${prebuilt_var} AND ${prebuilt_var})
            message(STATUS
                "${VRD_PACKAGE} dependency ${dependency}: using installation "
                "under ${visit_install_root}")
        elseif(TARGET ${dependency})
            message(STATUS
                "${VRD_PACKAGE} dependency ${dependency}: using current build target")
            list(APPEND build_dependencies ${dependency})
        elseif(DEFINED ${enable_var} AND ${enable_var})
            list(APPEND ordering_dependencies ${dependency})
        else()
            list(APPEND disabled_dependencies ${dependency})
        endif()
    endforeach()

    if(disabled_dependencies OR ordering_dependencies)
        set(error_message
            "Cannot configure ${VRD_PACKAGE}; required dependencies are unavailable.")

        if(disabled_dependencies)
            string(APPEND error_message
                "\n  Disabled or not installed: ${disabled_dependencies}"
                "\n  Enable each dependency with -DENABLE_<PACKAGE>=ON, or install"
                " its expected versioned artifacts below ${visit_install_root}.")
        endif()

        if(ordering_dependencies)
            string(APPEND error_message
                "\n  Enabled but target not yet defined: ${ordering_dependencies}"
                "\n  Ensure each dependency's package.cmake file is included before"
                " ${VRD_PACKAGE}'s package.cmake file.")
        endif()

        message(FATAL_ERROR "${error_message}")
    endif()

    set(${VRD_OUT_VAR} "${build_dependencies}" PARENT_SCOPE)
endfunction()
