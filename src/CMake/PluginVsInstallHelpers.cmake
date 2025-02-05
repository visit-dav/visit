# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Nov  8 10:08:38 PST 2018
#   Added string replacments related to VISIT_MESAGL_DIR.
#
#   Kathleen Biagas, Fri Jul 19 12:12:16 PDT 2019
#   Filter out path from OSPRAY libraries.
#
#   Kathleen Biagas, Mon Jan  4 18:15:45 PST 2021
#   Added logic for retrieving and filtering VTKm includes that come from
#   interface libraries.
#
#   Cyrus Harrison, Thu May 19 11:42:31 PDT 2022
#   Add PYTHON_FOUND guard for python path related logic to avoid
#   error with empty path passed to get_filename_component
#
#   Kathleen Biagas, Wed Jun  8 2022
#   Install VisItIncludeVars.cmake.
#
#   Kathleen Biagas, Fri Mar 10, 2023
#   Replaced VTKh logic with VTKm.
#
#******************************************************************************



foreach(VTK_TARGET ${VTK_LIBRARIES})
    if(TARGET ${VTK_TARGET})
        list(APPEND VTK_VALID_TARGETS  ${VTK_TARGET})
    endif()
endforeach()


# needed by PluginVsInstall

install(FILES ${VISIT_SOURCE_DIR}/CMake/VisItMacros.cmake
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ GROUP_WRITE
                    WORLD_READ
        )

install(FILES ${VISIT_SOURCE_DIR}/CMake/CheckMinimumCompilerVersion.cmake
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ GROUP_WRITE
                    WORLD_READ
        )

install(FILES ${VISIT_SOURCE_DIR}/CMake/VisItIncludeVars.cmake
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ GROUP_WRITE
                    WORLD_READ
        )

# extract just the filename from these TP libs that specify full path
set(check_libs SILO XDMF OPENEXR)
foreach(cl ${check_libs})
    string(REPLACE "${VISIT_${cl}_DIR}/lib/" ""
                    ${cl}_LIB
                    "${${cl}_LIB}")

endforeach()
unset(check_libs)

if(VISIT_OSPRAY)
    foreach(ol ${OSPRAY_LIBRARIES})
        get_filename_component(ol_name ${ol} NAME)
        list(APPEND new_ol_libraries ${ol_name})
    endforeach()
    set(OSPRAY_LIBRARIES ${new_ol_libraries})
endif()

if(VISIT_MESAGL_DIR)
    string(REPLACE "${VISIT_MESAGL_DIR}/include"
                   "\${VISIT_ROOT_INCLUDE_DIR}/mesgal/include"
                   OPENGL_INCLUDE_DIR
                   "${OPENGL_INCLUDE_DIR}")
    string(REPLACE "${VISIT_MESAGL_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   OPENGL_gl_LIBRARY
                   "${OPENGL_gl_LIBRARY}")
    string(REPLACE "${VISIT_LLVM_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   OPENGL_gl_LIBRARY
                   "${OPENGL_gl_LIBRARY}")
    string(REPLACE "${VISIT_MESAGL_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   OPENGL_glu_LIBRARY
                   "${OPENGL_glu_LIBRARY}")
    string(REPLACE "${VISIT_MESAGL_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   OPENGL_LIBRARIES
                   "${OPENGL_LIBRARIES}")
    string(REPLACE "${VISIT_LLVM_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   OPENGL_LIBRARIES
                   "${OPENGL_LIBRARIES}")
endif()

if(VTKM_FOUND)
    # VTKm_INCLUDE_DIRS isn't enough for use with our PluginVsInstall stucture,
    # because there are some includes related to vtkm interface libraries
    # that get automagically added by CMake when the vtkh library is used as a
    # link target during VisIt's build.
    # The following macros first determine the list of link dependencies for
    # vtkh, then finds includes for the vtkm interaface libraries.

    # create a list of link dependencies for target
    # this is a recursive macro
    # target is the input target
    # deplist is the output list
    macro(get_lib_dep target deplist)
        get_target_property(INT_LL ${target} INTERFACE_LINK_LIBRARIES)
        if(INT_LL)
            foreach(ll_dep ${INT_LL})
                # only look at targets
                if(TARGET ${ll_dep})
                    string(SUBSTRING "${ll_dep}" 0 4 ll_dep_prefix)
                    # only process libraries that start with vtkh or vtkm
                    if ("${ll_dep_prefix}" STREQUAL "vtkm")
                        list(FIND ${deplist} ${ll_dep} havetarg)
                        if(${havetarg} EQUAL -1)
                            list(APPEND ${deplist} ${ll_dep})
                            get_lib_dep(${ll_dep} ${deplist})
                        endif()
                    endif()
                endif()
            endforeach()
        endif()
    endmacro()

    # looks for interface include directories on INTERFACE targets
    # target is the input target
    # deplist is the output list
    macro(get_inc_dep target deplist)
        get_target_property(ttype ${target} TYPE)
        if (ttype STREQUAL "INTERFACE_LIBRARY")
            # look at the includes that may be needed.
            get_target_property(iid ${target} INTERFACE_INCLUDE_DIRECTORIES)
            if (iid)
                foreach(ii ${iid})
                    list(APPEND ${deplist} ${ii})
                endforeach()
            endif()
            get_target_property(isid ${target} INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
            if(isid)
                foreach(isi ${isid})
                    # system includes are relative paths, so be sure to prepend the correct dir
                    list(APPEND ${deplist} ${VTKM_DIR}/${isi})
                endforeach()
            endif()
        endif()
    endmacro()

    # find the link dependencies for vtkm
    list(APPEND vtkm_deps vtkm_filter)
    get_lib_dep(vtkm_filter vtkm_deps)

    # find the interface includes for all vtkm link dependencies
    set(ii_inc_dep "")
    foreach(vtkmll ${vtkm_deps})
        string(SUBSTRING "${vtkmll}" 0 4 ll_dep_prefix)
        # only process libraries that start with vtkm
        if("${ll_dep_prefix}" STREQUAL "vtkm")
           get_inc_dep(${vtkmll} ii_inc_dep)
        endif()
    endforeach()

    # create filtered_VTKm_INCLUDE_DIRS, starting with VTKm_INCLUDE_DIRS
    # and appending the interface list.  Then replace VTKM_DIR with
    # the install include location for vtkm
    set(temp_VTKm_INCLUDE_DIRS ${VTKm_INCLUDE_DIRS})
    list(APPEND temp_VTKm_INCLUDE_DIRS ${ii_inc_dep})
    string(REPLACE "${VTKM_DIR}/include" "\${VISIT_ROOT_INCLUDE_DIR}/vtkm/include"
                    filtered_VTKm_INCLUDE_DIRS
                    "${temp_VTKm_INCLUDE_DIRS}")
    unset(temp_VTKm_INCLUDE_DIRS)
endif()

#-----------------------------------------------------------------------------
# Create CMake/PluginVsInstall.cmake
#-----------------------------------------------------------------------------
# Obtain compiler version information for inclusion in PluginVsInstall.cmake
# Note: This code block makes use of indirect variable reference (${${...}})

if(NOT WIN32)
    # known different ways to get a compiler to spit out its version
    set(CMAKE_COMPILER_VERSION_FLAGS "--version" "-V" "-v")

    foreach(CMAKE_COMPILER_NAME CMAKE_C_COMPILER CMAKE_CXX_COMPILER)
        foreach(CMAKE_COMPILER_VERSION_FLAG ${CMAKE_COMPILER_VERSION_FLAGS})

            execute_process(COMMAND ${${CMAKE_COMPILER_NAME}}
                ${CMAKE_COMPILER_VERSION_FLAG} TIMEOUT 3
                RESULT_VARIABLE COMPILER_VERSION_RETURN
                OUTPUT_VARIABLE COMPILER_VERSION_STDOUT
                ERROR_VARIABLE COMPILER_VERSION_STDERR)

            # If stderr=="" AND $?==0 AND stdout!=""...
            if("${COMPILER_VERSION_STDERR}" STREQUAL "" AND
                ${COMPILER_VERSION_RETURN} EQUAL 0 AND
                NOT ("${COMPILER_VERSION_STDOUT}" STREQUAL ""))

                # Looks like we got something useful from the compiler.
                # Reformat it so it looks like a cmake comment and break
                # out of the loop over version flags.
                string(REGEX REPLACE "\n" "\n# "
                    ${CMAKE_COMPILER_NAME}_VERSION_MESSAGE
                    "${COMPILER_VERSION_STDOUT}")
                BREAK()

            endif("${COMPILER_VERSION_STDERR}" STREQUAL "" AND
                ${COMPILER_VERSION_RETURN} EQUAL 0 AND
                NOT ("${COMPILER_VERSION_STDOUT}" STREQUAL ""))

        endforeach(CMAKE_COMPILER_VERSION_FLAG ${CMAKE_COMPILER_VERSION_FLAGS})
    endforeach(CMAKE_COMPILER_NAME CMAKE_C_COMPILER CMAKE_CXX_COMPILER)
else(NOT WIN32)
    set(CMAKE_C_COMPILER_VERSION_MESSAGE " ${CMAKE_GENERATOR}")
    if(MSVC)
        if(CMAKE_VERSION VERSION_LESS "2.8.10")
            include(CMakeDetermineVSServicePack)
            DetermineVSServicePack(my_service_pack)
            if(my_service_pack)
                set(CMAKE_C_COMPILER_VERSION_MESSAGE "${CMAKE_C_COMPILER_VERSION_MESSAGE} (${my_service_pack})")
            endif(my_service_pack)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION)
            set(CMAKE_CXX_COMPILER_VERSION_MESSAGE " Visual C/C++ version number: ${CMAKE_CXX_COMPILER_VERSION}")
        else()
            set(CMAKE_CXX_COMPILER_VERSION_MESSAGE " Visual C/C++ version number: ${MSVC_VERSION}")
        endif()
        file(WRITE ${VISIT_BINARY_DIR}/CMake/VisItGenerator.cmake
            "set(CMAKE_GENERATOR \"${CMAKE_GENERATOR}\" CACHE INTERNAL \"\" FORCE)")
        install(FILES ${VISIT_BINARY_DIR}/CMake/VisItGenerator.cmake
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ GROUP_WRITE
                            WORLD_READ
        )
    endif(MSVC)
endif(NOT WIN32)

# If we're installing MPICH as our MPI then filter the parallel flags so
# libraries and includes will come from our internal MPICH installation.
if(VISIT_MPICH_INSTALL)
    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_CXXFLAGS
                    "${VISIT_PARALLEL_CXXFLAGS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_ROOT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_CXXFLAGS
                    "${f0_VISIT_PARALLEL_CXXFLAGS}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_LINKER_FLAGS
                    "${VISIT_PARALLEL_LINKER_FLAGS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_ROOT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_LINKER_FLAGS
                    "${f0_VISIT_PARALLEL_LINKER_FLAGS}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_LIBS
                    "${VISIT_PARALLEL_LIBS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_ROOT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_LIBS
                    "${f0_VISIT_PARALLEL_LIBS}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_INCLUDE
                    "${VISIT_PARALLEL_INCLUDE}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_ROOT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_INCLUDE
                    "${f0_VISIT_PARALLEL_INCLUDE}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_DEFS
                    "${VISIT_PARALLEL_DEFS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_ROOT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_DEFS
                    "${f0_VISIT_PARALLEL_DEFS}")
else(VISIT_MPICH_INSTALL)
    set(filtered_VISIT_PARALLEL_CXXFLAGS     "${VISIT_PARALLEL_CXXFLAGS}")
    set(filtered_VISIT_PARALLEL_LINKER_FLAGS "${VISIT_PARALLEL_LINKER_FLAGS}")
    set(filtered_VISIT_PARALLEL_LIBS         "${VISIT_PARALLEL_LIBS}")
    set(filtered_VISIT_PARALLEL_INCLUDE      "${VISIT_PARALLEL_INCLUDE}")
    set(filtered_VISIT_PARALLEL_DEFS         "${VISIT_PARALLEL_DEFS}")
endif(VISIT_MPICH_INSTALL)

# Fix some third-party include paths that are different on windows than unix.
if(UNIX)
   # python3's include dir has an 'm' after the version. For ease of
   # use with future versions, and to save having to figure this out again
   # get and use the last part of the PYTHON_INCLUDE_DIR
   if(PYTHON_FOUND)
       get_filename_component(py_inc_base ${PYTHON_INCLUDE_DIR} NAME)
       set(python_include_relative_path "/python/include/${py_inc_base}")
   else()
       set(python_include_relative_path "")
   endif()
   set(exodusii_include_relative_path "/exodusii/inc")
   set(vtk_include_relative_path "/vtk/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")
else(UNIX)
   set(python_include_relative_path "/python/include")
   set(exodusii_include_relative_path "/exodusii/include")
   set(vtk_include_relative_path "/vtk/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")
endif(UNIX)

configure_file(${VISIT_SOURCE_DIR}/CMake/PluginVsInstall.cmake.in
               ${VISIT_BINARY_DIR}/CMake/PluginVsInstall.cmake @ONLY IMMEDIATE)
install(FILES ${VISIT_BINARY_DIR}/CMake/PluginVsInstall.cmake
    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
    PERMISSIONS OWNER_READ OWNER_WRITE
                GROUP_READ GROUP_WRITE
                WORLD_READ
    )
unset(python_include_relative_path)
unset(exodusii_include_relative_path)
unset(vtk_include_relative_path)
unset(filtered_VISIT_PARALLEL_CXXFLAGS)
unset(filtered_VISIT_PARALLEL_LINKER_FLAGS)
unset(filtered_VISIT_PARALLEL_LIBS)
unset(filtered_VISIT_PARALLEL_INCLUDE)

#-----------------------------------------------------------------------------
# Done with Creating CMake/PluginVsInstall.cmake
#-----------------------------------------------------------------------------

export_library_dependencies(${VISIT_BINARY_DIR}/include/VisItLibraryDependencies.cmake.in)



configure_file(${VISIT_SOURCE_DIR}/CMake/FilterDependencies.cmake.in
              ${VISIT_BINARY_DIR}/include/FilterDependencies.cmake
              @ONLY)
install(SCRIPT "${VISIT_BINARY_DIR}/include/FilterDependencies.cmake")

install(FILES
        ${VISIT_BINARY_DIR}/include/VisItLibraryDependencies.cmake
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ GROUP_WRITE
                    WORLD_READ
        )

