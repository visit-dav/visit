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

#-----------------------------------------------------------------------------
# Runtime loading of Mesa relies on having OSMesa in a known path.
#
# For development builds we create symlink to libOSMesa along side
# VisIt's libraries.
#
# I (Cyrus) am not sure how this will work on windows - there may be a
# more general solution.
#
# For a 'make install'ed version libOSMesa should be properly installed
# by our install targets.
#
# Modifications:
#    Jeremy Meredith, Wed Jan 20 11:27:30 EST 2010
#    I added a command to put a symlink to python in the lib directory.
#    We need this to set python-related environment variables in
#    internallauncher.  I don't know if this is a good solution, as I
#    don't know the implication for an installed setup -- this is only
#    intended to fix running inside a development build.
#
#    Cyrus Harrison, Wed Feb  3 10:10:22 PST 2010
#    Add symlinks to allow us to locate the vtk python wrapper module.
#
#    Kathleen Bonnell, Wed Mar 17 10:08:17 MST 2010 
#    Use FILE(COPY ...) on windows, as windows doesn't support symlinks.
#    Handled python filters on windows differently, as the VTK directory 
#    structure is different.
#
#    Cyrus Harrison, Wed Jul  7 10:09:07 PDT 2010
#    Remove any existing symlinks before attempting to create new ones.
#
#    Kathleen Biagas, Thu May 10 10:58:39 MST 2012 
#    Change how python is handled on Windows to reflect recent changes.
#
#----------------------------------------------------------------------------

FOREACH(LIB ${MESA_LIB})
    IF (NOT WIN32)
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove -f
                                ${CMAKE_CURRENT_BINARY_DIR}/${LIB})
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
                                ${MESA_LIBRARY_DIR}/${LIB}
                                ${CMAKE_CURRENT_BINARY_DIR}/${LIB})
    ELSE (NOT WIN32)
        FILE(COPY        ${MESA_LIBRARY_DIR}/${LIB}
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    ENDIF (NOT WIN32)
ENDFOREACH(LIB in ${MESA_LIB})

FOREACH(LIB ${R_LIB})
    IF (NOT WIN32)
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove -f
                                ${CMAKE_CURRENT_BINARY_DIR}/${LIB})
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
                                ${VISIT_R_DIR}/lib/${LIB}
                                ${CMAKE_CURRENT_BINARY_DIR}/${LIB})
    ELSE (NOT WIN32)
        FILE(COPY        ${VISIT_R_DIR}/lib/${LIB}
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    ENDIF (NOT WIN32)
ENDFOREACH(LIB in ${MESA_LIB})


IF (NOT WIN32)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove -f
                            ${CMAKE_CURRENT_BINARY_DIR}/python)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
                            ${VISIT_PYTHON_DIR}
                            ${CMAKE_CURRENT_BINARY_DIR}/python)
ELSE (NOT WIN32)

    # because you don't need to choose your configuration up-front
    # on windows, make this a custom target so they are copied to the
    # correct lib/${CFG} directory 
    ADD_CUSTOM_TARGET(python_lib ALL)
    ADD_CUSTOM_COMMAND(TARGET python_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${VISIT_PYTHON_DIR}/Lib
            ${VISIT_LIBRARY_DIR}/python/Lib
            COMMENT "Copying ${VISIT_PYTHON_DIR}/Lib/ ${VISIT_LIBRARY_DIR}/python/Lib"
            )
ENDIF (NOT WIN32)

IF(VISIT_PYTHON_FILTERS)
# We need to be able to locate the vtk python wrapper lib at run time.
# Place symlinks to its components in lib/site-packages/
    MESSAGE(STATUS "Adding VTK python wrappers to lib/site-packages/")
    IF (NOT WIN32)
        ## MESSAGE("${VISIT_VTK_DIR}/lib/python${PYTHON_VERSION}/site-packages/*")
        FILE(GLOB VTK_PYTHON_MODULE_ITEMS "${VISIT_VTK_DIR}/lib/python${PYTHON_VERSION}/site-packages/*")
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${CMAKE_CURRENT_BINARY_DIR}/site-packages
                        RESULT_VARIABLE LIB_SITE_PKGS_MKDIR)
        #
        # make sure lib/site-packages exists before we try to
        # symlink or copy the vtk python wrappers
        #
    
        IF(NOT "${LIB_SITE_PKGS_MKDIR}" STREQUAL "0")
            MESSAGE(WARNING "Failed to create lib/site-packages/")
        ENDIF(NOT "${LIB_SITE_PKGS_MKDIR}" STREQUAL "0")

        FOREACH(ITM ${VTK_PYTHON_MODULE_ITEMS})
            GET_FILENAME_COMPONENT(ITM_ABS ${ITM} ABSOLUTE)
            GET_FILENAME_COMPONENT(ITM_BASE ${ITM_ABS} NAME)
            MESSAGE(STATUS "Symlinking ${ITM_ABS} to ${CMAKE_CURRENT_BINARY_DIR}/site-packages/${ITM_BASE}")
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove -f
                                    ${CMAKE_CURRENT_BINARY_DIR}/site-packages/${ITM_BASE}
                                    RESULT_VARIABLE VTK_MODULE_SYM_RM)
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink
                                    ${ITM_ABS}
                                    ${CMAKE_CURRENT_BINARY_DIR}/site-packages/${ITM_BASE}
                                    RESULT_VARIABLE VTK_MODULE_SYM_CREATE)
            IF(NOT "${VTK_MODULE_SYM_CREATE}" STREQUAL "0")
                MESSAGE(WARNING "Failed to create VTK Python module symlink in lib/site-packages/ for ${ITM_ABS}")
            ENDIF(NOT "${VTK_MODULE_SYM_CREATE}" STREQUAL "0")
        ENDFOREACH(ITM ${VTK_PYTHON_MODULE_ITEMS})

    ELSE (NOT WIN32)
        FILE(GLOB VTK_EGG_FILE "${VISIT_VTK_DIR}/lib/site-packages/*egg*")
        # because you don't need to choose your configuration up-front
        # on windows, make this a custom target so they are copied to the
        # correct lib/${CFG} directory 
        ADD_CUSTOM_TARGET(vtk_python_modules ALL)
        ADD_CUSTOM_COMMAND(TARGET vtk_python_modules POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory 
                    ${VISIT_VTK_DIR}/lib/site-packages/vtk 
                    ${VISIT_LIBRARY_DIR}/site-packages/vtk
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${VTK_EGG_FILE}
                    ${VISIT_LIBRARY_DIR}/site-packages
            COMMENT "Copying ${VISIT_VTK_DIR}/lib/site-packages to ${VISIT_LIBRARY_DIR}/site-packages/vtk"
            )
    ENDIF (NOT WIN32)
ENDIF(VISIT_PYTHON_FILTERS)

