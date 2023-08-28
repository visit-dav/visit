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
#*****************************************************************************

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    # cmake automatically uses _DEBUG and MDd for Debug flags, but our
    # third-party libs are NOT debug versions, so we won't be able to
    # link against them if those defines are used.  Replace those flags:
    # When we allow users to supply their own third-party libs, then
    # this will have to be rethought -- they may want to be able to c
    # change them and this currently forces.

    macro(REPLACE_FLAG OLD_FLAG NEW_FLAG FLAG_TYPE FLAG_STRING)
        string(REPLACE "${OLD_FLAG}" "${NEW_FLAG}" TMP "${${FLAG_TYPE}}")
        set(${FLAG_TYPE} "${TMP}" CACHE STRING "${FLAG_STRING}" FORCE)
    endmacro()

    # Remove /_DEBUG From debug builds
    if(VISIT_REPLACE_DEBUG_FLAGS)
        REPLACE_FLAG("/D_DEBUG" "" CMAKE_CXX_FLAGS_DEBUG
                     "Flags used by the compiler during debug builds")
        REPLACE_FLAG("/D_DEBUG" "" CMAKE_C_FLAGS_DEBUG
                     "Flags used by the compiler during debug builds")
        REPLACE_FLAG("/D_DEBUG" "" CMAKE_EXE_LINKER_FLAGS_DEBUG
                     "Flags used by the linker during debug builds")
        REPLACE_FLAG("/D_DEBUG" "" CMAKE_MODULE_LINKER_FLAGS_DEBUG
                     "Flags used by the linker during debug builds")
        # Change /MDd to /MD for debug builds
        REPLACE_FLAG("/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG
                     "Flags used by the compiler during debug builds")
        REPLACE_FLAG("/MDd" "/MD" CMAKE_C_FLAGS_DEBUG
                     "Flags used by the compiler during debug builds")
        REPLACE_FLAG("/MDd" "/MD" CMAKE_EXE_LINKER_FLAGS_DEBUG
                     "Flags used by the linker during debug builds")
        REPLACE_FLAG("/MDd" "/MD" CMAKE_MODULE_LINKER_FLAGS_DEBUG
                     "Flags used by the linker during debug builds")
    endif()
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
            RUNTIME_OUTPUT_DIRECTORY
                "${VISIT_BINARY_DIR}/exe/$<CONFIG>/${type}")
    else()
        set_target_properties(${ARGN} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${VISIT_BINARY_DIR}/plugins/${type}")
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


