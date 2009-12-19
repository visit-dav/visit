#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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
#   Kathleen Bonnell, Thu Dec 10 17:51:27 MT 2009
#   Use QT_X_LIBRARY_RELEASE instead of simple QT_X_LIBRARY, which may 
#   list both optimized and debug versions of the library if present.
#
#****************************************************************************/

#
# Use the QT_BIN hint from the config-site cmake file along
# with the standard FindQt4 cmake module to set up Qt4.
#

IF(NOT "${QT_BIN}" MATCHES "OFF")
  IF (NOT WIN32)
    # Make sure the VISIT_QT_BIN path is valid & qmake exists.
    FIND_PROGRAM(VISIT_LOC_QMAKE_EXE NAMES qmake qmake4 qmake-qt4
                 PATHS
                 ${QT_BIN}
                 NO_DEFAULT_PATH
                 NO_CMAKE_ENVIRONMENT_PATH
                 NO_CMAKE_PATH
                 NO_SYSTEM_ENVIRONMENT_PATH)

    IF ( NOT VISIT_LOC_QMAKE_EXE)
        MESSAGE(FATAL_ERROR "Invalid Qt4 Binary path: ${QT_BIN}")
    ENDIF ( NOT VISIT_LOC_QMAKE_EXE)

    # add VISIT_QT_BIN to the env, so standard FindQt4 module can locate qmake.
    SET(ENV{PATH} "${QT_BIN}:$ENV{PATH}")

    # Invoke cmake's built in module for locating & setting up Qt4
    INCLUDE(${CMAKE_ROOT}/Modules/FindQt4.cmake)

    IF(NOT QT_FOUND)
        MESSAGE(FATAL_ERROR "Qt4 is required to build VisIt.")
    ENDIF(NOT QT_FOUND)    
  ELSE (NOT WIN32)
    SET(QT_LIBRARY_DIR ${QT_DIR}/lib/${VISIT_MSVC_VERSION} CACHE PATH "Qt library dir" FORCE )
    SET(QT_INCLUDE_DIR ${QT_DIR}/include)
    SET(QT_BINARY_DIR  ${QT_DIR}/lib/${VISIT_MSVC_VERSION} CACHE INTERNAL "" FORCE )
    SET(QT_MOC_EXECUTABLE  ${QT_DIR}/lib/${VISIT_MSVC_VERSION}/moc.exe )
    SET(QT_INCLUDES ${QT_INCLUDE_DIR})
    SET(QT_WIN_LIBS QtCore QtDesigner QtDesignerComponents QtGui QtNetwork QtOpenGL QtSql QtXml QtSvg Qt Script QtTest QtMain QtAssistantClient QtHelp QtXml QtXMLPatterns QtUiTools)
    FOREACH(QTWINLIB ${QT_WIN_LIBS})
        STRING(TOUPPER ${QTWINLIB} upper_qtwinlib)
        SET(QT_${upper_qtwinlib}_FOUND 1)
        SET(QT_${upper_qtwinlib}_INCLUDE_DIR ${QT_INCLUDE_DIR}/${QTWINLIB} CACHE PATH "The Qt ${QTWINLIB} include dir" FORCE)
        IF (EXISTS ${QT_LIBRARY_DIR}/${QTWINLIB}4.lib)
            SET(QT_${upper_qtwinlib}_LIBRARY ${QT_LIBRARY_DIR}/${QTWINLIB}4.lib CACHE STRING "The Qt ${QTWINLIB} library" FORCE)
            SET(QT_${upper_qtwinlib}_LIBRARY_RELEASE ${QT_LIBRARY_DIR}/${QTWINLIB}4.lib)
        ELSE (EXISTS ${QT_LIBRARY_DIR}/${QTWINLIB}4.lib)
            SET(QT_${upper_qtwinlib}_LIBRARY ${QT_LIBRARY_DIR}/${QTWINLIB}.lib CACHE STRING "The Qt ${QTWINLIB} library" FORCE)
            SET(QT_${upper_qtwinlib}_LIBRARY_RELEASE ${QT_LIBRARY_DIR}/${QTWINLIB}.lib)
        ENDIF (EXISTS ${QT_LIBRARY_DIR}/${QTWINLIB}4.lib)
    ENDFOREACH(QTWINLIB)
  ENDIF (NOT WIN32)

    #
    # If we are using cocoa we need to define VISIT_MAC_NO_CARBON
    #
    IF(APPLE)
        IF(QT_MAC_USE_COCOA)
            ADD_DEFINITIONS(-DVISIT_MAC_NO_CARBON)
        ENDIF(QT_MAC_USE_COCOA)
    ENDIF(APPLE)

    # Since Qt was found, add install targets for its libraries.
    FOREACH(QTLIB 
        QT_QT3SUPPORT            
        QT_QTASSISTANT           
        QT_QAXCONTAINER           
        QT_QAXSERVER                
        QT_QTCORE                
        QT_QTDBUS                
        QT_QTDESIGNER            
        QT_QTDESIGNERCOMPONENTS  
        QT_QTGUI                 
        QT_QTMOTIF               
        QT_QTNETWORK             
        QT_QTNSPLUGIN            
        QT_QTOPENGL              
        QT_QTSQL                 
        QT_QTXML                 
        QT_QTSVG                 
        QT_QTSCRIPT              
        QT_QTTEST                
        QT_QTMAIN                
        QT_QTUITOOLS             
        QT_QTASSISTANTCLIENT     
        QT_QTHELP
        QT_QTWEBKIT              
        QT_QTXMLPATTERNS         
        QT_PHONON                
    )
        IF(${${QTLIB}_FOUND})
            IF(EXISTS ${${QTLIB}_LIBRARY_RELEASE})
                THIRD_PARTY_INSTALL_LIBRARY(${${QTLIB}_LIBRARY_RELEASE})
            ENDIF(EXISTS ${${QTLIB}_LIBRARY_RELEASE})
        ENDIF(${${QTLIB}_FOUND})
    ENDFOREACH(QTLIB)

    # Add install targets for Qt headers too
    FOREACH(H ${QT_INCLUDES})
        IF(${H} MATCHES "/include/Qt")
            INSTALL(DIRECTORY ${H}
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/qt/include
                FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            )
        ENDIF(${H} MATCHES "/include/Qt")
    ENDFOREACH(H)

    # Install moc, too
    IF(NOT WIN32)
        INSTALL(PROGRAMS ${QT_BIN}/moc
            DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
            PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
    ENDIF(NOT WIN32)
ENDIF(NOT "${QT_BIN}" MATCHES "OFF")





