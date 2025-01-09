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
#   Kathleen Biagas, Tue Jan 31, 2023
#   Clean up plugin install target on Windows, only need RUNTIME DESTINATION.
#   MSVC_IDE and non-ide should install the same.
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
            install(TARGETS ${ARGN}
                RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                PERMISSIONS ${VPERM})
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

function(visit_add_plot_plugin)
    # required arguments:
    #   PNAME             Name of the plot plugin
    # optional arguments:
    #   GSRC              additional sources for the gui target
    #   VSRC              additional sources for the viewer target
    #   ESRC              additional sources for the engine targets
    #   GLIBS             additional libraries for the gui target
    #   VLIBS             additional libraries for the viewer target
    #   SLIBS             additional libraries for the scripting target
    #   ESERLIBS          additional libraries for the serial engine targets
    #   EPARLIBS          additional libraries for the parallel engine targets
    #   DEFINES           any defines for viewer,engine targets
    #   DISABLE_AUTOGEN   disable xml autogeneration


    # NOTES:  not all of the target link libraries being added to the
    # targets here are necessary for every plot.  They are being added
    # for convenience to ease plugin developement

    set(OPTS DISABLE_AUTOGEN)
    set(VALS PNAME)
    set(MVALS GSRC VSRC ESRC GLIBS VLIBS SLIBS ESERLIBS EPARLIBS DEFINES)
    cmake_parse_arguments(plot "${OPTS}" "${VALS}" "${MVALS}" ${ARGN})

    if(NOT DEFINED plot_PNAME)
        message(FATAL_ERROR "Incomplete arguments to visit_add_plot_plugin. Required: PNAME")
    endif()

    project(${plot_PNAME}_plot)

    # if doing dev build ??
    if(NOT ${plot_DISABLE_AUTOGEN})
        ADD_PLOT_CODE_GEN_TARGETS(${plot_PNAME})
    endif()

    set(CATTS ${plot_PNAME}Attributes)
    set(PYATTS Py${plot_PNAME}Attributes)
    set(JATTS ${plot_PNAME}Attributes.java)
    set(COMMON_SOURCES
        ${plot_PNAME}PluginInfo.C
        ${plot_PNAME}CommonPluginInfo.C
        ${CATTS}.C)
    set(COMMON_HEADERS
        ${plot_PNAME}PluginInfo.h
        ${CATTS}.h)

    set(LIBI_SOURCES ${plot_PNAME}PluginInfo.C)
    set(LIBI_HEADERS ${plot_PNAME}PluginInfo.h)

    set(LIBG_SOURCES
        ${plot_PNAME}GUIPluginInfo.C
        Qvis${plot_PNAME}PlotWindow.C
        ${COMMON_SOURCES})
    set(LIBG_HEADERS
        Qvis${plot_PNAME}PlotWindow.h
        ${COMMON_HEADERS})
    if(DEFINED plot_GSRC)
        list(APPEND LIBG_SOURCES ${plot_GSRC})
        foreach(src ${plot_GSRC})
            string(REPLACE ".C" ".h" hdr ${src})
            list(APPEND LIBG_HEADERS ${hdr})
        endforeach()
    endif()

    set(LIBV_SOURCES
        ${plot_PNAME}ViewerEnginePluginInfo.C
        ${plot_PNAME}ViewerPluginInfo.C
        avt${plot_PNAME}Plot.C
        ${COMMON_SOURCES})
    set(LIBV_HEADERS
        avt${plot_PNAME}Plot.h
        ${COMMON_HEADERS})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/avt${plot_PNAME}Filter.C)
        list(APPEND LIBV_SOURCES avt${plot_PNAME}Filter.C)
        list(APPEND LIBV_HEADERS avt${plot_PNAME}Filter.h)
    endif()

    if(DEFINED plot_VSRC)
        list(APPEND LIBV_SOURCES ${plot_VSRC})
        foreach(src ${plot_VSRC})
            string(REPLACE ".C" ".h" src hdr)
            list(APPEND LIBV_HEADERS ${hdr})
        endforeach()
    endif()

    set(LIBE_SOURCES
        ${plot_PNAME}ViewerEnginePluginInfo.C
        ${plot_PNAME}EnginePluginInfo.C
        avt${plot_PNAME}Plot.C
        ${COMMON_SOURCES})
    set(LIBE_HEADERS
        avt${plot_PNAME}Plot.h
        ${COMMON_HEADERS})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/avt${plot_PNAME}Filter.C)
        list(APPEND LIBE_SOURCES avt${plot_PNAME}Filter.C)
        list(APPEND LIBE_HEADERS avt${plot_PNAME}Filter.h)
    endif()

    if(DEFINED plot_ESRC)
        list(APPEND LIBE_SOURCES ${plot_ESRC})
        foreach(src ${plot_ESRC})
            string(REPLACE ".C" ".h" src hdr)
            list(APPEND LIBE_HEADERS ${hdr})
        endforeach()
    endif()

    set(ITarget    I${plot_PNAME}Plot)
    set(GTarget    G${plot_PNAME}Plot)
    set(VTarget    V${plot_PNAME}Plot)
    set(STarget    S${plot_PNAME}Plot)
    set(ESerTarget E${plot_PNAME}Plot_ser)
    set(EParTarget E${plot_PNAME}Plot_par)

    # we are setting SKIP_INSTALL for all visit_add_library calls
    # because plugins don't need to be installed via the export-targets
    # install path. Will use standard install logic at end of the method.
    visit_add_library(
        NAME        ${ITarget}
        SOURCES     ${LIBI_SOURCES}
        HEADERS     ${LIBI_HEADERS}
        INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        DEPENDS_ON  visitcommon
        SKIP_INSTALL)

    set(INSTALLTARGETS ${ITarget})

    if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        visit_add_library(
            NAME        ${GTarget}
            SOURCES     ${LIBG_SOURCES}
            HEADERS     ${LIBG_HEADERS}
            INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEPENDS_ON  visitcommon gui ${plot_GLIBS}
            SKIP_INSTALL)

        set_target_properties(${GTarget} PROPERTIES AUTOMOC ON)
   
        visit_add_library(
            NAME        ${VTarget}
            SOURCES     ${LIBV_SOURCES}
            HEADERS     ${LIBV_HEADERS}
            INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEFINES     VIEWER ${plot_DEFINES}
            DEPENDS_ON  visitcommon viewer ${plot_VLIBS}
            SKIP_INSTALL)

        set_target_properties(${VTarget} PROPERTIES AUTOMOC ON)

        if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
            qt6_disable_unicode_defines(${GTarget})
            qt6_disable_unicode_defines(${VTarget})
        endif()
      
        list(APPEND INSTALLTARGETS ${GTarget} ${VTarget})

        if(VISIT_PYTHON_SCRIPTING)
            visit_add_library(
                NAME       ${STarget}
                SOURCES    ${plot_PNAME}ScriptingPluginInfo.C
                           ${PYATTS}.C
                           ${COMMON_SOURCES}
                HEADERS    ${PYATTS}.h
                           ${COMMON_HEADERS}
                INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${PYTHON_INCLUDE_DIR}>
                DEPENDS_ON visitcommon visitpy ${PYTHON_LIBRARY} ${plot_SLIBS}
                SKIP_INSTALL)

            if(WIN32)
                # This prevents python from #defining snprintf as _snprintf
                target_compile_definitions(${STarget} PRIVATE HAVE_SNPRINTF)
            endif()
            list(APPEND INSTALLTARGETS ${STarget})
        endif()

        if(VISIT_JAVA)
            file(COPY ${JATTS} DESTINATION ${JavaClient_BINARY_DIR}/src/plots)
            add_custom_target(Java${plot_PNAME} ALL ${Java_JAVAC_EXECUTABLE} ${VISIT_Java_FLAGS} -d ${JavaClient_BINARY_DIR} -classpath ${JavaClient_BINARY_DIR} -sourcepath ${JavaClient_BINARY_DIR} ${JATTS}
                DEPENDS_ON JavaClient
                WORKING_DIRECTORY $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
        endif()
    endif()

    visit_add_library(
        NAME        ${ESerTarget}
        SOURCES     ${LIBE_SOURCES}
        HEADERS     ${LIBE_HEADERS}
        INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        DEFINES     ENGINE ${plot_DEFINES}
        DEPENDS_ON  visitcommon avtplotter_ser ${plot_ESERLIBS}
        SKIP_INSTALL)

    list(APPEND INSTALLTARGETS ${ESerTarget})

    if(VISIT_PARALLEL)
        visit_add_parallel_library(
            NAME        ${EParTarget}
            SOURCES     ${LIBE_SOURCES}
            HEADERS     ${LIBE_HEADERS}
            INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEFINES     ENGINE ${plot_DEFINES}
            DEPENDS_ON  visitcommon
                        avtplotter_par
                        ${plot_EPARLIBS})
        list(APPEND INSTALLTARGETS ${EParTarget})
    endif()

    # one of these is not needed for plugin vs install, which one?
    VISIT_INSTALL_PLOT_PLUGINS(${INSTALLTARGETS})
    VISIT_PLUGIN_TARGET_OUTPUT_DIR(plots ${INSTALLTARGETS})
    VISIT_PLUGIN_TARGET_FOLDER(plots ${plot_PNAME} ${INSTALLTARGETS})
endfunction()

