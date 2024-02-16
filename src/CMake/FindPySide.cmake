# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu June 14 15:48:01 MST 2012
#   Add support for pyside 1.1.1 on windows, which no longer has a separate
#   generatorrunner module.  For PYSIDE_ADD_MODULE function, change the
#   target's output location on windows to the exe dir, explanation for this
#   is within the code below.  Change gen_pathsep on windows to "\;".
#
#   Gunther H. Weber, Thu Nov 21 18:28:56 PST 2013
#   Allow user to set Shiboken_FOUND and PySide_FOUND to set up relevant
#   CMake variables manually (necessary for MacPorts PySide install).
#   Use ${PYSIDE_PYTHONPATH} instead of ${VISIT_PYSIDE_DIR}/lib/site-packages
#   to support using system PySide.
#
#   Kathleen Biagas, Wed Dec 18 08:05:38 PST 2013
#   Exclude '.svn' from INSTALL command, probably a rare case, but a
#   developer did run into an issue.
#
#   Kathleen Biagas, Tue Jan 24 11:13:05 PST 2017
#   Add support for Qt5, PySide2.
#
#   Kevin Griffin, Wed Jan 10 10:46:43 PST 2018
#   Changes are for OSX and PySide2: Added the capability to optionally append
#   a 'v' to the Python version when creating the library names. Also
#   expanded the list of libraries for pyside and shiboken to include the
#   different naming schemes for the same library.
#
#   Kathleen Biagas, Tue Jun 30 13:22:03 PDT 2020
#   Updates for pyside2 5.14.2, also, look for pyside within python first,
#   then the stand-alone way via VISIT_PYSIDE_DIR.
#
#   Eric Brugger, Thu May 26 14:29:34 PDT 2022
#   Added the path of the shiboken2 SONAME to SET_UP_THIRD_PARTY so that
#   Python could find the shiboken2 module when the directory containing
#   the PySide2 build was no longer accessible.
#
#****************************************************************************/


if(NOT PySide_FOUND)

    if(NOT PYTHON_EXECUTABLE OR NOT PYTHON_LIBRARY)
        message(WARNING "PySide2 requires python, but python not found, disabling PySide")
        set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
        return()
    endif()

    # first test if it is installed in python
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import PySide2"
                    OUTPUT_QUIET
                    ERROR_QUIET
                    RESULT_VARIABLE HAVE_PYSIDE_IN_PYTHON)

    if(NOT HAVE_PYSIDE_IN_PYTHON EQUAL 0)
        # the above execute_process command *should* work.
        # When the compiler used to build TP libs and VisIt is NOT the default
        # compiler on the system, it will possibly fail due to a mismatch between libstdc++
        # used for compiling pyside2 and the system libstdc++
        # So, we perform an extra check if needed.
        if(EXISTS ${PYTHON_DIR}/bin/shiboken2 AND
           EXISTS ${PYTHON_SITE_PACKAGES_DIR}/PySide2 AND
           EXISTS ${PYTHON_SITE_PACKAGES_DIR}/shiboken2)
           # doesn't really make sense to set this to 0, other than it matches
           # with the result value if the above execute_process succeeds.
           set(HAVE_PYSIDE_IN_PYTHON 0)
        endif()
    endif()
    if(HAVE_PYSIDE_IN_PYTHON EQUAL 0)
        set(PYSIDE_IN_PYTHON true)
        # use a macro taken from an example within PySide2
        # The config script is located in  python's lib/site-packages/PySide2/examples/utils dir
        # perhaps we want to do this a different way?

        set(pyside_config_script ${PYTHON_SITE_PACKAGES_DIR}/PySide2/examples/utils/pyside2_config.py)

        if (NOT EXISTS ${pyside_config_script})
            message(WARNING "Cannot find PySide2 config script in ${pyside_config_script}, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)

            # should probably have a different find mechanism here perhaps along the lines of searching
            # for an installed version outside of python via VISIT_PYSIDE_DIR, or a find_package
            return()
        endif()

        macro(pyside2_config which_option output_var)
           if(${ARGC} GREATER 2)
                set(is_list ${ARGV2})
            else()
                set(is_list "")
            endif()

            execute_process(COMMAND ${PYTHON_EXECUTABLE} "${pyside_config_script}" ${which_option}
                OUTPUT_VARIABLE ${output_var}
                OUTPUT_STRIP_TRAILING_WHITESPACE)

            if ("${${output_var}}" STREQUAL "")
                message(WARNING "Error: Calling pyside2_config.py ${option} returned no output.")
            endif()
            if(is_list)
                string (REPLACE " " ";" ${output_var} "${${output_var}}")
            endif()
        endmacro()

        # Find the shiboken generator
        pyside2_config("--shiboken2-generator-path" shib2_gen_path)
        if(shib2_gen_path)
            find_program(SHIBOKEN_BINARY
                         NAMES shiboken2
                         PATHS ${shib2_gen_path}
                         DOC "Shiboken generator executable"
                         NO_DEFAULT_PATH)
        endif()

        if(NOT SHIBOKEN_BINARY)
            message(WARNING "Cannot find PySide2 shiboken generator, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            return()
        endif()

        pyside2_config("--pyside2-path" pyside2_path)

        if(pyside2_path)
            if(EXISTS ${pyside2_path}/typesystems)
                set(PYSIDE_TYPESYSTEMS ${pyside2_path}/typesystems)
            endif()
        endif()

        if(NOT PYSIDE_TYPESYSTEMS)
            message(WARNING "Cannot find PySide2 typesystems dir, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            return()
        endif()

        pyside2_config("--shiboken2-generator-include-path" SHIBOKEN_INCLUDE_DIR)
        if(NOT SHIBOKEN_INCLUDE_DIR)
            message(WARNING "Cannot find PySide2 shiboken include dir, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            return()
        endif()

        pyside2_config("--pyside2-include-path" PYSIDE_INCLUDE_DIR)
        if(NOT PYSIDE_INCLUDE_DIR)
            message(WARNING "Cannot find PySide2 include dir, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            return()
        endif()

        pyside2_config("--shiboken2-module-shared-libraries-cmake" SHIBOKEN_LIBRARY)
        if(NOT SHIBOKEN_LIBRARY)
            message(WARNING "Cannot find PySide2 shiboken library, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            return()
        endif()

        pyside2_config("--pyside2-shared-libraries-cmake" PYSIDE_LIBRARY)
        if(NOT PYSIDE_LIBRARY)
            message(WARNING "Cannot find PySide2 pyside library, disabling PySide")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            return()
        endif()

        set(HAVE_LIBPYSIDE TRUE CACHE BOOL "Have PySide2" FORCE)
        set(PySide_FOUND 1)
        message(STATUS "Pyside2 Found")
        message(STATUS "    shiboken generator: ${SHIBOKEN_BINARY}")
        message(STATUS "    shiboken include:   ${SHIBOKEN_INCLUDE_DIR}")
        message(STATUS "    shiboken libary:    ${SHIBOKEN_LIBRARY}")
        message(STATUS "    pyside include:     ${PYSIDE_INCLUDE_DIR}")
        message(STATUS "    pyside library:     ${PYSIDE_LIBRARY}")
        message(STATUS "    pyside typesystems: ${PYSIDE_TYPESYSTEMS}")
    else()
        set(PYSIDE_IN_PYTHON false)
        # PySide not installed in python, it's a stand-alone, handle in usual manner

        # pyside's cmake find logic unsets PYTHON_FOUND, so save it now for restoration after
        set(vpy_found ${PYTHON_FOUND})
        if(VISIT_PYSIDE_DIR)
            #  Find Shiboken
            set(CMAKE_PREFIX_PATH ${VISIT_PYSIDE_DIR}/lib/cmake/Shiboken2-${PYSIDE_VERSION}.2)

            if(NOT Shiboken2}_FOUND)
                find_package(Shiboken2 ${PYSIDE_VERSION})
                if(TARGET Shiboken2::shiboken2)
                    get_target_property(SHIBOKEN_BINARY Shiboken2::shiboken2 IMPORTED_LOCATION_RELEASE)
                    message("SHIBOKEN_BINARY: ${SHIBOKEN_BINARY}")
                else()
                    message(" no shiboken2 target")
                endif()
                if(TARGET Shiboken2::libshiboken)
                    get_target_property(SHIBOKEN_INCLUDE_DIR Shiboken2::libshiboken INTERFACE_INCLUDE_DIRECTORIES)
                    get_target_property(SHIBOKEN_LIBRARY Shiboken2::libshiboken IMPORTED_LOCATION_RELEASE)
                    get_target_property(SHIBOKEN_LIBRARY_SO Shiboken2::libshiboken IMPORTED_SONAME_RELEASE)
                    message("SHIBOKEN_INCLUDE: ${SHIBOKEN_INCLUDE_DIR}")
                    message("SHIBOKEN_LIBRARY: ${SHIBOKEN_LIBRARY}")
                    message("SHIBOKEN_LIBRARY_SO: ${SHIBOKEN_LIBRARY_SO}")
                else()
                    message(" no shiboken2 library target")
                endif()
            endif()

            #  Find PySide
            set(CMAKE_PREFIX_PATH ${VISIT_PYSIDE_DIR}/lib/cmake/PySide2-${PYSIDE_VERSION}.2)
            if (NOT PySide2_FOUND)
                find_package(PySide2 ${PYSIDE_VERSION})
                if(TARGET PySide2::pyside2)
                    get_target_property(PYSIDE_INCLUDE_DIR PySide2::pyside2 INTERFACE_INCLUDE_DIRECTORIES)
                    get_target_property(PYSIDE_LIBRARY PySide2::pyside2 IMPORTED_LOCATION_RELEASE)
                    message("PYSIDE_INCLUDE: ${PYSIDE_INCLUDE_DIR}")
                    message("PYSIDE_LIBRARY: ${PYSIDE_LIBRARY}")
                    message("PYSIDE_TYPESYSTEMS: ${PYSIDE_TYPESYSTEMS}")
                endif()
            endif()

        endif()

        if(NOT PySide2_FOUND OR NOT Shiboken2_FOUND)
            #If we dont have shiboken, force pyside off
            message(STATUS "PySide NOT found")
            set(HAVE_LIBPYSIDE false CACHE BOOL "Have PySide2" FORCE)
            set(PySide_FOUND 0)
        else()
            set(PySide_FOUND 1)
            set(HAVE_LIBPYSIDE true CACHE BOOL "Have PySide2" FORCE)
        endif()

        if(PySide_FOUND)
            # for pyside with python3, libnames are very verbose, so lets
            # retrieve them from XXX_LIBRARY vars set above by the find

            # SET_UP_THIRD_PARTY needs the library name without extension
            get_filename_component(pyside_lib_name ${PYSIDE_LIBRARY} NAME)
            get_filename_component(shiboken_lib_name ${SHIBOKEN_LIBRARY} NAME)

            # Is this call to SET_UP_THIRD_PARTY still needed?
            SET_UP_THIRD_PARTY(PYSIDE LIBS ${pyside_lib_name} ${shiboken_lib_name} ${SHIBOKEN_LIBRARY_SO})

            # Install the pyside and shiboken python site-packages
            install(DIRECTORY ${PYSIDE_PYTHONPATH}
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                    PATTERN ".svn" EXCLUDE
                    PATTERN ".git" EXCLUDE
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE
                    )

            install(DIRECTORY ${SHIBOKEN_PYTHON_MODULE_DIR}
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                    PATTERN ".svn" EXCLUDE
                    PATTERN ".git" EXCLUDE
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE
                    )
        endif(PySide_FOUND)
        set(PYTHON_FOUND ${vpy_found})
        unset(vpy_found)
    endif()
endif()

#****************************************************************************
# PYSIDE_ADD_GENERATOR_TARGET
# Defines a new PySide generator target.
#****************************************************************************
function(PYSIDE_ADD_GENERATOR_TARGET
            target_name gen_sources gen_include_paths gen_global gen_typesystem)

    set(PYSIDE_GENERATOR_EXTRA_FLAGS
                                 --generator-set=shiboken
                                 --enable-parent-ctor-heuristic
                                 --enable-return-value-heuristic
                                 --use-isnull-as-nb_nonzero
                                 --avoid-protected-hack)

    #
    # Create includes list, prepen '-I' to everything in gen_include_paths
    #
    set(gen_include_paths_arg "-I${CMAKE_CURRENT_SOURCE_DIR}")
    list(APPEND gen_include_paths_arg "-I${PYSIDE_INCLUDE_DIR}")


    # provide qt includes
    # Get all relevant Qt include dirs, to pass them on to shiboken.
    get_property(QT_CORE_INCLUDE_DIRS TARGET Qt5::Core PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    get_property(QT_WIDGETS_INCLUDE_DIRS TARGET Qt5::Widgets PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    get_property(QT_GUI_INCLUDE_DIRS TARGET Qt5::Gui PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    get_property(QT_WIDGETS_INCLUDE_DIRS TARGET Qt5::Widgets PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

    foreach(itm ${QT_CORE_INCLUDE_DIRS})
        list(APPEND gen_include_paths_arg "-I${itm}")
    endforeach()

    foreach(itm ${QT_WIDGETS_INCLUDE_DIRS})
        list(APPEND gen_include_paths_arg "-I${itm}")
    endforeach()

    foreach(itm ${QT_GUI_INCLUDE_DIRS})
        list(APPEND gen_include_paths_arg "-I${itm}")
    endforeach()

    foreach(itm ${QT_WIDGETS_INCLUDE_DIRS})
        list(APPEND gen_include_paths_arg "-I${itm}")
    endforeach()

    # On macOS, check if Qt is a framework build. This affects how include paths should be handled.
    # adapted from: https://code.qt.io/cgit/pyside/pyside-setup.git/tree/examples/scriptableapplication/CMakeLists.txt
    get_target_property(QtCore_is_framework Qt5::Core FRAMEWORK)
    if (QtCore_is_framework)
        get_target_property(qt_core_library_location Qt5::Core LOCATION)
        get_filename_component(qt_core_library_location_dir "${qt_core_library_location}" DIRECTORY)
        get_filename_component(lib_dir "${qt_core_library_location_dir}/../" ABSOLUTE)
        list(APPEND gen_include_paths_arg "--framework-include-paths=${lib_dir}")
    endif()

    # add the rest of the includes
    foreach(itm ${${gen_include_paths}})
        list(APPEND gen_include_paths_arg "-I${itm}")
    endforeach()



    if(VISIT_LLVM_DIR)
        # the generator needs to know where to find libclang, so ensure its
        # location is set in the LD_LIBRARY_PATH used by the process
        set(use_ld ${VISIT_LLVM_DIR}/lib:$ENV{LD_LIBRARY_PATH})
        # also need LLVM_INSTALL_DIR set
        set(setenv_cmd ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${use_ld}" "LLVM_INSTALL_DIR=${VISIT_LLVM_DIR}")
    else()
        # if we arent using a visit-compiled llvm, assume env is already set up.
        # not sure if this works!
        set(setenv_cmd "")
    endif()
    add_custom_command(OUTPUT ${${gen_sources}}
                   COMMAND ${setenv_cmd}
                   ${SHIBOKEN_BINARY} ${PYSIDE_GENERATOR_EXTRA_FLAGS}
                   ${gen_include_paths_arg}
                   -T${PYSIDE_TYPESYSTEMS}
                   -T${CMAKE_CURRENT_SOURCE_DIR}
                   --output-directory=${CMAKE_CURRENT_BINARY_DIR}
                   ${gen_global}
                   ${gen_typesystem}
                   DEPENDS  ${gen_global} ${gen_typesystem}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   COMMENT "Running generator for ${${gen_sources}}...")

    add_custom_target(${target_name} DEPENDS ${${gen_sources}})
endfunction(PYSIDE_ADD_GENERATOR_TARGET)


#****************************************************************************
# PYSIDE_ADD_MODULE
# Defines a new PySide module and creates a dependent generator target.
#****************************************************************************
function(PYSIDE_ADD_MODULE module_name
                           dest_dir
                           mod_sources
                           mod_gen_sources
                           mod_gen_include_paths
                           mod_gen_link_libs
                           mod_gen_global
                           mod_gen_typesystem)

    message(STATUS "Configuring PySide module: ${module_name}")


    PYSIDE_ADD_GENERATOR_TARGET("${module_name}_gen"
                                ${mod_gen_sources}
                                ${mod_gen_include_paths}
                                ${mod_gen_global}
                                ${mod_gen_typesystem})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                        ${SHIBOKEN_INCLUDE_DIR}
                        ${PYTHON_INCLUDE_DIR}
                        ${PYSIDE_INCLUDE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}/${module_name}
                        ${${mod_gen_include_paths}})

    add_library(${module_name} MODULE ${${mod_sources}} ${${mod_gen_sources}})

    set_target_properties(${module_name} PROPERTIES PREFIX "")

    if(UNIX)
        set_target_properties(${module_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${VISIT_LIBRARY_DIR}/${dest_dir})
    else()
        # set the appropriate suffix
        set_target_properties(${module_name} PROPERTIES SUFFIX ".pyd")

        # Since cmake will append $(Configuration) to output directories, we need
        # to override that by specifying LIBRARY_OUTPUT_DIRECTORY_${Configuration}
        # for each configuration.
        foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER ${cfg} UCFG)
            set_target_properties(${module_name} PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY_${UCFG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${cfg}/${dest_dir})
        endforeach()
    endif()

    target_link_libraries(${module_name}
                          ${PYTHON_LIBRARIES}
                          ${SHIBOKEN_LIBRARY}
                          ${PYSIDE_LIBRARY}
                          ${${mod_gen_link_libs}})

    add_dependencies(${module_name} "${module_name}_gen")

    VISIT_INSTALL_TARGETS_RELATIVE(${dest_dir} ${module_name})

endfunction(PYSIDE_ADD_MODULE)


#****************************************************************************
# PYSIDE_ADD_HYBRID_MODULE
# Defines a new PySide module and creates a dependent generator target and
# pip setup call.
#****************************************************************************
function(PYSIDE_ADD_HYBRID_MODULE module_name
                                  dest_dir
                                  mod_py_setup
                                  mod_py_sources
                                  mod_sources
                                  mod_gen_sources
                                  mod_gen_include_paths
                                  mod_gen_link_libs
                                  mod_gen_global
                                  mod_gen_typesystem)

    message(STATUS "Configuring PySide hybrid module: ${module_name}")

    PYTHON_ADD_PIP_SETUP("${module_name}_py_setup"
                         ${dest_dir}
                         ${mod_py_setup}
                         ${mod_py_sources})

    PYSIDE_ADD_MODULE(${module_name}
                  ${dest_dir}/${module_name}
                  ${mod_sources}
                  ${mod_gen_sources}
                  ${mod_gen_include_paths}
                  ${mod_gen_link_libs}
                  ${mod_gen_global}
                  ${mod_gen_typesystem})

    add_dependencies(${module_name} "${module_name}_py_setup")
    add_dependencies(${module_name} "${module_name}_gen")

endfunction(PYSIDE_ADD_HYBRID_MODULE)


