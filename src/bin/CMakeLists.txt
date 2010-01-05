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
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory
                            ${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_BIN_ITEM}
                            ${CMAKE_CURRENT_BINARY_DIR}/${VISIT_BIN_ITEM})
ENDFOREACH(VISIT_BIN_ITEM)

FOREACH(VISIT_BIN_ITEM ${VISIT_BIN_FILES} internallauncher)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                            ${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_BIN_ITEM}
                            ${CMAKE_CURRENT_BINARY_DIR}/${VISIT_BIN_ITEM})
ENDFOREACH(VISIT_BIN_ITEM)


#-----------------------------------------------------------------------------
# Create VisIt launcher binary for windows
#----------------------------------------------------------------------------
IF(WIN32)
    INCLUDE_DIRECTORIES(${VISIT_COMMON_INCLUDES})
    LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
    ADD_EXECUTABLE(visit_exe visit.c)
    SET_TARGET_PROPERTIES(visit_exe PROPERTIES OUTPUT_NAME visit)
    SET_SOURCE_FILES_PROPERTIES(visit.c PROPERTIES LANGUAGE CXX)
    TARGET_LINK_LIBRARIES(visit_exe shlwapi)
ENDIF(WIN32)


INSTALL(FILES ${VISIT_BIN_FILES}
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
IF(NOT WIN32)
    INSTALL(FILES internallauncher
        DESTINATION ${VISIT_VERSION}/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
ENDIF(NOT WIN32)
INSTALL(DIRECTORY movietemplates translations
    DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    PATTERN ".svn" EXCLUDE
)
