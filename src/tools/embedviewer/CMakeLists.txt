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

SET(SOURCES
CommandParser.C
SimpleVisApp.C
Synchronizer.C
main.C
)

# The subset of the sources that have Q_OBJECT in their header.
SET(MOC_SOURCES
CommandParser.h
SimpleVisApp.h
Synchronizer.h
)
QT_WRAP_CPP(embedded SOURCES ${MOC_SOURCES})

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/vtkqt
${QT_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
${QT_QTGUI_INCLUDE_DIR}
${QT_QTOPENGL_INCLUDE_DIR}
${QT_QTNETWORK_INCLUDE_DIR}
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

IF(NOT APPLE)
    ADD_EXECUTABLE(embedded ${SOURCES})
ELSE(NOT APPLE)
    SET(GUI_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    ADD_EXECUTABLE(embedded MACOSX_BUNDLE ${SOURCES} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "Embedded viewer example ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Embedded viewer example")
    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "Embedded viewer example version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "Embedded viewer example ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC")
ENDIF(NOT APPLE)

TARGET_LINK_LIBRARIES(embedded 
avtdatabase_ser 
avtdbatts
avtfilters_ser 
avtivp
avtmath
avtmir_ser 
avtpipeline_ser
avtplotter_ser 
avtqtviswindow
avtview 
avtviswindow_ser
avtwriter_ser 
engineproxy
enginerpc
lightweight_visit_vtk
mdserverproxy
mdserverrpc
vclproxy
vclrpc
viewer
viewerrpc 
visit_verdict 
visit_vtk
vtkqt
${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTNETWORK_LIBRARY}
)

IF(APPLE)
    SET_TARGET_PROPERTIES(embedded 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
ENDIF(APPLE)
