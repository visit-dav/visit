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
#   Kathleen Bonnell, Wed May 5 12:07:54 PST 2010
#   Disable optimization.
# 
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Brad Whitlock, Mon May 21 14:27:34 PST 2012
#   Turn qtssh back into a console application.
#
#   Brad Whitlock, Tue Jun 12 16:14:32 PST 2012
#   Redo qtssh using new sources and refactored viewer windows.
#
#****************************************************************************/

# All of the plink sources, according to the plink.dsp project file.
SET(PLINK_SOURCES
be_all_s.c
cmdline.c
cproxy.c
ldisc.c
logging.c
misc.c
pgssapi.c
pinger.c
portfwd.c
proxy.c
raw.c
rlogin.c
settings.c
ssh.c
sshaes.c
ssharcf.c
sshblowf.c
sshbn.c
sshcrc.c
sshcrcda.c
sshdes.c
sshdh.c
sshdss.c
sshgssc.c
sshmd5.c
sshpubk.c
sshrand.c
sshrsa.c
sshsh256.c
sshsh512.c
sshsha.c
sshzlib.c
telnet.c
timing.c
tree234.c
version.c
wildcard.c
windows/plink.rc
windows/putty.ico
windows/wincons.c
windows/windefs.c
windows/wingss.c
windows/winhandl.c
windows/winmisc.c
windows/winnet.c
windows/winnoise.c
windows/winnojmp.c
windows/winpgntc.c
windows/winproxy.c
windows/winser.c
windows/winstore.c
windows/wintime.c
windows/winx11.c
x11fwd.c
)

SET(QTSSH_SOURCES
qtssh.cpp
qtsshmain.c
${VISIT_SOURCE_DIR}/viewer/main/VisItChangeUsernameWindow.C
${VISIT_SOURCE_DIR}/viewer/main/VisItPasswordWindow.C
)

# The subset of the sources that have Q_OBJECT in their header.
SET(QTSSH_MOC_SOURCES
${VISIT_SOURCE_DIR}/viewer/main/VisItChangeUsernameWindow.h
${VISIT_SOURCE_DIR}/viewer/main/VisItPasswordWindow.h
)
QT_WRAP_CPP(qtssh QTSSH_SOURCES ${QTSSH_MOC_SOURCES})

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/windows
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_COMMON_INCLUDES}
${QT_INCLUDE_DIR}
${QT_QTGUI_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
)

LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR})

ADD_DEFINITIONS(-DSECURITY_WIN32)

ADD_EXECUTABLE(qtssh ${QTSSH_SOURCES} ${PLINK_SOURCES})
TARGET_LINK_LIBRARIES(qtssh QtCore4 QtGui4)

VISIT_INSTALL_TARGETS(qtssh)
VISIT_TOOLS_ADD_FOLDER(qtssh)


