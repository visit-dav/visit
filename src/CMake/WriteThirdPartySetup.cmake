# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Helper functions for writing a Setup<LIBRARY>.cmake file for complex
# libraries like VTK for which we cannot use 'visit_import_library'.
#
# The Setup<LIBRARY>.cmake should contain enough information for satisfying
# VisIt's dependencies on the library when building a plugin against
# an installed version of VisIt.
#
# The Setup<LIBRARY>.cmake file will be installed in the 'cmake' subdirectory
# alongside visitConfig.cmake, etc and will be 'included' by the
# 'FindDependencies.cmake' file.
#
# Modifications:
#   Kathleen Biagas, Thu Aug 20, 2026
#   Reworked logic so that an endless query of configuration-specific target
#   properties does not need to performed.  Instead, use CMake generator
#   expressions like $<TARGET_FILE_NAME>, written to a .cmake.in file that
#   then gets 'generated' by CMake when the expression can be resolved.
#   No longer write config-specific target properties to the Setup files.
#   Update/add function header comments where needed.
#
#****************************************************************************


# Test whether a target property is explicitly set.
#
# Arguments:
#   is_set      Output variable set to true or false.
#   prop        Target property name to test.
#   targetName  Target to inspect.
macro(is_prop_set is_set prop targetName)
    if(TARGET ${targetName})
        get_property(${is_set} TARGET ${targetName} PROPERTY ${prop} SET)
    else()
        message(WARNING "attempting to test if property is set on non existent target ${targetName}")
        set(${is_set} false)
    endif()
endmacro()

# Write a target property entry to a generated set_target_properties() block.
#
# Arguments:
#   fname       Setup file being written.
#   ptype       Target property name to copy.
#   targetName  Target to inspect.
function(write_prop_if_set fname ptype targetName)
    is_prop_set(propIsSet ${ptype} ${targetName})
    if(propIsSet)
        get_target_property(prop ${targetName} ${ptype})
        file(APPEND ${fname} "    ${ptype} \"${prop}\"\n")
    endif()
endfunction()

# Decide whether a target needs a set_target_properties() block.
#
# Arguments:
#   has_props   Output variable set to true when the target has properties or
#               platform artifact locations that should be written.
#   targetName  Target to inspect.
macro(has_props_we_care_about has_props targetName)

    set(props
        INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_LINK_LIBRARIES
        INTERFACE_COMPILE_OPTIONS
        INTERFACE_COMPILE_FEATURES
        INTERFACE_COMPILE_DEFINITIONS)
    foreach(item ${props})
        is_prop_set(hasit ${item} ${targetName})
        if(${hasit})
            break()
        endif()
    endforeach()
    get_target_property(ttype ${targetName} TYPE)
    if(NOT ${ttype} STREQUAL "INTERFACE_LIBRARY")
        set(hasit true)
    endif()
    set(${has_props} ${hasit})
endmacro()

# Write one imported artifact location property.
#
# Arguments:
#   fname            Setup file being written.
#   libType          Imported property to write, such as IMPORTED_LOCATION.
#   targetName       Target whose artifact name should be used.
#   prefix           Setup-file prefix variable, such as VTK_PREFIX.
#   libdir           Installed subdirectory below prefix; empty means prefix.
#   targetFileGenex  TARGET_*_FILE_NAME generator expression selector.
function(write_lib_location_type fname libType targetName prefix libdir targetFileGenex)
    set(ilib_name "@VISIT_TP_GENEX_${targetFileGenex}:${targetName}@")
    if(libdir STREQUAL "")
        file(APPEND ${fname} "    ${libType} \"\${${prefix}}/${ilib_name}\"\n")
    else()
        file(APPEND ${fname} "    ${libType} \"\${${prefix}}/${libdir}/${ilib_name}\"\n")
    endif()
endfunction()

# Write a generic IMPORTED_SONAME property using target soname genex.
#
# Arguments:
#   fname       Setup file being written.
#   targetName  Shared-library target whose soname should be used.
function(write_soname fname targetName)
    file(APPEND ${fname} "    IMPORTED_SONAME \"@VISIT_TP_GENEX_TARGET_SONAME_FILE_NAME:${targetName}@\"\n")
endfunction()

# Test whether a target represents an Apple framework.
#
# Arguments:
#   isFramework  Output variable set to true or false.
#   targetName   Target to inspect.
function(is_framework_target isFramework targetName)
    set(framework false)
    is_prop_set(propIsSet FRAMEWORK ${targetName})
    if(propIsSet)
        get_target_property(framework_prop ${targetName} FRAMEWORK)
        if(framework_prop)
            set(framework true)
        endif()
    endif()
    set(${isFramework} ${framework} PARENT_SCOPE)
endfunction()

# Write IMPORTED_LOCATION for an installed Apple framework directory.
#
# Arguments:
#   fname       Setup file being written.
#   targetName  Framework target whose framework directory should be used.
#   prefix      Setup-file prefix variable, such as QT_PREFIX.
function(write_framework_location fname targetName prefix)
    file(APPEND ${fname} "    IMPORTED_LOCATION \"\${${prefix}}/lib/@VISIT_TP_FRAMEWORK_DIR_NAME:${targetName}@\"\n")
endfunction()

# Write platform-specific imported artifact location properties.
#
# Arguments:
#   fname       Setup file being written.
#   targetName  Target whose installed artifact properties should be written.
#   prefix      Setup-file prefix variable, such as VTK_PREFIX.
function(write_lib_location fname targetName prefix)
    get_target_property(ttype ${targetName} TYPE)

    if(WIN32)
        if(${ttype} STREQUAL "SHARED_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_IMPLIB ${targetName} ${prefix} "lib" TARGET_LINKER_FILE_NAME)
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "" TARGET_FILE_NAME)
        elseif(${ttype} STREQUAL "STATIC_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "lib" TARGET_FILE_NAME)
        elseif(${ttype} STREQUAL "EXECUTABLE")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "" TARGET_FILE_NAME)
        endif()
    elseif(LINUX)
        if(${ttype} STREQUAL "EXECUTABLE")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "bin" TARGET_FILE_NAME)
        elseif(${ttype} STREQUAL "SHARED_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "lib" TARGET_FILE_NAME)
            write_soname(${fname} ${targetName})
        elseif(${ttype} STREQUAL "STATIC_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "archives" TARGET_FILE_NAME)
        endif()
    elseif(APPLE)
        is_framework_target(isFramework ${targetName})
        if(isFramework)
            write_framework_location(${fname} ${targetName} ${prefix})
            if(${ttype} STREQUAL "SHARED_LIBRARY")
                write_soname(${fname} ${targetName})
            endif()
        elseif(${ttype} STREQUAL "EXECUTABLE")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "bin" TARGET_FILE_NAME)
        elseif(${ttype} STREQUAL "SHARED_LIBRARY" OR ${ttype} STREQUAL "UNKNOWN_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "lib" TARGET_FILE_NAME)
            if(${ttype} STREQUAL "SHARED_LIBRARY")
                write_soname(${fname} ${targetName})
            endif()
        elseif(${ttype} STREQUAL "STATIC_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "archives" TARGET_FILE_NAME)
        endif()
    endif()
endfunction()

# Write the set_target_properties() block for one imported target.
#
# Arguments:
#   fname              Setup file being written.
#   targetName         Target whose properties should be written.
#   namespace          Target namespace being exported.
#   incBase            Installed include directory base below include/.
#   prefix             Setup-file prefix variable, such as VTK_PREFIX.
#   useSimpleInclude   If true, write include/${incBase} without inspecting
#                      target include paths.
#   hasNestedIncludes  If true, preserve nested include paths under incBase.
function(write_target_props fname targetName namespace incBase prefix useSimpleInclude hasNestedIncludes)

     has_props_we_care_about(${targetName}_hasProps ${targetName})
     if(NOT ${${targetName}_hasProps})
         return()
     endif()

     file(APPEND ${fname} "set_target_properties(${targetName} PROPERTIES\n")

     is_prop_set(incSet INTERFACE_INCLUDE_DIRECTORIES ${targetName})
     if(${incSet})
         string(LENGTH ${incBase} incBaseSize)

         file(APPEND ${fname} "    INTERFACE_INCLUDE_DIRECTORIES \"")
         if(${useSimpleInclude})
             file(APPEND ${fname} "\${${prefix}}/include/${incBase}")
         else()
             get_target_property(iid ${targetName} INTERFACE_INCLUDE_DIRECTORIES)
             set(allIncs)

             foreach(inc ${iid})
                 if(hasNestedIncludes)
                     string(FIND ${inc} ${incBase} pos)
                     if(NOT pos EQUAL -1)
                         math(EXPR pos "${pos} + ${incBaseSize}")
                         string(SUBSTRING ${inc} ${pos} -1 incTrunc)
                         if(NOT incTrun STREQUAL "" )
                             list(APPEND allIncs "\${${prefix}}/include/${incBase}${incTrunc}")

                         endif()
                     endif()
                 else()
                     cmake_path(GET inc FILENAME incname)
                     if(incname)
                         if(${incname} STREQUAL "include")
                             list(APPEND allIncs "\${${prefix}}/include/${incBase}")
                         elseif(${incname} STREQUAL ${incBase})
                             list(APPEND allIncs "\${${prefix}}/include/${incBase}")
                         else()
                             list(APPEND allIncs "\${${prefix}}/include/${incBase}/${incname}")
                         endif()
                     endif()
                 endif()
             endforeach()
             list(REMOVE_DUPLICATES allIncs)
             file(APPEND ${fname} "${allIncs}")
             unset(allIncs)
         endif()
         # close quote, newline
         file(APPEND ${fname} "\"\n")
     endif()
     # other properties that may be necessary

     write_prop_if_set(${fname} INTERFACE_COMPILE_DEFINITIONS ${targetName})
     write_prop_if_set(${fname} INTERFACE_COMPILE_FEATURES ${targetName})
     write_prop_if_set(${fname} INTERFACE_COMPILE_OPTIONS ${targetName})


     write_lib_location(${fname} ${targetName} ${prefix})
     get_target_property(ill ${targetName} INTERFACE_LINK_LIBRARIES)
     if(ill)
         file(APPEND ${fname} "    INTERFACE_LINK_LIBRARIES \"${ill}\"")
     endif()

     # end the set_target_properties call
     file(APPEND ${fname} ")\n\n")
endfunction()


# Write add_library()/add_executable() and properties for one target.
#
# Arguments:
#   fname              Setup file being written.
#   targetName         Target to recreate as an imported target.
#   namespace          Target namespace being exported.
#   incBase            Installed include directory base below include/.
#   prefix             Setup-file prefix variable, such as VTK_PREFIX.
#   useSimpleInclude   Passed through to write_target_props().
#   hasNestedIncludes  Passed through to write_target_props().
function(write_library_entry fname targetName namespace incBase prefix useSimpleInclude hasNestedIncludes)

    get_target_property(ttype ${targetName} TYPE)

    if(${ttype} STREQUAL "SHARED_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} SHARED IMPORTED)\n")
    elseif(${ttype} STREQUAL "INTERFACE_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} INTERFACE IMPORTED)\n")
    elseif(${ttype} STREQUAL "STATIC_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} STATIC IMPORTED)\n")
    elseif(${ttype} STREQUAL "UNKNOWN_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} UNKNOWN IMPORTED)\n")
    elseif(${ttype} STREQUAL "EXECUTABLE")
        file(APPEND ${fname} "add_executable(${targetName} IMPORTED)\n")
    endif()

    write_target_props(${fname} ${targetName} ${namespace} ${incBase}
                       ${prefix} ${useSimpleInclude} ${hasNestedIncludes})
endfunction()

# Write the standard VisIt copyright header.
#
# Arguments:
#   fname  Setup file being written.
#
# Assumes the initial file(WRITE) has already occurred.
function(write_copyright fname)
    file(APPEND ${fname} "# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt\n")
    file(APPEND ${fname} "# Project developers.  See the top-level LICENSE file for dates and other\n")
    file(APPEND ${fname} "# details.  No copyright assignment is required to contribute to VisIt.\n\n")
endfunction()

# Write the standard setup-file header and version variables.
#
# Arguments:
#   fname  Setup file being written.
#   kit    Third-party package name used in Setup<kit>.cmake and <kit>_PREFIX.
#
# Assumes the initial file(WRITE) has already occurred.
function(write_standard_header fname kit)
    write_copyright(${fname})

    file(APPEND ${fname} "# Create import targets for ${kit}'s libraries installed with VisIt.\n\n")
    file(APPEND ${fname} "cmake_path(SET ${kit}_PREFIX NORMALIZE \${CMAKE_CURRENT_LIST_DIR}/..)\n\n")

    if(${kit}_VERSION)
        file(APPEND ${fname} "set(${kit}_VERSION ${${kit}_VERSION})\n")
        string(REPLACE "." ";" v_list ${${kit}_VERSION})
        list(GET v_list 0 major)
        list(GET v_list 1 minor)
        file(APPEND ${fname} "set(${kit}_VERSION_MAJOR ${major})\n")
        file(APPEND ${fname} "set(${kit}_VERSION_MINOR ${minor})\n\n")
    endif()
endfunction()

# Collect namespace targets and same-namespace transitive link dependencies.
#
# Arguments:
#   NAME       Prefix for the cached output list variable (<NAME>_list).
#   NAMESPACE  Namespace targets must start with to be collected.
#   ITEMS      Initial targets or link entries to inspect.
function(get_all_targets)
    cmake_parse_arguments(gat "" "NAME;NAMESPACE" "ITEMS" ${ARGN})
    if(NOT DEFINED gat_NAME)
        message(FATAL_ERROR "get_all_targets missing NAME arg")
    endif()
    if(NOT DEFINED gat_NAMESPACE)
        message(FATAL_ERROR "get_all_targets missing NAMESPACE arg")
    endif()
    if( NOT DEFINED gat_ITEMS)
        message(FATAL_ERROR "get_all_targets missing ITEMS arg")
    endif()

    # used for determining if a target belongs to the given namespace
    set(_targets)
    string(LENGTH "${gat_NAMESPACE}" size)
    foreach(t IN LISTS gat_ITEMS)
        if(NOT TARGET "${t}")
            continue()
        endif()
        string(SUBSTRING "${t}" 0 ${size} starts_with_namespace)
        if(starts_with_namespace STREQUAL ${gat_NAMESPACE})
            list(APPEND _targets ${t})
        endif()
        get_target_property(ill "${t}" INTERFACE_LINK_LIBRARIES)
        if(ill)
            foreach(lib ${ill})
                get_all_targets(NAME ${gat_NAME}
                                NAMESPACE ${gat_NAMESPACE}
                                ITEMS ${lib})
                list(APPEND _targets ${${gat_NAME}_list})
            endforeach()
        endif()
    endforeach()
    set(${gat_NAME}_list ${_targets} PARENT_SCOPE)
endfunction()


# Return the intermediate setup-file input path for a package.
#
# Arguments:
#   outvar  Output variable that receives the path.
#   name    Third-party package name used in Setup<name>.cmake.in.
function(get_lib_setup_cmake_input_file outvar name)
    set(${outvar} ${VISIT_BINARY_DIR}/Setup${name}.cmake.in PARENT_SCOPE)
endfunction()

# Generate and install the final Setup<NAME>.cmake file.
#
# Arguments:
#   NAME  Third-party package name whose Setup<NAME>.cmake.in should be
#         processed with file(GENERATE) and installed.
function(generate_lib_setup_cmake)
    cmake_parse_arguments(glsc "" "NAME" "" ${ARGN})
    if(NOT DEFINED glsc_NAME)
        message(FATAL_ERROR "generate_lib_setup_cmake missing NAME arg")
    endif()

    get_lib_setup_cmake_input_file(fname ${glsc_NAME})
    if(CMAKE_CONFIGURATION_TYPES)
        set(generated_fname ${VISIT_BINARY_DIR}/$<CONFIG>/Setup${glsc_NAME}.cmake)
    else()
        set(generated_fname ${VISIT_BINARY_DIR}/Setup${glsc_NAME}.cmake)
    endif()

    file(READ ${fname} setup_content)
    string(REPLACE "$<" "$<1:$><" setup_content "${setup_content}")
    string(REGEX REPLACE "@VISIT_TP_GENEX_([^@]+)@" "$<\\1>" setup_content "${setup_content}")
    string(REGEX REPLACE "@VISIT_TP_FRAMEWORK_DIR_NAME:([^@]+)@" "$<PATH:GET_FILENAME,$<TARGET_FILE_DIR:\\1>>" setup_content "${setup_content}")

    file(GENERATE OUTPUT ${generated_fname}
                  CONTENT "${setup_content}")

    install(FILES ${generated_fname}
            DESTINATION ${VISIT_INSTALLED_VERSION}/cmake
            RENAME Setup${glsc_NAME}.cmake)
endfunction()

# Create or append to the intermediate Setup<NAME>.cmake.in file.
#
# This is the main entry point used by VisIt's Find modules for complex
# third-party libraries whose imported targets cannot be exported with
# visit_import_library(). It collects the requested targets and their
# same-namespace target dependencies, then writes add_library()/add_executable()
# and set_target_properties() commands into the intermediate setup file.
#
# Required arguments:
#   NAME       Third-party package name used in Setup<NAME>.cmake.in.
#   NAMESPACE  Target namespace to collect, such as VTK:: or Qt6::.
#   ITEMS      Initial list of targets to write.
#   INCBASE    Installed include directory base below include/.
#
# Optional arguments:
#   SIMPLE_INCLUDE   If set, write include/${INCBASE} directly instead of
#                    deriving include subdirectories from target properties.
#   NESTED_INCLUDES  If set, preserve nested include paths below INCBASE.
#   SKIP_HEADER      If set, append targets without rewriting the standard
#                    header. Used by multi-namespace setup files such as Qt.
function(create_lib_setup_cmake)
    cmake_parse_arguments(clsc "SIMPLE_INCLUDE;NESTED_INCLUDES;SKIP_HEADER" "NAME;NAMESPACE;INCBASE" "ITEMS" ${ARGN})
    if(NOT DEFINED clsc_NAME)
        message(FATAL_ERROR "create_lib_setup_make missing NAME arg")
    endif()
    if(NOT DEFINED clsc_NAMESPACE)
        message(FATAL_ERROR "create_lib_setup_make missing NAMESPACE arg")
    endif()
    if( NOT DEFINED clsc_ITEMS)
        message(FATAL_ERROR "create_lib_setup_make missing ITEMS arg")
    endif()
    if( NOT DEFINED clsc_INCBASE)
        message(FATAL_ERROR "create_lib_setup_make missing INCBASE arg")
    endif()



    # easiest to get all necessary targets up front
    # (those in the inital targetList and all their dependencies).
    set(${clsc_NAME}_list)
    get_all_targets(NAME       ${clsc_NAME}
                    NAMESPACE ${clsc_NAMESPACE}
                    ITEMS     ${clsc_ITEMS})
    list(REMOVE_DUPLICATES ${clsc_NAME}_list)

    get_lib_setup_cmake_input_file(fname ${clsc_NAME})
    if(NOT ${clsc_SKIP_HEADER})
        file(WRITE ${fname} "")
        write_standard_header(${fname} ${clsc_NAME})
    endif()

    set(prefix ${clsc_NAME}_PREFIX)
    foreach(onetarget ${${clsc_NAME}_list})
        write_library_entry(${fname} ${onetarget} ${clsc_NAMESPACE} ${clsc_INCBASE} ${prefix} ${clsc_SIMPLE_INCLUDE} ${clsc_NESTED_INCLUDES})
    endforeach()

    # cleanup
    unset(${clsc_NAME}_list CACHE)
    unset(fname)
    unset(prefix)
endfunction()

