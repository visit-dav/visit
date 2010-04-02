#*****************************************************************************
#
# Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
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
#  Hank Childs, Tue Jan  5 07:13:58 PST 2010
#  Remove xml2makefile and xml2projectfile.
#
#  Kathleen Bonnell, Wed Feb  3 17:22:12 PST 2010
#  Add install for visit_exe.
#
#  Kathleen Bonnell, Sun Mar  14 14:06:51 MST 2010
#  Change copy_directory for VISIT_BIN_DIRECTORIES to a FILE(COPY 
#  so that '.svn' can be skipped.
#
#  Kathleen Bonnell, Mon Mar  22 11:08:46 MST 2010
#  Don't copy VISIT_BIN_FILES on windows, instead only copy makemovie.py,
#  makemoviemain.py and visitdiff.py.
#
#****************************************************************************/

#-----------------------------------------------------------------------------
# Copy necessary bin files & directories to the build dir.
#----------------------------------------------------------------------------

SET(VISIT_BIN_DIRECTORIES movietemplates shaders translations)

SET(VISIT_BIN_FILES
        Doxyfile
        curv3dprep
        frontendlauncher
        makemili
        makemovie.py
        makemoviemain.py
        mkgrdbl
        mpeg2encode
        regen-ajp
        run_doxygen
        surfcomp
        text2polys
        time_annotation
        visit
        visit_composite
        visit_plugin
        visit_transition
        visitconvert
        visitdiff.py
        xml2atts
        xml2avt
        xml2cmake
        xml2info
        xml2java
        xml2plugin
        xml2python
        xml2window
        xmledit
        xmltest)

FOREACH(VISIT_BIN_ITEM ${VISIT_BIN_DIRECTORIES})
    # Resolve symlink 
    GET_FILENAME_COMPONENT(ITM_RP ${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_BIN_ITEM} 
                           REALPATH)
    FILE(COPY ${ITM_RP}
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
         PATTERN ".svn" EXCLUDE)
ENDFOREACH(VISIT_BIN_ITEM)

IF (NOT WIN32)
    FOREACH(VISIT_BIN_ITEM ${VISIT_BIN_FILES} internallauncher)
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                                ${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_BIN_ITEM}
                                ${CMAKE_CURRENT_BINARY_DIR}/${VISIT_BIN_ITEM})
    ENDFOREACH(VISIT_BIN_ITEM)


    INSTALL(FILES ${VISIT_BIN_FILES}
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE) 

    INSTALL(FILES internallauncher
        DESTINATION ${VISIT_VERSION}/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)

ELSE (NOT WIN32)

    #--------------------------------------------------------------------------
    # Create VisIt launcher binary for windows
    #--------------------------------------------------------------------------
    INCLUDE_DIRECTORIES(${VISIT_COMMON_INCLUDES})
    LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})
    ADD_EXECUTABLE(visit_exe visit.c)
    SET_TARGET_PROPERTIES(visit_exe PROPERTIES OUTPUT_NAME visit)
    SET_SOURCE_FILES_PROPERTIES(visit.c PROPERTIES LANGUAGE CXX)
    TARGET_LINK_LIBRARIES(visit_exe shlwapi)
    VISIT_INSTALL_TARGETS(visit_exe)

    #--------------------------------------------------------------------------
    # Copy movie helper scripts
    #--------------------------------------------------------------------------
    SET(VISIT_BIN_WIN_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/makemovie.py
        ${CMAKE_CURRENT_SOURCE_DIR}/makemoviemain.py
        ${CMAKE_CURRENT_SOURCE_DIR}/visitdiff.py)

    INSTALL(FILES ${VISIT_BIN_WIN_FILES}
            DESTINATION ${VISIT_INSTALLED_VERSION}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ GROUP_WRITE
                        WORLD_READ)

    FOREACH (CFG Release Debug RelWithDebugInfo MinSizeRel)
        FILE(COPY ${VISIT_BIN_WIN_FILES}
             DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CFG}
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                              GROUP_READ GROUP_WRITE
                              WORLD_READ)
    ENDFOREACH (CFG)
ENDIF(NOT WIN32)


INSTALL(DIRECTORY movietemplates translations
    DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE 
                     GROUP_READ GROUP_WRITE 
                     WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                          GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                          WORLD_READ             WORLD_EXECUTE
    PATTERN ".svn" EXCLUDE
)
