#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#*****************************************************************************
#  A place to store macros used by plugins, so that PluginVsInstall.cmake.in
#  will not have to rewrite them.  Should reduce maintenance headache for
#  that file.  This file should only include those that 
#  PluginVsInstall.cmake.in also uses without modification.
#
#*****************************************************************************

IF (WIN32)
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES -DNOMINMAX)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  ADD_DEFINITIONS(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

  # cmake automatically uses _DEBUG and MDd for Debug flags, but our
  # third-party libs are NOT debug versions, so we won't be able to
  # link against them if those defines are used.  Replace those flags:
  # When we allow users to supply their own third-party libs, then
  # this will have to be rethought -- they may want to be able to c
  # change them and this currently forces.

  MACRO(REPLACE_FLAG OLD_FLAG NEW_FLAG FLAG_TYPE FLAG_STRING)
      STRING(REPLACE "${OLD_FLAG}" "${NEW_FLAG}" TMP "${${FLAG_TYPE}}")
      SET(${FLAG_TYPE} "${TMP}" CACHE STRING "${FLAG_STRING}" FORCE)
  ENDMACRO(REPLACE_FLAG)

  # Remove /_DEBUG From debug builds
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
ENDIF (WIN32)

FUNCTION(ADD_TARGET_INCLUDE target)
    # Setting include_directories via set_target_properties overrides
    # the current settings, but we want to append, so first retrieve
    # the current values, then append the desired include.
    GET_TARGET_PROPERTY(currentInc ${target} INCLUDE_DIRECTORIES)
    LIST(APPEND currentInc ${ARGN})
    SET_TARGET_PROPERTIES(${target} PROPERTIES
                          INCLUDE_DIRECTORIES "${currentInc}")
    UNSET(currentInc)
ENDFUNCTION(ADD_TARGET_INCLUDE)

FUNCTION(ADD_TARGET_DEFINITIONS target newDefs)
    GET_TARGET_PROPERTY(currentDefs ${target} COMPILE_DEFINITIONS)
    # if there truly are current definitions, and more than 1 in the list, then
    # IF(${currentDefs} MATCHES NOTFOUND) causes a configure failure.
    # Grab the first item in the list for comparison instead.
    if (currentDefs)
        LIST(GET currentDefs 0 firstItem )
        IF(${firstItem} MATCHES NOTFOUND)
            SET(allDefs ${newDefs})
        ELSE(${firstItem} MATCHES NOTFOUND)
            SET(allDefs ${currentDefs} ${newDefs})
        ENDIF(${firstItem} MATCHES NOTFOUND)
    else()
        SET(allDefs ${newDefs})
    endif()
    SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_DEFINITIONS "${allDefs}"
        )
ENDFUNCTION(ADD_TARGET_DEFINITIONS)

FUNCTION(ADD_PARALLEL_LIBRARY target)
    ADD_LIBRARY(${target} ${ARGN})

    IF(UNIX)
      IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH(X)
        SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
        )
        IF(VISIT_PARALLEL_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH(X ${VISIT_PARALLEL_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
        ENDIF(VISIT_PARALLEL_LINKER_FLAGS)

        IF(VISIT_PARALLEL_RPATH)
            SET(PAR_RPATHS "")
            FOREACH(X ${CMAKE_INSTALL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            FOREACH(X ${VISIT_PARALLEL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                INSTALL_RPATH ${PAR_RPATHS}
            )
        ENDIF(VISIT_PARALLEL_RPATH)
      ENDIF(VISIT_PARALLEL_CXXFLAGS)

    ELSE(UNIX)
      ADD_TARGET_INCLUDE(${target} ${VISIT_PARALLEL_INCLUDE})
      ADD_TARGET_DEFINITIONS(${target} ${VISIT_PARALLEL_DEFS})
    ENDIF(UNIX)
    IF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
        TARGET_LINK_LIBRARIES(${target} ${VISIT_PARALLEL_LIBS})
    ENDIF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
ENDFUNCTION(ADD_PARALLEL_LIBRARY)

MACRO(VISIT_PLUGIN_TARGET_RTOD type) 
    IF(WIN32)
        SET_TARGET_PROPERTIES(${ARGN} PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY_RELEASE
                "${VISIT_PLUGIN_DIR}/${type}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG
                "${VISIT_PLUGIN_DIR}/${type}"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                "${VISIT_PLUGIN_DIR}/${type}"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
                "${VISIT_PLUGIN_DIR}/${type}"
        )
    ENDIF(WIN32)
ENDMACRO(VISIT_PLUGIN_TARGET_RTOD)


