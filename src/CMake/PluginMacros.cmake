# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Fri Nov 18 12:00:18 MST 2011
#   Added plugin name to VISIT_PLUGIN_TARGET_FOLDER args, so that plugins
#   are grouped by name in VisualStudio for ease of building/debugging
#   individual plugins.
#
#   Kathleen Biagas, Tue Nov 22 14:37:17 MST 2011
#   Remove VISIT_PLUGIN_TARGET_PREFIX in favor of VISIT_PLUGIN_TARGET_RTOD
#   which sets runtime output directory, which works with all versions of
#   Visual Studio, while the target prefix version didn't work with 2010.
#
#   Kathleen Biagas, Wed Jan 8 16:05:27 PST 2014
#   Fix non-msvc-ide filename path. Patch provided by John Cary.
#
#   Kathleen Biagas, Fri Oct 31 11:18:54 PDT 2014
#   Move VISIT_PLUGIN_TARGET_RTOD to VisItMacros.cmake, so it can be used
#   by PluginVsInstall.cmake (since it needs no re-write).
#
#   Kathleen Biagas, Wed Mar 4 2020
#   Enable code-gen targets on Windows, and use FOLDER property.
#
#   Kathleen Biagas, Wed April 27, 2022
#   Allow database plugins to skip creation of gen_info_XXX targets by adding
#   SKIP_INFO optional argument to ADD_DATABASE_CODE_GEN_TARGETS.
#
#   Kathleen Biagas, Thu Dec 1, 2022
#   Moved VISIT_SELECTED_PLUGIN_ERROR and CREATE_PLUGIN_DEPENDENCIES from
#   root CMakeLists.txt.
#
#****************************************************************************/


macro(VISIT_INSTALL_PLUGINS type)
    set(VPERM OWNER_READ OWNER_WRITE OWNER_EXECUTE
              GROUP_READ GROUP_WRITE GROUP_EXECUTE
              WORLD_READ             WORLD_EXECUTE)
    if(NOT VISIT_STATIC)
        if(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
            set_target_properties(${ARGN} PROPERTIES INSTALL_RPATH "$ORIGIN/../../lib")
        endif()
        if(NOT WIN32)
            install(TARGETS ${ARGN}
                LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                PERMISSIONS ${VPERM}
            )
        else()
            # ${BUILD_TYPE} refers to the configuration option chosen (Release,
            # Debug, etc). It is a var that will be given a value during compile
            # not configure, so the dollar sign must be escaped in the string
            # below.  Then during install, ${BUILD_TYPE} will be expanded.
            foreach(target ${ARGN})
                if(MSVC_IDE)
                  set(filename "${VISIT_BINARY_DIR}/exe/\${BUILD_TYPE}/${type}/${target}.dll")
                  install(FILES ${filename}
                    DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                    COMPONENT RUNTIME
                    PERMISSIONS ${VPERM}
                  )
                else()  # For no IDE, installed straight into exe
                  set(filename "${VISIT_BINARY_DIR}/exe/${type}/${target}.dll")
                  install(FILES ${filename}
                    DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                    COMPONENT RUNTIME
                    PERMISSIONS ${VPERM}
                  )
                endif()
            endforeach()
        endif()
    endif()
    unset(VPERM)
endmacro()

macro(VISIT_INSTALL_DATABASE_PLUGINS)
    VISIT_INSTALL_PLUGINS(databases ${ARGN})
endmacro()

macro(VISIT_INSTALL_OPERATOR_PLUGINS)
    VISIT_INSTALL_PLUGINS(operators ${ARGN})
endmacro()

macro(VISIT_INSTALL_PLOT_PLUGINS)
    VISIT_INSTALL_PLUGINS(plots ${ARGN})
endmacro()

macro(VISIT_PLUGIN_TARGET_FOLDER type pname)
    set_target_properties(${ARGN} PROPERTIES FOLDER "plugins/${type}/${pname}")
endmacro()


##############################################################################
# Function that adds all Code Gen Targets for a plugin
##############################################################################
function(ADD_PLUGIN_CODE_GEN_TARGETS gen_name)
    ####
    # only create code gen targets if our cmake for gen targets option is on
    ####
    if(VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
        set(gen_target_name "gen_plugin_${gen_name}")

        message(STATUS "Adding xml tools plugin generation target: ${gen_target_name}")

        ADD_CPP_GEN_TARGET(${gen_name}
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR})

        ADD_PYTHON_GEN_TARGET(${gen_name}
                              ${CMAKE_CURRENT_SOURCE_DIR}
                              ${CMAKE_CURRENT_SOURCE_DIR})


        ADD_JAVA_GEN_TARGET(${gen_name}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR})

        ADD_INFO_GEN_TARGET(${gen_name}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR})

        ADD_CMAKE_GEN_TARGET(${gen_name}
                             ${CMAKE_CURRENT_SOURCE_DIR}
                             ${CMAKE_CURRENT_SOURCE_DIR})

        add_custom_target(${gen_target_name})
        if(WIN32)
            set_target_properties(${gen_target_name} PROPERTIES
                FOLDER "generators/plugin")
        endif()

        set(gen_plugin_deps "")
        list(APPEND gen_plugin_deps "gen_cpp_${gen_name}")
        list(APPEND gen_plugin_deps "gen_python_${gen_name}")
        list(APPEND gen_plugin_deps "gen_java_${gen_name}")
        list(APPEND gen_plugin_deps "gen_info_${gen_name}")
        # we don't wan't to directly wire up xml2cmake

        add_dependencies(${gen_target_name} ${gen_plugin_deps})

        # connect this target to roll up target for plugin gen
        if(NOT TARGET gen_plugin_all)
            add_custom_target("gen_plugin_all")
            if(WIN32)
                set_target_properties(gen_plugin_all PROPERTIES
                    FOLDER "generators/all")
            endif()
        endif()

        add_dependencies(gen_plugin_all ${gen_target_name})
    endif()
endfunction()

##############################################################################
# Function that adds all Code Gen Targets for a plot plugin
##############################################################################
function(ADD_PLOT_CODE_GEN_TARGETS gen_name)
    ADD_PLUGIN_CODE_GEN_TARGETS(${gen_name})
endfunction()

##############################################################################
# Function that adds all Code Gen Targets for an operator plugin
##############################################################################
function(ADD_OPERATOR_CODE_GEN_TARGETS gen_name)
    ADD_PLUGIN_CODE_GEN_TARGETS(${gen_name})
endfunction()

##############################################################################
# Function that adds all Code Gen Targets for a database plugin
# If optional SKIP_INFO argument is found then the gen_info target
# will not be added.
##############################################################################
function(ADD_DATABASE_CODE_GEN_TARGETS gen_name)
    ####
    # only create code gen targets if our cmake for gen targets option is on
    ####
    if(VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
        cmake_parse_arguments(PARSE_ARGV 1 db "SKIP_INFO" "" "")

        set(gen_target_name "gen_plugin_${gen_name}")

        message(STATUS "Adding xml tools plugin generation target: ${gen_target_name}")

        add_custom_target(${gen_target_name})
        if(WIN32)
            set_target_properties(${gen_target_name} PROPERTIES
                FOLDER "generators/plugin")
        endif()

        if(NOT db_SKIP_INFO)
            ADD_INFO_GEN_TARGET(${gen_name}
                                ${CMAKE_CURRENT_SOURCE_DIR}
                                ${CMAKE_CURRENT_SOURCE_DIR})
            set(gen_plugin_deps "")
            list(APPEND gen_plugin_deps "gen_info_${gen_name}")
            # we don't wan't to directly wire up xml2cmake so its handled below
            add_dependencies(${gen_target_name} ${gen_plugin_deps})
        endif()

        ADD_CMAKE_GEN_TARGET(${gen_name}
                             ${CMAKE_CURRENT_SOURCE_DIR}
                             ${CMAKE_CURRENT_SOURCE_DIR})

        # connect this target to roll up target for plugin gen
        if(NOT TARGET gen_plugin_all)
            add_custom_target("gen_plugin_all")
            if(WIN32)
                set_target_properties(gen_plugin_all PROPERTIES
                    FOLDER "generators/all")
            endif()
        endif()

        add_dependencies(gen_plugin_all ${gen_target_name})
    endif()
endfunction()


macro(VISIT_SELECTED_PLUGIN_ERROR type plist msg)
  if(DEFINED VISIT_SELECTED_${type}_PLUGINS)
    foreach(plug ${plist})
      list(FIND VISIT_SELECTED_${type}_PLUGINS ${plug} foundPlug)
      if(NOT foundPlug EQUAL -1)
        message(FATAL_ERROR "Cannot build selected plugin (${plug}): ${msg}")
      endif()
    endforeach()
  endif()
endmacro()


function(CREATE_PLUGIN_DEPENDENCIES target comp type)
    foreach(P ${ARGN})
        # Like E + Pseudocolor + Plot_ser
        set(deptarget "${comp}${P}${type}")
        set(dependencies ${dependencies} ${deptarget})
    endforeach(P)
    # Construct a variable that contains the names of the dependencies so we
    # can use that later when we link the target's main exe.
    set(cachevar "${target}_${comp}${type}")
    set(${cachevar} ${dependencies} CACHE INTERNAL "dependencies for ${target}")
    #message("${cachevar} = ${${cachevar}}")
endfunction(CREATE_PLUGIN_DEPENDENCIES)

