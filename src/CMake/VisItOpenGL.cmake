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
#
#****************************************************************************/

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


