#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#ADD_DEPENDENCIES(rpc)
#ADD_DEPENDENCIES(proxy)
#ADD_DEPENDENCIES(viewer)

SET(VIEWERLIB_SOURCES
ViewerSubjectProxy.C
)

# If the viewer is static then we need some static symbol lookup functions
#IF(VISIT_STATIC)
#    SET(VIEWER_STATIC_SOURCES ViewerStaticSymbolLocator.C)
#ENDIF(VISIT_STATIC)

SET(VIEWERLIB_MOC_HEADERS
ViewerSubjectProxy.h
)

QT_WRAP_CPP(viewersubjectproxy VIEWERLIB_MOC_SOURCES ${VIEWERLIB_MOC_HEADERS})

#IF (NOT WIN32)
    # This keeps comm's exceptions visible when using -fvisibility=hidden
#    ADD_DEFINITIONS(-DCOMM_EXPORTS)
#ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/viewer/main
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/QtVisWindow
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Interactors
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/launcher/proxy
${VISIT_SOURCE_DIR}/launcher/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/vtkqt
${VISIT_SOURCE_DIR}/winutil
${QT_INCLUDE_DIR}
${QT_QTCORE_INCLUDE_DIR}
${QT_QTGUI_INCLUDE_DIR}
${QT_QTOPENGL_INCLUDE_DIR}
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${QT_LIBRARY_DIR} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

#*************************** The viewer library ******************************
ADD_LIBRARY(viewersubjectproxy ${VIEWERLIB_SOURCES} ${VIEWERLIB_MOC_SOURCES})
#IF (WIN32)
#    SET_TARGET_PROPERTIES(viewersubject PROPERTIES OUTPUT_NAME viewersubjectlib)
#ENDIF (WIN32)

SET(VIEWER_QT_LIBS 
${QT_QTGUI_LIBRARY}
${QT_QTOPENGL_LIBRARY}
${QT_QTCORE_LIBRARY}
)

IF(VISIT_STATIC)
    SET(VIEWER_QT_LIBS 
        ${VIEWER_QT_LIBS}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

TARGET_LINK_LIBRARIES(viewersubjectproxy
viewer
viewerrpc
viewerproxy
visitcommon
mdserverrpc
mdserverproxy
vclrpc
vclproxy
enginerpc
engineproxy
avtdbatts
avtview
avtmath
avtpipeline_ser
avtplotter_ser
avtfilters_ser
avtviswindow_ser
avtwriter_ser
avtqtviswindow
winutil
${VIEWER_QT_LIBS}
)

VISIT_INSTALL_TARGETS(viewersubjectproxy)
