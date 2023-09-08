# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

if(VISIT_MESAGL_DIR)
    # MesaGL, GLU, LLVM libs
    include(${VISIT_SOURCE_DIR}/CMake/FindMesaGL.cmake)
    if(VTK_VERSION VERSION_EQUAL "8.1.0")
        # OSMesa, LLVM libs
        set(VISIT_OSMESA_DIR ${VISIT_MESAGL_DIR})
        include(${VISIT_SOURCE_DIR}/CMake/FindOSMesa.cmake)
        unset(VISIT_OSMESA_DIR)
    endif()
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
