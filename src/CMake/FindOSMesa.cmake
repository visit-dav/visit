#*****************************************************************************
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
#   Kathleen Bonnell, Thu Dec  3 10:55:03 PST 2009
#   Wrap CMAKE_X_LIBS so that it won't parse on windows. Change ${MESA_FOUND}
#   to MESA_FOUND to remove cmake error.
#
#   Kathleen Bonnell, Wed Dec  9 15:13:27 MT 2009
#   Copy Mesa dlls to execution directory for OSMesa test on windows.
#
#   Kathleen Bonnell, Tue Jan  5 14:13:43 PST 2009
#   Use cmake 2.6.4 (rather than 2.8) compatible version of copying files.
#
#   Kathleen Bonnell, Tue Feb 16 14:00:02 MST 2010
#   Removed conditional check for OSMESA SIZE LIMIT, in case something wasn't
#   set up correctly during first configure pass (eg Mesa lib).
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#   Kathleen Biagas, Fri Mar 17 09:14:34 PDT 2017
#   Set HAVE_OSMESA flag when MESA_FOUND.
#
#   Eric Brugger, Thu May 18 15:51:13 PDT 2017
#   I added support for the LLVM and OpenSWR packages.
#
#   Kathleen Biagas, Wed Jun 27 14:40:39 MST 2018
#   Set OSMESA_INCLUDE_DIR OSMESA_LIBRARIES in cache.
#
#   Eric Brugger, Thu Feb 14 13:02:53 PST 2019
#   Only set HAVE_OSMESA flag when both OSMESA_LIBRARY and MESAGL_LIBRARY
#   are set.
#
#   Eric Brugger, Tue Feb 26 12:55:26 PST 2019
#   Add logic to install libOSMesa in lib directory.
#
#****************************************************************************/

# Use the OSMESA_DIR hint from the config-site .cmake file

if (VISIT_OSMESA_DIR)
    find_library(OSMESA_LIBRARY OSMesa
                 PATH ${VISIT_OSMESA_DIR}/lib
                 NO_DEFAULT_PATH)
    if (OSMESA_LIBRARY)
        set(OSMESA_FOUND true)
        if (MESAGL_LIBRARY)
            set(HAVE_OSMESA true CACHE BOOL "Have OSMesa library")
        endif()
        get_filename_component(OSMESA_LIB ${OSMESA_LIBRARY} NAME)
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                      ${VISIT_BINARY_DIR}/lib/osmesa
                      RESULT_VARIABLE GEN_OSMESA_DIR)

        if(NOT "${GEN_OSMESA_DIR}" STREQUAL "0")
            message(WARNING "Failed to create lib/osmesa/")
        endif()


        # find the SOName
        execute_process(COMMAND objdump -p ${OSMESA_LIBRARY}
                        COMMAND grep SONAME
                        RESULT_VARIABLE OSMESA_SONAME_RESULT
                        OUTPUT_VARIABLE OSMESA_SONAME
                        ERROR_VARIABLE  OSMESA_SONAME_ERROR)

        if(OSMESA_SONAME)
                string(REPLACE "SONAME" "" OSMESA_SONAME ${OSMESA_SONAME})
                string(STRIP ${OSMESA_SONAME} OSMESA_SONAME)
                set(OSMESA_LIBRARY ${VISIT_OSMESA_DIR}/lib/${OSMESA_SONAME})
        endif()
        set(OSMESA_LIBRARIES ${OSMESA_LIBRARY} CACHE STRING "OSMesa libraries")
        set(OSMESA_INCLUDE_DIR ${VISIT_OSMESA_DIR}/include)

        # for LD_LIB_PATH swap to work, libOSMesa needs to be 
        # called libGL.so.1
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${OSMESA_LIBRARY}
                                ${VISIT_BINARY_DIR}/lib/osmesa/libGL.so.1)

    else()
        message(STATUS "OSMesa not found, OSMESA_SIZE_LIMIT defaulting to 4096")
        set(HAVE_OSMESA_SIZE 0 CACHE INTERNAL "support for osmesa_size")
        set(OSMESA_SIZE_LIMIT 4096)
        return()
    endif()

    set(OSMESA_INCLUDE_DIR ${VISIT_OSMESA_DIR}/include CACHE PATH "OSMesa include path")

    find_library(GLAPI_LIBRARY glapi PATH ${VISIT_OSMESA_DIR}/lib
                 NO_DEFAULT_PATH)
    if (GLAPI_LIBRARY)
        get_filename_component(GLAPI_LIB ${GLAPI_LIBRARY} NAME)
        execute_process(COMMAND objdump -p ${GLAPI_LIBRARY}
                        COMMAND grep SONAME
                        RESULT_VARIABLE GLAPI_SONAME_RESULT
                        OUTPUT_VARIABLE GLAPI_SONAME
                        ERROR_VARIABLE  GLAPI_SONAME_ERROR)

        if(GLAPI_SONAME)
            string(REPLACE "SONAME" "" GLAPI_SONAME ${GLAPI_SONAME})
            string(STRIP ${GLAPI_SONAME} GLAPI_SONAME)
            set(GLAPI_LIBRARY ${VISIT_OSMESA_DIR}/lib/${GLAPI_SONAME})
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${GLAPI_LIBRARY}
                                ${VISIT_BINARY_DIR}/lib/osmesa/)

        list(APPEND OSMESA_LIBRARIES ${GLAPI_LIBRARY})
    endif()

    find_library(GLU_LIBRARY GLU PATH ${VISIT_OSMESA_DIR}/lib
                 NO_DEFAULT_PATH)
    if (GLU_LIBRARY)
        get_filename_component(GLU_LIB ${GLU_LIBRARY} NAME)
        execute_process(COMMAND objdump -p ${GLU_LIBRARY}
                        COMMAND grep SONAME
                        RESULT_VARIABLE GLU_SONAME_RESULT
                        OUTPUT_VARIABLE GLU_SONAME
                        ERROR_VARIABLE  GLU_SONAME_ERROR)

        if(GLU_SONAME)
            string(REPLACE "SONAME" "" GLU_SONAME ${GLU_SONAME})
            string(STRIP ${GLU_SONAME} GLU_SONAME)
            set(GLU_LIBRARY ${VISIT_OSMESA_DIR}/lib/${GLU_SONAME})
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${GLU_LIBRARY}
                                ${VISIT_BINARY_DIR}/lib/osmesa/)

        list(APPEND OSMESA_LIBRARIES ${GLU_LIBRARY})
    endif()

    # Check for OSMesa size limit --- IS THIS STILL NECESSARY?
    set(MY_LIBS ${OSMESA_LIBRARIES})
    if (CMAKE_X_LIBS)
        list(APPEND MY_LIBS ${CMAKE_X_LIBS})
    endif()
    if (CMAKE_THREAD_LIBS)
        list(APPEND MY_LIBS ${CMAKE_THREAD_LIBS})
    endif()
    set(MSG "Check for OSMesa size limit")
    message(STATUS ${MSG})
    set(TRY_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/CMakeTmp)
    try_run(TRY_RUN_RESULT HAVE_OSMESA_SIZE
      ${TRY_RUN_DIR}
      ${VISIT_SOURCE_DIR}/CMake/FindOSMesaSize.C
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${OSMESA_INCLUDE_DIR}"
                  "-DLINK_DIRECTORIES:STRING=${OSMESA_LIBRARY_DIR}"
                  "-DLINK_LIBRARIES:STRING=${MY_LIBS}"
      OUTPUT_VARIABLE OUTPUT
    )
    unset(MY_LIBS)

    if (HAVE_OSMESA_SIZE)
        if("${TRY_RUN_RESULT}" MATCHES "FAILED_TO_RUN")
            message(STATUS "${MSG} - failed to run, defaulting to 4096")
            set(OSMESA_SIZE_LIMIT 4096)
        else ()
            if(EXISTS ${CMAKE_BINARY_DIR}/junk.txt)
                file(STRINGS "${CMAKE_BINARY_DIR}/junk.txt" OSMESA_SIZE_LIMIT)
                file(REMOVE "${CMAKE_BINARY_DIR}/junk.txt")
                message(STATUS "${MSG} - found (${OSMESA_SIZE_LIMIT})")
                set(HAVE_OSMESA_SIZE 1 CACHE INTERNAL "support for osmesa_size")
            else()
                message(STATUS "${MSG} - could not find junk.txt")
            endif()
        endif()
    else()
        message(STATUS "${MSG} - not found, defaulting to 4096")
        set(HAVE_OSMESA_SIZE 0 CACHE INTERNAL "support for osmesa_size")
        set(OSMESA_SIZE_LIMIT 4096)
    endif()
    # end Check for OSMesa size limit --- IS THIS STILL NECESSARY?


    if (VISIT_LLVM_DIR)
        find_library(LLVM_LIBRARY LLVM
                         PATH ${VISIT_LLVM_DIR}/lib
                         NO_DEFAULT_PATH)
        if (LLVM_LIBRARY)
            get_filename_component(LLVM_LIB ${LLVM_LIBRARY} NAME)

            execute_process(COMMAND objdump -p ${LLVM_LIBRARY}
                            COMMAND grep SONAME
                            RESULT_VARIABLE LLVM_SONAME_RESULT
                            OUTPUT_VARIABLE LLVM_SONAME
                            ERROR_VARIABLE  LLVM_SONAME_ERROR)

            if(LLVM_SONAME)
                string(REPLACE "SONAME" "" LLVM_SONAME ${LLVM_SONAME})
                string(STRIP ${LLVM_SONAME} LLVM_SONAME)
                set(LLVM_LIBRARY ${VISIT_LLVM_DIR}/lib/${LLVM_SONAME})
            endif()
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                  ${LLVM_LIBRARY}
                                  ${VISIT_BINARY_DIR}/lib/osmesa/)

            list(APPEND OSMESA_LIBRARIES ${LLVM_LIBRARY})
            set(OSMESA_LIBRARIES ${OSMESA_LIBRARIES} CACHE STRING "OSMesa libraries" FORCE)
        endif()
    endif(VISIT_LLVM_DIR)

    message(STATUS "OSMESA_LIBRARIES: ${OSMESA_LIBRARIES}")

    install(DIRECTORY ${VISIT_BINARY_DIR}/lib/osmesa
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                  GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                  WORLD_READ             WORLD_EXECUTE
            FILE_PERMISSIONS      OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                  GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                  WORLD_READ             WORLD_EXECUTE
            CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel)

    # OSMESA_LIBRARY is a symbolic link, so we need to install the real
    # library as well as the link.
    get_filename_component(OSMESA_LIBRARY_REAL ${OSMESA_LIBRARY} REALPATH)
    install(FILES ${OSMESA_LIBRARY} ${OSMESA_LIBRARY_REAL}
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
            PERMISSIONS      OWNER_READ OWNER_WRITE OWNER_EXECUTE
                             GROUP_READ GROUP_WRITE GROUP_EXECUTE
                             WORLD_READ             WORLD_EXECUTE
            CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel)

endif(VISIT_OSMESA_DIR) 

