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
#
#*****************************************************************************

set(pysideviewer_include_paths ${CMAKE_CURRENT_SOURCE_DIR}
                    ${SHIBOKEN_INCLUDE_DIR}
                    ${SHIBOKEN_PYTHON_INCLUDE_DIR}
                    ${PYSIDE_INCLUDE_DIR}
                    ${QT_QTCORE_INCLUDE_DIR} 
                    ${QT_QTGUI_INCLUDE_DIR} 
                    ${QT_QTOPENGL_INCLUDE_DIR} 
                    ${QT_INCLUDE_DIR} 
                    ${VISIT_PYSIDE_DIR}/include/PySide 
                    ${VISIT_PYSIDE_DIR}/include/PySide/QtCore 
                    ${VISIT_PYSIDE_DIR}/include/PySide/QtGui 
                    ${VISIT_PYSIDE_DIR}/include/PySide/QtOpenGL
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
                    ${CMAKE_BINARY_DIR}/include)

set(pysideviewer_link_libs ${module_name}
                      #gui
                      #viewer
                      viewerproxy
                      viewersubjectproxy
                      vtkqt
                      ${SHIBOKEN_PYTHON_LIBRARIES}
                      ${SHIBOKEN_LIBRARY}
                      ${PYSIDE_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      #vtkCommon 
                      #vtkRendering
                      )

set(pysideviewer_sources ${CMAKE_CURRENT_BINARY_DIR}/pyside_pysideviewer/pysideviewer_wrapper.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/pyside_pysideviewer/pyside_pysideviewer_module_wrapper.cpp) 

PYSIDE_ADD_MODULE(pyside_pysideviewer 
                  pysideviewer_sources
                  pysideviewer_include_paths
                  pysideviewer_link_libs
                  global.h typesystem.xml)

VISIT_INSTALL_TARGETS(pyside_pysideviewer)

