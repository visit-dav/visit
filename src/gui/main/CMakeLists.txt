#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/common
${VISIT_SOURCE_DIR}/gui
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/winutil
${QT_INCLUDE_DIR} 
${QT_QTCORE_INCLUDE_DIR} 
${QT_QTGUI_INCLUDE_DIR} 
${QT_QTOPENGL_INCLUDE_DIR} 
${QT_QTNETWORK_INCLUDE_DIR} 
${QT_QTXML_INCLUDE_DIR} 
${QT_QTUITOOLS_INCLUDE_DIR}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR})

IF(VISIT_STATIC)
    # If the gui is static then we need some static symbol lookup functions
    SET(GUI_STATIC_SOURCES GUIStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

#****************************** The gui executable ***************************
IF(NOT APPLE)
    ADD_EXECUTABLE(gui_exe main.C ${GUI_STATIC_SOURCES})
ELSE(NOT APPLE)
    SET(GUI_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    ADD_EXECUTABLE(gui_exe MACOSX_BUNDLE main.C ${GUI_STATIC_SOURCES} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Graphical client for VisIt")
    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "VisIt version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "VisIt ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC")
ENDIF(NOT APPLE)

SET_TARGET_PROPERTIES(gui_exe PROPERTIES OUTPUT_NAME gui)
TARGET_LINK_LIBRARIES(gui_exe 
gui
${gui_exe_IOperator}
${gui_exe_GOperator}
${gui_exe_IPlot}
${gui_exe_GPlot}
)

# If we're building statically then the gui can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(gui_exe
        ${gui_exe_IOperator}
        ${gui_exe_GOperator}
        ${gui_exe_IPlot}
        ${gui_exe_GPlot}
    )
ENDIF(VISIT_STATIC)

# Install these targets
VISIT_INSTALL_TARGETS(gui_exe)
