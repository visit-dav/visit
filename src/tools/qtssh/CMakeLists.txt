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
#
#****************************************************************************/

ADD_SUBDIRECTORY(remotecommand)

SET(QTSSH_SOURCES
qtssh.cpp
${VISIT_SOURCE_DIR}/viewer/main/ViewerChangeUsernameWindow.C
${VISIT_SOURCE_DIR}/viewer/main/ViewerPasswordWindow.C
)

# The subset of the sources that have Q_OBJECT in their header.
SET(QTSSH_MOC_SOURCES
${VISIT_SOURCE_DIR}/viewer/main/ViewerChangeUsernameWindow.h
${VISIT_SOURCE_DIR}/viewer/main/ViewerPasswordWindow.h
)
QT_WRAP_CPP(qtssh QTSSH_SOURCES ${QTSSH_MOC_SOURCES})

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/tools/qtssh/remotecommand
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_COMMON_INCLUDES}
${QT_INCLUDE_DIR}
${QT_QTGUI_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
)

LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR})
ADD_EXECUTABLE(qtssh ${QTSSH_SOURCES})
ADD_DEPENDENCIES(qtssh remotecommand visitcommon viewer) 

# TARGET_LINK_LIBRARIES is the normal command we would use, but cmake pulls in 
# all of viewer's dependencies as well and adds them to the linker input in the
# visual studio project. So instead, add the necessary link dependencies as
# LINK_FLAGS.  No paths are needed as they have already been specified in the 
# LINK_DIRECTORIES command above. 

SET(QTSSH_DEP "remotecommand.lib visitcommon.lib viewerlib.lib QtCore4.lib QtGui4.lib")
SET_TARGET_PROPERTIES(qtssh PROPERTIES LINK_FLAGS ${QTSSH_DEP})

VISIT_INSTALL_TARGETS(qtssh)
