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
#
#*****************************************************************************

set(pyqtgui_include_paths ${CMAKE_CURRENT_SOURCE_DIR}
                    ${VISIT_PYTHON_DIR}/include
                    ${VISIT_SIP_DIR}/include
                    ${VISIT_SIP_DIR}/include/python${PYTHON_VERSION}
                    ${QT_QTCORE_INCLUDE_DIR} 
                    ${QT_QTGUI_INCLUDE_DIR} 
                    ${QT_QTOPENGL_INCLUDE_DIR} 
                    ${QT_INCLUDE_DIR} 
                    ${VISIT_SOURCE_DIR}/viewer/main 
                    ${VISIT_SOURCE_DIR}/viewer/proxy
                    ${VISIT_SOURCE_DIR}/viewer/subjectproxy
                    ${VISIT_SOURCE_DIR}/viewer/rpc
                    ${VISIT_SOURCE_DIR}/vtkqt
                    ${VISIT_SOURCE_DIR}/common/utility
                    ${VISIT_SOURCE_DIR}/common/state
                    ${VISIT_SOURCE_DIR}/common/misc
                    ${VISIT_SOURCE_DIR}/common/comm
                    ${VISIT_SOURCE_DIR}/common/plugin
                    ${VISIT_SOURCE_DIR}/common/Exceptions/Pipeline
                    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData/
                    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
                    ${VTK_INSTALL_PREFIX}/include/vtk-5.0
                    ${VISIT_SOURCE_DIR}/avt/Math
                    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
                    ${VISIT_SOURCE_DIR}/avt/Plotter/
                    ${VISIT_SOURCE_DIR}/avt/Plotter/vtk
                    ${VISIT_SOURCE_DIR}/visit_vtk/full
                    ${VISIT_SOURCE_DIR}/gui
                    ${VISIT_SOURCE_DIR}/visitpy/pyui/common
                    ${VISIT_SOURCE_DIR}/winutil
                    ${CMAKE_BINARY_DIR}/include
)

set(pyqtgui_link_libs
                      #gui
                      guiwrapper
                      #viewerproxy
                      #viewersubjectproxy
                      #vtkqt
                      #${QT_QTCORE_LIBRARY}
                      ${PYTHON_LIBRARIES}
                      )

SET(SIP_LIBRARY "pyqt_gui")
SET(SIP_EXECUTABLE "${VISIT_SIP_DIR}/bin/sip")
SET(SIP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pyqtgui.sip")
MESSAGE(STATUS "Configuring ${SIP_LIBRARY}")
SET(pyqtgui_sources 
                 ${CMAKE_CURRENT_SOURCE_DIR}/pyqtgui.C
                 ${CMAKE_CURRENT_BINARY_DIR}/sippyqt_guiPyQtGUI.cpp
                 ${CMAKE_CURRENT_BINARY_DIR}/sippyqt_guicmodule.cpp)

SET(PYQT_QT_VERSION "Qt_${QT_VERSION_MAJOR}_${QT_VERSION_MINOR}_0") #${QT_VERSION_PATCH}

if(WIN32)
  SET(PYQT_QT_DEVICE "WS_WIN")
elseif(APPLE)
#for some reason sip on mac does not like patch..
SET(PYQT_QT_VERSION "Qt_${QT_VERSION_MAJOR}_${QT_VERSION_MINOR}_0")
SET(PYQT_QT_DEVICE "WS_MACX")
else()
  SET(PYQT_QT_DEVICE "WS_X11")
endif()

MESSAGE(STATUS ${SIP_EXECUTABLE} "-t" ${PYQT_QT_VERSION} "-t" ${PYQT_QT_DEVICE} "-I" ${VISIT_SIP_DIR}/share/sip/PyQt4 "-I" ${VISIT_SIP_DIR}/share "-c" ${CMAKE_CURRENT_BINARY_DIR} ${SIP_FILE})
EXECUTE_PROCESS(COMMAND ${SIP_EXECUTABLE} "-t" ${PYQT_QT_VERSION} "-t" ${PYQT_QT_DEVICE} "-I" ${VISIT_SIP_DIR}/share/sip/PyQt4 "-I" ${VISIT_SIP_DIR}/share "-c" ${CMAKE_CURRENT_BINARY_DIR} ${SIP_FILE})

ADD_DEFINITIONS("-fvisibility=default")
include_directories(${pyqtgui_include_paths})
ADD_LIBRARY(${SIP_LIBRARY} MODULE ${pyqtgui_sources})
SET_TARGET_PROPERTIES(${SIP_LIBRARY} PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(${SIP_LIBRARY} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/site-packages/visit)
TARGET_LINK_LIBRARIES(${SIP_LIBRARY} ${pyqtgui_link_libs})
VISIT_INSTALL_TARGETS_RELATIVE("site-packages/visit" ${SIP_LIBRARY})

