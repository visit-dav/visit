# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
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
#   Kathleen Biagas, Tue Jan 31, 2023
#   Remove check for OSMESA_SIZE, it is no longer used.
#
#   Kathleen Biagas, Tue Sep 26, 2023
#   Replace ${CMAKE_THREAD_LIBS} with Threads::Threads.
#
#   Kathleen Biagas, Tue October 17, 2023
#   Threads is now required, so no need to test for existence.
#
#   Kathleen Biagas, Mon Dec 2, 2024
#   Handle installation differently for VTK-9.4, which only requires
#   libOSMesa to be available if needed, so it will be installed directly
#   to lib.
#
#   Kathleen Biagas, Thu Jun 19, 2025
#   Remove support for VTK versions < 9.5.
#
#   Kathleen Biagas, Fri Apr 3, 2026
#   Readd install of lib/osmesa if VISIT_MESAGL_DIR set.
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

        if(NOT EXISTS ${VISIT_BINARY_DIR}/lib)
            message("${VISIT_BINARY_DIR}/lib does not exist yet")
            execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                    ${VISIT_BINARY_DIR}/lib
                    RESULT_VARIABLE LIB_MKDIR)
        endif()
        message("copying ${OSMESA_LIBRARY} to ${VISIT_BINARY_DIR}/lib")
        # We want libOSMesa to be available if needed.
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${OSMESA_LIBRARY}
                                ${VISIT_BINARY_DIR}/lib/)
    else()
        return()
    endif()

    # OSMESA_LIBRARY is a symbolic link, so we need to install the real
    # library as well as the link.
    get_filename_component(OSMESA_LIBRARY_REAL ${OSMESA_LIBRARY} REALPATH)
    install(FILES ${OSMESA_LIBRARY} ${OSMESA_LIBRARY_REAL}
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
            PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE
                         GROUP_READ GROUP_WRITE GROUP_EXECUTE
                         WORLD_READ             WORLD_EXECUTE)

    message(STATUS "OSMESA_LIBRARIES: ${OSMESA_LIBRARIES}")


    # if VisIt was built with mesagl support, need to install osmesa-as-gl
    # for nowin mode runtime resolution of libGL, libglapi, etc
    if(VISIT_MESAGL_DIR)
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${VISIT_BINARY_DIR}/lib/osmesa
                        RESULT_VARIABLE GEN_OSMESA_DIR)

        if(NOT "${GEN_OSMESA_DIR}" STREQUAL "0")
            message(WARNING "Failed to create lib/osmesa/")
        endif()

        # for LD_LIB_PATH swap to work, libOSMesa needs to be
        # called libGL.so.1
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${OSMESA_LIBRARY}
                                ${VISIT_BINARY_DIR}/lib/osmesa/libGL.so.1)

        find_install_support_libs(${VISIT_OSMESA_DIR} osmesa)

        install(DIRECTORY ${VISIT_BINARY_DIR}/lib/osmesa
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                      GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                      WORLD_READ             WORLD_EXECUTE
                FILE_PERMISSIONS      OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                      GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                      WORLD_READ             WORLD_EXECUTE)

    endif()
endif()

