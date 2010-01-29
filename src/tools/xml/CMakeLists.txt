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
#****************************************************************************/

SET(PROGRAM1_SOURCES main1.C)

SET(PROGRAM2_SOURCES main2.C)

SET(PROGRAM3_SOURCES main3.C)

SET(PROGRAM4_SOURCES main4.C)

SET(PROGRAM5_SOURCES main5.C)

SET(PROGRAM6_SOURCES main6.C)

SET(PROGRAM7_SOURCES main7.C)

SET(PROGRAM8_SOURCES main8.C)

SET(PROGRAM9_SOURCES main9.C)

SET(PROGRAM10_SOURCES main10.C)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${VISIT_SOURCE_DIR}/common/utility
${QT_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
)

# Add link directories needed to use Qt.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR})

ADD_EXECUTABLE(xmltest         ${PROGRAM1_SOURCES})
ADD_EXECUTABLE(xml2atts        ${PROGRAM2_SOURCES})
ADD_EXECUTABLE(xml2window      ${PROGRAM3_SOURCES})
ADD_EXECUTABLE(xml2info        ${PROGRAM4_SOURCES})
ADD_EXECUTABLE(xml2avt         ${PROGRAM6_SOURCES})
ADD_EXECUTABLE(xml2python      ${PROGRAM7_SOURCES})
ADD_EXECUTABLE(xml2java        ${PROGRAM8_SOURCES})
ADD_EXECUTABLE(xml2cmake       ${PROGRAM10_SOURCES})

SET(RELEVANT_QT_LIBS 
${QT_QTXML_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    SET(RELEVANT_QT_LIBS
        ${RELEVANT_QT_LIBS}
        ${QT_QTXML_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

# Specify the libraries to link against on Windows & Mac
TARGET_LINK_LIBRARIES(xmltest    ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2atts   ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2window ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2info   ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2avt    ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2python ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2java   ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2cmake  ${RELEVANT_QT_LIBS})

# Install the targets
VISIT_INSTALL_TARGETS(xmltest xml2atts xml2window xml2info xml2avt xml2python xml2java xml2cmake)
