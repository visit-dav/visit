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
#****************************************************************************/


MACRO(VISIT_INSTALL_PLUGINS type)
    IF(NOT VISIT_STATIC)
        IF(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
            SET_TARGET_PROPERTIES(${ARGN} PROPERTIES INSTALL_RPATH "$ORIGIN/../../lib")
        ENDIF(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
        IF(NOT WIN32)
            INSTALL(TARGETS ${ARGN}
                LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                            WORLD_READ             WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
            )
        ELSE(NOT WIN32)
            # ${BUILD_TYPE} refers to the configuration option chosen (Release,
            # Debug, etc). It is a var that will be given a value during compile
            # not configure, so the dollar sign must be escaped in the string
            # below.  Then during install, ${BUILD_TYPE} will be expanded.
            FOREACH(target ${ARGN})
                IF(MSVC_IDE)
                  SET(filename "${VISIT_BINARY_DIR}/exe/\${BUILD_TYPE}/${type}/${target}.dll")
                  INSTALL(FILES ${filename}
                    DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                    COMPONENT RUNTIME
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                                WORLD_READ             WORLD_EXECUTE
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                  )
                ELSE()  # For no IDE, installed straight into exe
                  SET(filename "${VISIT_BINARY_DIR}/exe/${type}/${target}.dll")
                  INSTALL(FILES ${filename}
                    DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                    COMPONENT RUNTIME
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                                WORLD_READ             WORLD_EXECUTE
                  )
                ENDIF()
            ENDFOREACH(target)
        ENDIF(NOT WIN32)
    ENDIF(NOT VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_PLUGINS)

MACRO(VISIT_INSTALL_DATABASE_PLUGINS)
    VISIT_INSTALL_PLUGINS(databases ${ARGN})
ENDMACRO(VISIT_INSTALL_DATABASE_PLUGINS)

MACRO(VISIT_INSTALL_OPERATOR_PLUGINS)
    VISIT_INSTALL_PLUGINS(operators ${ARGN})
ENDMACRO(VISIT_INSTALL_OPERATOR_PLUGINS)

MACRO(VISIT_INSTALL_PLOT_PLUGINS)
    VISIT_INSTALL_PLUGINS(plots ${ARGN})
ENDMACRO(VISIT_INSTALL_PLOT_PLUGINS)

MACRO(VISIT_PLUGIN_TARGET_FOLDER type pname) 
    SET_TARGET_PROPERTIES(${ARGN} PROPERTIES FOLDER "plugins/${type}/${pname}")
ENDMACRO(VISIT_PLUGIN_TARGET_FOLDER)


##############################################################################
# Function that adds all Code Gen Targets for a plugin
##############################################################################
FUNCTION(ADD_PLUGIN_CODE_GEN_TARGETS gen_name)
    ####
    # only create code gen targets if:
    #  we aren't on windows 
    #  our cmake for gen targets option is on
    ####
    if(NOT WIN32 AND VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
        set(gen_target_name "gen_plugin_${gen_name}")

        MESSAGE(STATUS "Adding xml tools plugin generation target: ${gen_target_name}")

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
        endif()

        add_dependencies(gen_plugin_all ${gen_target_name})
    endif()
ENDFUNCTION(ADD_PLUGIN_CODE_GEN_TARGETS)

##############################################################################
# Function that adds all Code Gen Targets for a plot plugin
##############################################################################
FUNCTION(ADD_PLOT_CODE_GEN_TARGETS gen_name)
    ADD_PLUGIN_CODE_GEN_TARGETS(${gen_name})
ENDFUNCTION(ADD_PLOT_CODE_GEN_TARGETS)

##############################################################################
# Function that adds all Code Gen Targets for an operator plugin
##############################################################################
FUNCTION(ADD_OPERATOR_CODE_GEN_TARGETS gen_name)
    ADD_PLUGIN_CODE_GEN_TARGETS(${gen_name})
ENDFUNCTION(ADD_OPERATOR_CODE_GEN_TARGETS)

##############################################################################
# Function that adds all Code Gen Targets for a database plugin
##############################################################################
FUNCTION(ADD_DATABASE_CODE_GEN_TARGETS gen_name)
    ####
    # only create code gen targets if:
    #  we aren't on windows 
    #  our cmake for gen targets option is on
    ####
    if(NOT WIN32 AND VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
        set(gen_target_name "gen_plugin_${gen_name}")

        MESSAGE(STATUS "Adding xml tools plugin generation target: ${gen_target_name}")

        add_custom_target(${gen_target_name})

        # only xml2info and xml2cmake for db plugins
        ADD_INFO_GEN_TARGET(${gen_name}
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR})

        ADD_CMAKE_GEN_TARGET(${gen_name}
                             ${CMAKE_CURRENT_SOURCE_DIR}
                             ${CMAKE_CURRENT_SOURCE_DIR})

        set(gen_plugin_deps "")
        list(APPEND gen_plugin_deps "gen_info_${gen_name}")
        # we don't wan't to directly wire up xml2cmake

        add_dependencies(${gen_target_name} ${gen_plugin_deps})

        # connect this target to roll up target for plugin gen
        if(NOT TARGET gen_plugin_all)
         add_custom_target("gen_plugin_all")
        endif()

        add_dependencies(gen_plugin_all ${gen_target_name})
    endif()
ENDFUNCTION(ADD_DATABASE_CODE_GEN_TARGETS)

