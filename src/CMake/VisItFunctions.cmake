# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


# ------------------------------------------------------------------------
#  Modifications:
#    Kathleen Biagas, Tue Jan 31, 2023
#    Add visit_install_system_libs and visit_install_mpich functions.
#
# ------------------------------------------------------------------------

function(VISIT_INSTALL_TARGETS_RELATIVE dest_dir)
    if(VISIT_STATIC)
        # Skip installation of static libraries when we build statically
        foreach(T ${ARGN})
            get_target_property(pType ${T} TYPE)
            if(NOT ${pType} STREQUAL "STATIC_LIBRARY")
                install(TARGETS ${T}
                    RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
                    BUNDLE  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
                    LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/${dest_dir}
                    ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}/${dest_dir}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                )
            endif()
        endforeach()
    else()
        INSTALL(TARGETS ${ARGN}
            RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
            BUNDLE  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/${dest_dir}
            LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/${dest_dir}
            ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}/${dest_dir}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE
            CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
        )
    endif()
endfunction()

function(VISIT_INSTALL_TARGETS)
    VISIT_INSTALL_TARGETS_RELATIVE("" ${ARGN})
endfunction()

function(VISIT_3RDPARTY_VAR libvar comment)
    set(cache_type ${ARGN})
    if(NOT cache_type)
        set(cache_type "PATH")
    endif()
    if(NOT DEFINED VISIT_${libvar})
        # Create an empty entry for this path variable
        set(VISIT_${libvar} "" CACHE ${cache_type} "${comment}")
    else()
        if(DEFINED ${libvar})
            # Ensure VISIT_FOO_DIR=FOO_DIR (override from CL)
            set(VISIT_${libvar} ${${libvar}} CACHE ${cache_type} "${comment}" FORCE)
        else()
            # Set FOO_DIR=VISIT_FOO_DIR (default from config-site file)
            set(${libvar} ${VISIT_${libvar}} CACHE INTERNAL "${comment}")
        endif()
    endif()
endfunction()

function(VISIT_3RDPARTY_DEP libvar comment)
    VISIT_3RDPARTY_VAR(${libvar} ${comment} "STRING")
endfunction()

function(VISIT_3RDPARTY_LIBDEP pkg)
    VISIT_3RDPARTY_DEP(${pkg}_LIBDEP "${pkg} library dependencies")
endfunction()

function(VISIT_3RDPARTY_INCDEP pkg)
    VISIT_3RDPARTY_DEP(${pkg}_INCDEP "${pkg} include dependencies")
endfunction()

function(VISIT_3RDPARTY_DIR pkg)
    VISIT_3RDPARTY_VAR(${pkg}_DIR "Path containing ${pkg}")
endfunction()

#-----------------------------------------------------------------------------
# Helper function for setting build options via config-site file.
# Supports optional argument pair: TYPE [CMAKE_TYPE], default type = PATH
#-----------------------------------------------------------------------------
function(VISIT_OPTION_DEFAULT var)
    set(opt_ARGS "")
    set(opt_TYPE "PATH")
    set(opt_ARG_PREV "[unset]")
    foreach(opt_ARG ${ARGN})
        ## if arg is TYPE:
        ##   we want to use the next value as the cmake var type
        ## otherwise:
        ##   we want to append the argument to the list of args used for "SET"
        if(${opt_ARG_PREV} STREQUAL "TYPE")
            set(opt_TYPE ${opt_ARG})
        elseif(NOT ${opt_ARG} STREQUAL "TYPE")
            list(APPEND opt_ARGS ${opt_ARG})
        endif()
        set(opt_ARG_PREV ${opt_ARG})
    endforeach()
    ##
    set(default_base "${var}_DEFAULT")
    set(default_base_val  "${${default_base}}")
    if(NOT "${default_base_val}" STREQUAL "${opt_ARGS}")
        set("${default_base}" "${opt_ARGS}" CACHE INTERNAL "${var} default value" FORCE)
        set(${var} "${opt_ARGS}" CACHE ${opt_TYPE} "${var} value" FORCE)
    endif()
endfunction()


function(SETUP_APP_VERSION name ver_ascii )
    if(${ver_ascii} VERSION_LESS ${${name}_MINIMUM_VERSION}})
        message(FATAL_ERROR "${name} version must be at least ${${name}_MINIMUM_VERSION}}")
    endif()
    string( REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+[.0-9]*.*" "\\1" ver_major ${ver_ascii} )
    string( REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+[.0-9]*.*" "\\1" ver_minor ${ver_ascii} )
    string( REGEX REPLACE "[0-9]+.[0-9]+.([0-9]+)[.0-9]*.*" "\\1" ver_patch ${ver_ascii} )
    set(${name}_VERSION "${ver_ascii}"  CACHE INTERNAL "")
    set(${name}_MAJOR_VERSION "${ver_major}" CACHE INTERNAL "")
    set(${name}_MINOR_VERSION "${ver_minor}" CACHE INTERNAL "")
    set(${name}_PATCH_VERSION "${ver_patch}" CACHE INTERNAL "")
    math( EXPR ver_hex "(${ver_major}<<16)|(${ver_minor}<<8)|(${ver_patch})")
    set(${name}_VERSION_HEX "${ver_hex}" CACHE INTERNAL "")
endfunction()


function(visit_install_system_libs)
    #-------------------------------------------------------------------------
    # Install the system libraries required by Qt to the lib directory.
    # The versions of these libraries change frequently, so including
    # them means VisIt may continue to run even if the OS is upgraded.
    # "/lib64" is for rhel7 and fedora31, "/usr/lib/x86_64-linux-gnu"
    # is for ubuntu 18, debian 10 and debian 11, "/lib/x86_64-linux-gnu"
    # is for ubuntu 20.
    #-------------------------------------------------------------------------
    if(EXISTS "/lib64/libicui18n.so")
        file(GLOB libicui18n_libs "/lib64/libicui18n.so*")
        file(GLOB libicudata_libs "/lib64/libicudata.so*")
        file(GLOB libicuuc_libs "/lib64/libicuuc.so*")
        set(icu_libs "")
        list(APPEND icu_libs ${libicui18n_libs})
        list(APPEND icu_libs ${libicudata_libs})
        list(APPEND icu_libs ${libicuuc_libs})
        install(FILES ${icu_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    elseif(EXISTS "/lib/x86_64-linux-gnu/libicui18n.so")
        file(GLOB libicui18n_libs "/lib/x86_64-linux-gnu/libicui18n.so*")
        file(GLOB libicudata_libs "/lib/x86_64-linux-gnu/libicudata.so*")
        file(GLOB libicuuc_libs "/lib/x86_64-linux-gnu/libicuuc.so*")
        set(icu_libs "")
        list(APPEND icu_libs ${libicui18n_libs})
        list(APPEND icu_libs ${libicudata_libs})
        list(APPEND icu_libs ${libicuuc_libs})
        install(FILES ${icu_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    elseif(EXISTS "/usr/lib/x86_64-linux-gnu/libicui18n.so")
        file(GLOB libicui18n_libs "/usr/lib/x86_64-linux-gnu/libicui18n.so*")
        file(GLOB libicudata_libs "/usr/lib/x86_64-linux-gnu/libicudata.so*")
        file(GLOB libicuuc_libs "/usr/lib/x86_64-linux-gnu/libicuuc.so*")
        set(icu_libs "")
        list(APPEND icu_libs ${libicui18n_libs})
        list(APPEND icu_libs ${libicudata_libs})
        list(APPEND icu_libs ${libicuuc_libs})
        install(FILES ${icu_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    endif()

    #-------------------------------------------------------------------------
    # If the g++ standard library is specified, then install it in the lib
    # directory. This is typically needed if using a non-standard version
    # of the g++ compiler.
    #-------------------------------------------------------------------------
    if(VISIT_CXX_LIBRARY)
        file(GLOB libstdcxx_libs "${VISIT_CXX_LIBRARY}*")
        install(FILES ${libstdcxx_libs}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    endif()
endfunction()


function(visit_install_mpich)
    set(vfp OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
    set(vdp ${vfp} OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)

    install(DIRECTORY ${VISIT_MPICH_DIR}/include
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/mpich
            FILE_PERMISSIONS ${vfp}
            DIRECTORY_PERMISSIONS ${vdp})

    install(DIRECTORY ${VISIT_MPICH_DIR}/bin
            DESTINATION ${VISIT_INSTALLED_VERSION}
            FILE_PERMISSIONS ${vfp}
            DIRECTORY_PERMISSIONS ${vdp})

    install(DIRECTORY ${VISIT_MPICH_DIR}/lib
            DESTINATION ${VISIT_INSTALLED_VERSION}
            FILE_PERMISSIONS ${vfp}
            DIRECTORY_PERMISSIONS ${vdp})
endfunction()

