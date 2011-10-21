#*****************************************************************************
#
# Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#   Brad Whitlock, Tue Nov 30 11:32:08 PST 2010
#   I added multiwindow example.
#
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Brad Whitlock, Thu Oct 20 15:42:10 PDT 2011
#   Fix static build.
#
#****************************************************************************/

SET(SOURCES1
CommandParser.C
SimpleVisApp.C
Synchronizer.C
main.C
)

SET(SOURCES2
MultiWindowVisApp.C
main.C
)

# If the viewer is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES1 ${CMAKE_CURRENT_SOURCE_DIR}/../../viewer/main/ViewerStaticSymbolLocator.C)
    SET(STATIC_SOURCES2 ${CMAKE_CURRENT_SOURCE_DIR}/../../viewer/main/ViewerStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

# The subset of the sources that have Q_OBJECT in their header.
SET(MOC_SOURCES1
CommandParser.h
SimpleVisApp.h
Synchronizer.h
)
QT_WRAP_CPP(embedded SOURCES1 ${MOC_SOURCES1})

# The subset of the sources that have Q_OBJECT in their header.
SET(MOC_SOURCES2
MultiWindowVisApp.h
)
QT_WRAP_CPP(multiwindow SOURCES2 ${MOC_SOURCES2})

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

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

IF(NOT APPLE)
    ADD_EXECUTABLE(embedded ${SOURCES1} ${STATIC_SOURCES1})
    ADD_EXECUTABLE(multiwindow ${SOURCES2} ${STATIC_SOURCES2})
ELSE(NOT APPLE)
    SET(GUI_ICON_FILE VisItIcon.icns)
    SET_SOURCE_FILES_PROPERTIES(${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    ADD_EXECUTABLE(embedded MACOSX_BUNDLE ${SOURCES1}  ${STATIC_SOURCES1} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_BUNDLE_NAME          "Embedded viewer example ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_INFO_STRING          "Embedded viewer example")
    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "Embedded viewer example version ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "Embedded viewer example ${VISIT_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC")

    ADD_EXECUTABLE(multiwindow MACOSX_BUNDLE ${SOURCES2}  ${STATIC_SOURCES2} ${VISIT_SOURCE_DIR}/common/icons/${GUI_ICON_FILE})
#    SET(MACOSX_BUNDLE_BUNDLE_NAME          "Multiwindow embedded viewer example ${VISIT_VERSION}")
#    SET(MACOSX_BUNDLE_INFO_STRING          "Multiwindow embedded viewer example")
#    SET(MACOSX_BUNDLE_ICON_FILE            ${GUI_ICON_FILE})
#    SET(MACOSX_BUNDLE_GUI_IDENTIFIER       VISI)
#    SET(MACOSX_BUNDLE_LONG_VERSION_STRING  "Multiwindow embedded viewer example version ${VISIT_VERSION}")
#    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "Multiwindow embedded viewer example ${VISIT_VERSION}")
#    SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${VISIT_VERSION})
#    SET(MACOSX_BUNDLE_COPYRIGHT            "Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC")
ENDIF(NOT APPLE)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTNETWORK_LIBRARY}
${QT_QTCORE_LIBRARY}
)
IF(VISIT_STATIC)
    MAC_NIB_INSTALL(embedded)
    MAC_NIB_INSTALL(multiwindow)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTNETWORK_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

SET(EMBEDDED_VIEWER_LIBS
${viewer_exe_IOperator}
${viewer_exe_VOperator}
${viewer_exe_IPlot}
${viewer_exe_VPlot}
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
${VIEWER_QT_LIBS}
)

TARGET_LINK_LIBRARIES(embedded ${EMBEDDED_VIEWER_LIBS})
TARGET_LINK_LIBRARIES(multiwindow ${EMBEDDED_VIEWER_LIBS})

IF(APPLE)
    # Hey, do we need this on 10.6?
    SET_TARGET_PROPERTIES(embedded 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
    SET_TARGET_PROPERTIES(multiwindow 
        PROPERTIES
        LINK_FLAGS "-framework Carbon"
    )
ENDIF(APPLE)

# If we're building statically then the viewer can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(embedded
        ${viewer_exe_IOperator}
        ${viewer_exe_VOperator}
        ${viewer_exe_IPlot}
        ${viewer_exe_VPlot}
    )

    ADD_DEPENDENCIES(multiwindow
        ${viewer_exe_IOperator}
        ${viewer_exe_VOperator}
        ${viewer_exe_IPlot}
        ${viewer_exe_VPlot}
    )
ENDIF(VISIT_STATIC)

VISIT_TOOLS_ADD_FOLDER(embedded embedded multiwindow)
