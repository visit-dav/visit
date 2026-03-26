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
#
#****************************************************************************

macro(visit_tp_append_list)
    cmake_parse_arguments(arg "" "NAME" "ITEMS" ${ARGN})
    if(NOT DEFINED arg_NAME OR NOT DEFINED arg_ITEMS)
        message(FATAL_ERROR "visit_tp_append_list called with invalid arguments. Must supply 'NAME' (name of list) and 'ITEMS' (list of items to be added to named list.)")
    endif()
    set(${arg_NAME} ${${arg_NAME}};${arg_ITEMS} CACHE STRING "" FORCE)
endmacro()


macro(is_prop_set is_set prop targetName)
    if(TARGET ${targetName})
        get_property(${is_set} TARGET ${targetName} PROPERTY ${prop} SET)
    else()
        message(WARNING "attempting to test if property is set on non existent target ${targetName}")
        set(${is_set} false)
    endif()
endmacro()

function(write_prop_if_set fname ptype targetName)
    is_prop_set(propIsSet ${ptype} ${targetName})
    if(propIsSet)
        get_target_property(prop ${targetName} ${ptype})
        file(APPEND ${fname} "    ${ptype} \"${prop}\"\n")
    endif()
endfunction()

macro(has_props_we_care_about has_props targetName)

    list(APPEND props INTERFACE_INCLUDE_DIRECTORIES)
    list(APPEND props IMPORTED_IMPLIB)
    list(APPEND props IMPORTED_IMPLIB_RELEASE)
    list(APPEND props IMPORTED_LOCATION)
    list(APPEND props IMPORTED_LOCATION_RELEASE)
    list(APPEND props INTERFACE_LINK_LIBRARIES)
    list(APPEND props INTERFACE_COMPILE_OPTIONS)
    list(APPEND props INTERFACE_COMPILE_DEFINITIONS)
    list(APPEND props IMPORTED_SONAME)
    list(APPEND props IMPORTED_SONAME_RELEASE)
    foreach(item ${props})
        is_prop_set(hasit ${item} ${targetName})
        if(${hasit})
            break()
        endif()
    endforeach()
    set(${has_props} ${hasit})
endmacro()

function(write_lib_location_type fname libType targetName prefix libdir)
    is_prop_set(isLibSet ${libType} ${targetName})
    if(isLibSet)
        get_target_property(ilib ${targetName} ${libType})
        cmake_path(GET ilib FILENAME ilib_name)
        if(libdir STREQUAL "")
            file(APPEND ${fname} "    ${libType} \"\${${prefix}}/${ilib_name}\"\n")
        else()
            file(APPEND ${fname} "    ${libType} \"\${${prefix}}/${libdir}/${ilib_name}\"\n")
        endif()
    endif()
endfunction()

function(write_lib_location fname targetName prefix)
    get_target_property(ttype ${targetName} TYPE)

    if(WIN32)
        if(${ttype} STREQUAL "SHARED_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_IMPLIB ${targetName} ${prefix} "lib")
            write_lib_location_type(${fname} IMPORTED_IMPLIB_RELEASE ${targetName} ${prefix} "lib")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "")
            write_lib_location_type(${fname} IMPORTED_LOCATION_RELEASE ${targetName} ${prefix} "")
        elseif(${ttype} STREQUAL "STATIC_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "lib")
            write_lib_location_type(${fname} IMPORTED_LOCATION_RELEASE ${targetName} ${prefix} "lib")
        elseif(${ttype} STREQUAL "EXECUTABLE")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "")
            write_lib_location_type(${fname} IMPORTED_LOCATION_RELEASE ${targetName} ${prefix} "")
        endif()
    elseif(LINUX)
        if(${ttype} STREQUAL "EXECUTABLE")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "bin")
            write_lib_location_type(${fname} IMPORTED_LOCATION_RELEASE ${targetName} ${prefix} "bin")
        elseif(${ttype} STREQUAL "SHARED_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "lib")
            write_lib_location_type(${fname} IMPORTED_LOCATION_RELEASE ${targetName} ${prefix} "lib")
        elseif(${ttype} STREQUAL "STATIC_LIBRARY")
            write_lib_location_type(${fname} IMPORTED_LOCATION ${targetName} ${prefix} "archives")
            write_lib_location_type(${fname} IMPORTED_LOCATION_RELEASE ${targetName} ${prefix} "archives")
        endif()
    else()
        # install locations for mac are same as linux for visit tp, but
        # I don't know about the 'IMPORTED_ZZZ', need someone with a Mac
        # to provide samples from VTKm, Qt, VTK, anari
    endif()
endfunction()

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


function(write_library_entry fname targetName namespace incBase prefix useSimpleInclude hasNestedIncludes)

    get_target_property(ttype ${targetName} TYPE)

    if(${ttype} STREQUAL "SHARED_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} SHARED IMPORTED)\n")
    elseif(${ttype} STREQUAL "INTERFACE_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} INTERFACE IMPORTED)\n")
    elseif(${ttype} STREQUAL "STATIC_LIBRARY")
        file(APPEND ${fname} "add_library(${targetName} STATIC IMPORTED)\n")
    elseif(${ttype} STREQUAL "EXECUTABLE")
        file(APPEND ${fname} "add_executable(${targetName} IMPORTED)\n")
    endif()

    is_prop_set(propIsSet IMPORTED_CONFIGURATIONS ${targetName})
    if(propIsSet)
        get_target_property(conf ${targetName} IMPORTED_CONFIGURATIONS)
        file(APPEND ${fname} "set_property(TARGET ${targetName} APPEND PROPERTY IMPORTED_CONFIGURATIONS \"${conf}\")\n")
    endif()

    write_target_props(${fname} ${targetName} ${namespace} ${incBase}
                       ${prefix} ${useSimpleInclude} ${hasNestedIncludes})
endfunction()

# assumes initial WRITE has already occurred
function(write_copyright fname)
    file(APPEND ${fname} "# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt\n")
    file(APPEND ${fname} "# Project developers.  See the top-level LICENSE file for dates and other\n")
    file(APPEND ${fname} "# details.  No copyright assignment is required to contribute to VisIt.\n\n")
endfunction()

# assumes initial WRITE has already occurred
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

function(get_all_targets)
    cmake_parse_arguments(gat "" "KIT;NAMESPACE" "ITEMS" ${ARGN})
    if(NOT DEFINED gat_KIT)
        message(FATAL_ERROR "get_all_targets missing KIT arg")
    endif()
    if(NOT DEFINED gat_NAMESPACE)
        message(FATAL_ERROR "get_all_targets missing NAMESPACE arg")
    endif()
    if( NOT DEFINED gat_ITEMS)
        message(FATAL_ERROR "get_all_targets missing ITEMS arg")
    endif()

    # used for determining if a target belongs to the given namespace
    string(LENGTH ${gat_NAMESPACE} size)
    foreach(t ${gat_ITEMS})
        if(NOT TARGET ${t})
            continue()
        endif()
        if(size GREATER_EQUAL 0)
            string(SUBSTRING ${t} 0 ${size} starts_with_namespace)
            if(starts_with_namespace STREQUAL ${gat_NAMESPACE})
                visit_tp_append_list(NAME ${gat_KIT}_list ITEMS ${t})
            endif()
        else()
        endif()
        get_target_property(ill ${t} INTERFACE_LINK_LIBRARIES)
        if(ill)
            foreach(lib ${ill})
                get_all_targets(KIT ${gat_KIT}
                                NAMESPACE ${gat_NAMESPACE}
                                ITEMS ${lib})
            endforeach()
        endif()
    endforeach()
endfunction()


function(create_lib_setup_cmake)
    # this is the main function that should be called from one of VisIt's
    # 'Find' modules. Used for complex Third party libraries for which
    # 'blt_import_library' cannot be used with the EXPORTABLE flag on.
    #
    # required arguments:
    #   KIT               Name of the ThirdParty library
    #                         (will be used for the filename).
    #   NAMESPACE         Namespace used by KIT.
    #   ITEMS             Initial list of targets to be written.
    #   INCBASE           Base of include directory
    #
    # optional arguments:
    #   SIMPLE_INCLUDE    If true, don't get includes from targets.
    #   NESTED_INCLUDES   If true, targets may have deeply nested includes.
    #   SKIP_HEADER       Don't write the header. Useful for multi-namespace
    #                     libraries that need to call this function twice
    #                     (like Qt).
    #
    cmake_parse_arguments(clsc "SIMPLE_INCLUDE;NESTED_INCLUDES;SKIP_HEADER" "KIT;NAMESPACE;INCBASE" "ITEMS" ${ARGN})
    if(NOT DEFINED clsc_KIT)
        message(FATAL_ERROR "create_lib_setup_make missing KIT arg")
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
    set(${clsc_KIT}_list)
    get_all_targets(KIT       ${clsc_KIT}
                    NAMESPACE ${clsc_NAMESPACE}
                    ITEMS     ${clsc_ITEMS})
    list(REMOVE_DUPLICATES ${clsc_KIT}_list)

    set(fname ${VISIT_BINARY_DIR}/Setup${clsc_KIT}.cmake)
    if(NOT ${clsc_SKIP_HEADER})
        file(WRITE ${fname} "")
        write_standard_header(${fname} ${clsc_KIT})
    endif()

    set(prefix ${clsc_KIT}_PREFIX)
    foreach(onetarget ${${clsc_KIT}_list})
        write_library_entry(${fname} ${onetarget} ${clsc_NAMESPACE} ${clsc_INCBASE} ${prefix} ${clsc_SIMPLE_INCLUDE} ${clsc_NESTED_INCLUDES})
    endforeach()

    install(FILES ${fname}
            DESTINATION  ${VISIT_INSTALLED_VERSION}/cmake)

    # cleanup
    unset(${clsc_KIT}_list CACHE)
    unset(fname)
    unset(prefix)
endfunction()

