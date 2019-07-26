# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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
        include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
        set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
    endif()
endif()


# We use libGLU for its tessellation abilities but it requires libGL sometimes.
if(UNIX AND NOT APPLE)
    set(TESSELLATION_LIBRARY ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
else()
    set(TESSELLATION_LIBRARY ${OPENGL_glu_LIBRARY})
endif()


