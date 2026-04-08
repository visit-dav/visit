# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Fri Apr 3, 2026
#   Added function to find and install the gl support libraries, if found.
#   Logic copied from FindMesaGL.cmake and modified for generality.
#
#****************************************************************************


function(find_install_support_libs glpath glname)
   # intended for Mesa/OSMesa built by VisIt
   # glpath should be one of VISIT_MESAGL_DIR or VISIT_OSMESA_DIR
   # glname should be one of mesagl or osmesa.

    if(NOT EXISTS ${glpath})
        message(FATAL_ERROR "Invalid path to Mesa/OSMesa: ${glpath}")
    endif()

    #
    # GLAPI
    #
    find_library(${glname}_API_LIBRARY glapi  PATH ${glpath}/lib NO_DEFAULT_PATH)

    if (${glname}_API_LIBRARY)
        get_filename_component(${glname}_API_LIB ${${glname}_API_LIBRARY} NAME)

        execute_process(COMMAND objdump -p ${${glname}_API_LIBRARY}
                        COMMAND grep SONAME
                        RESULT_VARIABLE ${glname}_API_SONAME_RESULT
                        OUTPUT_VARIABLE ${glname}_API_SONAME
                        ERROR_VARIABLE ${glname}_API_SONAME_ERROR)

        if(${glname}_API_SONAME)
            string(REPLACE "SONAME" "" ${glname}_API_SONAME ${${glname}_API_SONAME})
            string(STRIP ${${glname}_API_SONAME} ${glname}_API_SONAME)
            set(${glname}_API_LIBRARY ${glpath}/lib/${${glname}_API_SONAME})
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                        ${${glname}_API_LIBRARY}
                        ${VISIT_BINARY_DIR}/lib/${glname}/)
    endif()

    #
    # GLU
    #
    find_library(${glname}_GLU_LIBRARY GLU  PATH ${glpath}/lib NO_DEFAULT_PATH)

    if (${glname}_GLU_LIBRARY)
        get_filename_component(${glname}_GLU_LIB ${${glname}_GLU_LIBRARY} NAME)
        execute_process(COMMAND objdump -p ${${glname}_GLU_LIBRARY}
                        COMMAND grep SONAME
                        RESULT_VARIABLE ${glname}_GLU_SONAME_RESULT
                        OUTPUT_VARIABLE ${glname}_GLU_SONAME
                        ERROR_VARIABLE ${glname}_GLU_SONAME_ERROR)

        if(${glname}_GLU_SONAME)
            string(REPLACE "SONAME" "" ${glname}_GLU_SONAME ${${glname}_GLU_SONAME})
            string(STRIP ${${glname}_GLU_SONAME} ${glname}_GLU_SONAME)
            set(MESAGLU_LIBRARY ${glpath}/lib/${${glname}_GLU_SONAME})
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                        ${${glname}_GLU_LIBRARY}
                        ${VISIT_BINARY_DIR}/lib/${glname}/)
    endif()

    #
    # LLVM
    #
    if (VISIT_LLVM_DIR)
        find_library(${glname}_LLVM_LIBRARY LLVM  PATH ${VISIT_LLVM_DIR}/lib NO_DEFAULT_PATH)

        if (${glname}_LLVM_LIBRARY)
            get_filename_component(${glname}_LLVM_LIB ${${glname}_LLVM_LIBRARY} NAME)
            execute_process(COMMAND objdump -p ${${glname}_LLVM_LIBRARY}
                            COMMAND grep SONAME
                            RESULT_VARIABLE ${glname}_LLVM_SONAME_RESULT
                            OUTPUT_VARIABLE ${glname}_LLVM_SONAME
                            ERROR_VARIABLE ${glname}_LLVM_SONAME_ERROR)

            if(${glname}_LLVM_SONAME)
                string(REPLACE "SONAME" "" ${glname}_LLVM_SONAME ${${glname}_LLVM_SONAME})
                string(STRIP ${${glname}_LLVM_SONAME} ${glname}_LLVM_SONAME)
                set(${glname}_LLVM_LIBRARY ${VISIT_LLVM_DIR}/lib/${${glname}_LLVM_SONAME})
            endif()

            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                            ${${glname}_LLVM_LIBRARY}
                            ${VISIT_BINARY_DIR}/lib/${glname}/)

        endif()
    endif()

endfunction()



if(VISIT_MESAGL_DIR)
    # MesaGL, GLU, LLVM libs
    include(${VISIT_SOURCE_DIR}/CMake/FindMesaGL.cmake)
    # OSMesa, LLVM libs
    set(VISIT_OSMESA_DIR ${VISIT_MESAGL_DIR})
    include(${VISIT_SOURCE_DIR}/CMake/FindOSMesa.cmake)
    unset(VISIT_OSMESA_DIR)
elseif(VISIT_OSMESA_DIR)
    # OSMesa, LLVM libs
    include(${VISIT_SOURCE_DIR}/CMake/FindOSMesa.cmake)
endif()

if(WIN32 AND VISIT_MESA_REPLACE_OPENGL)
    # Standard find of system GL, still needed in this instance.
    include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
    set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
    return()
endif()


if(NOT VISIT_MESAGL_DIR)
    if(VISIT_OPENGL_DIR)
        # not sure if this section is still needed ????
        set(OPENGL_FOUND ON)
        set(OPENGL_GLU_FOUND ON)
        set(OPENGL_INCLUDE_DIR ${VISIT_OPENGL_DIR}/include)

        if(VISIT_OPENGL_LIBRARY)
            set(OPENGL_gl_LIBRARY ${VISIT_OPENGL_LIBRARY})
        else()
            # Hack for BG/Q.
            if(BLUEGENEQ)
                set(LIBGL OSMesa)
            else()
                set(LIBGL GL)
            endif()
            IF(VISIT_STATIC)
                set(OPENGL_gl_LIBRARY ${VISIT_OPENGL_DIR}/lib/lib${LIBGL}.a)
            else()
                set(OPENGL_gl_LIBRARY ${VISIT_OPENGL_DIR}/lib/lib${LIBGL}.so)
            endif()
        endif()

        if(VISIT_GLU_LIBRARY)
            set(OPENGL_glu_LIBRARY ${VISIT_GLU_LIBRARY})
        else()
            if(VISIT_STATIC)
                set(OPENGL_glu_LIBRARY ${VISIT_OPENGL_DIR}/lib/libGLU.a)
            else()
                set(OPENGL_glu_LIBRARY ${VISIT_OPENGL_DIR}/lib/libGLU.so)
            endif()
        endif()

        set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
        message(STATUS "Found OpenGL ${OPENGL_gl_LIBRARY}")
        message(STATUS "Found GLU ${OPENGL_glu_LIBRARY}")
    else()
        # Standard find of system GL
        message(STATUS "Using CMake's OpenGL locator!")
        include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
    endif()
endif()



message(STATUS "**** OPENGL_gl_LIBRARY=${OPENGL_gl_LIBRARY}")
message(STATUS "**** OPENGL_opengl_LIBRARY=${OPENGL_opengl_LIBRARY}")
message(STATUS "**** OPENGL_glu_LIBRARY=${OPENGL_glu_LIBRARY}")
message(STATUS "**** OPENGL_glx_LIBRARY=${OPENGL_glx_LIBRARY}")
message(STATUS "**** OPENGL_egl_LIBRARY=${OPENGL_egl_LIBRARY}")
message(STATUS "**** OPENGL_FOUND=${OPENGL_FOUND}")
message(STATUS "**** OPENGL_XMESA_FOUND=${OPENGL_XMESA_FOUND}")
message(STATUS "**** OPENGL_GLU_FOUND=${OPENGL_GLU_FOUND}")
message(STATUS "**** OpenGL_OpenGL_FOUND=${OpenGL_OpenGL_FOUND}")
message(STATUS "**** OpenGL_GLX_FOUND=${OpenGL_GLX_FOUND}")
message(STATUS "**** OPENGL_INCLUDE_DIR=${OPENGL_INCLUDE_DIR}")
message(STATUS "**** OPENGL_LIBRARIES=${OPENGL_LIBRARIES}")
