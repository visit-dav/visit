# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
#  A place to store macros used by plugins, so that PluginVsInstall.cmake.in
#  will not have to rewrite them.  Should reduce maintenance headache for
#  that file.  This file should only include those that
#  PluginVsInstall.cmake.in also uses without modification.
#
#  Modifications:
#    Kathleen Biagas, Mon Jan 30, 2023
#    Use lowercase for CMake functions/macros.
#    Change VISIT_PLUGIN_TARGET_RTOD to VISIT_PLUGIN_TARGET_OUTPUT_DIR and
#    added  support for non windows (moved from <plugintype>/CMakeLists.txt).
#
#    Kathleen Biagas, Mon Dec 11, 2023
#    Remove replacement of debug flags. Now handled by setting of
#    CMAKE_MSVC_RUNTIME_LIBRARY to "MultiThreadedDLL" in root CMakeLists.txt.
#
#    Kathleen Biagas, Thu May 2, 2024
#    Use VISIT_PLUGIN_DIR when setting RUNTIME_OUTPUT_DIR for plugins.
#
#*****************************************************************************

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

function(ADD_TARGET_INCLUDE target)
      set_property(TARGET ${target}
                   APPEND
                   PROPERTY INCLUDE_DIRECTORIES ${ARGN})
endfunction()

function(ADD_TARGET_DEFINITIONS target newDefs)
        set_property(TARGET ${target}
                     APPEND
                     PROPERTY COMPILE_DEFINITIONS ${newDefs})
endfunction()

function(ADD_PARALLEL_LIBRARY target)
    add_library(${target} ${ARGN})

    if(UNIX)
      if(VISIT_PARALLEL_CXXFLAGS)
        set(PAR_COMPILE_FLAGS "")
        foreach(X ${VISIT_PARALLEL_CXXFLAGS})
            set(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        endforeach()
        set_property(TARGET ${target}
                     APPEND PROPERTY COMPILE_FLAGS ${PAR_COMPILE_FLAGS})
        if(VISIT_PARALLEL_LINKER_FLAGS)
            set(PAR_LINK_FLAGS "")
            foreach(X ${VISIT_PARALLEL_LINKER_FLAGS})
                set(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            endforeach()
            set_property(TARGET ${target}
                     APPEND PROPERTY LINK_FLAGS ${PAR_LINK_FLAGS})
        endif()

        if(VISIT_PARALLEL_RPATH)
            set(PAR_RPATHS "")
            foreach(X ${CMAKE_INSTALL_RPATH})
                set(PAR_RPATHS "${PAR_RPATHS} ${X}")
            endforeach()
            foreach(X ${VISIT_PARALLEL_RPATH})
                set(PAR_RPATHS "${PAR_RPATHS} ${X}")
            endforeach()
            set_property(TARGET ${target}
                     APPEND PROPERTY INSTALL_RPATH ${PAR_RPATHS})
        endif()
      endif()

    else()
      ADD_TARGET_INCLUDE(${target} ${VISIT_PARALLEL_INCLUDE})
      ADD_TARGET_DEFINITIONS(${target} ${VISIT_PARALLEL_DEFS})
    endif()
    if(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
        target_link_libraries(${target} ${VISIT_PARALLEL_LIBS})
    endif()
endfunction()

macro(VISIT_PLUGIN_TARGET_OUTPUT_DIR type)
    if(WIN32)
        # sets plugin RUNTIME_OUTPUT_DIRECTORY on Windows for plugins
        # prevents $<CONFIG> from being appended to the path, resulting
        # in exe/Release/plots/Release etc.
        set_target_properties(${ARGN} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${VISIT_PLUGIN_DIR}/${type}")
    else()
        set_target_properties(${ARGN} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${VISIT_PLUGIN_DIR}/${type}")
    endif()
endmacro()



##############################################################################
# Function that creates a generic XML tool Code Gen Target
# (helper for functions below, don't call directly )
##############################################################################
function(ADD_XML_TOOLS_GEN_TARGET gen_name
                                  src_dir
                                  dest_dir
                                  tool_name
                                  gen_type)
    ####
    # only create code gen targets if:
    #  we aren't on windows
    #  our cmake for gen targets option is on
    ####
    if(VISIT_CREATE_XMLTOOLS_GEN_TARGETS)
        set(gen_target_name "gen_${gen_type}_${gen_name}")

        MESSAGE(STATUS "Adding ${tool_name} generation target: ${gen_target_name}")

        if(WIN32)
            # need to test this in the future
            set(xml_gen_tool visit_exe -${tool_name})
        else()
            set(xml_gen_tool ${CMAKE_BINARY_DIR}/bin/${tool_name})
        endif()

        # construct path to source file, we need to run
        # in the dir where we want the code to gen
        set(xml_input ${src_dir}/${gen_name}.xml)

        if(NOT IS_ABSOLUTE ${xml_input})
            set(xml_input "${CMAKE_CURRENT_SOURCE_DIR}/${xml_input}")
        endif()

        add_custom_target(${gen_target_name}
            COMMAND ${xml_gen_tool} -clobber ${xml_input}
            DEPENDS  ${xml_file} ${tool_name}
            WORKING_DIRECTORY ${dest_dir}
            COMMENT "Running ${tool_name} on ${gen_name}" VERBATIM)

        if(WIN32)
            set_target_properties(${gen_target_name} PROPERTIES
                FOLDER "generators/${gen_type}")
            add_dependencies(${gen_target_name} visit_exe)
        endif()

        # connect this target to roll up target for all python gen
        set(top_level_target "gen_${gen_type}_all")
        if(NOT TARGET ${top_level_target})
            add_custom_target(${top_level_target})
            if(WIN32)
                set_target_properties(${top_level_target} PROPERTIES
                    FOLDER "generators/all")
            endif()
        endif()

        add_dependencies(${top_level_target} ${gen_target_name})
    endif()
endfunction()

#############################################################################
# Function that creates XML tools C++ Code Gen Target
##############################################################################
function(ADD_CPP_GEN_TARGET gen_name
                            src_dir
                            dest_dir)

    ADD_XML_TOOLS_GEN_TARGET(${gen_name}
                             ${src_dir}
                             ${dest_dir}
                             "xml2atts"
                             "cpp")

endfunction()

##############################################################################
# Function that creates XML tools Python Code Gen Target
##############################################################################
function(ADD_PYTHON_GEN_TARGET gen_name
                               src_dir
                               dest_dir)

   ADD_XML_TOOLS_GEN_TARGET(${gen_name}
                            ${src_dir}
                            ${dest_dir}
                            "xml2python"
                            "python")

endfunction()

##############################################################################
# Function that creates XML tools Java Code Gen Target
##############################################################################
function(ADD_JAVA_GEN_TARGET gen_name
                             src_dir
                             dest_dir)

     ADD_XML_TOOLS_GEN_TARGET(${gen_name}
                              ${src_dir}
                              ${dest_dir}
                              "xml2java"
                              "java")

endfunction()

##############################################################################
# Function that creates XML tools Info Code Gen Target
##############################################################################
function(ADD_INFO_GEN_TARGET gen_name
                             src_dir
                             dest_dir)

    ADD_XML_TOOLS_GEN_TARGET(${gen_name}
                             ${src_dir}
                             ${dest_dir}
                             "xml2info"
                             "info")

endfunction()

##############################################################################
# Function that creates XML tools Info Code Gen Target
##############################################################################
function(ADD_CMAKE_GEN_TARGET gen_name
                              src_dir
                              dest_dir)

    ADD_XML_TOOLS_GEN_TARGET(${gen_name}
                             ${src_dir}
                             ${dest_dir}
                             "xml2cmake"
                             "cmake")

endfunction()


##############################################################################
# This macro appends to a CACHE var list denoted by the NAME argument.
# Caller is responsible for unsetting the CACHE var when no longer needed
# to avoid polluting the CACHE.
#
# Designed mainly for targets whose sources live in subdirectories,
# as a means for them to add to the parent's list of source/includes/etc
# to prepare for a 'blt_add_library' call which requires SOURCES.
#
##############################################################################

macro(visit_append_list)
    cmake_parse_arguments(arg "" "NAME" "ITEMS" ${ARGN})
    if(NOT DEFINED arg_NAME OR NOT DEFINED arg_ITEMS)
        message(FATAL_ERROR "visit_append_list called with invalid arguments. Must supply 'NAME' (name of list) and 'ITEMS' (list of items to be added to named list.)")
    endif()
    set(${arg_NAME} ${${arg_NAME}} ${arg_ITEMS} CACHE STRING "" FORCE)
endmacro()

##############################################################################
# Adds a library target. Wrapper around blt_add_library so that CACHE vars
# possibly created by visit_append_list for forming SOURCES/INCLUDES, etc
# can be unset.
#
# ARGUMENTS:
#    NAME         library name               REQUIRED
#    SOURCES      [source1 [source2 ...]]    REQUIRED
#    HEADERS      [header1 [header2 ...]]    OPTIONAL (except for header-only)
#    INCLUDES     [dir1 [dir2 ...]]          OPTIONAL
#    DEFINES      [define1 [define2 ...]]    OPTIONAL
#    DEPENDS_ON   [dep1 ...]                 OPTIONAL
#    OUTPUT_NAME  [name]                     OPTIONAL
#    FEATURES     [feat1 [feat2 ...]]        OPTIONAL
#    FOLDER       [name])                    OPTIONAL
#
##############################################################################

macro(visit_add_library)
    set(options)
    set(singleValueArgs NAME OUTPUT_NAME FOLDER)
    set(multiValueArgs SOURCES HEADERS INCLUDES DEFINES DEPENDS_ON FEATURES)

    # parse the arguments
    cmake_parse_arguments(val
        "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN} )

    # Sanity checks
    if(NOT val_NAME)
        message(FATAL_ERROR "visit_add_library() must be called with argument NAME <name>")
    endif()

    if (NOT val_SOURCES AND NOT val_HEADERS)
        message(FATAL_ERROR "visit_add_library(NAME ${val_NAME} ...) called with no given sources or headers (at least one is required).")
    endif()

    blt_add_library(
        NAME       ${val_NAME}
        SOURCES    ${val_SOURCES}
        HEADERS    ${val_HEADERS}
        INCLUDES   ${val_INCLUDES}
        DEFINES    ${val_DEFINES}
        DEPENDS_ON ${val_DEPENDS_ON}
        FOLDER     ${val_FOLDER})

    # currently not a part of blt_add_library, and causes a CMake
    # error if we call blt_add_library(${ARGV})
    if (val_FEATURES)
        target_compile_features(${val_NAME} PRIVATE ${val_FEATURES})
    endif()

    visit_install_export_targets(${val_NAME})

    # vars that may have been created by calls to visit_append_list
    unset(${val_NAME}_SOURCES CACHE)
    unset(${val_NAME}_HEADERS CACHE)
    unset(${val_NAME}_INCLUDES CACHE)
    unset(${val_NAME}_DEFINES CACHE)
    unset(${val_NAME}_DEPENDS CACHE)
    unset(${val_NAME}_FEATURES CACHE)
endmacro()

