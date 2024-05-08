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
#   Kathleen Biagas, Mon Oct 16 14:40:58 PDT 2023
#   Add visit_add_operator_plugin macro.
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


# generation of operator plugin libraries (called from a plugin's CMakeLists.txt)

function(visit_add_operator_plugin)
    # required arguments:
    #   NAME              Name of the operator plugin
    # optional arguments:
    #   ATTSNAME          Name of attributes class.  # THIS PROBABLY ISN"T NEEDED AS LOGIC TO FIGURE OUT THE NAME APPEARS TO BE WORKING
    #   CREATEEXPR        if set, this operator creates expressions
    #   GSRC              additional sources for the gui target
    #   VSRC              additional sources for the viewer target
    #   ESRC              additional sources for the engine targets
    #   GLIBS             additional libraries for the gui target
    #   VLIBS             additional libraries for the viewer target
    #   SLIBS             additional libraries for the scripting target
    #   ESERLIBS          additional libraries for the serial engine targets
    #   EPARLIBS          additional libraries for the parallel engine targets


    # NOTES:  not all of the target link libraries being added to the
    # targets here are necessary for every operator.  They are being added
    # for convenience to ease plugin developement
    # (eg avtdbatts needed by those operators that create expressions)
    set(OPT CREATEEXPR)
    set(VALS NAME ATTSNAME)
    set(MVALS GSRC VSRC ESRC GLIBS VLIBS SLIBS ESERLIBS EPARLIBS)
    cmake_parse_arguments(op "${OPT}" "${VALS}" "${MVALS}" ${ARGN})

    if(NOT DEFINED op_NAME)
        message(FATAL_ERROR "Incomplete arguments to visit_add_operator_plugin. Required: NAME")
    endif()

    # if doing dev build ??
    ADD_OPERATOR_CODE_GEN_TARGETS(${op_NAME})
    # endif()

    # Handle the different ways some operator's atts have been named.
    # This was handled nicely in GenerateCMake when it was writing
    # the full CMake code, becuase the Atts name is specified in the XML file.
    # Could possibly have the CMake gen code add an ATTSNAME argument
    # to this function instead of this logic ... Make it required so
    # all plugins use it.  Could do the same for FILTERNAME since some use
    # 'Plugin' in the name.
    if(op_ATTSNAME)
        set(CATTS ${op_ATTSNAME}.C)
        set(PYATTS Py${op_ATTSNAME}.C)
        set(JATTS ${op_ATTSNAME}.java)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${op_NAME}Attributes.C)
        set(CATTS ${op_NAME}Attributes.C)
        set(PYATTS Py${op_NAME}Attributes.C)
        set(JATTS ${op_NAME}Attributes.java)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${op_NAME}OperatorAttributes.C)
        set(CATTS ${op_NAME}OperatorAttributes.C)
        set(PYATTS Py${op_NAME}OperatorAttributes.C)
        set(JATTS ${op_NAME}OperatorAttributes.java)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${op_NAME}Atts.C)
        set(CATTS ${op_NAME}Atts.C)
        set(PYATTS Py${op_NAME}Atts.C)
        set(JATTS ${op_NAME}Atts.java)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${op_NAME}.C)
        set(CATTS ${op_NAME}.C)
        set(PYATTS Py${op_NAME}.C)
        set(JATTS ${op_NAME}.java)
    else()
        message(FATAL_ERROR "Cound not find name for ${op_NAME}'s Attributes class. Expecting ${op_NAME}Attributes, ${op_NAME}OperatorAttributes, ${op_NAME}Atts or ${op_NAME}")
    endif()
    set(COMMON_SOURCES
        ${op_NAME}PluginInfo.C
        ${op_NAME}CommonPluginInfo.C
        ${CATTS})

    set(LIBI_SOURCES ${op_NAME}PluginInfo.C)

    set(LIBG_SOURCES
        ${op_NAME}GUIPluginInfo.C
        Qvis${op_NAME}Window.C
        ${COMMON_SOURCES})
    if(DEFINED op_GSRC)
        list(APPEND LIBG_SOURCES ${op_GSRC})
    endif()

    set(LIBV_SOURCES
        ${op_NAME}ViewerEnginePluginInfo.C
        ${op_NAME}ViewerPluginInfo.C
        ${COMMON_SOURCES})

    if(DEFINED op_VSRC)
        list(APPEND LIBV_SOURCES ${op_VSRC})
    endif()

    set(LIBE_SOURCES
        ${op_NAME}ViewerEnginePluginInfo.C
        ${op_NAME}EnginePluginInfo.C
        ${COMMON_SOURCES})

    # some operators don't use the standard filter name
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/avt${op_NAME}Filter.C)
        list(APPEND LIBE_SOURCES avt${op_NAME}Filter.C)
        list(APPEND LIBV_SOURCES avt${op_NAME}Filter.C)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/avt${op_NAME}PluginFilter.C)
        list(APPEND LIBE_SOURCES avt${op_NAME}PluginFilter.C)
        list(APPEND LIBV_SOURCES avt${op_NAME}PluginFilter.C)
    else()
        message("FATAL_ERROR Could not determine name of ${op_NAME}'s filter class. Expecting avt${op_NAME}Filter or avt${op_NAME}PluginFilter.")
    endif()

    if(DEFINED op_ESRC)
        list(APPEND LIBE_SOURCES ${op_ESRC})
    endif()

    set(ITarget    I${op_NAME}Operator)
    set(GTarget    G${op_NAME}Operator)
    set(VTarget    V${op_NAME}Operator)
    set(STarget    S${op_NAME}Operator)
    set(ESerTarget E${op_NAME}Operator_ser)
    set(EParTarget E${op_NAME}Operator_par)

    visit_add_library(
        NAME     ${ITarget}
        SOURCES  ${LIBI_SOURCES}
        INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        DEPENDS  visitcommon)

    set(INSTALLTARGETS ${ITarget})

    if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        visit_add_library(
            NAME     ${GTarget}
            SOURCES  ${LIBG_SOURCES}
            INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEPENDS  visitcommon gui winutil ${QT_QTWIDGETS_LIBRARY} ${op_GLIBS})

        set_target_properties(${GTarget} PROPERTIES AUTOMOC ON)
   
        #if(DEFINED op_CREATEEXPR)
            target_link_libraries(${GTarget} PRIVATE avtdbatts)
        #endif()

        visit_add_library(
            NAME     ${VTarget}
            SOURCES  ${LIBV_SOURCES}
            INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEFINES  VIEWER
            DEPENDS  visitcommon
                     viewer
                     avtexpressions_ser
                     ${op_VLIBS})

        if(VTK_VERSION VERSION_GREATER_EQUAL "9.1.0")
            target_link_libraries(${VTarget} PUBLIC VTK::CommonDataModel)
        else()
            target_link_libraries(${VTarget} PUBLIC vtkCommonDataModel)
        endif()
      
        #if(DEFINED op_CREATEEXPR)
            target_link_libraries(${VTarget} PRIVATE avtdbatts)
        #endif()

        list(APPEND INSTALLTARGETS ${GTarget} ${VTarget})

        if(VISIT_PYTHON_SCRIPTING)
            visit_add_library(
                NAME     ${STarget}
                SOURCES  ${op_NAME}ScriptingPluginInfo.C
                         ${PYATTS}
                         ${COMMON_SOURCES}
                INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                DEPENDS  visitcommon visitpy ${PYTHON_LIBRARY} ${op_SLIBS})

            #if(DEFINED op_CREATEEXPR)
                target_link_libraries(${STarget} PRIVATE avtdbatts)
            #endif()
            if(WIN32)
                # This prevents python from #defining snprintf as _snprintf
                target_compile_definitions(${STarget} PRIVATE HAVE_SNPRINTF)
            endif()
            list(APPEND INSTALLTARGETS ${STarget})
        endif()

        if(VISIT_JAVA)
            file(COPY ${JATTS} DESTINATION ${JavaClient_BINARY_DIR}/src/operators)
            add_custom_target(Java${op_NAME} ALL ${Java_JAVAC_EXECUTABLE} ${VISIT_Java_FLAGS} -d ${JavaClient_BINARY_DIR} -classpath ${JavaClient_BINARY_DIR} -sourcepath ${JavaClient_BINARY_DIR} ${JATTS}
                DEPENDS JavaClient
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        endif()
    endif()

    set(engine_DEPENDS visitcommon lightweight_visit_vtk)
    if(VTK_VERSION VERSION_GREATER_EQUAL "9.1.0")
        list(APPEND engine_DEPENDS VTK::CommonDataModel VTK::CommonExecutionModel)
    else()
        list(APPEND engine_DEPENDS vtkCommonDataModel vtkCommonExecutionModel)
    endif()
    #if(DEFINED op_CREATEEXPR)
        list(APPEND engine_DEPENDS avtdbatts)
    #endif()
    visit_add_library(
        NAME     ${ESerTarget}
        SOURCES  ${LIBE_SOURCES}
        INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        DEPENDS  ${engine_DEPENDS}
                 avtpipeline_ser
                 avtexpressions_ser
                 avtfilters_ser
                 ${op_ESERLIBS})

    list(APPEND INSTALLTARGETS ${ESerTarget})

    if(VISIT_PARALLEL)
        visit_add_parallel_library(
            NAME     ${EParTarget}
            SOURCES  ${LIBE_SOURCES}
            INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEPENDS  ${engine_DEPENDS}
                     avtpipeline_par
                     avtexpressions_par
                     avtfilters_par
                     ${op_EPARLIBS})
        list(APPEND INSTALLTARGETS ${EParTarget})
    endif()

    VISIT_INSTALL_OPERATOR_PLUGINS(${INSTALLTARGETS})
    # These two aren't needed for pluginVsInstall, so should have
    # some sort of "if(not-doing-dev) test. 
    VISIT_PLUGIN_TARGET_OUTPUT_DIR(operators ${INSTALLTARGETS})
    VISIT_PLUGIN_TARGET_FOLDER(operators ${op_NAME} ${INSTALLTARGETS})
endfunction()

function(visit_add_plot_plugin)
    # required arguments:
    #   NAME             Name of the plot plugin
    # optional arguments:
    #   GSRC              additional sources for the gui target
    #   VSRC              additional sources for the viewer target
    #   ESRC              additional sources for the engine targets
    #   GLIBS             additional libraries for the gui target
    #   VLIBS             additional libraries for the viewer target
    #   SLIBS             additional libraries for the scripting target
    #   ESERLIBS          additional libraries for the serial engine targets
    #   EPARLIBS          additional libraries for the parallel engine targets


    # NOTES:  not all of the target link libraries being added to the
    # targets here are necessary for every plot.  They are being added
    # for convenience to ease plugin developement
    # (eg ??? needed by those plots that ???)
    set(VALS NAME)
    set(MVALS GSRC VSRC ESRC GLIBS VLIBS SLIBS ESERLIBS EPARLIBS)
    cmake_parse_arguments(plot "" "${VALS}" "${MVALS}" ${ARGN})

    if(NOT DEFINED plot_NAME)
        message(FATAL_ERROR "Incomplete arguments to visit_add_plot_plugin. Required: NAME")
    endif()

    project(${plot_NAME}_plot)

    # if doing dev build ??
    ADD_PLOT_CODE_GEN_TARGETS(${plot_NAME})
    # endif()

    # Handle the different ways some plots's atts have been named.
    # This was handled nicely in GenerateCMake when it was writing
    # the full CMake code, becuase the Atts name is specified in the XML file.
    # Could possibly have the CMake gen code add an ATTSNAME argument
    # to this function instead of this logic ... Make it required so
    # all plugins use it.  Could do the same for FILTERNAME since some use
    # 'Plugin' in the name.
    set(CATTS ${plot_NAME}Attributes.C)
    set(PYATTS Py${plot_NAME}Attributes.C)
    set(JATTS ${plot_NAME}Attributes.java)
    set(COMMON_SOURCES
        ${plot_NAME}PluginInfo.C
        ${plot_NAME}CommonPluginInfo.C
        ${CATTS})

    set(LIBI_SOURCES ${plot_NAME}PluginInfo.C)

    set(LIBG_SOURCES
        ${plot_NAME}GUIPluginInfo.C
        Qvis${plot_NAME}PlotWindow.C
        ${COMMON_SOURCES})
    if(DEFINED plot_GSRC)
        list(APPEND LIBG_SOURCES ${plot_GSRC})
    endif()

    set(LIBV_SOURCES
        ${plot_NAME}ViewerEnginePluginInfo.C
        ${plot_NAME}ViewerPluginInfo.C
        avt${plot_NAME}Plot.C
        ${COMMON_SOURCES})

    if(DEFINED plot_VSRC)
        list(APPEND LIBV_SOURCES ${plot_VSRC})
    endif()

    set(LIBE_SOURCES
        ${plot_NAME}ViewerEnginePluginInfo.C
        ${plot_NAME}EnginePluginInfo.C
        avt${plot_NAME}Plot.C
        ${COMMON_SOURCES})

    if(DEFINED plot_ESRC)
        list(APPEND LIBE_SOURCES ${plot_ESRC})
    endif()

    set(ITarget    I${plot_NAME}Plot)
    set(GTarget    G${plot_NAME}Plot)
    set(VTarget    V${plot_NAME}Plot)
    set(STarget    S${plot_NAME}Plot)
    set(ESerTarget E${plot_NAME}Plot_ser)
    set(EParTarget E${plot_NAME}Plot_par)

    visit_add_library(
        NAME     ${ITarget}
        SOURCES  ${LIBI_SOURCES}
        INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        DEPENDS  visitcommon)

    set(INSTALLTARGETS ${ITarget})

    if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        visit_add_library(
            NAME     ${GTarget}
            SOURCES  ${LIBG_SOURCES}
            INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEPENDS  visitcommon gui ${plot_GLIBS})

        set_target_properties(${GTarget} PROPERTIES AUTOMOC ON)
   
        visit_add_library(
            NAME     ${VTarget}
            SOURCES  ${LIBV_SOURCES}
            INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEFINES  VIEWER
            DEPENDS  visitcommon
                     viewer
                     ${plot_VLIBS})
      
        list(APPEND INSTALLTARGETS ${GTarget} ${VTarget})

        if(VISIT_PYTHON_SCRIPTING)
            visit_add_library(
                NAME     ${STarget}
                SOURCES  ${plot_NAME}ScriptingPluginInfo.C
                         ${PYATTS}
                         ${COMMON_SOURCES}
                INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                DEPENDS  visitcommon visitpy ${PYTHON_LIBRARY} ${plot_SLIBS})

            if(WIN32)
                # This prevents python from #defining snprintf as _snprintf
                target_compile_definitions(${STarget} PRIVATE HAVE_SNPRINTF)
            endif()
            list(APPEND INSTALLTARGETS ${STarget})
        endif()

        if(VISIT_JAVA)
            file(COPY ${JATTS} DESTINATION ${JavaClient_BINARY_DIR}/src/operators)
            add_custom_target(Java${plot_NAME} ALL ${Java_JAVAC_EXECUTABLE} ${VISIT_Java_FLAGS} -d ${JavaClient_BINARY_DIR} -classpath ${JavaClient_BINARY_DIR} -sourcepath ${JavaClient_BINARY_DIR} ${JATTS}
                DEPENDS JavaClient
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        endif()
    endif()

    visit_add_library(
        NAME     ${ESerTarget}
        SOURCES  ${LIBE_SOURCES}
        INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        DEPENDS  visitcommon
                 avtplotter_ser
                 ${plot_ESERLIBS})

    list(APPEND INSTALLTARGETS ${ESerTarget})

    if(VISIT_PARALLEL)
        visit_add_parallel_library(
            NAME     ${EParTarget}
            SOURCES  ${LIBE_SOURCES}
            INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            DEPENDS  visitcommon
                     avtplotter_par
                     ${plot_EPARLIBS})
        list(APPEND INSTALLTARGETS ${EParTarget})
    endif()

    VISIT_INSTALL_PLOT_PLUGINS(${INSTALLTARGETS})
    # These two aren't needed for pluginVsInstall, so should have
    # some sort of "if(not-doing-dev) test. 
    VISIT_PLUGIN_TARGET_OUTPUT_DIR(plots ${INSTALLTARGETS})
    VISIT_PLUGIN_TARGET_FOLDER(plots ${plot_NAME} ${INSTALLTARGETS})
endfunction()

function(visit_add_database_plugin)
    # required arguments:
    #   NAME              Name of the database plugin
    # optional arguments:
    #   HASOPTIONS        set if database has Options
    #   HASWRITER         set if database has Writer
    #   INCLUDES          additional include directories for target (NON TP)
    #   SRC               additional sources for mdserver and engine targets
    #   MSRC              additional sources for mdserver target
    #   ESRC              additional sources for engine targets
    #   LIBS              additional libraries for mdserver and engine targets
    #   MLIBS             additional libraries for mdserver target
    #   ESERLIBS          additional libraries for serial engine targets
    #   EPARLIBS          additional libraries for parallel engine targets
    #   DEFINES           Compiler defines for mdserver and engine targets
    #   TPINC             ThirdParty include paths
    #   TPLIBS            ThirdParty libraries
    #   TPLIBDIR          ThirdParty library paths
    #   TPINCPAR          ThirdParty parallel include paths
    #   TPLIBSPAR         ThirdParty parallel libraries
    #   TPLIBDIRPAR       ThirdParty parallel library paths


    set(OPT HASOPTIONS HASWRITER)
    set(VALS NAME)
    set(MVALS INCLUDES 
              SRC MSRC ESRC
              LIBS MLIBS ESERLIBS EPARLIBS
              DEFINES
              TPINC TPLIBS TPLIBDIR
              TPINCPAR TPLIBSPAR TPLIBDIRPAR)
    cmake_parse_arguments(db "${OPT}" "${VALS}" "${MVALS}" ${ARGN})

    if(NOT DEFINED db_NAME)
        message(FATAL_ERROR "Incomplete arguments to visit_add_database_plugin. Required: NAME")
    endif()

    project(${db_NAME}_database)

    # if doing dev build ??
    ADD_DATABASE_CODE_GEN_TARGETS(${db_NAME})
    # endif()

    # take TPLIBS, TPLIBDIR and create full-path-to library.
    if(DEFINED db_TPLIBS AND DEFINED db_TPLIBDIR)
        set(use_TPLIBS)
        foreach(l ${db_TPLIBS})
            if(TARGET ${l})
                # use a defined target as-is
                list(APPEND use_TPLIBS ${l})
                continue()
            endif() 
            if(EXISTS ${l})
                # this lib already has a path
                list(APPEND use_TPLIBS ${l})
                continue()
            endif()
            foreach(d ${db_TPLIBDIR})
                if(EXISTS ${d}/${l})
                   list(APPEND use_TPLIBS ${d}/${l})
                endif()
            endforeach()
        endforeach()
    elseif(DEFINED db_TPLIBS)
        # no TPLIBDIR specified, use TPLIBS as-is
        set(use_TPLIBS ${db_TPLIBS})
    endif()
    if(DEFINED db_TPLIBSPAR AND DEFINED db_TPLIBDIRPAR)
        set(use_TPLIBSPAR)
        foreach(l ${db_TPLIBSPAR})
            if(TARGET ${l})
                # use a defined target as-is
                list(APPEND use_TPLIBSPAR ${l})
                continue()
            endif() 
            if(EXISTS ${l})
                # this lib already has a path
                list(APPEND use_TPLIBSPAR ${l})
                continue()
            endif()
            foreach(d ${db_TPLIBDIRPAR})
                if(EXISTS ${d}/${l})
                   list(APPEND use_TPLIBSPAR ${d}/${l})
                endif()
            endforeach()
        endforeach()
    elseif(DEFINED db_TPLIBSPAR)
        # no TPLIBDIR specified, use TPLIBS as-is
        set(use_TPLIBSPAR ${db_TPLIBSPAR})
    endif()

    set(COMMON_SOURCES
        ${db_NAME}PluginInfo.C
        ${db_NAME}CommonPluginInfo.C)

    set(LIBI_SOURCES ${db_NAME}PluginInfo.C)

    set(LIBM_SOURCES
        ${db_NAME}MDServerPluginInfo.C
        ${COMMON_SOURCES}
        ${db_SRC}
        ${db_MSRC})

    set(LIBE_SOURCES
        ${db_NAME}EnginePluginInfo.C
        ${COMMON_SOURCES}
        ${db_SRC}
        ${db_ESRC})

    set(m_defines MDSERVER)
    set(e_defines ENGINE)
    if(DEFINED db_DEFINES)
        list(APPEND m_defines ${db_DEFINES})
        list(APPEND e_defines ${db_DEFINES})
    endif()

    # format file may be either avtXXFileFormat or avtXXFileFormatInterface
    # or neither.
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/avt${db_NAME}FileFormat.C)
        list(APPEND LIBM_SOURCES avt${db_NAME}FileFormat.C)
        list(APPEND LIBE_SOURCES avt${db_NAME}FileFormat.C)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/avt${db_NAME}FileFormatInterface.C)
        list(APPEND LIBM_SOURCES avt${db_NAME}FileFormatInterface.C)
        list(APPEND LIBE_SOURCES avt${db_NAME}FileFormatInterface.C)
    endif()


    if(db_HASOPTIONS)
        list(APPEND LIBM_SOURCES avt${db_NAME}Options.C)
        list(APPEND LIBE_SOURCES avt${db_NAME}Options.C)
    endif()

    if(db_HASWRITER)
        list(APPEND LIBE_SOURCES avt${db_NAME}Writer.C)
    endif()

    set(ITarget    I${db_NAME}Database)
    set(MTarget    M${db_NAME}Database)
    set(ESerTarget E${db_NAME}Database_ser)
    set(EParTarget E${db_NAME}Database_par)

    visit_add_library(
        NAME     ${ITarget}
        SOURCES  ${LIBI_SOURCES}
        INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS  visitcommon)

    set(INSTALLTARGETS ${ITarget})

    if(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
   
        visit_add_library(
            NAME     ${MTarget}
            SOURCES  ${LIBM_SOURCES}
            INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
                     ${VISIT_INCLUDE_DIR}/include
                     ${db_INCLUDES}
                     ${db_TPINC}
            DEFINES  ${m_defines}
            DEPENDS  visitcommon
                     avtdatabase_ser
                     ${db_LIBS}
                     ${db_MLIBS}
                     ${use_TPLIBS})

        list(APPEND INSTALLTARGETS ${MTarget})
    endif()

    visit_add_library(
        NAME     ${ESerTarget}
        SOURCES  ${LIBE_SOURCES}
        INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
                 ${VISIT_INCLUDE_DIR}/include
                 ${db_INCLUDES}
                 ${db_TPINC}
        DEFINES  ${e_defines}
        DEPENDS  visitcommon
                 avtdatabase_ser
                 ${db_LIBS}
                 ${db_ESERLIBS}
                 ${use_TPLIBS})

    list(APPEND INSTALLTARGETS ${ESerTarget})

    if(VISIT_PARALLEL)
        if(use_TPLIBSPAR)
            set(use_these_libs ${use_TPLIBSPAR})
        else()
            set(use_these_libs ${use_TPLIBS})
        endif()
        if(DEFINED db_TPINCPAR)
            set(use_these_includes ${db_TPINCPAR})
        else()
            set(use_these_includes ${db_TPINC})
        endif()
        visit_add_parallel_library(
            NAME     ${EParTarget}
            SOURCES  ${LIBE_SOURCES}
            INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
                     ${VISIT_INCLUDE_DIR}/include
                     ${db_INCLUDES}
                     ${use_these_includes}
            DEFINES  ${e_defines}
            DEPENDS  visitcommon
                     avtdatabase_par
                     ${db_LIBS}
                     ${db_EPARLIBS}
                     ${use_these})

        list(APPEND INSTALLTARGETS ${EParTarget})
    endif()


    VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})
    # These two aren't needed for pluginVsInstall, so should have
    # some sort of "if(not-doing-dev) test. 
    VISIT_PLUGIN_TARGET_OUTPUT_DIR(databases ${INSTALLTARGETS})
    VISIT_PLUGIN_TARGET_FOLDER(databases ${db_NAME} ${INSTALLTARGETS})
endfunction()

