#******************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#   Kathleen Biagas, Thu Nov  8 10:08:38 PST 2018
#   Added string replacments related to VISIT_MESAGL_DIR.
#
#   Kathleen Biagas, Fri Jul 19 12:12:16 PDT 2019
#   Filter out path from OSPRAY libraries.
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
                   "\${VISIT_INCLUDE_DIR}/mesgal/include"
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
    string(REPLACE "${VISIT_MESAGL_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   TESSELLATION_LIBRARY
                   "${TESSELLATION_LIBRARY}")
    string(REPLACE "${VISIT_LLVM_DIR}/lib"
                   "\${VISIT_LIBRARY_DIR}/mesagl"
                   TESSELLATION_LIBRARY
                   "${TESSELLATION_LIBRARY}")
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
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_CXXFLAGS
                    "${f0_VISIT_PARALLEL_CXXFLAGS}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_LINKER_FLAGS
                    "${VISIT_PARALLEL_LINKER_FLAGS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_LINKER_FLAGS
                    "${f0_VISIT_PARALLEL_LINKER_FLAGS}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_LIBS
                    "${VISIT_PARALLEL_LIBS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_LIBS
                    "${f0_VISIT_PARALLEL_LIBS}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_INCLUDE
                    "${VISIT_PARALLEL_INCLUDE}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_INCLUDE_DIR}/mpich/include"
                    filtered_VISIT_PARALLEL_INCLUDE
                    "${f0_VISIT_PARALLEL_INCLUDE}")

    string(REPLACE "${VISIT_MPICH_DIR}/lib" "\${VISIT_LIBRARY_DIR}"
                    f0_VISIT_PARALLEL_DEFS
                    "${VISIT_PARALLEL_DEFS}")
    string(REPLACE "${VISIT_MPICH_DIR}/include" "\${VISIT_INCLUDE_DIR}/mpich/include"
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
   set(python_include_relative_path "/python/include/python${PYTHON_VERSION}")
   set(exodusii_include_relative_path "/exodusii/inc")
   set(vtk_include_relative_path "/vtk/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")
else(UNIX)
   set(python_include_relative_path "/python")
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

