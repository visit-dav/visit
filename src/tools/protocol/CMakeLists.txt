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

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/launcher/rpc
${VISIT_SOURCE_DIR}/launcher/proxy
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/mdserver/proxy
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/viewer/proxy
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
)

# If static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(STATIC_SOURCES ../../engine/main/EngineStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
${VTK_LIBRARY_DIRS} 
${MESA_LIBRARY_DIR}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
)

SET(VTK_EXTRA_LIBS
vtktiff
vtkjpeg 
vtkpng 
vtkexpat
vtkzlib 
vtkDICOMParser 
vtkMPEG2Encode 
vtkfreetype 
vtkftgl
)

ADD_EXECUTABLE(visitprotocol dump_protocol.C ${STATIC_SOURCES})
TARGET_LINK_LIBRARIES(visitprotocol 
${engine_ser_exe_IDatabase}
${engine_ser_exe_EDatabase_ser}
${engine_ser_exe_IOperator}
${engine_ser_exe_EOperator_ser}
${engine_ser_exe_IPlot}
${engine_ser_exe_EPlot_ser}
viewerrpc
viewerproxy
enginerpc
engineproxy
vclrpc
vclproxy
mdserverrpc
mdserverproxy
visitcommon
${VTK_EXTRA_LIBS}
${ALL_THIRDPARTY_IO_LIB}
${MESA_LIB}
${OPENGL_LIBRARIES}
${CMAKE_THREAD_LIBS}
)
IF(NOT WIN32)
    TARGET_LINK_LIBRARIES(visitprotocol dl)
    IF(NOT APPLE)
        TARGET_LINK_LIBRARIES(visitprotocol ${X11_LIBRARIES})
    ENDIF(NOT APPLE)
ENDIF(NOT WIN32)

# If we're building statically then we can't be link until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(visitprotocol
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        ${engine_ser_exe_IOperator}
        ${engine_ser_exe_EOperator_ser}
        ${engine_ser_exe_IPlot}
        ${engine_ser_exe_EPlot_ser}
    )
ENDIF(VISIT_STATIC)

VISIT_INSTALL_TARGETS(visitprotocol)
