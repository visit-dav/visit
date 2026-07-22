# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Jun 27 14:40:39 MST 2018
#   Set OPENGL_gl_LIBRARY, OPENGL_LIBRARIES, OPENGL_glu_LIBRARY, and
#   OPENGL_INCLUDE_DIR in cache.
#
#   Kathleen Biagas, Thu Nov  8 10:12:32 PST 2018
#   Added MESAGL_API_LIBRARY, to ensure it gets installed.
#   Added includes to install.
#
#   Kathleen Biagas, Wed Jan  8 10:52:55 MST 2020
#   Added a path for Windows when we want to use mesgl as a dropin replacement
#   for opengl on systems without sufficient opengl version.
#
#   Kathleen Biagas, Thu Aug 18, 2022
#   Set OpenGL_GL_PREFERENCE to LEGACY. Doesn't hurt with VTK 8 and is needed
#   by VTK 9, otherwise it will say it needs system GLVND OpenGL version.
#   Also, don't clutter OPENGL_gl_LIBRARY with anything other than GL.
#
#   Kathleen Biagas, Friday Apr 3, 2024
#   Put finding of support libs (glapi, glu, llvm) into a function so that
#   FindOSMesa can use the same logic, and code duplication is reduced.
#   The new function lives in VisItOpenGL.cmake.
#
#****************************************************************************/

#
# if VISIT_MESAGL_DIR is set, then it was determined that system GL lib
# is insufficient for running VisIt (VTK 8), and Mesa-17x was compiled,
# as was LLVM.
#
# This Find module simply determines the correct soname's for mesa's GL and
# LLVM and copies them to the build-dir/lib/mesagl.
#
# This new lib path will be used to set LD_LIBRARY_PATH at run-time for gui
# and viewer (running with window).
#

if(WIN32 AND VISIT_MESA_REPLACE_OPENGL AND VISIT_MESAGL_DIR)
    # used as a replacement for system opengl32.dll on Windows systems without
    # proper OpenGL version (3.2). Just need the dll, installed in
    # VisIt's bin dir, mesagl subdir.  The installer script for Windows will
    # handle testing the system and dropping in the mesa dll if needed.
    if(EXISTS ${VISIT_MESAGL_DIR}/bin/opengl32.dll)
        install(FILES ${VISIT_MESAGL_DIR}/bin/opengl32.dll
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}/mesagl
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
                )
    endif()
    return()
endif()

if (VISIT_MESAGL_DIR)

  find_library(MESAGL_LIBRARY GL  PATH ${VISIT_MESAGL_DIR}/lib NO_DEFAULT_PATH)
  if (MESAGL_LIBRARY)
      execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                      ${VISIT_BINARY_DIR}/lib/mesagl
                      RESULT_VARIABLE GEN_MESAGL_DIR)

      if(NOT "${GEN_MESAGL_DIR}" STREQUAL "0")
          message(WARNING "Failed to create lib/mesagl/")
      endif()

      get_filename_component(MESAGL_LIB ${MESAGL_LIBRARY} NAME)
      execute_process(COMMAND objdump -p ${MESAGL_LIBRARY}
                      COMMAND grep SONAME
                      RESULT_VARIABLE MESAGL_SONAME_RESULT
                      OUTPUT_VARIABLE MESAGL_SONAME
                      ERROR_VARIABLE MESAGL_SONAME_ERROR)
      if(MESAGL_SONAME)
          string(REPLACE "SONAME" "" MESAGL_SONAME ${MESAGL_SONAME})
          string(STRIP ${MESAGL_SONAME} MESAGL_SONAME)
          set(MESAGL_LIBRARY ${VISIT_MESAGL_DIR}/lib/${MESAGL_SONAME})
      endif()

      execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                              ${MESAGL_LIBRARY}
                              ${VISIT_BINARY_DIR}/lib/mesagl/)

      set(OPENGL_gl_LIBRARY ${MESAGL_LIBRARY} CACHE STRING "OpenGL library")
      set(OPENGL_LIBRARIES ${MESAGL_LIBRARY} CACHE STRING "OpenGL libraries")
      set(OPENGL_INCLUDE_DIR ${VISIT_MESAGL_DIR}/include CACHE PATH "OpenGL include path")
      # needed for VTK 9, doesn't cause harm with VTK 8.
      set(OpenGL_GL_PREFERENCE LEGACY CACHE STRING "OpenGL GL preference")

  else()
      message(FATAL_ERROR "VISIT_MESAGL_DIR provided, but it doesn't contain GL library")
  endif()

  find_install_support_libs(${VISIT_MESAGL_DIR} mesagl)

  if (mesagl_API_LIBRARY)
      list(APPEND OPENGL_LIBRARIES ${mesagl_API_LIBRARY})
  endif()

  if (mesagl_GLU_LIBRARY)
      set(OPENGL_glu_LIBRARY ${mesagl_GLU_LIBRARY} CACHE STRING "OpenGL glu library")
  endif()

  if (mesagl_LLVM_LIBRARY)
      list(APPEND OPENGL_LIBRARIES ${mesagl_LLVM_LIBRARY})
      set(OPENGL_LIBRARIES ${OPENGL_LIBRARIES} CACHE STRING "OpenGL libraries" FORCE)
  endif()


  install(DIRECTORY ${VISIT_BINARY_DIR}/lib/mesagl
          DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
          DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE
          FILE_PERMISSIONS      OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE)

  if(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
      THIRD_PARTY_INSTALL_INCLUDE(mesagl ${VISIT_MESAGL_DIR}/include)
  endif()
endif(VISIT_MESAGL_DIR)

