#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#
#****************************************************************************/

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF(VISIT_PYSIDE_DIR)

    #  Find PySide 
    set(CMAKE_PREFIX_PATH ${VISIT_PYSIDE_DIR}/lib/cmake/ ${CMAKE_PREFIX_PATH})
    set(CMAKE_LIBRARY_PATH ${VISIT_PYSIDE_DIR}/lib ${CMAKE_LIBRARY_PATH})

    find_package(GeneratorRunner 0.6.11)
    find_package(Shiboken 1.0.4)
    find_package(PySide 1.0.4)

ENDIF(VISIT_PYSIDE_DIR)


IF(NOT GeneratorRunner_FOUND OR NOT Shiboken_FOUND)
#If we dont have generator runner  & shiboken, force pyside off
    MESSAGE(STATUS "PySide NOT found")
    set(PySide_FOUND 0)
ELSEIF(PySide_FOUND)
    SET(PYSIDE_FOUND 1)
    SET_UP_THIRD_PARTY(PYSIDE lib include pyside-python${PYTHON_VERSION} shiboken-python${PYTHON_VERSION})
    # The PySide module is symlinked into the python install VisIt uses for dev builds.
    # For 'make install' and 'make package' we need to actually install the PySide SOs.
    SET(PYSIDE_MODULE_SRC  ${VISIT_PYSIDE_DIR}/lib/python${PYTHON_VERSION}/site-packages/PySide/)
    SET(PYSIDE_MODULE_INSTALLED_DIR ${VISIT_INSTALLED_VERSION_LIB}/python/lib/python${PYTHON_VERSION}/site-packages/PySide/)

    FILE(GLOB pysidelibs ${PYSIDE_MODULE_SRC}/*)
    INSTALL(FILES ${pysidelibs}
            DESTINATION ${PYSIDE_MODULE_INSTALLED_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
            )

    # On OSX patch install names for the PySide module.
    FOREACH(pysidelib ${pysidelibs})
        IF(APPLE)
            GET_FILENAME_COMPONENT(libname ${pysidelib} NAME)
            INSTALL(CODE
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                    COMMAND /bin/sh ${VISIT_SOURCE_DIR}/CMake/osxfixup -lib ${VISIT_OSX_USE_RPATH}
                    \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${PYSIDE_MODULE_INSTALLED_DIR}/${libname}\"
                    OUTPUT_VARIABLE OSXOUT)
                    MESSAGE(STATUS \"\${OSXOUT}\")
                    ")
        ENDIF(APPLE)
        ENDFOREACH(pysidelib ${pysidelibs})

ENDIF(NOT GeneratorRunner_FOUND OR NOT Shiboken_FOUND)




#****************************************************************************
# PYSIDE_ADD_MODULE
# Defines a new PySide module and creates a dependent generator target and
# distutils setup call.
#****************************************************************************
FUNCTION(PYSIDE_ADD_MODULE module_name
                           dest_dir
                           mod_sources
                           mod_gen_sources
                           mod_gen_include_paths
                           mod_gen_link_libs
                           mod_gen_global
                           mod_gen_typesystem)

MESSAGE(STATUS "Configuring PySide module: ${module_name}")


PYSIDE_ADD_GENERATOR_TARGET("${module_name}_gen"
                            ${mod_gen_sources}
                            ${mod_gen_include_paths}
                            ${mod_gen_global}
                            ${mod_gen_typesystem})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${SHIBOKEN_INCLUDE_DIR}
                    ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                    ${PYSIDE_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/${module_name}
                    ${${mod_gen_include_paths}})

add_library(${module_name} MODULE ${${mod_sources}} ${${mod_gen_sources}})

SET_TARGET_PROPERTIES(${module_name} PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(${module_name} PROPERTIES
                                     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${dest_dir})

target_link_libraries(${module_name}
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${PYSIDE_LIBRARY}
                      ${${mod_gen_link_libs}})

add_dependencies(${module_name} "${module_name}_gen")

VISIT_INSTALL_TARGETS_RELATIVE(${dest_dir} ${module_name})

ENDFUNCTION(PYSIDE_ADD_MODULE)

#****************************************************************************
# PYSIDE_ADD_HYBRID_MODULE
# Defines a new PySide module and creates a dependent generator target and
# distutils setup call.
#****************************************************************************
FUNCTION(PYSIDE_ADD_HYBRID_MODULE module_name
                                  dest_dir
                                  mod_py_setup
                                  mod_py_sources
                                  mod_sources
                                  mod_gen_sources
                                  mod_gen_include_paths
                                  mod_gen_link_libs
                                  mod_gen_global
                                  mod_gen_typesystem)

MESSAGE(STATUS "Configuring PySide module: ${module_name}")

PYTHON_ADD_DISTUTILS_SETUP("${module_name}_py_setup"
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

ENDFUNCTION(PYSIDE_ADD_HYBRID_MODULE)


#****************************************************************************
# PYSIDE_ADD_GENERATOR_TARGET
# Defines a new PySide generator target.
#****************************************************************************
FUNCTION(PYSIDE_ADD_GENERATOR_TARGET 
            target_name gen_sources gen_include_paths gen_global gen_typesystem)

set(PYSIDE_GENERATOR_EXTRA_FLAGS --generator-set=shiboken 
                                 --enable-parent-ctor-heuristic 
                                 --enable-return-value-heuristic 
                                 --avoid-protected-hack)

#
# Create string for --include-paths arg from gen_include_paths list.
#

IF(WIN32)
    set(gen_pathsep ";")
ELSE(WIN32)
    set(gen_pathsep ":")
ENDIF(WIN32)

SET(gen_include_paths_arg "${CMAKE_CURRENT_SOURCE_DIR}")

FOREACH(itm ${${gen_include_paths}})
    SET(gen_include_paths_arg "${gen_include_paths_arg}${gen_pathsep}${itm}")
ENDFOREACH(itm ${gen_include_paths})


add_custom_command(OUTPUT ${${gen_sources}}
                   COMMAND ${GENERATORRUNNER_BINARY} ${PYSIDE_GENERATOR_EXTRA_FLAGS}
                   ${gen_global}
                   --include-paths=${gen_include_paths_arg}
                   --typesystem-paths=${PYSIDE_TYPESYSTEMS}
                   --output-directory=${CMAKE_CURRENT_BINARY_DIR}
                   ${gen_typesystem}
                   DEPENDS  ${gen_global} ${gen_typesystem}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(${target_name} DEPENDS ${${gen_sources}})
ENDFUNCTION(PYSIDE_ADD_GENERATOR_TARGET)

