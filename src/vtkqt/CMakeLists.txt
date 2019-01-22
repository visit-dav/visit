#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
#    Kathleen Biagas, Tue Feb 5 15:15:27 PST 2013
#    Removed GUISupport, we use VTK's version now.
#
#    Kevin Griffin, Tue Apr 25 11:43:15 PDT 2017
#    Added Cocoa framework for OSX helper function.
#
#****************************************************************************/

set(VTKQT_SOURCES
    vtkQtRenderWindow.C
    vtkQtImagePrinter.C
)

if(APPLE)
    list(APPEND VTKQT_SOURCES osxHelper.mm)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VISIT_BINARY_DIR}/include
    ${VISIT_SOURCE_DIR}/include
    ${VISIT_SOURCE_DIR}/vtkqt
    ${VISIT_SOURCE_DIR}/common/utility
    ${VTK_INCLUDE_DIRS}
)

add_library(vtkqt ${VTKQT_SOURCES})

# Specify the libraries to link against on Windows & Mac
target_link_libraries(vtkqt 
    vtkCommonCore vtkGUISupportQt vtkGUISupportQtOpenGL vtkIOImage
    vtkRenderingOpenGL2 vtkRenderingFreeType ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${OPENGL_LIBRARIES})

if(HAVE_OSMESA)
    target_link_libraries(vtkqt ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(vtkqt ${EGL_LIBRARIES})
endif()

if(APPLE)
    set_target_properties(vtkqt PROPERTIES
        LINK_FLAGS "-framework Cocoa"
    )
endif()

VISIT_INSTALL_TARGETS(vtkqt)

